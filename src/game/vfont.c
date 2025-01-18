#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "vfont.h"

BITMAP * vfscratch = NULL;

//load font from bitmap..
int load_vfont_bmp(V_FONT * vf, int w, int h, char * filename){
        BITMAP * tmp_pic = load_pcx(filename, NULL);
        if(!tmp_pic)
        {
	        return 0;
        }
        int i,j,k;

        vf->w = w;
        vf->h = h;

        for(i = 0; i < 128; i++){
                vf->c[i].pic = create_bitmap(vf->w, vf->h);
                for(j = 0; j < vf->h; j++){
                        for(k = 0; k < vf->w; k++){
                                putpixel(vf->c[i].pic, k, j, getpixel(tmp_pic, ((i+16)%16)*vf->w+k, (i/16)*vf->h+j));
                                }
                        }
                }

        return 1;
}

//load font from file..
int load_vfont(V_FONT * vf, char * filename){
        PACKFILE * in_file;

        in_file = pack_fopen(filename, "r");
        pack_fclose(in_file);

        return 1;
}

//save font to specified file..
int save_vfont(V_FONT * vf, char * filename){
        PACKFILE * out_file;

        out_file = pack_fopen(filename, "w");
        pack_fclose(out_file);

        return 1;
}

void draw_av_char(BITMAP * bp, FONT * fp, int l, int x, int y, int c, int bc)
{
	textprintf_ex(bp, fp, x - 1, y - 1, bc, -1, "%c", l);
	textprintf_ex(bp, fp, x + 1, y - 1, bc, -1, "%c", l);
	textprintf_ex(bp, fp, x - 1, y + 1, bc, -1, "%c", l);
	textprintf_ex(bp, fp, x + 1, y + 1, bc, -1, "%c", l);
	textprintf_ex(bp, fp, x, y, c, -1, "%c", l);
}

void draw_av_char_scaled(BITMAP * bp, FONT * fp, int l, int x, int y, int c, int bc, float scale)
{
	char letter[2] = {0, 0};
	letter[0] = l;
	clear_bitmap(vfscratch);
	textprintf_ex(vfscratch, fp, 1, 0, bc, -1, "%c", l);
	textprintf_ex(vfscratch, fp, 1, 2, bc, -1, "%c", l);
	textprintf_ex(vfscratch, fp, 0, 1, bc, -1, "%c", l);
	textprintf_ex(vfscratch, fp, 2, 1, bc, -1, "%c", l);
	textprintf_ex(vfscratch, fp, 1, 1, c, -1, "%c", l);
	masked_stretch_blit(vfscratch, bp, 0, 0, text_length(fp, letter) + 2, text_height(fp) + 2, x, y, (int)((text_length(fp, letter) + 2)*scale), (int)((text_height(fp) + 2)*scale));
//	stretch_sprite(bp, vfscratch, x, y, 32, 32);
}

void draw_av_char_rotated(BITMAP * bp, FONT * fp, int l, int x, int y, int c, int bc, int angle, float scale)
{
	char letter[2] = {0, 0};
	int w, h;
	letter[0] = l;
	w = text_length(fp, letter) + 2;
	h = text_height(fp) + 2;
	clear_bitmap(vfscratch);
	textprintf_ex(vfscratch, fp, 1, 0, bc, -1, "%c", l);
	textprintf_ex(vfscratch, fp, 1, 2, bc, -1, "%c", l);
	textprintf_ex(vfscratch, fp, 0, 1, bc, -1, "%c", l);
	textprintf_ex(vfscratch, fp, 2, 1, bc, -1, "%c", l);
	textprintf_ex(vfscratch, fp, 1, 1, c, -1, "%c", l);
	pivot_scaled_sprite(bp, vfscratch, x + w / 2, y + h / 2, w / 2, h / 2, itofix(angle), ftofix(scale));
}

