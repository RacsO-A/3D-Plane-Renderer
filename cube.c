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

double UD_ang = 0.032341; // Up down
double LR_ang = 0.014321; // Left right
double ROT_ang = 0.041231; // Grab and spin left right

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

	while(colors[i] != ch) {
		i++;
	}

	sprintf(color_buffer + color_buf_itr, "\033[%im|", escape_c[i]);
  color_buf_itr += 6;
  prev_char = ch;
}

double calculateX(struct point3* pt, double B, double C, double A) {
  double i = pt->x;
  double j = pt->y;
  double k = pt->z;
  return j * sin(A) * sin(B) * cos(C) + k * cos(A) * sin(B) * cos(C) -
         j * cos(A) * sin(C) + k * sin(A) * sin(C) + i * cos(B) * cos(C);
}

double calculateY(struct point3* pt, double B, double C, double A) {
  double i = pt->x;
  double j = pt->y;
  double k = pt->z;
  return j * cos(A) * cos(C) - k * sin(A) * cos(C) +
         j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) +
         i * cos(B) * sin(C);
}

double calculateZ(struct point3* pt, double B, double C, double A) {
  double i = pt->x;
  double j = pt->y;
  double k = pt->z;
  return k * cos(A) * cos(B) + j * sin(A) * cos(B) - i * sin(B);
}

// rotates a point pt around the normal (unit) vector cw t radians
void rotate_around_normal(struct point3* pt, struct point3 nor, double t) {
  double i = pt->x;
  double j = pt->y;
  double k = pt->z;

  double ux = nor.x;
  double uy = nor.y;
  double uz = nor.z;

  double st = sin(t);
  double ct = cos(t);
  double mct = 1 - ct;

  pt->x = i * (ux * ux * mct + ct) + j * (ux * uy * mct - uz * st) + k * (ux * uz * mct + uy * st);
  pt->y = i * (ux * uy * mct + uz * st) + j * (uy * uy * mct + ct) + k * (uy * uz * mct - ux * st);
  pt->z = i * (ux * uz * mct - uy * st) + j * (uy * uz * mct + ux * st) + k * (uz * uz * mct + ct);
}

void move_R(struct cube* c) {
  // Rotate corners
  int c_idx[4] = {1, 2, 5, 6};
  struct point3 normal = c->normals[1];
  for (int i = 0; i < 4; i++) {
    struct corner_p* cor = &(c->corners[c_idx[i]]);
    for (int j = 0; j < 3; j++) {
      struct plane* cur_plane = &(cor->face[j]);
      rotate_around_normal(&(cur_plane->corner), normal, -PI / 12);
      rotate_around_normal(&(cur_plane->vertex[0]), normal, -PI / 12);
      rotate_around_normal(&(cur_plane->vertex[1]), normal, -PI / 12);
      rotate_around_normal(&(cur_plane->normal), normal, -PI / 12);
    }
    for (int j = 0; j < 3; j++) {
      struct plane* cur_plane = &(cor->internal[j]);
      rotate_around_normal(&(cur_plane->corner), normal, -PI / 12);
      rotate_around_normal(&(cur_plane->vertex[0]), normal, -PI / 12);
      rotate_around_normal(&(cur_plane->vertex[1]), normal, -PI / 12);
      rotate_around_normal(&(cur_plane->normal), normal, -PI / 12);
    }
  }

  // Rotate center
  struct plane* center_pl = &(c->centers[2]);
  rotate_around_normal(&(center_pl->corner), normal, -PI / 12);
  rotate_around_normal(&(center_pl->vertex[0]), normal, -PI / 12);
  rotate_around_normal(&(center_pl->vertex[1]), normal, -PI / 12);
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
      if (i == 0 || j == 0 || j == len || i == len) {
        calculateForStaticSurface(cur_pt.x, cur_pt.y, cur_pt.z, 'w');
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

void rotate_point(struct point3* pt, double UD, double LR, double ROT) {
  *pt = (struct point3){calculateX(pt, UD, LR, ROT), 
                        calculateY(pt, UD, LR, ROT), 
                        calculateZ(pt, UD, LR, ROT)};
}

void rotate_plane(struct plane* p) {
  rotate_point(&(p->corner), UD_ang, LR_ang, ROT_ang);
  rotate_point(&(p->vertex[0]), UD_ang, LR_ang, ROT_ang);
  rotate_point(&(p->vertex[1]), UD_ang, LR_ang, ROT_ang);
  rotate_point(&(p->normal), UD_ang, LR_ang, ROT_ang);
}

void rotate_cube_ang(struct cube* c, double UD, double LR, double ROT) {
  double t1 = UD_ang;
  double t2 = LR_ang;
  double t3 = ROT_ang;
  UD_ang = UD;
  LR_ang = LR;
  ROT_ang = ROT;

  // Rotates all corners
  for (int i = 0; i < 8; i++) {
    // Rotates all planes in a corner
    struct corner_p* cor = &(c->corners[i]);
    for (int j = 0; j < 3; j++) {
      rotate_plane(&(cor->face[j]));
    }
    for (int j = 0; j < 3; j++) {
      rotate_plane(&(cor->internal[j]));
    }
  }

  // Rotate all centers
  for (int i = 0; i < 6; i++) {
    rotate_plane(&(c->centers[i]));
  }

  UD_ang = t1;
  LR_ang = t2;
  ROT_ang = t3;
}

void rotate_cube(struct cube* c) {
  // Rotates all corners
  for (int i = 0; i < 8; i++) {
    // Rotates all planes in a corner
    struct corner_p* cor = &(c->corners[i]);
    for (int j = 0; j < 3; j++) {
      rotate_plane(&(cor->face[j]));
    }
    for (int j = 0; j < 3; j++) {
      rotate_plane(&(cor->internal[j]));
    }
  }

  // Rotate all centers
  for (int i = 0; i < 6; i++) {
    rotate_plane(&(c->centers[i]));
  }

  // Rotates all normals
  for (int i = 0; i < 3; i++) {
    rotate_point(&(c->normals[i]), UD_ang, LR_ang, ROT_ang);
  }
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