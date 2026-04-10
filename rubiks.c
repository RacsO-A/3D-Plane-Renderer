#include "rubiks.h"
float piece_size = 4;

void plane_init(struct plane* p, struct point3 corner, struct point3 v1, struct point3 v2, struct point3 normal, char sym) {
	p->symbol = sym;
	p->normal = normal;
	p->corner = corner;
	p->vertex[0] = v1;
	p->vertex[1] = v2;
}

void edge_init(struct edge_p* e, struct point3 center) {

}

const char colorss[6] = {'@', '$', 'w', '#', ';', '+'}; 

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
				   'b');
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
	struct plane centers[6];

	// Inits the normals
	struct point3 normals[3];
}