//draws one vector character..
void draw_v_char(BITMAP * bp, V_LETTER * vl, int x, int y){
        masked_blit(vl->pic, bp, 0, 0, x, y, vl->pic->w, vl->pic->h);
}

//draw a vector character scaled by a specified amount
void draw_v_char_scaled(BITMAP * bp, V_LETTER * vl, int x, int y, float scale){
        masked_stretch_blit(vl->pic, bp, 0, 0, vl->pic->w, vl->pic->h, x, y, (int)(vl->pic->w*scale), (int)(vl->pic->h*scale));
}

//draw a vector character rotated by a certain angle
void draw_v_char_rotated(BITMAP * bp, V_LETTER * vl, int x, int y, int angle, float scale){
        rotate_scaled_sprite(bp, vl->pic, x, y, itofix(angle), ftofix(scale));
}

//write a string to the screen using the vector font
void vfont_textout(BITMAP * bp, FONT * vf, char * txt, int x, int y, char method)
{
	int i;
	int px;
	char letter[2] = {0};

	if(method == 0)
	{
		px = x;
		for(i = 0; i < strlen(txt); i++)
		{
			draw_av_char(bp, vf, txt[i], px, y, makecol(0, 192, 0), makecol(0, 0, 0));
			px += text_length(vf, letter);
		}
	}
	else if(method > 0)
	{
  //              for(i = 0; i < strlen(txt); i++){
//                        draw_av_char_scaled(bp, &vf->c[txt[i]], x+i*(float)((int)method), y, (float)((int)method)/vf->w);
    //                    }
	}
}

//writes a string centered at a certain point
void vfont_textout_center_breaks(BITMAP * bp, V_FONT * vf, char * txt, int ox, int oy, char method){
        int i, x[32], line_off = 0, num_lines = 0, l_break[32] = {-1};
	
        for(i = 0; txt[i] != '\0'; i++){            //find all the line breaks in this text
        	if(txt[i] == '\n'){
	        	l_break[num_lines++] = i;   //add this char # to the line break list
        		}
        	}

  	if(!num_lines) x[0] = ox - ((strlen(txt) / 2) * vf->w) - (strlen(txt)%2)*vf->w/2;
  	else{
	  	if(num_lines == 1){
		  	x[0] = ox - ((l_break[0]/2)*vf->w) - (l_break[0]%2)*vf->w/2;
		  	x[1] = ox - (((strlen(txt)-l_break[0])/2)*vf->w) - ((strlen(txt)-l_break[0])%2)*vf->w/2;
	  		}
	  	if(num_lines == 2){
		  	x[0] = ox - ((l_break[0]/2)*vf->w) - (l_break[0]%2)*vf->w/2;
		  	x[1] = ox - (((l_break[1]-l_break[0])/2)*vf->w) - ((l_break[1]-l_break[0])%2)*vf->w/2;
		  	x[2] = ox - (((strlen(txt)-l_break[1])/2)*vf->w) - ((strlen(txt)-l_break[1])%2)*vf->w/2;
	  		}
  		}

        if(method == 0){
                for(i = 0; txt[i] != '\0'; i++){
	                if(i == l_break[line_off]){  //see if a line break is present at this char slot
		                line_off++;   //add one to the line offset
		                i++;          //skip over the \n
	                	}
                        draw_v_char(bp, &vf->c[(int)txt[i]], x[line_off]+i*vf->w, oy+line_off*(vf->h+1));
                        }
                }
        else if(method > 0){
                for(i = 0; txt[i] != '\0'; i++){
	                if(i == l_break[line_off]){  //see if a line break is present at this char slot
		                line_off++;   //add one to the line offset
		                i++;          //skip over the \n
	                	}
                        draw_v_char_scaled(bp, &vf->c[(int)txt[i]], x[line_off]+i*vf->w, oy+line_off*(vf->h+1), (float)((int)method)/vf->w);
                        }
                }
}

