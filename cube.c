#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "rubiks.h"
#define _GNU_SOURCE
#define PI 3.14159265359f

double print_time = 0;
double calc_time = 0;
double total_time = 0;
double render_time = 0;
double static_time = 0;
clock_t temp;
clock_t calc_temp;
clock_t static_temp;

int width = 160, height = 50;
double zBuffer[160 * 50];
char buffer[160 * 50];
char color_buffer[160 * 50 * 10];
int color_buf_itr = 0;
int backgroundASCIICode = ' ';
int distanceFromCam = 40;
double K1 = 60;
int prev_char = ' ';

double x, y, z;
double oox;
int xp, yp;
int idx;

const char colors[7] = {'b', 'o', 'g', 'r', 'y', 'B', 'w'}; 

void char_to_col(char ch) {
	int escape_c[7] = {34, 35, 32, 31, 33, 30, 37};
	int i = 0;

  if (ch == 'o') {
    	sprintf(color_buffer + color_buf_itr, "\033[38;5;214m|");
      color_buf_itr += 12;
      prev_char = ch;
      return;
  }

	while(colors[i] != ch) {
		i++;
	}

	sprintf(color_buffer + color_buf_itr, "\033[%im|", escape_c[i]);
  color_buf_itr += 6;
  prev_char = ch;
}

void move_R(struct cube* c) {
  int c_idx[4] = {1, 2, 5, 6};
  int e_idx[4] = {1, 5, 6, 9};
  struct point3 normal = c->normals[1];

  // Rotate corners
  for (int i = 0; i < 4; i++) {
    struct corner_p* cor = &(c->corners[c_idx[i]]);
    for (int j = 0; j < 3; j++) {
      struct plane* cur_plane = &(cor->face[j]);
      rotate_plane_around_normal(cur_plane, normal, -PI / 12);
    }
    for (int j = 0; j < 3; j++) {
      struct plane* cur_plane = &(cor->internal[j]);
      rotate_plane_around_normal(cur_plane, normal, -PI / 12);
    }
  }

  // Rotate edegs
  for (int i = 0; i < 4; i++) {
    struct edge_p* edge = &(c->edges[e_idx[i]]);
    for (int j = 0; j < 2; j++) {
      struct plane* cur_plane = &(edge->face[j]);
      rotate_plane_around_normal(cur_plane, normal, -PI / 12);
    }
    for (int j = 0; j < 2; j++) {
      struct plane* cur_plane = &(edge->internal[j]);
      rotate_plane_around_normal(cur_plane, normal, -PI / 12);
    }
  }

  // Rotate center
  struct plane* center_pl = &(c->centers[2]);
  rotate_plane_around_normal(center_pl, normal, -PI / 12);
}

void add_color_char(char ch) {
    if (ch == ' ' || ch == '\n') {
	      color_buffer[color_buf_itr] = ch;
        color_buf_itr++;
    } else if (ch == prev_char) {
      	color_buffer[color_buf_itr] = '|';
        color_buf_itr++;
    } else {
        char_to_col(ch);
    }
}

void calculateForStaticSurface(double cubeX, double cubeY, double cubeZ, int ch) {
  static_temp = clock();
  x = distanceFromCam - cubeX;
  y = cubeY;
  z = cubeZ;

  oox = 1 / x;

  xp = (int)(width / 2 + K1 * oox * y * 2);
  yp = (int)(height / 2 + K1 * oox * z);

  idx = xp + (height - yp) * width;
  if (idx >= 0 && idx < width * height) {
    if (oox > zBuffer[idx]) {
      zBuffer[idx] = oox;
      buffer[idx] = ch;
    }
  }
  static_time += (double)(clock() - static_temp) / CLOCKS_PER_SEC;
}

int get_render_width(struct plane* p) {
  double x;
  double y;
  // Use the closest point to the camera to overestimate
  if (p->corner.x > p->vertex[0].x && p->corner.x > p->vertex[1].x) {
    x = p->corner.x;
    y = p->corner.y;
  } else if (p->vertex[0].x > p->corner.x && p->vertex[0].x > p->vertex[1].x) {
    x = p->vertex[0].x;
    y = p->vertex[0].y;
  } else {
    x = p->vertex[1].x;
    y = p->vertex[1].y;
  }

  double oox1 = 1 / (distanceFromCam - x);
  
  // Calculate the distance it would be on the screen. Worse
  // case when the plane would be diagonal so along the diag
  // it would be piece_size * sqrt(2) ~ piece_size * 1.5
  int x1 = (int)(width / 2 + K1 * oox1 * y * 2);
  int x2 = (int)(width / 2 + K1 * oox1 * (y + piece_size * 1.5) * 2);
  return x2 - x1;
}

