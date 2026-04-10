#include "gridify.h"

struct plane {
	struct point3 corner;
	struct point3 vertex[2];
	struct point3 normal; // unit
	char symbol;
};

// Corner piece
struct corner_p {
	struct plane face[3];
	struct plane internal[3];
};

// Edge piece
struct edge_p {
	struct plane face[2];
	struct plane internal[2];
};

// Green front white top
// corners and edges are in order from:
// top to bottom, tr clockwise
// centers are top to bottom, tr clockwise
struct cube {
	// normal unit vectors to top, front, right
	struct point3 normals[3];
	
	// 8 corners
	// 	3 face planes
	//	3 internal planes
	struct corner_p corners[8];
	
	// 12 edges
	//	2 face planes
	//	2 internal planes
	struct edge_p edges[12];
	
	// 6 centers
	//	1 face plane
	struct plane centers[6];

	// 0 if is currently doing a move, 1 when in static position
	int is_static;
};

void plane_init(struct plane*, struct point3, struct point3, struct point3, struct point3, char);
void edge_init(struct edge_p*, struct point3);
void corner_init(struct corner_p*, struct point3);
void cube_init(struct cube*);