//writes a string centered at a certain point
void vfont_textout_center(BITMAP * bp, V_FONT * vf, char * txt, int ox, int oy, char method){
        int i, x;

        x = ox - ((strlen(txt) / 2) * vf->w) - (strlen(txt)%2)*vf->w/2;

        if(method == 0){
                for(i = 0; txt[i] != '\0'; i++){
                        draw_v_char(bp, &vf->c[(int)txt[i]], x+i*vf->w, oy);
                        }
                }
        else if(method > 0){
                for(i = 0; txt[i] != '\0'; i++){
                        draw_v_char_scaled(bp, &vf->c[(int)txt[i]], x+i*vf->w, oy, (float)((int)method)/vf->w);
                        }
                }
}

//formatted string output
void vfont_printf(BITMAP * bp, FONT * vf, int x, int y, char method, char * format, ...){
    	char buf[1024];

    	va_list ap;
    	va_start(ap, format);
    	vsprintf(buf, format, ap);
    	va_end(ap);

	vfont_textout(bp, vf, buf, x, y, method);
}

//formatted string output (centered)
void vfont_printf_center(BITMAP * bp, V_FONT * vf, int x, int y, char method, char * format, ...){
    char buf[1024];

    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    vfont_textout_center(bp, vf, buf, x, y, method);
}

//formatted string output (centered with line breaks)
void vfont_printf_center_breaks(BITMAP * bp, V_FONT * vf, int x, int y, char method, char * format, ...){
    char buf[1024];

    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    vfont_textout_center_breaks(bp, vf, buf, x, y, method);
}

//do the vmessage scaling routine
void vfont_message_scale_logic(V_MESSAGE * vmsg, char method, char increment){
	if(vmsg->timer_end >= 0){
		int t_end1 = vmsg->attrib_ref[2]*2;
		int t_end2 = vmsg->attrib_ref[3]*2;
		
        	if(vmsg->attrib_ref[0] == 1){
                	if(vmsg->attrib_ref[1] > t_end1) vmsg->attrib_ref[1] -= increment;
                        else vmsg->attrib_ref[0] = 0;
                        }
                else{
                        if(vmsg->attrib_ref[1] < t_end2) vmsg->attrib_ref[1] += increment;
                        else vmsg->attrib_ref[0] = 1;
                        }
                if(vmsg->timer < vmsg->timer_end) vmsg->timer += increment;
                else{
                        vmsg->timer = 0;
                        vmsg->active = 2;
                        }
                }
	else{
		int t_end1 = vmsg->attrib_ref[2]*2;
		int t_end2 = vmsg->attrib_ref[3]*2;
		
                if(vmsg->attrib_ref[0] == 0){
        	        if(vmsg->attrib_ref[1] < t_end2) vmsg->attrib_ref[1] += increment;
                        else vmsg->attrib_ref[0] = 1;
                        }
                else if(vmsg->attrib_ref[0] == 1){
                       if(vmsg->attrib_ref[1] > t_end1) vmsg->attrib_ref[1] -= increment;
                       else vmsg->attrib_ref[0] = 2;
                       }
                else if(vmsg->attrib_ref[0] == 2){
                       if(vmsg->attrib_ref[1] < 0) vmsg->attrib_ref[1] += increment;
                       else vmsg->active = 2;
                       }
                }
}

//do the message floating routine
void vfont_message_float_logic(V_MESSAGE * vmsg, char method, char increment){
	if(!vmsg->attrib_ref[0]){
        	if(vmsg->attrib_ref[1] < vmsg->attrib_ref[3]) vmsg->attrib_ref[1] += increment;
                else vmsg->attrib_ref[0] = 1;
                }
	else{
                if(vmsg->attrib_ref[1] > vmsg->attrib_ref[2]) vmsg->attrib_ref[1] -= increment;
                else vmsg->attrib_ref[0] = 0;
                }
}
	