void render_plane(struct plane* p) {
  // If the plane's normal is pointed away, don't render
  if (point3_dot((struct point3){-1, 0, 0}, p->normal) > 0) {
    return;
  }
  
  int len = get_render_width(p); 

  // Takes the 3 defining points and makes a len x len grid
  // of the plane that the 3 points make
  struct point3** grid_points = gridify((struct point3[3]){p->corner, p->vertex[0], p->vertex[1]}, len);
	
  // Renders each point in the plane
  for (int i = 0; i <= len; i++) {
		for (int j = 0; j <= len; j++) {
			struct point3 cur_pt = grid_points[i][j];
      
      // If its on the border of the plane, color it a different color
      if (i <= 0.05 * len || j <= 0.05 * len || j >= 0.95 * len || i >= 0.95 * len) {
        calculateForStaticSurface(cur_pt.x, cur_pt.y, cur_pt.z, 'B');
      } else {
        calculateForStaticSurface(cur_pt.x, cur_pt.y, cur_pt.z, p->symbol);
      }
		}
	}
  
  for (int i = 0; i <= len; i++) {
    free(grid_points[i]);
  }
  free(grid_points);
}

void render_cube(struct cube* c) {
  // Render all corners
  for (int i = 0; i < 8; i++) {
    // Renders all planes in a corner
    struct corner_p* cor = &(c->corners[i]);
    for (int j = 0; j < 3; j++) {
      render_plane(&(cor->face[j]));
    }
    for (int j = 0; j < 3; j++) {
      render_plane(&(cor->internal[j]));
    }
  }

  // Render all edges
  for (int i = 0; i < 12; i++) {
    // Renders all planes in a corner
    struct edge_p* edge = &(c->edges[i]);
    for (int j = 0; j < 2; j++) {
      render_plane(&(edge->face[j]));
    }
    for (int j = 0; j < 2; j++) {
      render_plane(&(edge->internal[j]));
    }
  }

  // Render all centers
  for (int i = 0; i < 6; i++) {
    render_plane(&(c->centers[i]));
  }
}

int main() {
  total_time = clock();
  printf("\x1b[2J");
  
  struct cube c;
  cube_init(&c);
  int main_itr = 0;

  while (main_itr <= 12 * 20) {
    memset(buffer, backgroundASCIICode, width * height);
    memset(zBuffer, 0, width * height * 8);
    memset(color_buffer, backgroundASCIICode, width * height * 10);
    color_buf_itr = 0;
    
    calc_temp = clock();
    render_cube(&c);
    render_time += (double)(clock() - calc_temp) / CLOCKS_PER_SEC;

    calc_temp = clock();
    move_R(&c);
    rotate_cube(&c);
    calc_time += (double)(clock() - calc_temp) / CLOCKS_PER_SEC;

    printf("\x1b[H");

    temp = clock();
    for (int k = 0; k < width * height; k++) {
        add_color_char(k % width ? buffer[k] : 10);
    }
    color_buffer[color_buf_itr] = '\0';
    printf("%s", color_buffer); 
    print_time += (double)(clock() - temp) / CLOCKS_PER_SEC;

    /*
    char ch = getchar();
    
    //if (ch == 'w') {
        A += PI / 12;
    } else if (ch == 's') {
        A -= PI / 12;
    } else if (ch == 'a') {
        B -= PI / 12;
    } else if (ch == 'd') {
        B += PI / 12;
    }
    */

    usleep(45000);
    main_itr++;
  }
  printf("\033[0m");
  printf("\nPrint: %f, Total: %f\n", print_time, (double)(clock() - total_time) / CLOCKS_PER_SEC);
  printf("Total Render: %f, Rotation: %f, Projection: %f\n", render_time, calc_time, static_time);
  return 0;
}