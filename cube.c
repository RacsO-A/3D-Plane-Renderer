#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "rubiks.h"
#define _GNU_SOURCE
#define PI 3.14159265359f

// Cubing variables
// Edge and corner indexs associated with each 
// turn in order: U, D, L, R, B, F: in clockwise order
// looking from the outside to in
const int edge_idxs[6][4] = {{0, 1, 2, 3}, {11, 10, 9, 8},
                             {3, 7, 11, 4}, {1, 5, 9, 6},
                             {0, 4, 8, 5}, {2, 6, 10, 7}};
const int corner_idxs[6][4] = {{0, 1, 2, 3}, {7, 6, 5, 4},
                               {0, 3, 7, 4}, {2, 1, 5, 6},
                               {1, 0, 4, 5}, {3, 2, 6, 7}};
const char* valid_moves[18] = {"U", "U'", "2U", "D", "D'", "2D",
                               "L", "L'", "2L", "R", "R'", "2R",
                               "B", "B'", "2B", "F", "F'", "2F"};
const int frames_per_turn = 12;

// Timing variables
double print_time = 0;
double calc_time = 0;
double total_time = 0;
double render_time = 0;
double static_time = 0;
clock_t temp;
clock_t calc_temp;
clock_t static_temp;

// Rendering variables
int width = 160, height = 50;
double zBuffer[160 * 50];
char buffer[160 * 50];
char color_buffer[160 * 50 * 10];
int color_buf_itr = 0;
int backgroundASCIICode = ' ';
int distanceFromCam = 40;
double K1 = 60;
double x, y, z;
double oox;
int xp, yp;
int idx;

// Printing variables
int prev_char = ' ';
char cur_move[3] = "";
int frame_left_for_turn = 0;

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

// Returns 1 if input is a valid move
int is_valid_move(char* move) {
  for (int i = 0; i < 18; i++) {
    if (!strcmp(valid_moves[i], move)) {
      return 1;
    }
  }
  return 0;
}

// Gets the base movement from a move, R, L, U, D, B, or F
char get_base_move(char* move) {
  if (move[0] == '2') {
    return move[1];
  } else {
    return move[0];
  }
}

void update_move_idxs(char base_move, int edge_idx[4], int corner_idx[4]) {
  int idx;
  if (base_move == 'U') {
    idx = 0;
  } else if (base_move == 'D') {
    idx = 1;
  } else if (base_move == 'L') {
    idx = 2;
  } else if (base_move == 'R') {
    idx = 3;    
  } else if (base_move == 'B') {
    idx = 4;    
  } else if (base_move == 'F') {
    idx = 5;    
  }

  for (int i = 0; i < 4; i++) {
    edge_idx[i] = edge_idxs[idx][i];
    corner_idx[i] = corner_idxs[idx][i];
  }
}

// TODO Make this better
void arb_move(struct cube* c, int e_idx[4], int c_idx[4], int normal_idx, int back, int double_m) {
  int reps = (back == 1 ? 3 : (double_m == 1 ? 2 : 1));
  int back_n = (back == 1 ? -1 : 1);
  float rot_step = back_n * (double_m + 1) * PI / (2 * frames_per_turn);
  struct point3 normal = c->normals[normal_idx];

  // Rotate corners
  for (int i = 0; i < 4; i++) {
    struct corner_p* cor = &(c->corners[c_idx[i]]);
    for (int j = 0; j < 3; j++) {
      struct plane* cur_plane = &(cor->face[j]);
      rotate_plane_around_normal(cur_plane, normal, -rot_step);
    }
    for (int j = 0; j < 3; j++) {
      struct plane* cur_plane = &(cor->internal[j]);
      rotate_plane_around_normal(cur_plane, normal, -rot_step);
    }
  }

  // Rotate edegs
  for (int i = 0; i < 4; i++) {
    struct edge_p* edge = &(c->edges[e_idx[i]]);
    for (int j = 0; j < 2; j++) {
      struct plane* cur_plane = &(edge->face[j]);
      rotate_plane_around_normal(cur_plane, normal, -rot_step);
    }
    for (int j = 0; j < 2; j++) {
      struct plane* cur_plane = &(edge->internal[j]);
      rotate_plane_around_normal(cur_plane, normal, -rot_step);
    }
  }

  // Rotate center
  struct plane* center_pl = &(c->centers[normal_idx]);
  rotate_plane_around_normal(center_pl, normal, -rot_step);
}

