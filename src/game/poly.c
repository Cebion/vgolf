#include <allegro.h>
#include <stdio.h>
#include <math.h>
//#include "structs.h"
#include "poly.h"

//find the angle between three line segments
float angle_between_segs(VERTEX a, VERTEX b, VERTEX d, VERTEX c)
{
        return seg_normal(b, d) - seg_normal(a, b);
}

//find the normal of this wall/line segment
float seg_normal(VERTEX a, VERTEX b){
        return atan2(a.y-b.y, a.x-b.x)-PI/2;
}

//keeps the angle in [-2pi, 2pi]
float mod_angle(float ang){
//	return (ang < 0 ? ang+2*PI : (ang > 2*PI ? ang-2*PI : ang));
        if(ang < 0) return ang+2*PI;
        else if(ang > 2*PI) return ang-2*PI;
        return ang;
}

/*set all members of a v_int to an initial value*/
V_INT set_v_int(int set){
        int i;
        V_INT a;

        for(i = 0; i < MAX_V_INT; i++) a.c[i] = set;
        return a;
}

/*translate all points in a poly by some vertex*/
void translate_poly(POLY * po, VERTEX v){
        int i;

        for(i = 0; i < po->num_vertices; i++){
                po->p[i].x += v.x;
                po->p[i].y += v.y;
                }
}

/*translate all points in a poly horizontally*/
void translate_poly_x(POLY * po, double xoff){
        int i;

        for(i = 0; i < po->num_vertices; i++){
                po->p[i].x += xoff;
                }
}

/*translate all points in a poly vertically*/
void translate_poly_y(POLY * po, double yoff){
        int i;

        for(i = 0; i < po->num_vertices; i++){
                po->p[i].y += yoff;
                }
}

/*offset all points in a poly by some vertex*/
POLY poly_v_offset(POLY p, VERTEX v){
        int i;
        POLY temp = p;

        for(i = 0; i < p.num_vertices; i++){
                temp.p[i].x += v.x;
                temp.p[i].y += v.y;
                }

        return temp;
}

/*returns a poly offset a certain x value*/
POLY poly_x_offset(POLY p, double xoff){
        int i;
        POLY temp = p;

        for(i = 0; i < p.num_vertices; i++){
                temp.p[i].x += xoff;
                }

        return temp;
}

/*returns a poly offset a certain y value*/
POLY poly_y_offset(POLY p, double yoff){
        int i;
        POLY temp = p;

        for(i = 0; i < p.num_vertices; i++){
                temp.p[i].y += yoff;
                }

        return temp;
}

//dot product of 2 vectors
double vertex_dot_product(VERTEX a, VERTEX b){
        double d = a.x*b.x + a.y*b.y;
        return d;
}

//set new vector values
VERTEX new_vertex(float x, float y){
        VERTEX temp_v;
        temp_v.x = x; temp_v.y = y;
		temp_v.closed = 0;
        return temp_v;
}

//add 2 vectors together
VERTEX vertex_add(VERTEX a, VERTEX b){
        VERTEX temp_v;
        temp_v.x = a.x+b.x; temp_v.y = a.y+b.y;
		temp_v.closed = 0;
        return temp_v;
}

//subtract a vector from another
VERTEX vertex_subtract(VERTEX a, VERTEX b){
        VERTEX temp_v;
        temp_v.x = a.x-b.x; temp_v.y = a.y-b.y;
		temp_v.closed = 0;
        return temp_v;
}

//returns the magnitude of the vector
float vertex_magnitude(VERTEX a){
        return sqrt(a.x*a.x+a.y*a.y);
}

//normalize this vector
float vertex_normalize(VERTEX * a){
        float mag = vertex_magnitude(*a);

        if(mag == 0) return 0;

        a->x /= mag;
        a->y /= mag;

        return mag;
}

/*returns the zero vertex (x = y = z = 0)*/
VERTEX zero_v(void){
        VERTEX temp;

        temp.x = temp.y = 0;
		temp.closed = 0;

        return temp;
}

/*returns a vertex offset by another vertex*/
VERTEX v_offset(VERTEX v, VERTEX o){
        VERTEX temp;

        temp.x = v.x + o.x;
        temp.y = v.y + o.y;
		temp.closed = 0;

        return temp;
}

/*returns a vertex offset by a negative vertex*/
VERTEX v_subtract(VERTEX v0, VERTEX v1){
        VERTEX temp;

        temp.x = v0.x - v1.x;
        temp.y = v0.y - v1.y;
		temp.closed = 0;

        return temp;
}

//point_in_poly_anglesum and angle_between come from
//http://astronomy.swin.edu.au/~pbourke/geometry/insidepoly/
//with minor adjustments
double angle_between(VERTEX v1, VERTEX v2){
        double dtheta,theta1,theta2;

        theta1 = atan2(v1.x, v1.y);
        theta2 = atan2(v2.x, v2.y);
        dtheta = theta2 - theta1;
        while(dtheta > PI) dtheta -= 2*PI;
        while(dtheta < -PI) dtheta += 2*PI;
        return dtheta;
}        

/*another method for determining if a point lies within a polygon*/
char point_in_poly_anglesum(POLY poly, VERTEX c, VERTEX p){
        int i;
        double angle=0;
        VERTEX v1, v2;

        for(i = 0; i < poly.num_vertices; i++){
                v1.x = c.x+poly.p[i].x - p.x;
                v1.y = c.y+poly.p[i].y - p.y;
                v2.x = c.x+poly.p[(i+1)%poly.num_vertices].x - p.x;
                v2.y = c.y+poly.p[(i+1)%poly.num_vertices].y - p.y;
                angle += angle_between(v1, v2);
                }
        return (abs(angle) >= PI);
}

