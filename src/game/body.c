#include "body.h"

//returns a body with an offset center vertex
BODY body_offset(BODY bo, VERTEX o){
	BODY bod = bo;
		
	bod.pos.x = bo.pos.x+o.x;
	bod.pos.y = bo.pos.y+o.y;
		
	return bod;
}

//returns a body that is formed by a line segment
BODY new_seg_body(VERTEX a, VERTEX b, VERTEX c, int * num_sides){
        BODY bod;
        
        bod.pos = c;                  //set the body center as the center vertex passed in
        bod.vel = new_vertex(0, 0);   //body has 0 velocity
        bod.density = 1;              //body density is 1.0
        bod.friction = 0;             //body has no friction
        bod.restitution = bod.mass = bod.inv_mass = 0;   //various body settings
        bod.poly.num_vertices = 2;    //body has two endpoints..
        bod.poly.p[0] = a;            //a and b are the body's endpoints
        bod.poly.p[1] = b;
        //place a vertex at the negative normal to this line segment
        //so the "center" of the line segment lies behind the wall
        //so the ball will bounce properly..
        {
                float ang;
                VERTEX n, m, nn;

                //make a and b global coordinates by adding the center coords to them
                a = vertex_add(a, c); 
                b = vertex_add(b, c);
                //find the normal to the angle between a and b
                ang = atan2(a.y-b.y, a.x-b.x)-M_PI/2;   
                //ang = mod_angle(atan2(a.y-b.y, a.x-b.x)-M_PI/2);   
		//create a normal vector that extends 10 units out from the midpoint of the line between a and b
		n = new_vertex(10*cos(ang), 10*sin(ang));
		m = new_vertex((a.x+b.x)/2, (a.y+b.y)/2);
                //n = new_vertex(10*cos(ang), 10*sin(ang));  
                //make n a global coordinate
                //n = vertex_add(n, c);
                nn.x = m.x+n.x;
                nn.y = m.y+n.y;
                //subtract the normal point from both endpoints..
                bod.poly.p[0] = vertex_subtract(a, nn);
                bod.poly.p[1] = vertex_subtract(b, nn);
                //make the normal point the center of this body so the normal vector will point out
                bod.pos = nn;
                }

        return bod;
}

/*
//returns a body that is formed by a line segment
BODY new_seg_body(VERTEX a, VERTEX b, VERTEX c, int * num_sides){
        BODY bod;
        
        bod.pos = c;                  //set the body center as the center vertex passed in
        bod.vel = new_vertex(0, 0);   //body has 0 velocity
        bod.density = 1;              //body density is 1.0
        bod.friction = 0;             //body has no friction
        bod.restitution = bod.mass = bod.inv_mass = 0;   //various body settings
        bod.poly.num_vertices = 2;    //body has two endpoints..
        bod.poly.p[0] = a;            //a and b are the body's endpoints
        bod.poly.p[1] = b;
        //place a vertex at the negative normal to this line segment
        //so the "center" of the line segment lies behind the wall
        //so the ball will bounce properly..
        {
                float ang;
                VERTEX n, m, nn;

                //make a and b global coordinates by adding the center coords to them
                a = vertex_add(a, c); 
                b = vertex_add(b, c);
                //find the normal to the angle between a and b
                ang = atan2(a.y-b.y, a.x-b.x)-M_PI/2;   
                //ang = mod_angle(atan2(a.y-b.y, a.x-b.x)-M_PI/2);   
		//create a normal vector that extends 10 units out from the midpoint of the line between a and b
		n = new_vertex(10*cos(ang)*(a.x+b.x)/2, 10*sin(ang)*(a.y+b.y)/2);
                //make n a global coordinate
                n = vertex_add(n, c);
                //subtract the normal point from both endpoints..
                bod.poly.p[0] = vertex_subtract(a, n);
                bod.poly.p[1] = vertex_subtract(b, n);
                //make the normal point the center of this body so the normal vector will point out
                bod.pos = n;
                }

        return bod;
}
*/