int get_normal_idx_from_move(struct cube* c, char base_move) {
  int idx;
  if (base_move == 'U') {
    idx = 0;
  } else if (base_move == 'D') {
    idx = 5;
  } else if (base_move == 'L') {
    idx = 4;
  } else if (base_move == 'R') {
    idx = 2;    
  } else if (base_move == 'B') {
    idx = 1;    
  } else if (base_move == 'F') {
    idx = 3;    
  }
  return idx;
}

void do_move(struct cube* c, char* move) {
  if (!is_valid_move(move)) {
        return;
  }

  int double_move = 0;
  if (move[0] == '2') {
    double_move = 1;
  }

  int back_move = 0;
  if (move[1] == '\'') {
    back_move = 1;
  }

  char base_move = get_base_move(move);
  int normal_idx = get_normal_idx_from_move(c, base_move);

  int edge_idx[4];
  int cor_idx[4];
  update_move_idxs(base_move, edge_idx, cor_idx);

  arb_move(c, edge_idx, cor_idx, normal_idx, back_move, double_move);

  frame_left_for_turn--;
  if (!frame_left_for_turn) {
    int reps = (back_move == 1 ? 3 : (double_move == 1 ? 2 : 1));
    
    // Rearange the edges
    for (int i = 0; i < reps; i++) {
      struct edge_p temp = c->edges[edge_idx[3]];
      for (int j = 3; j > 0; j--) {
        c->edges[edge_idx[j]] = c->edges[edge_idx[j - 1]];
      }
      c->edges[edge_idx[0]] = temp;
    }

    // Rearange the corners
    for (int i = 0; i < reps; i++) {
      struct corner_p temp = c->corners[cor_idx[3]];
      for (int j = 3; j > 0; j--) {
        c->corners[cor_idx[j]] = c->corners[cor_idx[j - 1]];
      }
      c->corners[cor_idx[0]] = temp;
    }

    strcpy(cur_move, "");
  }
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

void move_tick(struct cube* c, char* move, int* itr) {
  // If done with moves
  if (!strcmp(cur_move, "Done")) {

  // Else if cur_move is not empty
  } else if (strcmp(cur_move, "")) {
    // Do a frame advance of it
    do_move(c, cur_move);
  } else {
    // Start next move
    frame_left_for_turn = frames_per_turn;
    strcpy(cur_move, move);
    (*itr)++;
    if (*itr == 18) {
      *itr = 0;
    }
    do_move(c, cur_move);
  }
}

int main() {
  total_time = clock();
  printf("\x1b[2J");
  
  struct cube c;
  cube_init(&c);
  rotate_cube(&c);
  int main_itr = 0;
  int moves_itr = 0;
  char* moves[18] = {"U", "U'", "2U", "D", "D'", "2D",
                     "L", "L'", "2L", "R", "R'", "2R",
                     "B", "B'", "2B", "F", "F'", "2F"};

  while (main_itr <= 12 * 20) {
    memset(buffer, backgroundASCIICode, width * height);
    memset(zBuffer, 0, width * height * 8);
    memset(color_buffer, backgroundASCIICode, width * height * 10);
    color_buf_itr = 0;
    
    calc_temp = clock();
    render_cube(&c);
    render_time += (double)(clock() - calc_temp) / CLOCKS_PER_SEC;
    
    calc_temp = clock();
    rotate_cube(&c);
    move_tick(&c, moves[rand() % 18], &moves_itr);
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