//this code is derived from code posted at
//http://astronomy.swin.edu.au/~pbourke/geometry/insidepoly/ also
//tests to see if point is within polygon (using extended ray edge-intersection method)
int point_in_poly(VERTEX v, POLY s1, VERTEX c1){
        int i, j, count = 0;
        for(i = 0, j = s1.num_vertices-1; i < s1.num_vertices; i++, j = i-1){
                if ((((c1.y+s1.p[i].y <= v.y) && (v.y < c1.y+s1.p[j].y)) ||
                   ((c1.y+s1.p[j].y <= v.y) && (v.y < c1.y+s1.p[i].y))) &&
                   (v.x < ((c1.x+s1.p[j].x) - (c1.x+s1.p[i].x)) * (v.y - (c1.y+s1.p[i].y)) / ((c1.y+s1.p[j].y) - (c1.y+s1.p[i].y)) + (c1.x+s1.p[i].x))){
                   //x < (x2-x1)/(y2-y1) * (y - y1)
                        count++;
                        }
                }
        return (count%2);  //if the number of intersections is even,
                           //the point is within the polygon
}

/*determines if two rectangles have collided*/
char rect_collision(RECT r1, RECT r2, VERTEX c1, VERTEX c2)
{
	if(c1.x + r1.x1 <= c2.x + r2.x2 && c2.x + r2.x1 <= c1.x + r1.x2 && c1.y + r1.y1 <= c2.y + r2.y2 && c2.y + r2.y1 <= c1.y + r1.y2)
	{
		return 1;
	}
	return 0;
}

/*determines if two rectangles have collided (doesn't use rect struct)*/
char rect_collision_f(int r1x1, int r1y1, int r1x2, int r1y2, int r2x1, int r2y1, int r2x2, int r2y2, VERTEX c1, VERTEX c2)
{
	if(c1.x + r1x1 <= c2.x + r2x2 && c2.x + r2x1 <= c1.x + r1x2 && c1.y + r1y1 <= c2.y + r2y2 && c2.y + r2y1 <= c1.y + r1y2)
	{
		return 1;
	}
	return 0;
//	if(c1.x+r1x1 >= c2.x+r2x1 && c1.y+r1y1 >= c2.y+r2y1 && c1.x+r1x1 <= c2.x+r2x2 && c1.y+r1y1 <= c2.y+r2y2) return 1;
//	if(c1.x+r1x2 >= c2.x+r2x1 && c1.y+r1y1 >= c2.y+r2y1 && c1.x+r1x2 <= c2.x+r2x2 && c1.y+r1y1 <= c2.y+r2y2) return 1;
//	if(c1.x+r1x1 >= c2.x+r2x1 && c1.y+r1y2 >= c2.y+r2y1 && c1.x+r1x1 <= c2.x+r2x2 && c1.y+r1y2 <= c2.y+r2y2) return 1;
//	if(c1.x+r1x2 >= c2.x+r2x1 && c1.y+r1y2 >= c2.y+r2y1 && c1.x+r1x2 <= c2.x+r2x2 && c1.y+r1y2 <= c2.y+r2y2) return 1;
//	return 0;
}

//evaluate a polynomial at x
float poly_f(float x, float coefficient[], int degree){
        float value = coefficient[degree];
        float coeff[20];
        int i;

        for(i = degree; i >= 0; i--) coeff[i] = coefficient[i];

        for(i = degree-1; i >= 0; i--){
                value = value*x + coeff[i];
                }
        return value;
}

//secant method (with aitken's acceleration)
float poly_secant_aitken(float x1, float x2, float (*f)(float, float[], int), float coeff[], float epsilon){
        int counter = 0;
        int max_iterations = 50;
        float f_x0 = f(x1, coeff, 1);
        float f_x1 = f(x2, coeff, 1);
        float d_new = f_x1 - f_x0; 
        float d_old;
        float x_n0 = x1;
        float x_n1 = x2;
        float x_n2;
        float f_x2;
        float x_p1; // = x_n2 - ((x_n2-x_n1)*(x_n2-x_n1))/(x_n2 - 2*x_n1+x_n0);

        //printf("Secant\t\t\tAitken\n");
        while(counter < max_iterations){
                x_n2 = x_n1 - (x_n1 - x_n0)*f_x1/d_new;
                f_x2 = f(x_n2, coeff, 1);
                if(fabs(x_n2 - x_n1) < epsilon*fabs(x_n2) && fabs(f_x2) < epsilon){
                        break;
                        }
                d_old = d_new;
                d_new = f_x2 - f_x1;
                if(fabs(d_new) < 1e-20) d_new = d_old;
                x_n0 = x_n1;
                x_n1 = x_n2;
                f_x1 = f(x_n1, coeff, 1);
                x_n2 = x_n1 - (x_n1 - x_n0)*f_x1/d_new;
                f_x1 = f_x2;
                //x' = xn+1 - ((xn+1 - xn)*(xn+1 - xn))/(xn+1 - 2xn + xn-1)
                x_p1 = x_n2 - ((x_n2-x_n1)*(x_n2-x_n1))/(x_n2 - 2*x_n1+x_n0);
                //printf("xn+1: %.10f\tx_p1: %.10f\n", x_n2, x_p1);
                counter++;
                }
        return x_n2;
        //return x_p1;
}

