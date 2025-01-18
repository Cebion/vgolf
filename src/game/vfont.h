#ifndef VFONT_H
#define VFONT_H

#include <allegro.h>
#include "poly.h"

#define V_MESSAGE_MAX_LENGTH 32
#define VMSG_TEXT_PLAIN 0
#define VMSG_TEXT_SCALE_ALL_VERT_FREEZE 1
#define VMSG_TEXT_ENTER_FROMRIGHT 2
#define VMSG_TEXT_ENTER_FROMBOTTOM 4
#define VMSG_TEXT_ENTER_FROMLEFT 5
#define VMSG_TEXT_ENTER_FROMTOP 6
#define VMSG_TEXT_ENTER_FROMRIGHT_SCALED 8
#define VMSG_TEXT_ENTER_FROMBOTTOM_SCALED 9
#define VMSG_TEXT_ENTER_FROMLEFT_SCALED 10
#define VMSG_TEXT_ENTER_FROMTOP_SCALED 11
#define VMSG_TEXT_SCALE_ALL 3
#define VMSG_TEXT_SCALE_ALL_CENTERED 15
#define VMSG_TEXT_ROTATE_ALL 7
#define VMSG_TEXT_FLOAT 12
#define VMSG_TEXT_FLOAT_CENTERED 13

typedef struct{
        BITMAP * pic;
        short ref;
        }V_LETTER;

typedef struct
{
//        V_LETTER c[V_MESSAGE_MAX_LENGTH];
//        char c[V_MESSAGE_MAX_LENGTH];
        VERTEX v,s;
        char txt[V_MESSAGE_MAX_LENGTH];
        int length;                       //length of string/message
        short attrib_ref[4];
        short timer;
        short timer_end;
        char method;
        char active;
        }V_MESSAGE;

typedef struct{
        V_LETTER c[128];
        int w,h;
        }V_FONT;

extern BITMAP * vfscratch;

int load_vfont_bmp(V_FONT * vf, int w, int h, char * filename);
int load_vfont(V_FONT * vf, char * filename);
int save_vfont(V_FONT * vf, char * filename);
void draw_v_char(BITMAP * bp, V_LETTER * vl, int x, int y);
void draw_v_char_scaled(BITMAP * bp, V_LETTER * vl, int x, int y, float scale);
void vfont_textout(BITMAP * bp, FONT * vf, char * txt, int x, int y, char method);
void vfont_textout_center(BITMAP * bp, V_FONT * vf, char * txt, int ox, int oy, char method);
void vfont_textout_center_breaks(BITMAP * bp, V_FONT * vf, char * txt, int ox, int oy, char method);
void vfont_printf(BITMAP * bp, FONT * vf, int x, int y, char method, char * format, ...);
void vfont_printf_center(BITMAP * bp, V_FONT * vf, int x, int y, char method, char * format, ...);
void vfont_printf_center_breaks(BITMAP * bp, V_FONT * vf, int x, int y, char method, char * format, ...);
void vfont_message_scale_logic(V_MESSAGE * vmsg, char method, char increment);
void vfont_message_float_logic(V_MESSAGE * vmsg, char method, char increment);
void vfont_message_move_logic(FONT * vf, V_MESSAGE * vmsg, char method, char increment, int dir, float scale);
void vfont_message_rotate_logic(V_MESSAGE * vmsg, char method, char increment);
void vfont_message_logic(FONT * vf, V_MESSAGE * vmsg, char method, char increment);
void vfont_message_textout(BITMAP * bp, FONT * vf, V_MESSAGE * vmsg, char method, char increment);
void set_vfont_message(V_MESSAGE * vmsg, FONT * vf, char * txt, int x, int y, int z, char method, int m_ref1, int m_ref2, int m_ref3, int m_ref4, int t_end);
void deactivate_vfont_message(V_MESSAGE * vmsg);
void add_vfont_queue_message(V_MESSAGE vmsg[], int num_vmessages, FONT * vf, int * cur_slot, char * txt, int x, int y, int z, char method, int m_ref1, int m_ref2, int m_ref3, int m_ref4, int t_end);

#endif
