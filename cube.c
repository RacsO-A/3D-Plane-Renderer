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
clock_t temp;
clock_t calc_temp;

float UD_ang = 0.032341; // Up down
float LR_ang = 0.014321; // Left right
float ROT_ang = 0.041231; // Grab and spin left right

float cubeWidth = 10;
int width = 160, height = 44;
float zBuffer[160 * 44];
char buffer[160 * 44];
char color_buffer[160 * 44 * 10];
int color_buf_itr = 0;
int backgroundASCIICode = ' ';
int distanceFromCam = 25;
float K1 = 40;

float incrementSpeed = 0.6;

float x, y, z;
float ooz;
int xp, yp;
int idx;

const char colors[8] = {'@', '$', '~', '#', ';', '+', 'b', 'w'}; 

void char_to_col(char ch) {
	int escape_c[8] = {36, 35, 32, 31, 34, 33, 30, 37};
	int i = 0;

	while(colors[i] != ch) {
		i++;
	}
  if (ch == 'w') {
    sprintf(color_buffer + color_buf_itr, "\033[%im|", escape_c[i]);
  } else {
	  sprintf(color_buffer + color_buf_itr, "\033[%im|", escape_c[i]);
  }
  color_buf_itr += 6;
}

float calculateX(struct point3 pt, float A, float B, float C) {
  float i = pt.x;
  float j = pt.y;
  float k = pt.z;
  return j * sin(A) * sin(B) * cos(C) - k * cos(A) * sin(B) * cos(C) +
         j * cos(A) * sin(C) + k * sin(A) * sin(C) + i * cos(B) * cos(C);
}

float calculateY(struct point3 pt, float A, float B, float C) {
  float i = pt.x;
  float j = pt.y;
  float k = pt.z;
  return j * cos(A) * cos(C) + k * sin(A) * cos(C) -
         j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) -
         i * cos(B) * sin(C);
}

float calculateZ(struct point3 pt, float A, float B, float C) {
  float i = pt.x;
  float j = pt.y;
  float k = pt.z;
  return k * cos(A) * cos(B) - j * sin(A) * cos(B) + i * sin(B);
}

// rotates a point pt around the normal (unit) vector cw t radians
struct point3 rotate_around_normal(struct point3 pt, struct point3 nor, float t) {
  float i = pt.x;
  float j = pt.y;
  float k = pt.z;

  float ux = nor.x;
  float uy = nor.y;
  float uz = nor.z;

  float st = sin(t);
  float ct = cos(t);
  float mct = 1 - ct;

  return (struct point3){i * (ux * ux * mct + ct) + j * (ux * uy * mct + uz * st) + k * (ux * uz * mct - uy * st),
                         i * (ux * uy * mct - uz * st) + j * (uy * uy * mct + ct) + k * (uy * uz * mct + ux * st),
                         i * (ux * uz * mct + uy * st) + j * (uy * uz * mct - ux * st) + k * (uz * uz * mct + ct)};
}

void add_color_char(char ch) {
    if (ch == ' ' || ch == '\n') {
	      color_buffer[color_buf_itr] = ch;
        color_buf_itr++;
    } else {
        char_to_col(ch);
    }
}

void calculateForStaticSurface(float cubeX, float cubeY, float cubeZ, int ch) {
  x = cubeX;
  y = cubeY;
  z = cubeZ + distanceFromCam;

  ooz = 1 / z;

  xp = (int)(width / 2 + K1 * ooz * x * 2);
  yp = (int)(height / 2 + K1 * ooz * y);

  idx = xp + yp * width;
  if (idx >= 0 && idx < width * height) {
    if (ooz > zBuffer[idx]) {
      zBuffer[idx] = ooz;
      buffer[idx] = ch;
    }
  }
}

void render_plane(struct plane* p, float len) {
  
  // If the plane's normal is pointed away, don't render
  if (point3_dot((struct point3){0,0,1}, p->normal) > 0) {
    return;
  }
  
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
}

