#include <stdio.h>
#include <stdlib.h>

struct point3 {
	float x;
	float y;
	float z;
};

struct point2 {
	float x;
	float y;
};

void point3_print(struct point3);
void point2_print(struct point2);
struct point2 point2_add(struct point2, struct point2);
struct point2 point2_mult(float, struct point2);
struct point3 point3_add(struct point3, struct point3);
struct point3 point3_mult(float, struct point3);
struct point3** point3_init_matrix(float);
struct point3** gridify(struct point3 points[3], float);