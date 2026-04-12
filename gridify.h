#include <stdio.h>
#include <stdlib.h>

struct point3 {
	double x;
	double y;
	double z;
};

struct point2 {
	double x;
	double y;
};

double absf(double);
void point3_print(struct point3);
void point2_print(struct point2);
struct point2 point2_add(struct point2, struct point2);
struct point2 point2_mult(double, struct point2);
struct point3 point3_add(struct point3, struct point3);
struct point3 point3_mult(double, struct point3);
double point3_dot(struct point3, struct point3);
struct point3** point3_init_matrix(double);
struct point3** gridify(struct point3 points[3], double);