//returns a triangular body that is formed by two line segments
BODY new_seg_body_triangle(VERTEX a, VERTEX b, VERTEX d, VERTEX c, int * num_sides){
        BODY bod;
        bod.pos = c;
        bod.vel = new_vertex(0, 0);
        bod.density = 1;
        bod.friction = 0;
        bod.restitution = bod.mass = bod.inv_mass = 0;
        bod.poly.num_vertices = 3;
        bod.poly.p[0] = a;
        bod.poly.p[1] = b;
        bod.poly.p[2] = d;
        //place a vertex at the negative normal to this line segment
        //so the "center" of the line segment lies behind the wall
        //so the ball will bounce properly..
        {
                float ang;
                VERTEX n;

                a = vertex_add(a, c);
                b = vertex_add(b, c);
                d = vertex_add(d, c);
                ang = atan2(a.y-b.y, a.x-b.x)-M_PI/2 + atan2(b.y-d.y, b.x-d.x)-M_PI/2;
                n = new_vertex(10*cos(ang)*(a.x+b.x)/2, 10*sin(ang)*(a.y+b.y)/2);
                n = vertex_add(n, c);
                bod.poly.p[0] = vertex_subtract(a, n);
                bod.poly.p[1] = vertex_subtract(d, n);
                bod.poly.p[2] = vertex_subtract(b, n);
                bod.pos = n;
                }

        return bod;
}

//calculate all the wall normals for this body's poly..
void calc_body_normals(BODY * bo){
        int i;

        for(i = 0; i < bo->poly.num_vertices-1; i++){
                bo->seg_norm[i] = atan2(bo->poly.p[i].y-bo->poly.p[(i+1)%bo->poly.num_vertices].y, bo->poly.p[i].x-bo->poly.p[(i+1)%bo->poly.num_vertices].x)-M_PI/2;
                }
}

// two objects collided at time t. stop them at that time
void process_body_collision(BODY * a, BODY * b, const VERTEX normal, float t)
{
	
	/* D is the subtraction of the velocity vectors of a and b */
	VERTEX D = new_vertex(a->vel.x-b->vel.x, a->vel.y-b->vel.y);

        float n = vertex_dot_product(D, normal);
        float dt, cof;
        float m0, m1, m, r0, r1;

        VERTEX Dn = new_vertex(normal.x*n, normal.y*n);
        VERTEX Dt = vertex_subtract(D, Dn);
	
        if(n > 0.0f) Dn = new_vertex(0, 0);

        dt = vertex_dot_product(Dt, Dt);
        cof = a->friction;
	
        if(dt < a->glue*a->glue) cof = 1.01f;

//        D = new_vertex(-(1.0f + a->restitution)*Dn.x - cof*Dt.x, -(1.0f + a->restitution)*Dn.y - cof*Dt.y);
	D.x = -(1.0f + a->restitution) * Dn.x - cof * Dt.x;
	D.y = -(1.0f + a->restitution) * Dn.y - cof * Dt.y;
//	D = -(1.0f + s_fRestitution) * Dn - (CoF) * Dt;

        m0 = a->inv_mass;
        m1 = b->inv_mass;
        m  = m0 + m1;
        r0 = m0 / m;
        r1 = m1 / m;
	
//        a->vel.x += 1.818175*D.x*r0;
//        a->vel.y += 1.818175*D.y*r0;
        a->vel.x += BOUNCE_FACTOR*D.x*r0;
        a->vel.y += BOUNCE_FACTOR*D.y*r0;
        b->vel.x -= D.x*r1;
        b->vel.y -= D.y*r1;
}

// two objects overlapped. push them away from each other
void process_body_overlap(BODY * a, BODY * b, const VERTEX mtd){
        VERTEX n;
        if (!a->moveable)
        {
                b->pos.x -= mtd.x;
                b->pos.y -= mtd.y;
        }
        else if (!b->moveable)
        {
                a->pos.x += mtd.x;
                a->pos.y += mtd.y;
        }
        else
        {
                a->pos.x += mtd.x * 0.5f;
                a->pos.y += mtd.y * 0.5f;
                b->pos.x -= mtd.x * 0.5f;
                b->pos.y -= mtd.y * 0.5f;
        }

        n = mtd;
        vertex_normalize(&n);
        process_body_collision(a, b, n, 0.0f);
}

char body_collide(BODY * a, BODY * b)
{
        float t = 1.0;
        VERTEX n;
        VERTEX x_rel_pos = new_vertex(a->pos.x-b->pos.x, a->pos.y-b->pos.y);
        VERTEX x_rel_dis = new_vertex(a->vel.x-b->vel.x, a->vel.y-b->vel.y);

        if(poly_collide(a->poly, b->poly, &x_rel_pos, &x_rel_dis, &n, &t)){
                if(t < 0){
                        process_body_overlap(a, b, new_vertex(n.x*(-t), n.y*(-t)));
                        }
                else{
                        process_body_collision(a, b, n, t);
                        }
                return 1;
                }
        return 0;
}