/*determines if two line segments are intersecting (equate two lines using point-slope equation)*/
char lineseg_collision(VERTEX l1p1, VERTEX l1p2, VERTEX l2p1, VERTEX l2p2, VERTEX c1, VERTEX c2){
        float x_root;
        float l1_coeff[2];  //coefficient array
        float l2_coeff[2];  //coefficient array
        float coeff[2];
        float l1m, l2m, l1b, l2b;
        float l1x[2], l2x[2];
        float l1y[2], l2y[2];

        l1m = (l1p1.y - l1p2.y)/(l1p1.x - l1p2.x + 0.05*(l1p1.x == l1p2.x));

        l2m = (l2p1.y - l2p2.y)/(l2p1.x - l2p2.x + 0.05*(l2p1.x == l2p2.x));

        //y = m(x-x1) + y1
        l1b = c1.y+l1p1.y;
        l1b -= l1m*(c1.x+l1p1.x);

        l2b = c2.y+l2p1.y;
        l2b -= l2m*(c2.x+l2p1.x);

        //now should have the m and be for the equations..
        l1_coeff[0] = l1b;
        l1_coeff[1] = l1m;

        l2_coeff[0] = l2b;
        l2_coeff[1] = l2m;

        //get equation to be parsed into root solver
        coeff[0] = l1_coeff[0] - l2_coeff[0];
        coeff[1] = l1_coeff[1] - l2_coeff[1];

        //sort points
        if(l1p1.x <= l1p2.x){
                l1x[0] = l1p1.x;
                l1x[1] = l1p2.x;
                }
        else{
                l1x[1] = l1p1.x;
                l1x[0] = l1p2.x;
                }

        if(l2p1.x <= l2p2.x){
                l2x[0] = l2p1.x;
                l2x[1] = l2p2.x;
                }
        else{
                l2x[1] = l2p1.x;
                l2x[0] = l2p2.x;
                }

        if(l1p1.y <= l1p2.y){
                l1y[0] = l1p1.y;
                l1y[1] = l1p2.y;
                }
        else{
                l1y[1] = l1p1.y;
                l1y[0] = l1p2.y;
                }

        if(l2p1.y <= l2p2.y){
                l2y[0] = l2p1.y;
                l2y[1] = l2p2.y;
                }
        else{
                l2y[1] = l2p1.y;
                l2y[0] = l2p2.y;
                }

        //x_root = poly_secant_aitken(0, SCREEN_W, poly_f, coeff, 1e-5);
        x_root = poly_secant_aitken(c1.x+l1x[0], c1.x+l1x[1], poly_f, coeff, 1e-5);

        //if(x_root >= c1.x+l1x[0] && x_root <= c1.x+l1x[1] && x_root >= c2.x+l2x[0] && x_root <= c2.x+l2x[1]){
        if(x_root >= c1.x+l1x[0] && x_root <= c1.x+l1x[1]){
                return 1;
                }

        return 0;
}

//returns the point of intersection between the two segments..
VERTEX v_lineseg_collision(VERTEX l1p1, VERTEX l1p2, VERTEX l2p1, VERTEX l2p2, VERTEX c1, VERTEX c2){
        float x_root;
        float l1_coeff[2];  //coefficient array
        float l2_coeff[2];  //coefficient array
        float coeff[2];
        float l1m, l2m, l1b, l2b;
        VERTEX v_root;

        l1m = (l1p1.y - l1p2.y)/(l1p1.x - l1p2.x + 0.025*(l1p1.x == l1p2.x));
        l2m = (l2p1.y - l2p2.y)/(l2p1.x - l2p2.x + 0.025*(l2p1.x == l2p2.x));

        //y = m(x-x1) + y1
        l1b = c1.y+l1p1.y;
        l1b -= l1m*(c1.x+l1p1.x);

        l2b = c2.y+l2p1.y;
        l2b -= l2m*(c2.x+l2p1.x);

        //now should have the m and be for the equations..
        l1_coeff[0] = l1b;
        l1_coeff[1] = l1m;

        l2_coeff[0] = l2b;
        l2_coeff[1] = l2m;

        //get equation to be parsed into root solver
        coeff[0] = l1_coeff[0] - l2_coeff[0];
        coeff[1] = l1_coeff[1] - l2_coeff[1];

        x_root = poly_secant_aitken(0, SCREEN_W, poly_f, coeff, 1e-5);

        v_root.x = x_root;
        v_root.y = poly_f(x_root, l1_coeff, 1);
		v_root.closed = 0;

        return v_root;
}

//using intersection detection method
/*returns the line segments on each poly that are causing an intersection (if any)*/
V_INT v_poly_collision(POLY s1, POLY s2, VERTEX c1, VERTEX c2){
        int i,j;     
        V_INT a;

        a = set_v_int(-1);
        for(i = 0; i < s1.num_vertices-1; i++){
                for(j = 0; j < s2.num_vertices-1; j++){
                        if(lineseg_collision(s1.p[i], s1.p[i+1], s2.p[j], s2.p[j+1], c1, c2)){
                                a.c[0] = i;
                                a.c[1] = j;
                                return a;
                                }
                        }    
                }

        return a;
}

