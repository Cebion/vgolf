#ifndef HOLE_H
#define HOLE_H

#include <allegro.h>
#include "poly.h"
#include "defines.h"
#include "body.h"

typedef struct{
        VERTEX s;
        int flag;
        int ref_no[MAX_VERT_REFS];
        int radius;
        int num_refs;
        }VERT;

typedef struct{
        int angle[MAX_OBJECT_VERTICES];
        }POLY_ANG;

typedef struct{
        BITMAP * layer_pic[MAX_HOLE_PIC_LAYERS];
        //POLY poly[MAX_HOLE_POLYS];
        BODY body[MAX_HOLE_POLYS];
        VERT vert[MAX_HOLE_VERTS];
        VERTEX s;
        int par_score;
        int num_polys;
        int num_verts;
        int num_coins;
        float deccel_factor;
        char name[128];
        }HOLE;
        
extern int vgolf_hole_editor_mode;

int load_hole_fp(HOLE * hole, PACKFILE * fp);
int load_hole(HOLE * hole, char * filename);
void free_hole(HOLE * hole);
int save_hole_fp(HOLE * hole, PACKFILE * fp);
int save_hole(HOLE * hole, char * filename);

#endif