//do the message moving routine
void vfont_message_move_logic(FONT * vf, V_MESSAGE * vmsg, char method, char increment, int dir, float scale)
{
	if(dir < 0)
	{  //moves in from right
		if(vmsg->attrib_ref[3] == 0)
		{
			if(vmsg->s.x > vmsg->attrib_ref[2])
			{
				vmsg->v.x = vmsg->attrib_ref[0];
			}
			else
			{
				vmsg->attrib_ref[3] = 1;
				vmsg->v.x = 0;
			}
		}
		else if(vmsg->attrib_ref[3] == 1)
		{
			if(vmsg->s.x > -text_length(vf, vmsg->txt) * scale)
			{
				vmsg->v.x = vmsg->attrib_ref[0];
			}
			else
			{
				vmsg->attrib_ref[3] = 3;
				vmsg->v.x = 0;
			}
		}
		else if(vmsg->attrib_ref[3] == 3)
		{
			vmsg->active = 2;
		}
	}
	else{  //moves in from left
		if(vmsg->attrib_ref[3] == 0){
                	if(vmsg->s.x < vmsg->attrib_ref[2]) vmsg->v.x = vmsg->attrib_ref[0];
                        else{
                                vmsg->attrib_ref[3] = 1;
                                vmsg->v.x = 0;
                                }
                	}
		else if(vmsg->attrib_ref[3] == 1){
                        if(vmsg->s.x < vmsg->length*scale) vmsg->v.x = vmsg->attrib_ref[0];
                        else{
                                vmsg->attrib_ref[3] = 3;
                                vmsg->v.x = 0;
                                }
                        }
                else if(vmsg->attrib_ref[3] == 3){
                       vmsg->active = 2;
                       }
        	}
	
}

//do the rotating message routine
void vfont_message_rotate_logic(V_MESSAGE * vmsg, char method, char increment){
	if(vmsg->timer_end >= 0){
        	if(vmsg->attrib_ref[0] == 1){
                	if(vmsg->attrib_ref[1] > vmsg->attrib_ref[2]) vmsg->attrib_ref[1] -= increment;
                        else vmsg->attrib_ref[0] = 0;
                        }
                else{
                        if(vmsg->attrib_ref[1] < vmsg->attrib_ref[3]) vmsg->attrib_ref[1] += increment;
                        else vmsg->attrib_ref[0] = 1;
                        }
                if(vmsg->timer < vmsg->timer_end) vmsg->timer += increment;
                else{
                        vmsg->timer = 0;
                        vmsg->active = 2;
                        }
                }
	else{
                if(vmsg->attrib_ref[0] == 0){
        	        if(vmsg->attrib_ref[1] < vmsg->attrib_ref[3]) vmsg->attrib_ref[1] += increment;
                        else vmsg->attrib_ref[0] = 1;
                        }
                else if(vmsg->attrib_ref[0] == 1){
                       if(vmsg->attrib_ref[1] > vmsg->attrib_ref[2]) vmsg->attrib_ref[1] -= increment;
                       else vmsg->attrib_ref[0] = 2;
                       }
                else if(vmsg->attrib_ref[0] == 2){
                       if(vmsg->attrib_ref[1] < 0) vmsg->attrib_ref[1] += increment;
                       else vmsg->active = 2;
                       }
                }
}

//handle the logic for this message
void vfont_message_logic(FONT * vf, V_MESSAGE * vmsg, char method, char increment){
        switch(method){
                case VMSG_TEXT_PLAIN:
                        break;

                case VMSG_TEXT_FLOAT:
			vfont_message_float_logic(vmsg, method, increment);
                        break;

                case VMSG_TEXT_FLOAT_CENTERED:
			vfont_message_float_logic(vmsg, method, increment);
                        break;

                case VMSG_TEXT_SCALE_ALL:
			vfont_message_scale_logic(vmsg, method, increment);
                        break;

		case VMSG_TEXT_SCALE_ALL_CENTERED:
			vfont_message_scale_logic(vmsg, method, increment);
                        break;
                        
                case VMSG_TEXT_ENTER_FROMRIGHT:
			vfont_message_move_logic(vf, vmsg, method, increment, -1, 1);
                        break;

                case VMSG_TEXT_ENTER_FROMRIGHT_SCALED:
			vfont_message_move_logic(vf, vmsg, method, increment, -1, vmsg->attrib_ref[1]);
                        break;

                case VMSG_TEXT_ROTATE_ALL:
			vfont_message_rotate_logic(vmsg, method, increment);
                        break;

                default:
                        break;

                }

        vmsg->s.x += vmsg->v.x;
        vmsg->s.y += vmsg->v.y;
}

