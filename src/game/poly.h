#ifndef POLY_H
#define POLY_H

#include <allegro.h>

#define MAX_OBJECT_VERTICES 128
#define MAX_V_INT 4
#define MIN_RECT_SPAN 8

#ifndef PI
#define PI            3.14159265358979323846
#endif

typedef struct{
        int c[MAX_V_INT];
        }V_INT;

typedef struct{
        float x,y;
        char closed;            //draw a line to the next vertex?
        }VERTEX;

typedef struct{
        VERTEX p[MAX_OBJECT_VERTICES];  //vertices that make up object's shape
        short num_vertices;
        short color;
        }POLY;

typedef struct{
        int x1, y1;
        int x2, y2;
        }RECT;

float angle_between_segs(VERTEX a, VERTEX b, VERTEX d, VERTEX c);
float seg_normal(VERTEX a, VERTEX b);
float mod_angle(float ang);
VERTEX zero_v(void);
VERTEX v_offset(VERTEX v, VERTEX o);
VERTEX v_subtract(VERTEX v0, VERTEX v1);
double v_distance(VERTEX a, VERTEX b);
double distance(double x1, double y1, double x2, double y2);
POLY poly_v_offset(POLY p, VERTEX v);
POLY poly_x_offset(POLY p, double xoff);
POLY poly_y_offset(POLY p, double yoff);
double angle_between(VERTEX v1, VERTEX v2);
char point_in_poly_anglesum(POLY poly, VERTEX c, VERTEX p);
int point_in_poly(VERTEX v, POLY s1, VERTEX c1);
char rect_collision(RECT r1, RECT r2, VERTEX c1, VERTEX c2);
char rect_collision_f(int r1x1, int r1y1, int r1x2, int r1y2, int r2x1, int r2y1, int r2x2, int r2y2, VERTEX c1, VERTEX c2);
float poly_f(float x, float coefficient[], int degree);
float poly_secant_aitken(float x1, float x2, float (*f)(float, float[], int), float coeff[], float epsilon);
char lineseg_collision(VERTEX l1p1, VERTEX l1p2, VERTEX l2p1, VERTEX l2p2, VERTEX c1, VERTEX c2);
VERTEX v_lineseg_collision(VERTEX l1p1, VERTEX l1p2, VERTEX l2p1, VERTEX l2p2, VERTEX c1, VERTEX c2);
int poly_collision(POLY s1, POLY s2, VERTEX c1, VERTEX c2);
V_INT v_poly_collision(POLY s1, POLY s2, VERTEX c1, VERTEX c2);
V_INT v_poly_collision_boundcheck(POLY s1, POLY s2, VERTEX c1, VERTEX c2, RECT r1);
int v_poly_collision_boundcheck2(POLY s1, POLY s2, VERTEX c1, VERTEX c2, RECT r1, int seg[], int num_segs);
V_INT set_v_int(int set);
char poly_collision2(POLY s1, POLY s2, VERTEX c1, VERTEX c2);
char poly_collision3(POLY s1, POLY s2, VERTEX c1, VERTEX c2);
void get_interval(const POLY a, const VERTEX x_axis, float minmax[]);
char interval_intersect(const POLY a, const POLY b, const VERTEX x_axis, const VERTEX x_offset, const VERTEX x_vel, float * t_axis, float t_max);
char find_mtd(VERTEX x_axis[], float t_axis[], int num_axes, VERTEX * normal, float * t);
char poly_collide(const POLY a, const POLY b, VERTEX *x_offset, VERTEX *x_vel, VERTEX *normal, float *t);
void keep_vertex_in_bounds(VERTEX * p, int x1, int y1, int x2, int y2);
void rotate_vertex(VERTEX * vx, float rot_angle);
void rotate_poly(POLY * po, int rot_angle);
void translate_poly(POLY * po, VERTEX v);
void translate_poly_x(POLY * po, double xoff);
void translate_poly_y(POLY * po, double yoff);
void rotate_poly_v_offset(POLY * po, VERTEX v, int rot_angle);
void move_vertex(VERTEX * s, VERTEX v);
//void draw_poly(BITMAP * bp, POLY * po, int cx, int cy);
//void draw_poly_scaled(BITMAP * bp, POLY * po, int cx, int cy, float scale);
//void draw_poly_scaled_vh(BITMAP * bp, POLY * po, int cx, int cy, float h_scale, float v_scale);
int check_circle_collision(VERTEX a, VERTEX b, int a_rad, int b_rad);
int check_circle_collision_debug(VERTEX a, VERTEX b, int a_rad, int b_rad);
void v_int_heap_sort_ref(V_INT * v_a, V_INT * v_b, int len);
void v_int_heap_sort_ref_rev(V_INT * v_a, V_INT * v_b, int len);
double vertex_dot_product(VERTEX a, VERTEX b);
VERTEX new_vertex(float x, float y);
VERTEX vertex_add(VERTEX a, VERTEX b);
VERTEX vertex_subtract(VERTEX a, VERTEX b);
float vertex_magnitude(VERTEX a);
float vertex_normalize(VERTEX * a);
void draw_line_twostep(BITMAP * bp, int x0, int y0, int x1, int y1, int color, int noise);
void draw_dotted_line(BITMAP * bp, int x1, int y1, int x2, int y2, int color, int counter, int dlen);


#endif
