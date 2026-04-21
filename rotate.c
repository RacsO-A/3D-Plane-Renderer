#include "rotate.h"
#include <math.h>

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

void rotate_point(struct point3* pt, double UD, double LR, double ROT) {
  *pt = (struct point3){calculateX(pt, UD, LR, ROT), 
                        calculateY(pt, UD, LR, ROT), 
                        calculateZ(pt, UD, LR, ROT)};
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