//write a message to the screen using the vector font
//active = 0 means message is inactive
//active = 1 means message is active
//active = 2 means message is finished
void vfont_message_textout(BITMAP * bp, FONT * vf, V_MESSAGE * vmsg, char method, char increment)
{
	int i;
	float scale;
	char letter[2] = {0, 0};
	int px;
	int c = makecol(0, 192, 0), bc = makecol(0, 0, 0);
        
	switch(method)
	{
		case VMSG_TEXT_PLAIN:
		
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char(bp, vf, vmsg->txt[i], vmsg->s.x+i*text_length(vf, letter), vmsg->s.y, c, bc);
			}
			break;

		case VMSG_TEXT_FLOAT:
		
			px = vmsg->s.x;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char(bp, vf, vmsg->txt[i], px, vmsg->s.y+(float)vmsg->attrib_ref[1]/text_height(vf), bc, bc);
				px += text_length(vf, letter);
			}
			px = vmsg->s.x;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char(bp, vf, vmsg->txt[i], px + 2, vmsg->s.y+(float)vmsg->attrib_ref[1]/text_height(vf) - 2, c, bc);
				px += text_length(vf, letter);
			}
			break;

		case VMSG_TEXT_FLOAT_CENTERED:
		
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char(bp, vf, vmsg->txt[i], vmsg->s.x-strlen(vmsg->txt)*text_length(vf, letter)/2+i*text_length(vf, letter), vmsg->s.y+(float)vmsg->attrib_ref[1]/text_height(vf), c, bc);
			}
			break;

		case VMSG_TEXT_SCALE_ALL:
		
			px = 0;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				scale = (float)(vmsg->attrib_ref[1])/text_length(vf, letter);
				draw_av_char_scaled(bp, vf, vmsg->txt[i], px, vmsg->s.y, c, bc, scale);
				px += text_length(vf, letter) * scale;
			}
			break;

		case VMSG_TEXT_SCALE_ALL_CENTERED:
		
			scale = (float)(vmsg->attrib_ref[1]) / 10.0;
			px = vmsg->s.x - (scale * text_length(vf, vmsg->txt)) / 2.0;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char_scaled(bp, vf, vmsg->txt[i], px, vmsg->s.y, c, bc, scale);
				px += text_length(vf, letter) * scale;
			}
			break;

		case VMSG_TEXT_ENTER_FROMRIGHT:
			
			px = vmsg->s.x;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char(bp, vf, vmsg->txt[i], px, vmsg->s.y, bc, bc);
				px += text_length(vf, letter);
			}
			px = vmsg->s.x;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char(bp, vf, vmsg->txt[i], px + 2, vmsg->s.y - 2, c, bc);
				px += text_length(vf, letter);
			}
			break;

		case VMSG_TEXT_ENTER_FROMRIGHT_SCALED:
		
			px = vmsg->s.x;
			scale = (float)(vmsg->attrib_ref[1]);
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char_scaled(bp, vf, vmsg->txt[i], px, vmsg->s.y, bc, bc, scale);
				px += text_length(vf, letter) * scale;
			}
			px = vmsg->s.x;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char_scaled(bp, vf, vmsg->txt[i], px + 4, vmsg->s.y - 4, c, bc, scale);
				px += text_length(vf, letter) * scale;
			}
			break;

		case VMSG_TEXT_ROTATE_ALL:
		
			px = vmsg->s.x;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char_rotated(bp, vf, vmsg->txt[i], px, vmsg->s.y, bc, bc, vmsg->attrib_ref[1], 1);
				px += text_length(vf, letter);
			}
			px = vmsg->s.x;
			for(i = 0; i < strlen(vmsg->txt); i++)
			{
				letter[0] = vmsg->txt[i];
				draw_av_char_rotated(bp, vf, vmsg->txt[i], px + 2, vmsg->s.y - 2, c, bc, vmsg->attrib_ref[1], 1);
				px += text_length(vf, letter);
			}
			break;

		default:
		
			break;

	}

	vmsg->s.x += vmsg->v.x;
	vmsg->s.y += vmsg->v.y;
}

