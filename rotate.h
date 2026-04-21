#include "gridify.h"
#include <math.h>

double calculateX(struct point3*, double, double, double);
double calculateY(struct point3*, double, double, double);
double calculateZ(struct point3*, double, double, double);
void rotate_point(struct point3*, double, double, double);
void rotate_around_normal(struct point3*, struct point3, double);