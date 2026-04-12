#include "rubiks.h"
double piece_size = 5;

void plane_init(struct plane* p, struct point3 corner, struct point3 v1, struct point3 v2, struct point3 normal, char sym) {
	p->symbol = sym;
	p->normal = normal;
	p->corner = corner;
	p->vertex[0] = v1;
	p->vertex[1] = v2;
}

// Center is the center of the plane, not the center of the center_peice
void center_init(struct plane* p, struct point3 center, char color) {
	double p5ps = 0.5 * piece_size;
	struct point3 normal;
	struct point3 corner;
	struct point3 v1;
	struct point3 v2;
	if (center.x != 0) {
		normal = (struct point3){center.x / absf(center.x), 0, 0};
		corner = point3_add(center, (struct point3){0, p5ps, p5ps});
		v1 = point3_add(corner, (struct point3){0, 0, -piece_size});
		v2 = point3_add(corner, (struct point3){0, -piece_size, 0});
	} else if (center.y != 0) {
		normal = (struct point3){0, center.y / absf(center.y), 0};
		corner = point3_add(center, (struct point3){p5ps, 0, p5ps});
		v1 = point3_add(corner, (struct point3){0, 0, -piece_size});
		v2 = point3_add(corner, (struct point3){-piece_size, 0, 0});
	} else {
		normal = (struct point3){0, 0, center.z / absf(center.z)};
		corner = point3_add(center, (struct point3){p5ps, p5ps, 0});
		v1 = point3_add(corner, (struct point3){-piece_size, 0, 0});
		v2 = point3_add(corner, (struct point3){0, -piece_size, 0});
	}
	plane_init(p, corner, v1, v2, normal, color);
}

void edge_init(struct edge_p* e, struct point3 center) {

}

const char colorss[6] = {'b', 'o', 'g', 'r', 'y', 'w'}; 

void corner_init(struct corner_p* c, struct point3 center) {
	// Inits the face planes
	struct point3 center_1 = point3_mult((center.x > 0 ? 1 / center.x : -1 / center.x), center);
	struct point3 corner = point3_add(center, point3_mult(piece_size / 2, center_1));
	struct point3 verticies[3];
	struct point3 offsets[3] = {{-center_1.x * piece_size, 0, 0}, 
								{0, -center_1.y * piece_size, 0}, 
								{0, 0, -center_1.z * piece_size}};
	for (int i = 0; i < 3; i++) {
		verticies[i] = point3_add(corner, offsets[i]);
	}

	for (int i = 0; i < 3; i++) {
		plane_init(&(c->face[i]), 
				   corner, verticies[i], verticies[(i + 1) % 3], 
				   point3_mult(-1 / piece_size, offsets[(i + 2) % 3]), 
				   colorss[rand() % 6]);
	}

	// Inits the internal planes
	corner = point3_add(center, point3_mult(-piece_size / 2, center_1));
	struct point3 offsets2[3] = {{center_1.x * piece_size, 0, 0}, 
								{0, center_1.y * piece_size, 0}, 
								{0, 0, center_1.z * piece_size}};
	for (int i = 0; i < 3; i++) {
		verticies[i] = point3_add(corner, offsets2[i]);
	}

	for (int i = 0; i < 3; i++) {
		plane_init(&(c->internal[i]), 
				   corner, verticies[i], verticies[(i + 1) % 3], 
				   point3_mult(-1 / piece_size, offsets2[(i + 2) % 3]), 
				   'B');
	}
}

void cube_init(struct cube* c) {
    c->is_static = 1;

	// Inits all corners
	int template[4] = {-1, -1, 1, 1};
	for (int i = 0; i < 8; i++) {
		struct point3 center = point3_mult(piece_size, 
										   (struct point3){template[i % 4], template[(i + 1) % 4], i < 4 ? 1 : -1});
		corner_init(&(c->corners[i]), center);
	}

	// Inits all edges
	struct edge_p edges[12];

	// Inits the centers
	double th_psize = 3.0 / 2.0 * piece_size;
	struct point3 center_points[6] = {{0, 0, th_psize}, {-th_psize, 0, 0}, {0, th_psize, 0}, 
									  {th_psize, 0, 0}, {0, -th_psize, 0}, {0, 0, -th_psize}};
	char center_colors[6] = {'w', 'b', 'r', 'g', 'o', 'y'};
	for (int i = 0; i < 6; i++) {
		center_init(&(c->centers[i]), center_points[i], center_colors[i]);
	}

	// Inits the normals
	c->normals[0] = (struct point3){0, 0, 1};
	c->normals[1] = (struct point3){0, 1, 0};
	c->normals[2] = (struct point3){1, 0, 0};
}