//set a vmessage
void set_vfont_message(V_MESSAGE * vmsg, FONT * vf, char * txt, int x, int y, int z, char method, int m_ref1, int m_ref2, int m_ref3, int m_ref4, int t_end){

        vmsg->length = text_length(vf, txt);
//        vmsg->length = 32;
        vmsg->method = method;
        vmsg->attrib_ref[0] = m_ref1;
        vmsg->attrib_ref[1] = m_ref2;
        vmsg->attrib_ref[2] = m_ref3;
        vmsg->attrib_ref[3] = m_ref4;
        vmsg->timer = 0;
        vmsg->timer_end = t_end;
        vmsg->active = -1;

        strcpy(vmsg->txt, txt);

        if(method == VMSG_TEXT_ENTER_FROMLEFT || method == VMSG_TEXT_ENTER_FROMLEFT_SCALED){
                vmsg->v.x = m_ref1;
                vmsg->v.y = 0;
                vmsg->s.x = -vmsg->length * vmsg->attrib_ref[0];
                vmsg->s.y = y;
                vmsg->attrib_ref[2] = x;
                vmsg->attrib_ref[3] = 0;
                }
        else if(method == VMSG_TEXT_ENTER_FROMRIGHT || method == VMSG_TEXT_ENTER_FROMRIGHT_SCALED){
                vmsg->v.x = m_ref1;
                vmsg->v.y = 0;
                vmsg->s.x = 640;
                vmsg->s.y = y;
                vmsg->attrib_ref[2] = x;
                vmsg->attrib_ref[3] = 0;
                }
        else if(method == VMSG_TEXT_ENTER_FROMTOP || method == VMSG_TEXT_ENTER_FROMTOP_SCALED){
                vmsg->v.x = 0;
                vmsg->v.y = m_ref1;
                vmsg->s.x = x;
                vmsg->s.y = -text_height(vf);
                vmsg->attrib_ref[2] = y;
                vmsg->attrib_ref[3] = 0;
                }
        else if(method == VMSG_TEXT_ENTER_FROMBOTTOM || method == VMSG_TEXT_ENTER_FROMBOTTOM_SCALED){
                vmsg->v.x = 0;
                vmsg->v.y = m_ref1;
                vmsg->s.x = x;
                vmsg->s.y = 480;
                vmsg->attrib_ref[2] = y;
                vmsg->attrib_ref[3] = 0;
                }
        else{
                vmsg->s.x = x;
                vmsg->s.y = y;
                vmsg->v.x = vmsg->v.y = 0;
                }
}

//add a vmessage to the queue
void add_vfont_queue_message(V_MESSAGE vmsg[], int num_vmessages, FONT * vf, int * cur_slot, char * txt, int x, int y, int z, char method, int m_ref1, int m_ref2, int m_ref3, int m_ref4, int t_end)
{
	set_vfont_message(&vmsg[*cur_slot], vf, txt, x, y, z, method, m_ref1, m_ref2, m_ref3, m_ref4, t_end);
	if(++(*cur_slot) >= num_vmessages) *cur_slot = 0;
}

//deactivate a v_message
void deactivate_vfont_message(V_MESSAGE * vmsg)
{
	vmsg->active = 2;
}
