#ifndef BODY_H
#define BODY_H

#include "poly.h"
#include <math.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#define BOUNCE_FACTOR 2.0  //apparently this is 20/11 .. ?
//#define BOUNCE_FACTOR 1.81818181818181  //apparently this is 20/11 .. ?

typedef struct{
        POLY poly;
        float seg_norm[MAX_OBJECT_VERTICES];
        VERTEX pos, vel;
        VERTEX impulse;
        char moveable;
        float mass;
        float inv_mass;
        float density;
        float friction;
        float restitution;
        float glue;
        }BODY;

BODY body_offset(BODY bo, VERTEX o);
BODY new_seg_body(VERTEX a, VERTEX b, VERTEX c, int * num_sides);
BODY new_seg_body_triangle(VERTEX a, VERTEX b, VERTEX d, VERTEX c, int * num_sides);
void calc_body_normals(BODY * bo);
void process_body_collision(BODY * a, BODY * b, const VERTEX normal, float t);
void process_body_overlap(BODY * a, BODY * b, const VERTEX mtd);
char body_collide(BODY * a, BODY * b);

#endif
