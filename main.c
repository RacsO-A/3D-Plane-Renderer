#include <stdio.h>

struct point3 {
	float x;
	float y;
	float z;
};

struct point2 {
	float x;
	float y;
};

void point3_print(struct point3 p) {
	printf("(%.5f,%.5f,%.5f)", p.x, p.y, p.z);
}

void point2_print(struct point2 p) {
	printf("(%.2f,%.2f)", p.x, p.y);
}

struct point2 point2_add(struct point2 p1, struct point2 p2) {
	return (struct point2){.x = p1.x + p2.x, .y = p1.y + p2.y};
}

// Multiply a 2d point by a const
struct point2 point2_mult(float c, struct point2 p) {
	return (struct point2){.x = p.x * c, .y = p.y * c};
}

struct point3 point3_add(struct point3 p1, struct point3 p2) {
	return (struct point3){.x = p1.x + p2.x, .y = p1.y + p2.y, .z = p1.z + p2.z};
}

// Multiply a 3d point by a const
struct point3 point3_mult(float c, struct point3 p) {
	return (struct point3){.x = p.x * c, .y = p.y * c, .z = p.z * c};
}

// Takes in 3 points and returns a len x len grid of points that gridify the
// 3 points that construct the plane
// Corner is at points[0]
void grid_points(struct point3 points[3], float len) {
	struct point2 corner2 = (struct point2){points[0].x, points[0].y};
	struct point2 b_vertex1 = (struct point2){points[1].x, points[1].y};
	struct point2 b_vertex2 = (struct point2){points[2].x, points[2].y};

	// To increment from corner -> vertex 1
	// 1/len (corner - vertex1)
	struct point2 b1 = point2_mult(1 / len, point2_add(b_vertex1, point2_mult(-1, corner2)));
	struct point2 b2 = point2_mult(1 / len, point2_add(b_vertex2, point2_mult(-1, corner2)));
	
	// How much the z changes in the basis directions
	float z1 = 1 / len * (points[1].z - points[0].z);
	float z2 = 1 / len * (points[2].z - points[0].z);

	for (int i = 0; i <= len; i++) {
		for (int j = 0; j <= len; j++) {
			struct point2 xy_grid_pt =
				point2_add(corner2, point2_add(point2_mult(i, b1), point2_mult(j, b2)));
			
			float z_val = points[0].z + i * z1 + j * z2;
			struct point3 grid_pt = {xy_grid_pt.x, xy_grid_pt.y, z_val};
			point3_print(grid_pt);
			printf(",");
		}
	}
}

int main() {
	struct point3 corner = {-3.084,5.538,-4.337};
	struct point3 p1 = {-1.812,0.4474,-4.827};
	struct point3 p2 = {-2.643,4.001,1.455};
	struct point3 plane[3] = {corner, p1, p2};

	grid_points(plane, 4);

	return 0;
}