void init_cube(struct point3 planes[6][3]) {
  planes[0][0] = (struct point3){5,5,-5};
  planes[0][1] = (struct point3){5,-5,-5};
  planes[0][2] = (struct point3){-5,5,-5};

  planes[1][0] = (struct point3){5,5,5};
  planes[1][1] = (struct point3){5,-5,5};
  planes[1][2] = (struct point3){-5,5,5};

  planes[2][0] = (struct point3){5,-4.999,5};
  planes[2][1] = (struct point3){-5,-5,5};
  planes[2][2] = (struct point3){5,-5,-5};

  planes[3][0] = (struct point3){5,4.999,5};
  planes[3][1] = (struct point3){-5,5,5};
  planes[3][2] = (struct point3){5,5,-5};

  planes[4][0] = (struct point3){-5,5,5};
  planes[4][1] = (struct point3){-5,-5,5};
  planes[4][2] = (struct point3){-5,4.999,-5};

  planes[5][0] = (struct point3){5,5,5};
  planes[5][1] = (struct point3){5,-5,5};
  planes[5][2] = (struct point3){5,4.999,-5};
}

struct point3 rotate_point(struct point3 pt, float UD, float LR, float ROT) {
  return (struct point3){calculateX(pt, UD, LR, ROT), 
                         calculateY(pt, UD, LR, ROT), 
                         calculateZ(pt, UD, LR, ROT)};
}

void rotate_plane(struct plane* p) {
  p->corner = rotate_point(p->corner, UD_ang, LR_ang, ROT_ang);
  p->vertex[0] = rotate_point(p->vertex[0], UD_ang, LR_ang, ROT_ang);
  p->vertex[1] = rotate_point(p->vertex[1], UD_ang, LR_ang, ROT_ang);
  p->normal = rotate_point(p->normal, UD_ang, LR_ang, ROT_ang);
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
}

void render_cube(struct cube* c) {
  // Render all corners
  for (int i = 0; i < 8; i++) {
    // Renders all planes in a corner
    struct corner_p* cor = &(c->corners[i]);
    for (int j = 0; j < 3; j++) {
      render_plane(&(cor->face[j]), 150);
    }
    for (int j = 0; j < 3; j++) {
      render_plane(&(cor->internal[j]), 150);
    }
  }
}

int main() {
  total_time = clock();
  printf("\x1b[2J");
  
  /*
  // 6 planes with 3 defining points
  struct point3 planes[6][3];
  init_cube(planes);
  rotate_cube(planes);
  */
  struct cube c;
  cube_init(&c);
  int main_itr = 0;

  while (main_itr <= 200) {
    memset(buffer, backgroundASCIICode, width * height);
    memset(zBuffer, 0, width * height * 4);
    memset(color_buffer, backgroundASCIICode, width * height * 10);
    color_buf_itr = 0;
    
    calc_temp = clock();
    /*
    render_plane(planes[0], 50, 'w'); // Cyan
    render_plane(planes[1], 50, '$'); // Purple
    render_plane(planes[2], 50, '~'); // Green
    render_plane(planes[3], 50, '#'); // Red
    render_plane(planes[4], 50, ';'); // Cyan
    render_plane(planes[5], 50, '+'); // Yellow
    */
    render_cube(&c);
    render_time += (double)(clock() - calc_temp) / CLOCKS_PER_SEC;

    calc_temp = clock();
    //rotate_cube(planes);
    rotate_cube(&c);
    calc_time += (double)(clock() - calc_temp) / CLOCKS_PER_SEC;
    /*
    // Renders the cube
    calculateForStaticSurface(cubeX, cubeY, -cubeWidth, '@'); // Cyan
    calculateForStaticSurface(cubeWidth, cubeY, cubeX, '$'); // Purple
    calculateForStaticSurface(-cubeWidth, cubeY, -cubeX, '~'); // Green
    calculateForStaticSurface(-cubeX, cubeY, cubeWidth, '#'); // Red
    calculateForStaticSurface(cubeX, -cubeWidth, -cubeY, ';');
    calculateForStaticSurface(cubeX, cubeWidth, cubeY, '+');
    */

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
  printf("Render: %f, Rotation: %f\n", render_time, calc_time);
  return 0;
}