//using intersection detection method
/*returns the line segments on each poly that are causing an intersection (if any)*/
/*uses bounding rects to minimize number of intersection-detection function calls*/
V_INT v_poly_collision_boundcheck(POLY s1, POLY s2, VERTEX c1, VERTEX c2, RECT r1){
        int i,j;     
        V_INT a;
        RECT lseg_r;

        a = set_v_int(-1);
        for(i = 0; i < s1.num_vertices-1; i++){
                for(j = 0; j < s2.num_vertices-1; j++){
                        if(s2.p[j].x > s2.p[j+1].x){
                                lseg_r.x1 = s2.p[j+1].x-(abs(s2.p[j].x-s2.p[j+1].x) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                lseg_r.x2 = s2.p[j].x+(abs(s2.p[j].x-s2.p[j+1].x) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                }
                        else{
                                lseg_r.x1 = s2.p[j].x-(abs(s2.p[j].x-s2.p[j+1].x) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                lseg_r.x2 = s2.p[j+1].x+(abs(s2.p[j].x-s2.p[j+1].x) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                }
                        if(s2.p[j].y > s2.p[j+1].y){
                                lseg_r.y1 = s2.p[j+1].y-(abs(s2.p[j].y-s2.p[j+1].y) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                lseg_r.y2 = s2.p[j].y+(abs(s2.p[j].y-s2.p[j+1].y) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                }
                        else{
                                lseg_r.y1 = s2.p[j].y-(abs(s2.p[j].y-s2.p[j+1].y) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                lseg_r.y2 = s2.p[j+1].y+(abs(s2.p[j].y-s2.p[j+1].y) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                }

                        if(rect_collision(r1, lseg_r, c1, c2)){
                                if(lineseg_collision(s1.p[i], s1.p[i+1], s2.p[j], s2.p[j+1], c1, c2)){
                                        a.c[0] = i;
                                        a.c[1] = j;
                                        return a;
                                        }
                                }
                        }
                }

        return a;
}

//using intersection detection method
/*returns the line segments on each poly that are causing an intersection (if any)*/
/*uses bounding rects to minimize number of intersection-detection function calls*/
int v_poly_collision_boundcheck2(POLY s1, POLY s2, VERTEX c1, VERTEX c2, RECT r1, int seg[], int num_segs){
        int i,j,count = 0;     
        RECT lseg_r;

        seg[0] = 0;        //set the # of hit walls to 0
        for(i = 0; i < s1.num_vertices-1; i++){
                for(j = 0; j < s2.num_vertices-1; j++){
                        if(s2.p[j].x > s2.p[j+1].x){
                                lseg_r.x1 = s2.p[j+1].x-(abs(s2.p[j].x-s2.p[j+1].x) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                lseg_r.x2 = s2.p[j].x+(abs(s2.p[j].x-s2.p[j+1].x) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                }
                        else{
                                lseg_r.x1 = s2.p[j].x-(abs(s2.p[j].x-s2.p[j+1].x) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                lseg_r.x2 = s2.p[j+1].x+(abs(s2.p[j].x-s2.p[j+1].x) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                }
                        if(s2.p[j].y > s2.p[j+1].y){
                                lseg_r.y1 = s2.p[j+1].y-(abs(s2.p[j].y-s2.p[j+1].y) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                lseg_r.y2 = s2.p[j].y+(abs(s2.p[j].y-s2.p[j+1].y) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                }
                        else{
                                lseg_r.y1 = s2.p[j].y-(abs(s2.p[j].y-s2.p[j+1].y) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                lseg_r.y2 = s2.p[j+1].y+(abs(s2.p[j].y-s2.p[j+1].y) < MIN_RECT_SPAN)*MIN_RECT_SPAN;
                                }

                        if(rect_collision(r1, lseg_r, c1, c2)){
                                if(lineseg_collision(s1.p[i], s1.p[i+1], s2.p[j], s2.p[j+1], c1, c2)){
                                        seg[++count] = i;
                                        seg[++count] = j;
                                        //return 1;
                                        }
                                }
                        }
                }

        return 0;
}

//using intersection detection method
int poly_collision(POLY s1, POLY s2, VERTEX c1, VERTEX c2){
        int i,j;     

        for(i = 0; i < s1.num_vertices-1; i++){
                for(j = 0; j < s2.num_vertices-1; j++){
                        if(lineseg_collision(s1.p[i], s1.p[i+1], s2.p[j], s2.p[j+1], c1, c2)){
                                return 1;
                                }
                        }    
                }

        return 0;
}

//using ray crossing method
char poly_collision2(POLY s1, POLY s2, VERTEX c1, VERTEX c2){
        int i;     

        for(i = 0; i < s1.num_vertices-1; i++){
                if(point_in_poly(v_offset(s1.p[i], c1), s2, c2)){
                        return 1;
                        }
                }

        return 0;
}

//using angle summation method
char poly_collision3(POLY s1, POLY s2, VERTEX c1, VERTEX c2){
        int i;

        for(i = 0; i < s1.num_vertices-1; i++){
                if(point_in_poly_anglesum(s2, c2, v_offset(s1.p[i], c1))){
                        return 1;
                        }
                }

        return 0;
}

// calculate the projection range of a polygon along an axis
void get_interval(const POLY a, const VERTEX x_axis, float minmax[]){
        int i;
        minmax[0] = minmax[1] = vertex_dot_product(a.p[0], x_axis);

        for(i = 1; i < a.num_vertices; i++){
                float d = vertex_dot_product(a.p[i], x_axis);
                if (d < minmax[0]) minmax[0] = d; else if (d > minmax[1]) minmax[1] = d;
                }
}

char interval_intersect(const POLY a, const POLY b, const VERTEX x_axis, const VERTEX x_offset, const VERTEX x_vel, float * t_axis, float t_max){
	float min0, max0;
	float min1, max1;
        float h, d0, d1;
        float minmax0[2];
        float minmax1[2];

        get_interval(a, x_axis, minmax0);
        get_interval(b, x_axis, minmax1);
        min0 = minmax0[0]; max0 = minmax0[1];
        min1 = minmax1[0]; max1 = minmax1[1];

        h = vertex_dot_product(x_offset, x_axis);
	min0 += h;
	max0 += h;

        d0 = min0 - max1; // if overlapped, do < 0
        d1 = min1 - max0; // if overlapped, d1 > 0

	// separated, test dynamic intervals
        if(d0 > 0.0f || d1 > 0.0f){
                float v = vertex_dot_product(x_vel, x_axis);
                float t0, t1;

		// small velocity, so only the overlap test will be relevant. 
                if(fabs(v) < 0.0000001f) return 0;

                t0 = -d0 / v; // time of impact to d0 reaches 0
                t1 =  d1 / v; // time of impact to d0 reaches 1

                if (t0 > t1){
                        float temp = t0;
                        t0 = t1;
                        t1 = temp;
                        }
                *t_axis = (t0 > 0.0f) ? t0 : t1;

                if (*t_axis < 0.0f || *t_axis > t_max) return 0;

                return 1;
                }
        else{
		// overlap. get the interval, as a the smallest of |d0| and |d1|
		// return negative number to mark it as an overlap
                *t_axis = (d0 > d1) ? d0 : d1;
                return 1;
	}
}

char find_mtd(VERTEX x_axis[], float t_axis[], int num_axes, VERTEX * normal, float * t){
	// find collision first
        int mini = -1, i;
        *t = 0.0f;

        for(i = 0; i < num_axes; i++){ 
                if(t_axis[i] > 0){
                        if(t_axis[i] > *t){
				mini = i;
                                *t = t_axis[i];
                                *normal = x_axis[i];
                                vertex_normalize(normal);
                                }
                        }
                }

	// found one
        if(mini != -1) return 1; 

	// nope, find overlaps
	mini = -1;
        for(i = 0; i < num_axes; i++){
                float n = vertex_normalize(&x_axis[i]);
                t_axis[i] /= n;

                if(t_axis[i] > *t || mini == -1){
			mini = i;
                        *t = t_axis[i];
                        *normal = x_axis[i];
                        }
                }

        if(mini == -1) printf("Error\n");

	return (mini != -1);
}

//check for polygon collision using separating axis method
char poly_collide(const POLY a, const POLY b, VERTEX *x_offset, VERTEX *x_vel, VERTEX *normal, float *t){
        VERTEX x_axis[64];
        float t_axis[64];
        int num_axes = 0, i, j;
        float f_vel2;

        //if(a == NULL || b == NULL) return 0;

        //all the separation axes
        x_axis[num_axes] = new_vertex(-x_vel->y, x_vel->x);

        f_vel2 = vertex_dot_product(*x_vel, *x_vel);
        if(f_vel2 > 0.000001f){
                if(!interval_intersect(a, b, x_axis[num_axes], *x_offset, *x_vel, &t_axis[num_axes], *t)){
                        return 0;
                        }
                num_axes++;
                }

	// test separation axes of A
        for(j = a.num_vertices-1, i = 0; i < a.num_vertices; j = i, i++){
                VERTEX E0 = a.p[j];
                VERTEX E1 = a.p[i];
                VERTEX E  = vertex_subtract(E1, E0);
                x_axis[num_axes] = new_vertex(-E.y, E.x);

                if(!interval_intersect(a, b, x_axis[num_axes], *x_offset, *x_vel, &t_axis[num_axes], *t)){
                        return 0;
                        }
                num_axes++;
                }

        // test separation axes of B
        for(j = b.num_vertices-1, i = 0; i < b.num_vertices; j = i, i++){
                VERTEX E0 = b.p[j];
                VERTEX E1 = b.p[i];
                VERTEX E  = vertex_subtract(E1, E0);
                x_axis[num_axes] = new_vertex(-E.y, E.x);

                if(!interval_intersect(a, b, x_axis[num_axes], *x_offset, *x_vel, &t_axis[num_axes], *t)){
                        return 0;
                        }
                num_axes++;
                }


        // special case for segments
        if(b.num_vertices == 2){
                VERTEX E = vertex_subtract(b.p[1], b.p[0]);
                x_axis[num_axes] = E;
		
                if(!interval_intersect(a, b, x_axis[num_axes], *x_offset, *x_vel, &t_axis[num_axes], *t)){
                        return 0;
                        }
                num_axes++;
                }

	// special case for segments
        if(a.num_vertices == 2){
                VERTEX E = vertex_subtract(a.p[1], a.p[0]);
                x_axis[num_axes] = E;
		
                if(!interval_intersect(a, b,x_axis[num_axes], *x_offset, *x_vel, &t_axis[num_axes], *t)){
                        return 0;
                        }
                num_axes++;
                }


        if(!find_mtd(x_axis, t_axis, num_axes, normal, t))
                return 0;

	// make sure the polygons gets pushed away from each other.
        if(vertex_dot_product(*normal, *x_offset) < 0.0f){
                normal->x = -normal->x;
                normal->y = -normal->y;
                }

        return 1;
}

//keep vertex within this boundary, by wrapping
void keep_vertex_in_bounds(VERTEX * p, int x1, int y1, int x2, int y2){
        if(p->x < x1) p->x = x2;
        else if(p->x > x2) p->x = x1;

        if(p->y < y1) p->y = y2;
        else if(p->y > y2) p->y = y1;
}

//rotate vertex using matrix transformation
void rotate_vertex(VERTEX * vx, float rot_angle){
        fixed vx_x = ftofix(vx->x);
        fixed vx_y = ftofix(vx->y);
	float f_ang = rot_angle*128/PI;
	fixed ang = ftofix(f_ang);
        
        // [  cos[a]  | -sin[a]  ]  [ x ] = [ w ]
        // [  sin[a]  |  cos[a]  ]  [ y ]   [ z ]
        vx->x = fixtof(fmul(vx_x, fcos(ang)) - fmul(vx_y, fsin(ang)));
        vx->y = fixtof(fmul(vx_x, fsin(ang)) + fmul(vx_y, fcos(ang)));
//        vx->x = vx->x*cos(rot_angle)-vx->y*sin(rot_angle);
//        vx->y = vx->x*sin(rot_angle)+vx->y*cos(rot_angle);
}

//rotate polygon
void rotate_poly(POLY * po, int rot_angle){
        int i;

        for(i = 0; i < po->num_vertices; i++){
                rotate_vertex(&po->p[i], rot_angle);
                }
}

//rotate polygon (offset by a certain vertex)
void rotate_poly_v_offset(POLY * po, VERTEX v, int rot_angle){
        int i;
        POLY tmp_poly = *po;

        for(i = 0; i < po->num_vertices; i++){
                tmp_poly.p[i].x -= v.x;
                tmp_poly.p[i].y -= v.y;
                rotate_vertex(&tmp_poly.p[i], rot_angle);
                }
        for(i = 0; i < tmp_poly.num_vertices; i++){
                tmp_poly.p[i].x += v.x;
                tmp_poly.p[i].y += v.y;
                }
        po = &tmp_poly;
}

//move position vector by a velocity vector..
void move_vertex(VERTEX * s, VERTEX v){
        s->x += v.x;
        s->y += v.y;
}

/*
void draw_poly(BITMAP * bp, POLY * po, int cx, int cy){
        int i;

        for(i = 0; i < po->num_vertices-1; i++){
                line(bp, cx+po->p[i].x, cy+po->p[i].y, cx+po->p[i+1].x, cy+po->p[i+1].y, 1);
                //line(bp, cx+po->p[i].x, cy+po->p[i].y, cx+po->p[i+1].x, cy+po->p[i+1].y, po->color);
                }
}

//draw a polygon, scaled by a certain factor
void draw_poly_scaled(BITMAP * bp, POLY * po, int cx, int cy, float scale){
        int i;

        for(i = 0; i < po->num_vertices-1; i++){
                line(bp, cx+po->p[i].x*scale, cy+po->p[i].y*scale, cx+po->p[i+1].x*scale, cy+po->p[i+1].y*scale, po->color);
                }
}

//draw a polygon, scaled by a certain factor (vertical and horizontal scales delimited)
void draw_poly_scaled_vh(BITMAP * bp, POLY * po, int cx, int cy, float h_scale, float v_scale){
        int i;

        for(i = 0; i < po->num_vertices-1; i++){
                line(bp, cx+po->p[i].x*h_scale, cy+po->p[i].y*v_scale, cx+po->p[i+1].x*h_scale, cy+po->p[i+1].y*v_scale, po->color);
                }
}
*/

//check collision between two circles..
int check_circle_collision(VERTEX a, VERTEX b, int a_rad, int b_rad){
        float deltax, deltay, r;

        deltax = (b.x-a.x);
        deltax *= deltax;
        deltay = (b.y-a.y);
        deltay *= deltay;
        r = a_rad + b_rad;
        r *= r;
        if(r >= deltax + deltay){
                return 1;
                }
        return 0;
}

//check collision between two circles..
int check_circle_collision_debug(VERTEX a, VERTEX b, int a_rad, int b_rad){
        float deltax, deltay, r;

        deltax = (b.x-a.x);
        deltax *= deltax;
        deltay = (b.y-a.y);
        deltay *= deltay;
        r = a_rad + b_rad;
        r *= r;
        if(r >= deltax + deltay){
                return 1;
                }
        //return 0;
        return r;
}

/*returns distance between two vertices*/
double v_distance(VERTEX a, VERTEX b){
        return sqrt((b.x-a.x)*(b.x-a.x)+(b.y-a.y)*(b.y-a.y));
}

/*returns distance between two points*/
double distance(double x1, double y1, double x2, double y2){
        return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

/*sorts a v_int and its reference v_int (smallest to largest)*/
void v_int_heap_sort_ref(V_INT * v_a, V_INT * v_b, int len){
        int i,j;
        int temp_val, temp_ref;

        if(len < 2){
                if(v_a->c[0] < v_a->c[1]){
                        temp_val = v_a->c[0];
                        temp_ref = v_b->c[0];
                        v_a->c[0] = v_a->c[1];
                        v_b->c[0] = v_b->c[1];
                        v_a->c[1] = temp_val;
                        v_b->c[1] = temp_ref;
                        }
                }
        else{
                for(i = 1; i < len; i++){
                        temp_val = v_a->c[i];
                        temp_ref = v_b->c[i];
                        j = i;
                        while(j > 0 && v_a->c[j-1] > temp_val){
                                v_a->c[j] = v_a->c[j-1];
                                v_b->c[j] = v_b->c[j-1];
                                j--;
                                }
                        v_a->c[j] = temp_val;
                        v_b->c[j] = temp_ref;
                        }
                }
}

/*sorts a v_int and its reference v_int (largest to smallest)*/
void v_int_heap_sort_ref_rev(V_INT * v_a, V_INT * v_b, int len){
        int i,j;
        int temp_val, temp_ref;

        if(len < 2){
                if(v_a->c[0] > v_a->c[1]){
                        temp_val = v_a->c[0];
                        temp_ref = v_b->c[0];
                        v_a->c[0] = v_a->c[1];
                        v_b->c[0] = v_b->c[1];
                        v_a->c[1] = temp_val;
                        v_b->c[1] = temp_ref;
                        }
                }
        else{
                for(i = 1; i < len; i++){
                        temp_val = v_a->c[i];
                        temp_ref = v_b->c[i];
                        j = i;
                        while(j > 0 && v_a->c[j-1] < temp_val){
                                v_a->c[j] = v_a->c[j-1];
                                v_b->c[j] = v_b->c[j-1];
                                j--;
                                }
                        v_a->c[j] = temp_val;
                        v_b->c[j] = temp_ref;
                        }
                }
}

//draws a dotted line between the two points depending on the counter and the dot length
void draw_dotted_line(BITMAP * bp, int x1, int y1, int x2, int y2, int color, int counter, int dlen){
	int i, deltax, deltay;
	int numpixels, d, dinc1, dinc2;
	int x, xinc1, xinc2;
	int y, yinc1, yinc2;
	char draw = 1;
  
	//{ Calculate deltax and deltay for initialisation }
  	deltax = abs(x2 - x1);
  	deltay = abs(y2 - y1);

  	//{ Initialize all vars based on which is the independent variable }
  	if(deltax >= deltay){
      		//{ x is independent variable }
      		numpixels = deltax + 1;
      		d = (2 * deltay) - deltax;
      		dinc1 = deltay * 2;
      		dinc2 = (deltay - deltax) * 2;
      		xinc1 = 1;
      		xinc2 = 1;
      		yinc1 = 0;
      		yinc2 = 1;
		}
  	else{
      		//{ y is independent variable }
      		numpixels = deltay + 1;
      		d = (2 * deltax) - deltay;
      		dinc1 = deltax * 2;
      		dinc2 = (deltax - deltay) * 2;
      		xinc1 = 0;
      		xinc2 = 1;
      		yinc1 = 1;
      		yinc2 = 1;
		}

  	//{ Make sure x and y move in the right directions }
  	if(x1 > x2){
      		xinc1 = -xinc1;
      		xinc2 = -xinc2;
		}
  	if(y1 > y2){
      		yinc1 = -yinc1;
      		yinc2 = -yinc2;
		}

  	//{ Start drawing at  }
  	x = x1;
  	y = y1;

  	//printf("incoming paramaters: (%f, %f)->(%f, %f)\n", x1, y1, x2, y2);
  	//printf("num pixels: %i, start pt(%i, %i)\n", numpixels, x, y);
  	//{ Draw the pixels }
  	for(i = 1; i < numpixels; i++){
	  	//stop/start drawing when the length interval has been reached
	  	if(!(i%dlen)) draw = !draw;
	  	//if(!(i%(counter))) draw = !draw;
	  	
	  	if(draw) putpixel(bp, x, y, color);
      		if(d < 0){
          		d = d + dinc1;
          		x = x + xinc1;
          		y = y + yinc1;
  			}
      		else{
          		d = d + dinc2;
          		x = x + xinc2;
          		y = y + yinc2;
  			}
		}
}

//draws a line using the two-step line algorithm (noise parameter will put random pixels here and there along the line's outside)
//algorithm obtained from http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
void draw_line_twostep(BITMAP * bp, int x0, int y0, int x1, int y1, int color, int noise){
        int dy = y1 - y0;
        int dx = x1 - x0;
        int stepx, stepy;

        if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
        if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }

        putpixel(bp, x0, y0, color);
        putpixel(bp, x1, y1, color);
        if (dx > dy) {
            int length = (dx - 1) >> 2;
            int extras = (dx - 1) & 3;
            int incr2 = (dy << 2) - (dx << 1);
            if (incr2 < 0) {
                int c = dy << 1;
                int i;
                int incr1 = c << 1;
                int d =  incr1 - dx;
                for (i = 0; i < length; i++) {
                    x0 += stepx;
                    x1 -= stepx;
                    if (d < 0) {// Pattern:
                        if(noise){
	                        putpixel(bp, x0+(rand()%noise), y0+(rand()%noise), color);
                        	}
                        putpixel(bp, x0, y0, color);//
                        putpixel(bp, x0 += stepx, y0, color);//  x o o
                        putpixel(bp, x1, y1, color);//
                        putpixel(bp, x1 -= stepx, y1, color);
                        d += incr1;
                    } else {
                        if (d < c) {// Pattern:
                            putpixel(bp, x0, y0, color);//      o
                            putpixel(bp, x0 += stepx, y0 += stepy, color);//  x o
                            putpixel(bp, x1, y1, color);//
                            putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                        } else {
                            putpixel(bp, x0, y0 += stepy, color);// Pattern:
                            putpixel(bp, x0 += stepx, y0, color);//    o o 
                            putpixel(bp, x1, y1 -= stepy, color);//  x
                            putpixel(bp, x1 -= stepx, y1, color);//
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d < 0) {
                        putpixel(bp, x0 += stepx, y0, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0, color);
                        if (extras > 2) putpixel(bp, x1 -= stepx, y1, color);
                    } else
                    if (d < c) {
                        putpixel(bp, x0 += stepx, y0, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 2) putpixel(bp, x1 -= stepx, y1, color);
                    } else {
                        putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0, color);
                        if (extras > 2) putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                    }
                }
            } else {
                int c = (dy - dx) << 1;
                int i;
                int incr1 = c << 1;
                int d =  incr1 + dx;
                for (i = 0; i < length; i++) {
                    x0 += stepx;
                    x1 -= stepx;
                    if(noise){
                        putpixel(bp, x0-(rand()%noise), y0-(rand()%noise), color);
                       	}
                    if (d > 0) {
                        putpixel(bp, x0, y0 += stepy, color);// Pattern:
                        putpixel(bp, x0 += stepx, y0 += stepy, color);//      o
                        putpixel(bp, x1, y1 -= stepy, color);//    o
                        putpixel(bp, x1 -= stepx, y1 -= stepy, color);//  x
                        d += incr1;
                    } else {
                        if (d < c) {
                            putpixel(bp, x0, y0, color);// Pattern:
                            putpixel(bp, x0 += stepx, y0 += stepy, color);       //      o
                            putpixel(bp, x1, y1, color);                         //  x o
                            putpixel(bp, x1 -= stepx, y1 -= stepy, color);       //
                        } else {
                            putpixel(bp, x0, y0 += stepy, color);// Pattern:
                            putpixel(bp, x0 += stepx, y0, color);//    o o
                            putpixel(bp, x1, y1 -= stepy, color);//  x
                            putpixel(bp, x1 -= stepx, y1, color);//
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if(noise){
                        putpixel(bp, x0+(rand()%noise), y0+(rand()%noise), color);
                       	}
                    if (d > 0) {
                        putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 2) putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                    } else
                    if (d < c) {
                        putpixel(bp, x0 += stepx, y0, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 2) putpixel(bp, x1 -= stepx, y1, color);
                    } else {
                        putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0, color);
                        if (extras > 2) {
                            if (d > c)
                                putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                            else
                                putpixel(bp, x1 -= stepx, y1, color);
                        }
                    }
                }
            }
        } else {
            int length = (dy - 1) >> 2;
            int extras = (dy - 1) & 3;
            int incr2 = (dx << 2) - (dy << 1);
            if (incr2 < 0) {
                int c = dx << 1;
                int i;
                int incr1 = c << 1;
                int d =  incr1 - dy;
                for (i = 0; i < length; i++) {
                    y0 += stepy;
                    y1 -= stepy;
                    if(noise){
                        putpixel(bp, x0-(rand()%noise), y0+(rand()%noise), color);
                       	}
                    if (d < 0) {
                        putpixel(bp, x0, y0, color);
                        putpixel(bp, x0, y0 += stepy, color);
                        putpixel(bp, x1, y1, color);
                        putpixel(bp, x1, y1 -= stepy, color);
                        d += incr1;
                    } else {
                        if (d < c) {
                            putpixel(bp, x0, y0, color);
                            putpixel(bp, x0 += stepx, y0 += stepy, color);
                            putpixel(bp, x1, y1, color);
                            putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                        } else {
                            putpixel(bp, x0 += stepx, y0, color);
                            putpixel(bp, x0, y0 += stepy, color);
                            putpixel(bp, x1 -= stepx, y1, color);
                            putpixel(bp, x1, y1 -= stepy, color);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d < 0) {
                        putpixel(bp, x0, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0, y0 += stepy, color);
                        if (extras > 2) putpixel(bp, x1, y1 -= stepy, color);
                    } else
                    if (d < c) {
                        putpixel(bp, stepx, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 2) putpixel(bp, x1, y1 -= stepy, color);
                    } else {
                        putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0, y0 += stepy, color);
                        if (extras > 2) putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                    }
                }
            } else {
                int c = (dx - dy) << 1;
		int i;
                int incr1 = c << 1;
                int d =  incr1 + dy;
                for (i = 0; i < length; i++) {
                    y0 += stepy;
                    y1 -= stepy;
                    if(noise){
                        putpixel(bp, x0+(rand()%noise), y0-(rand()%noise), color);
                       	}
                    if (d > 0) {
                        putpixel(bp, x0 += stepx, y0, color);
                        putpixel(bp, x0 += stepx, y0 += stepy, color);
                        putpixel(bp, x1 -= stepy, y1, color);
                        putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                        d += incr1;
                    } else {
                        if (d < c) {
                            putpixel(bp, x0, y0, color);
                            putpixel(bp, x0 += stepx, y0 += stepy, color);
                            putpixel(bp, x1, y1, color);
                            putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                        } else {
                            putpixel(bp, x0 += stepx, y0, color);
                            putpixel(bp, x0, y0 += stepy, color);
                            putpixel(bp, x1 -= stepx, y1, color);
                            putpixel(bp, x1, y1 -= stepy, color);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 2) putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                    } else
                    if (d < c) {
                        putpixel(bp, x0, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 2) putpixel(bp, x1, y1 -= stepy, color);
                    } else {
                        putpixel(bp, x0 += stepx, y0 += stepy, color);
                        if (extras > 1) putpixel(bp, x0, y0 += stepy, color);
                        if (extras > 2) {
                            if (d > c)
                                putpixel(bp, x1 -= stepx, y1 -= stepy, color);
                            else
                                putpixel(bp, x1, y1 -= stepy, color);
                        }
                    }
                }
            }
        }
    }

