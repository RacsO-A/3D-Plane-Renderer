#include "rubiks.h"
#define PI 3.14159265359f

double UD_ang = 0.032341; // Up down
double LR_ang = 0.014321; // Left right
double ROT_ang = 0.041231; // Grab and spin left right

void rotate_plane_around_normal(struct plane* p, struct point3 nor, double t) {
    rotate_around_normal(&(p->corner), nor, t);
	rotate_around_normal(&(p->vertex[0]), nor, t);
	rotate_around_normal(&(p->vertex[1]), nor, t);
	rotate_around_normal(&(p->normal), nor, t);
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

	// Rotates all edges
	for (int i = 0; i < 12; i++) {
		// Rotates all planes in an edge
		struct edge_p* edge = &(c->edges[i]);
		for (int j = 0; j < 2; j++) {
		rotate_plane(&(edge->face[j]));
		}
		for (int j = 0; j < 2; j++) {
		rotate_plane(&(edge->internal[j]));
		}
	}

	// Rotate all centers
	for (int i = 0; i < 6; i++) {
		rotate_plane(&(c->centers[i]));
	}

	// Rotates all normals
	for (int i = 0; i < 6; i++) {
		rotate_point(&(c->normals[i]), UD_ang, LR_ang, ROT_ang);
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

	// Rotates all edges
	for (int i = 0; i < 12; i++) {
		// Rotates all planes in an edge
		struct edge_p* edge = &(c->edges[i]);
		for (int j = 0; j < 2; j++) {
		rotate_plane(&(edge->face[j]));
		}
		for (int j = 0; j < 2; j++) {
		rotate_plane(&(edge->internal[j]));
		}
	}

	// Rotate all centers
	for (int i = 0; i < 6; i++) {
		rotate_plane(&(c->centers[i]));
	}

	// Rotates all normals
	for (int i = 0; i < 6; i++) {
		rotate_point(&(c->normals[i]), UD_ang, LR_ang, ROT_ang);
	}
}

void rotate_edge_ang(struct edge_p* e, double UD, double LR, double ROT) {
	double t1 = UD_ang;
	double t2 = LR_ang;
	double t3 = ROT_ang;
	UD_ang = UD;
	LR_ang = LR;
	ROT_ang = ROT;

	for (int j = 0; j < 2; j++) {
		rotate_plane(&(e->face[j]));
	}
	for (int j = 0; j < 2; j++) {
		rotate_plane(&(e->internal[j]));
	}

	UD_ang = t1;
	LR_ang = t2;
	ROT_ang = t3;
}

// Deep copies p -> copy
void plane_copy(struct plane* p, struct plane* copy) {
	copy->symbol = p->symbol;
	copy->normal = p->normal;
	copy->corner = p->corner;
	copy->vertex[0] = p->vertex[0];
	copy->vertex[1] = p->vertex[1];
}

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

const char colorss[6] = {'b', 'o', 'g', 'r', 'y', 'w'}; 

// Deep copies e -> copy
void edge_p_copy(struct edge_p* e, struct edge_p* copy) {
	plane_copy(&(e->face[0]), &(copy->face[0]));
	plane_copy(&(e->face[1]), &(copy->face[1]));
	e->face[0].symbol = colorss[rand() % 6];
	e->face[1].symbol = colorss[rand() % 6];

	plane_copy(&(e->internal[0]), &(copy->internal[0]));
	plane_copy(&(e->internal[1]), &(copy->internal[1]));
}

void all_edge_init(struct cube* c) {
	struct point3 offset = {0, piece_size, 0};
	struct point3 e1 = {1.5 * piece_size, -0.5 * piece_size, 1.5 * piece_size};
	struct point3 b1 = {0.5 * piece_size, -0.5 * piece_size, 1.5 * piece_size};
	struct point3 f1 = {1.5 * piece_size, -0.5 * piece_size, 0.5 * piece_size};
	
	struct point3 e2 = point3_add(e1, offset);
	struct point3 b2 = point3_add(b1, offset);
	struct point3 f2 = point3_add(f1, offset);

	// Init the template edge
	struct edge_p template_e;
	plane_init(&(template_e.face[0]), 
	           e1, e2, b1, (struct point3){0, 0, 1}, 
			   colorss[rand() % 6]);
	plane_init(&(template_e.face[1]), 
	           e1, e2, f1, (struct point3){1, 0, 0}, 
			   colorss[rand() % 6]);

	plane_init(&(template_e.internal[0]), 
	           e1, b1, f1, (struct point3){0, -1, 0}, 
			   'B');
	plane_init(&(template_e.internal[1]), 
	           e2, b2, f2, (struct point3){0, 1, 0}, 
			   'B');
	rotate_edge_ang(&template_e, 0, PI, 0);


	// Init the top ring of edges around white center
	for (int i = 0; i < 4; i++) {
		edge_p_copy(&template_e, &(c->edges[i]));
		rotate_edge_ang(&template_e, 0, -0.5 * PI, 0);
	}
	rotate_edge_ang(&template_e, 0, 0, 0.5 * PI);

	// Init the middle ring of edges
	for (int i = 4; i < 8; i++) {
		edge_p_copy(&template_e, &(c->edges[i]));
		rotate_edge_ang(&template_e, 0, -0.5 * PI, 0);
	}
	rotate_edge_ang(&template_e, 0, 0, 0.5 * PI);

	// Init the bottom ring of edges around yellow center
	for (int i = 8; i < 12; i++) {
		edge_p_copy(&template_e, &(c->edges[i]));
		rotate_edge_ang(&template_e, 0, -0.5 * PI, 0);
	}
}

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
	all_edge_init(c);

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
	c->normals[1] = (struct point3){-1, 0, 0};
	c->normals[2] = (struct point3){0, 1, 0};
	c->normals[3] = (struct point3){1, 0, 0};
	c->normals[4] = (struct point3){0, -1, 0};
	c->normals[5] = (struct point3){0, 0, -1};
}