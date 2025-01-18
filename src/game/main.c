#include <allegro.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include "../modules/t3net/t3net.h"
#include "../modules/gametime.h"
#include "../modules/ncds.h"
#ifdef ALLEGRO_WINDOWS
	#include "../modules/g-idle.h"
#endif
#include "../modules/fx.h"
#include "hyperlink.h"
#include "structs.h"
#include "defines.h"
#include "body.h"
#include "poly.h"
#include "hole.h"
#include "vfont.h"
#include "newmenu.h"
#include "main.h"
#include "profile.h"
#include "message.h"
#include "credits.h"
#include "avatar.h"

#define MAX_POLY_VERTICES 128
#define MAX_POLY_EDGES 32
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

/* hyperlink pages to supplement menus */
HYPERLINK_PAGE * player_page[4] = {NULL};
HYPERLINK_PAGE * player_start_page[4] = {NULL};
HYPERLINK_PAGE * level_select_page = NULL;
HYPERLINK_PAGE * player_page_pointer[4] = {NULL};
HYPERLINK_PAGE * player_global_page = NULL;
HYPERLINK_PAGE * player_avatar_page = NULL;
HYPERLINK_PAGE * player_account_page = NULL;
HYPERLINK_PAGE * player_new_account_page = NULL;
HYPERLINK_PAGE * profile_page = NULL;
HYPERLINK_PAGE * leaderboard_page = NULL;
HYPERLINK_PAGE * profile_global_page = NULL;
HYPERLINK_PAGE * leaderboard_global_page = NULL;
HYPERLINK_PAGE * game_complete_page = NULL;
HYPERLINK_PAGE * sub_menu_page = NULL;
HYPERLINK_PAGE * scoreboard_page = NULL;
int              vgolf_player_choosing = 0;
int              player_account_selected_page[4] = {0};
int              vgolf_avatar_selected_page = 0;
char             profile_text[16][256] = {{0}};
char             temp_text[256] = {0};
int              show_records = 0;
int              profile_show_details = 0;
int              last_commentary = -1;
int              vgolf_color_black;
int              vgolf_color_white;
int              vgolf_color_box;
int              vgolf_color_box_outline;
char             mouse_unused = 0;
char             mouse_hidden = 0;
int              vgolf_mouse_x, vgolf_mouse_y, vgolf_mouse_z, vgolf_mouse_start_z, vgolf_mouse_b, vgolf_old_mouse_x, vgolf_old_mouse_y = 0;

HYPERLINK_PAGE * course_page = NULL;
HYPERLINK_PAGE * course_select_page = NULL;
char             player_page_text_buffer[4][256];
char             course_page_name[256] = {0};
char             course_page_par[256] = {0};
char             course_page_holes[256] = {0};

/* [[new stuff */
#define BALL_COLOR makecol(255, 255, 255)
#define BALL_SHADOW_COLOR makecol(100, 100, 100)
#define DEFAULT_CLUB_COLOR makecol(255, 2, 0)
#define CLUB_FILL_COLOR makecol(255, 255, 255)
#define CLUB_LINE_COLOR makecol(170, 170, 170)
/* new stuff]] */

int vgolf_player_count;
int vgolf_player_list[4] = {0};
int debug_mode = 0;
int vgolf_start_hole = 0;
int next_hole;
int block_click = 0;


/* new stuff */
BITMAP * scratch_pad = NULL;
int vgolf_frames;
int vgolf_ticks = 0;
int vgolf_state = 0;
NEWMENU * nmenu[256] = {NULL};
BITMAP * buffer;
BITMAP * thanks_pic;
BITMAP * scorecard_pic = NULL;
int done = 0;
TITLE_LETTER letter[5];
//PALETTE mypalette;
PALETTE white_palette;
PALETTE blue_palette;
FONT * gfont;
FONT * gfont2;
FONT * gfont3;
FONT * gfont4;
int players = 2;
VGOLF_COURSE_DATABASE vgolf_courses;
VGOLF_AVATAR_DATABASE vgolf_avatars;
VGOLF_COMMENTARY_DATABASE vgolf_commentaries;
PROFILE_LIST vgolf_profiles;
COLOR_MAP shadow_map;
COLOR_MAP fade_map;
int bfade_level;
int bfade_target;
int bfade_speed;
int oldmvol;
int cvoice = -1;
SAMPLE * csample = NULL;
int cvtime = 0;
int profile_course = 0;
int selected_profile = 0;

BITMAP * credit_image[8] = {NULL};
BITMAP * logo_image = NULL;

int network_play = 0;

/* old stuff */
int scount;
PLAYER game_player[MAX_PLAYERS];
MAIN_GAME_VARS main_game_vars;
VGOLF_COURSE * game_course = NULL;
PALETTE game_palette;
COLOR_MAP trans_table;
COLOR_MAP shadow_table;
V_FONT game_vfont;
V_MESSAGE game_vmessage[GAME_VMESSAGE_QUEUE_SIZE];
PLAYER * current_player = NULL;
HOLE * current_hole = NULL;

int vgolf_config[128] = {0};
char vgolf_nick[128] = {0};
char old_vgolf_nick[128] = {0};

char fname_buffer[1024] = {0};

//debugging stuff (remove later)
int v_tag = 0;
int vl_tag = 0;
int orth_tag = 0;
int r_tag = 0;
VERTEX vert_tag;
volatile int game_speed_counter = 0;
char debug_str[64];

char entered_text[1024] = {0};
char old_entered_text[256] = {0};
int entered_text_max = 255;
int entering_text = 0;
int entered_text_pos;
int selected_course = 0;
NCDMSG_QUEUE vgolf_messages;
CREDITS vgolf_credits;

float lc_cx, lc_cy, lc_cr;
int lc_collided;
char text_buffer[256];

unsigned long left_click = 0;
unsigned long right_click = 0;

T3NET_LEADERBOARD * leaderboard = NULL;
int leaderboard_mode = 0;
char leaderboard_menu_text[32] = {0};

int player_course_stroke_total(PLAYER pl, int cur_hole);

void point_in_circle(BITMAP * bmp, int x, int y, int d)
{
	if(!lc_collided)
	{
		if(distance(lc_cx, lc_cy, x, y) < lc_cr)
		{
			lc_collided = 1;
		}
	}
}

int line_circle_collide(float cx, float cy, float cr, float x1, float y1, float x2, float y2)
{
	float line_length;
	
	/* first see if the circle is close enough to the line for a possible collision */
	line_length = distance(x1, y1, x2, y2);
	if(distance(cx, cy, x1, y1) > line_length || distance(cx, cy, x2, y2) > line_length)
	{
		return 0;
	}
	
	lc_cx = cx;
	lc_cy = cy;
	lc_cr = cr;
	lc_collided = 0;
	do_line(NULL, x1, y1, x2, y2, 0, point_in_circle);
	return lc_collided;
}

void myblend(PALETTE pal, int x, int y, RGB *rgb)
{
	int light;
	
	light = (pal[x].r + pal[x].g + pal[x].b) / 3;
	rgb->r = fixtoi(fmul(itofix(pal[y].r), fdiv(itofix(light), itofix(63))));
	rgb->g = fixtoi(fmul(itofix(pal[y].g), fdiv(itofix(light), itofix(63))));
	rgb->b = fixtoi(fmul(itofix(pal[y].b), fdiv(itofix(light), itofix(63))));
}

void vgolf_talk(char * dp)
{
	ncdmsg_add(&vgolf_messages, dp, makecol(255, 255, 255), 300);
}

/* [[new stuff (testing stuff..)*/
const char * getCurVertFlagString()
{
	if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_TEE) return "TEE";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_HOLE) return "CUP";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_TREE_0) return "TREE_0";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_TREE_1) return "TREE_1";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_TREE_2) return "TREE_2";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_TREE_3) return "TREE_3";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_ANIMAL_0) return "ANIMAL_0";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_ANIMAL_1) return "ANIMAL_1";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_ANIMAL_2) return "ANIMAL_2";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_HAZARD_WATER) return "HAZARD_WATER";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_HAZARD_SAND) return "HAZARD_SAND";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_HAZARD_ROUGH) return "HAZARD_ROUGH";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_HAZARD_SLIME) return "HAZARD_SLIME";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_PIPE_ENTER) return "PIPE_ENTER";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_PIPE_EXIT) return "PIPE_EXIT";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_INDENT_HILL) return "HILL";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_INDENT_RUT) return "RUT";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_POLY_MORPH) return "POLY_MORPH";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_POLY_MOVE_HORIZ) return "POLY_MOVE_HORIZ";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_POLY_MOVE_VERT) return "POLY_MOVE_VERT";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_POLY_MOTION_AREA) return "POLY_MOTION_AREA";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_CLOUD_VERT) return "CLOUD_VERT";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_CLOUD_HORIZ) return "CLOUD_HORIZ";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_COIN_0) return "COIN_0";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_LASER_SEND) return "LASER_SEND";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_LASER_RECV) return "LASER_RECV";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_LASER_RESPAWN) return "LASER_RESPAWN";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_TELEPORT) return "TELEPORT";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_CONVEYOR) return "CONVEYOR";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_BLINKER) return "BLINKER";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_POLY_DOOR_HORIZ) return "DOOR_HORIZ";
	else if(game_course->current_hole.vert[game_player[main_game_vars.cur_player].ball.cur_vert].flag == VERT_FLAG_POLY_DOOR_VERT) return "DOOR_VERT";
	return "NULL";	
}

const char * getCurBallModeString()
{
	if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_IDLE) return "BALL_MODE_IDLE";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_HIT) return "BALL_MODE_HIT";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_SUNK) return "BALL_MODE_SUNK";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_WATER_SINKING) return "BALL_MODE_WATER_SINKING";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_WATER_SUNK) return "BALL_MODE_WATER_SUNK";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_PIPE_ENTER) return "BALL_MODE_PIPE_ENTER";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_PIPE_EXIT) return "BALL_MODE_PIPE_EXIT";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_ON_INDENT) return "BALL_MODE_ON_INDENT";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_IN_POLYMORPH) return "BALL_MODE_IN_POLYMORPH";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_DROP) return "BALL_MODE_DROP";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_IN_MOTION_AREA) return "BALL_MODE_IN_MOTION_AREA";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_COIN_FINISHED) return "BALL_MODE_COIN_FINISHED";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_LASER_ZAPPING) return "BALL_MODE_LASER_ZAPPING";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_LASER_ZAPPED) return "BALL_MODE_LASER_ZAPPED";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_ON_CONVEYOR) return "BALL_MODE_ON_CONVEYOR";
	else if(game_player[main_game_vars.cur_player].ball.mode == BALL_MODE_DOOR_SMASHED) return "BALL_MODE_DOOR_SMASHED";
	return "NULL";
}
/* new stuff]] */

void text_enter_logic(void)
{
	char k;
	
	if(entering_text)
	{
		if(keypressed())
		{
			k = readkey() & 0xFF;
			switch(k)
			{
				/* enter */
				case 0xD:
				{
	            	entered_text[entered_text_pos] = '\0';
					key[KEY_ENTER] = 0;
					if(entering_text == 1)
					{
						entering_text = 0;
					}
					else if(entering_text == 2)
					{
						strcpy(vgolf_nick, entered_text);
						newmenu_enable_controls();
						entering_text = 0;
					}
					else if(entering_text == 3)
					{
						if(strlen(entered_text) > 0)
						{
							hl_player_add_account(0, NULL);
							entering_text = 0;
						}
					}
					break;
				}
		
				/* escape */
				case 0x1B:
				{
					if(entering_text == 1)
					{
						strcpy(entered_text, old_entered_text);
						key[KEY_ESC] = 0;
						entering_text = 0;
					}
					else if(entering_text == 2)
					{
						newmenu_enable_controls();
						key[KEY_ESC] = 0;
						entering_text = 0;
					}
					else if(entering_text == 3)
					{
						entering_text = 0;
					}
					break;
				}	
		
				/* backspace */
				#ifndef ALLEGRO_MACOSX
				case 0x8:
				#else
				case 127:
				#endif
				{
	       			entered_text[entered_text_pos] = '\0';
					if(entered_text_pos > 0)
					{	
	           			entered_text_pos--;
   						entered_text[entered_text_pos] = '\0';
					}
					break;
				}
				
				/* normal key */
				default:
				{
					if(entered_text_pos < entered_text_max - 1)
					{
	       				entered_text[entered_text_pos] = k;
						entered_text_pos++;
						entered_text[entered_text_pos] = '\0';
					}
					break;
				}
			}
		}
	}
}

void do_ball_laser_zap(BALL * ball, PLAYER * pl, HOLE * hole);


//play a random sound when the ball hits a wall
void play_ball_wall_hit_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_ball_collision[random()%MAX_SFX_COLLISION_SAMPLES], 128, 1000, 0);
}


//plays the crowd's "ahh" sound
void play_ahhh_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_hole_ahhh[random()%MAX_SFX_HOLE_AHHH_SAMPLES], 128, 1000, 0);
}

//plays a "bad job" sound effect
void play_poor_finish_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_hole_finish[0][random()%MAX_SFX_HOLE_FINISH_SAMPLES], 128, 1000, 0);
}                
                
//plays a "good job" sound effect
void play_good_finish_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_hole_finish[1][random()%MAX_SFX_HOLE_FINISH_SAMPLES], 128, 1000, 0);
}
                        
//play a random "hole-in-one" sample
void play_hole_in_one_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_hole_finish[2][random()%MAX_SFX_HOLE_FINISH_SAMPLES], 128, 1000, 0);
}
                        
//play the "ball hitting cup" sample
void play_ball_holesink_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_ball_holesink, 128, 1000, 0);
}                
                
//play the teleport sample
void play_ball_teleport_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_ball_teleport, 128, 1000, 0);
}

//play the "splash" sound
void play_ball_splash_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_ball_splash, 128, 1000, 0);
}

//plays the "zap" sample
void play_ball_zap_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_ball_zapped, 128, 1000, 0);
}

/* [[new stuff */
//plays the "smash" sample
void play_ball_smash_sample(void)
{
}
/* new stuff]] */

//plays the club hitting ball sample
void play_club_ball_hit_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_swing[random() % MAX_SFX_SWING_SAMPLES], 128, 1000, 0);
}

//plays the club rotation sample
void play_club_rotate_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_ball_rotate, 128, 1000, 0);
}

//plays the sample associated with pressing enter and selecting a menu item
void play_select_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_menu_select, 128, 1000, 0);
}

//plays the sample associated with hitting the esc key
void play_esc_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_menu_escape, 128, 1000, 0);
}

//plays the sample associated with a menu slider changing value
void play_slider_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_menu_slider, 128, 1000, 0);
}

//plays the sample associated with changing the current menu item selection
void play_item_select_sample(void)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_menu_itemsel, 128, 1000, 0);
}

//add the hole-in-one message to the queue
void add_hole_in_one_message(void)
{
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Hole in one!", SCREEN_W, SCREEN_H / 2 - text_height(gfont) * 2, 0, VMSG_TEXT_ENTER_FROMRIGHT_SCALED, -20, 4, 0, 0, -1);
}

//add the messages to the queue when a player gets albatross
void add_albatross_finish_messages(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_EXCELLENT].count;

	//add a random message to spice things up	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_EXCELLENT].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);
	
	//add the score message
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Albatross!!", SCREEN_W / 2 - text_length(gfont, "Albatross!!") / 2, SCREEN_H / 2 - text_height(gfont) / 2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add the messages to the queue when a player gets eagle
void add_eagle_finish_messages(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_GREAT].count;

	//add a random message to spice things up	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_GREAT].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);

	//add the score message
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Eagle!!", SCREEN_W / 2 - text_length(gfont, "Eagle!!") / 2, SCREEN_H / 2 - text_height(gfont) / 2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}


//add the messages to the queue when a player gets birdie
void add_birdie_finish_messages(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_BETTER].count;

	//add a random message to spice things up	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_BETTER].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 60;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);

	//add the score message
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Birdie!", SCREEN_W / 2 - text_length(gfont, "Birdie!") / 2, SCREEN_H / 2 - text_height(gfont) / 2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add the messages to the queue when a player gets par
void add_par_finish_messages(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_GOOD].count;

	//add a random message to spice things up	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_GOOD].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);
	
	//add the score message
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Par", SCREEN_W / 2 - text_length(gfont, "Par") / 2, SCREEN_H / 2 - text_height(gfont) / 2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add the messages to the queue when a player gets bogey
void add_bogey_finish_messages(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_POOR].count;

	//add a random message to spice things up	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_POOR].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);

	//add the score message
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Bogey", SCREEN_W / 2 - text_length(gfont, "Bogey") / 2, SCREEN_H / 2 - text_height(gfont) / 2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add the messages to the queue when a player scores worse than bogey
void add_under_bogey_finish_messages(int diff)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_VERYPOOR].count;
	char textbuffer[256];

	//add a random message to spice things up	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_FINISH_VERYPOOR].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);

    //add score message
	if(diff == -2)
	{       //double bogey..
		add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Double bogey", SCREEN_W / 2 - text_length(gfont, "Double bogey") / 2, SCREEN_H / 2 - text_height(gfont) / 2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
	}
	else if(diff == -3)
	{  //triple bogey..
		add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Triple bogey", SCREEN_W / 2 - text_length(gfont, "Triple bogey") / 2, SCREEN_H / 2 - text_height(gfont) / 2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
	}
	//X-bogey
	else
	{
		sprintf(textbuffer, "+%d", -diff);
		add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, textbuffer, SCREEN_W / 2 - text_length(gfont, textbuffer) / 2, SCREEN_H / 2 - text_height(gfont) / 2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
	}
}

//add a "so close..." message to the queue
void add_so_close_message(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_CLOSE].count;
                                
	//add the message to the queue..	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_CLOSE].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);
}

//add a "zapped" message to the queue
void add_ball_zap_message(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_ZAPPED].count;
                                
	//add the message to the queue..	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_ZAPPED].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);
}

/* [[new stuff */
//add a "smashed" message to the queue
void add_ball_smash_message(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_CRUSHED].count;
                                
	//add the message to the queue.. 
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_CRUSHED].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);
}
/* new stuff]] */

//add the messages related to the ball water sink to the queue
void add_ball_water_sunk_messages(void)
{
	int rand_no = random()%main_game_vars.commentary->comments[MESSAGE_TYPE_WATER].count;
                                
	//add the comment message to the queue..	
	csample = main_game_vars.commentary->comments[MESSAGE_TYPE_WATER].comment[rand_no].voice;
	cvoice = -1;
	cvtime = 40;
	if(ncds_music_volume == vgolf_config[CONFIG_MUSIC_VOLUME])
	{
		oldmvol = ncds_music_volume;
	}
	ncds_set_music_volume(ncds_music_volume / 4);
}

//add the quit prompt message to the queue
void add_quit_prompt_message(void)
{
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, "Quit? [Y / N]", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_FLOAT_CENTERED, 0, 0, -32, 32, -1);
}

//draw this polygon to the bitmap centered at this point
void draw_polygon(BITMAP * bp, POLY po, int cx, int cy)
{
	int i;

	for(i = 0; i < po.num_vertices - 1; i++)
	{
		line(bp, cx + po.p[i].x, cy + po.p[i].y, cx + po.p[i+1].x, cy + po.p[i + 1].y, 1);
	}
}

//try and free a bitmap from memory..
int trash_bitmap(BITMAP * bp)
{
	if(bp != NULL)
	{
		destroy_bitmap(bp);
	}
	return -1;
}

//draw this animation def
void draw_anim_def(BITMAP * bp, ANIM_DEF * an, int x, int y)
{
	masked_blit(an->pic[an->cur_frame / an->speed], bp, 0, 0, x, y, an->pic[an->cur_frame / an->speed]->w, an->pic[an->cur_frame / an->speed]->h);
}

//handle the animation def
int do_anim_def_logic(ANIM_DEF * an, int x, int y, char loop)
{
	if(an->cur_frame < an->num_frames * an->speed) an->cur_frame++;  //increment the current frame
	else
	{
		if(loop) an->cur_frame = 0;     //if we're looping, start back at 0
	}
	return an->cur_frame;
}

//handle the animation def
int do_anim_def(BITMAP * bp, ANIM_DEF * an, int x, int y, char loop)
{
	draw_anim_def(bp, an, x, y);   //draw the current frame to the bitmap
	return 1;
}

//returns the filename to the datfile so we can load the file.. (ie. "vgolf.dat#blah.pcx")
char * base_filename(char * filename)
{
	strcpy(fname_buffer, "vgolf.dat#");
	strcat(fname_buffer, filename);
	
	return fname_buffer;
}

//gives the hole filename for use in the file loading routine from the datfile
void hole_fname(const char * str, char * str_b, int num)
{
	sprintf(str_b, "%s%s%s%i%i_hol", str, ".crs#", str, num / 10, num % 10);
}

//draw a transparent sprite, testing to see if it's null first..
int draw_trans_sprite_check(BITMAP * bp, BITMAP * sp, int x, int y){
        if(sp != NULL){
                draw_trans_sprite(bp, sp, x, y);
                return 0;
                }
        return -1;
}

//total strokes by player in course so far..
int player_course_stroke_total(PLAYER pl, int cur_hole)
{
	int c, sum = 0;

	//make sure we don't go over the # of holes
	//cur_hole = (cur_hole < num_holes ? cur_hole : num_holes-1);
	for(c = 0; c < cur_hole; c++)
	{
		sum += pl.scorecard.score[c];
	}
	return sum;
}

//total strokes by player in course so far..
int course_stroke_total(PLAYER pl, int cur_hole, int num_holes)
{
	int c, sum = 0;

	for(c = 0; c < cur_hole; c++)
	{
		sum += pl.scorecard.par_score[c];
	}
	return sum;
}

//total strokes by player in course so far..
int record_course_stroke_total(PLAYER pl, int cur_hole, int num_holes)
{
	int c, sum = 0;

	//make sure we don't go over the # of holes
	//cur_hole = (cur_hole < num_holes ? cur_hole : num_holes-1);
	for(c = 0; c < cur_hole; c++)
	{
		sum += vgolf_profiles.item[pl.profile].course_info[selected_course].score[c];
	}
	return sum;
}

//total strokes on final scorecard
int scorecard_stroke_total(SCORECARD * sc, int num_holes)
{
	int c, sum = 0;

	for(c = 0; c < num_holes; c++)
	{
		sum += sc->score[c];
	}
	return sum;
}

//total par on final scorecard
int scorecard_par_total(SCORECARD * sc, int num_holes)
{
	int c, sum = 0;

	for(c = 0; c < num_holes; c++)
	{
		sum += sc->par_score[c];
	}
	return sum;
}


//a wrapper to add a message to the game message queue
void add_game_message(char * txt, int x, int y)
{
}

//set the ball polygon (just an arbitrary number of points)
void set_ball_poly(BALL * ball, float rad)
{
	int i;

	ball->body.mass = 100.0f;
	ball->body.vel = new_vertex(0, 0);
	ball->body.pos = new_vertex(0, 0);
	ball->body.inv_mass = (ball->body.mass > 0.0000001f) ? 1.0f / ball->body.mass : 0.0f;
	ball->body.pos.x = 400;
	ball->body.pos.y = 400;
	ball->body.poly.num_vertices = 32;
	ball->body.moveable = 1;

	ball->radius = rad;
	for(i = 0; i < 32; i++)
	{
		ball->body.poly.p[i].x = ball->radius*cos(i*M_PI/16.0f);
		ball->body.poly.p[i].y = ball->radius*sin(i*M_PI/16.0f);
	}
}

void body_move(BODY * bo)
{
	if(!bo->moveable)
	{
		bo->vel.x = bo->vel.y = 0;
		return;
	}
	bo->pos.x += bo->vel.x;
	bo->pos.y += bo->vel.y;
}

//function to keep the in-game running consistently on different machines..
void increment_ingame_timer(void)
{
	main_game_vars.ingame_timer++;
}END_OF_FUNCTION(increment_ingame_timer);

//initialize the framerate timer
void initialize_ingame_timer(void)
{
	main_game_vars.ingame_timer = 0;
	LOCK_VARIABLE(main_game_vars.ingame_timer);
	LOCK_FUNCTION(increment_ingame_timer);

	install_int_ex(increment_ingame_timer, BPS_TO_TIMER(60));
}

//save configuration to file
int save_config(char * filename)
{
	PACKFILE * out_file;
	
	out_file = pack_fopen(filename, "w");
	
	//write header
	pack_putc('C', out_file);
	pack_putc('F', out_file);
	pack_putc('G', out_file);
	pack_putc(14, out_file);
	
	pack_iputl(vgolf_config[CONFIG_GFX_MODE], out_file);
	pack_iputl(vgolf_config[CONFIG_FX_ON], out_file);
	pack_iputl(vgolf_config[CONFIG_RESET_HUDS], out_file);
	pack_iputl(vgolf_config[CONFIG_SOUND_ON], out_file);
	pack_iputl(vgolf_config[CONFIG_MUSIC_ON], out_file);
	pack_iputl(vgolf_config[CONFIG_SFX_VOLUME], out_file);
	pack_iputl(vgolf_config[CONFIG_MUSIC_VOLUME], out_file);
	pack_iputl(vgolf_config[CONFIG_VSYNC], out_file);
	pack_iputl(vgolf_config[CONFIG_NET_MODE], out_file);
	pack_iputl(vgolf_config[CONFIG_COMMENTARY], out_file);
	pack_iputl(vgolf_config[CONFIG_COMMENTARY_VOLUME], out_file);
	pack_fwrite(vgolf_nick, 128, out_file);
	pack_iputl(vgolf_config[CONFIG_DEPTH], out_file);
	pack_iputl(vgolf_config[CONFIG_UPLOAD], out_file);
	pack_iputl(vgolf_config[CONFIG_FINE_TUNE], out_file);
	pack_iputl(vgolf_config[CONFIG_MIX], out_file);
	
	pack_fclose(out_file);
	
	return 1;
}

//load saved configuration
int load_config(char * filename)
{
	char temp_char;
	PACKFILE * in_file;
	
	in_file = pack_fopen(filename, "r");
	if(in_file == NULL) return 0;
	
	//read header
	temp_char = pack_getc(in_file);
	if(temp_char != 'C') return 0;
	temp_char = pack_getc(in_file);
	if(temp_char != 'F') return 0;
	temp_char = pack_getc(in_file);
	if(temp_char != 'G') return 0;
	temp_char = pack_getc(in_file);
	if(temp_char != 14) return 0;
	
	vgolf_config[CONFIG_GFX_MODE] = pack_igetl(in_file);
	vgolf_config[CONFIG_FX_ON] = pack_igetl(in_file);
	vgolf_config[CONFIG_RESET_HUDS] = pack_igetl(in_file);
	vgolf_config[CONFIG_SOUND_ON] = pack_igetl(in_file);
	vgolf_config[CONFIG_MUSIC_ON] = pack_igetl(in_file);
	vgolf_config[CONFIG_SFX_VOLUME] = pack_igetl(in_file);
	vgolf_config[CONFIG_MUSIC_VOLUME] = pack_igetl(in_file);
	vgolf_config[CONFIG_VSYNC] = pack_igetl(in_file);
	vgolf_config[CONFIG_NET_MODE] = pack_igetl(in_file);
	vgolf_config[CONFIG_COMMENTARY] = pack_igetl(in_file);
	vgolf_config[CONFIG_COMMENTARY_VOLUME] = pack_igetl(in_file);
	pack_fread(vgolf_nick, 128, in_file);
	vgolf_config[CONFIG_DEPTH] = pack_igetl(in_file);
	vgolf_config[CONFIG_UPLOAD] = pack_igetl(in_file);
	if(vgolf_config[CONFIG_FINE_TUNE] == 0)
	{
		vgolf_config[CONFIG_FINE_TUNE] = pack_igetl(in_file);
		if(vgolf_config[CONFIG_FINE_TUNE] < 0 || vgolf_config[CONFIG_FINE_TUNE] > 1000)
		{
			vgolf_config[CONFIG_FINE_TUNE] = 0;
		}
		ncds_set_fine_tune(vgolf_config[CONFIG_FINE_TUNE]);
	}
	else
	{
		pack_igetl(in_file);
	}
	vgolf_config[CONFIG_MIX] = pack_igetl(in_file);
	if(vgolf_config[CONFIG_MIX] < 0 || vgolf_config[CONFIG_MIX] > 48000)
	{
		vgolf_config[CONFIG_MIX] = 44100;
	}
	ncds_set_music_volume(vgolf_config[CONFIG_MUSIC_VOLUME]);
	ncds_set_sound_volume(vgolf_config[CONFIG_SFX_VOLUME]);
	
	pack_fclose(in_file);
	
	return 1;
}

//set a default configuration
int set_default_config(void)
{
	int i;
	#ifdef ALLEGRO_MACOSX
		vgolf_config[CONFIG_GFX_MODE] = GFX_AUTODETECT_WINDOWED;
	#else
		vgolf_config[CONFIG_GFX_MODE] = GFX_AUTODETECT;
	#endif
	for(i = 0; i < 32; i++)
	{
		vgolf_config[CONFIG_FX_ON] |= (int)pow(2, i);
	}
	vgolf_config[CONFIG_RESET_HUDS] = 1;
	vgolf_config[CONFIG_SOUND_ON] = 1;
	ncds_enable_effects();
	vgolf_config[CONFIG_MUSIC_ON] = 1;
	ncds_enable_music();
	vgolf_config[CONFIG_SFX_VOLUME] = 50;
	ncds_set_sound_volume(100);
	vgolf_config[CONFIG_MUSIC_VOLUME] = 100;
	ncds_set_music_volume(100);
	vgolf_config[CONFIG_COMMENTARY_VOLUME] = 100;
	vgolf_config[CONFIG_VSYNC] = 1;
	vgolf_config[CONFIG_COMMENTARY] = 0;
	vgolf_config[CONFIG_DEPTH] = 0;
	vgolf_config[CONFIG_UPLOAD] = 1;
	strcpy(vgolf_nick, "Unnamed");

	return 0;
}

void config_toggle_gfx_mode(void)
{
	if(vgolf_config[CONFIG_GFX_MODE] == GFX_AUTODETECT_WINDOWED)
	{
		vgolf_config[CONFIG_GFX_MODE] = GFX_AUTODETECT_FULLSCREEN;
	}
	else
	{
		vgolf_config[CONFIG_GFX_MODE] = GFX_AUTODETECT_WINDOWED;
	}
	ncds_pause_music();
	if(vgolf_config[CONFIG_DEPTH])
	{
		set_color_depth(8);
	}
	else
	{
		int depth = desktop_color_depth();
		if(depth >= 8)
		{
			set_color_depth(depth);
		}
		else
		{
			set_color_depth(8);
		}
	}
	set_gfx_mode(vgolf_config[CONFIG_GFX_MODE], 640, 480, 0, 0);
	set_display_switch_mode(SWITCH_BACKGROUND);
	set_color_depth(8);
	set_mouse_sprite(NULL);
	if(vgolf_state == VGOLF_STATE_PAUSE)
	{
		set_palette(game_course->palette);
	}
	else
	{
		set_palette(game_palette);
	}
	ncds_resume_music();
}

void config_toggle_vsync(void)
{
	if(vgolf_config[CONFIG_VSYNC])
	{
		vgolf_config[CONFIG_VSYNC] = 0;
	}
	else
	{
		vgolf_config[CONFIG_VSYNC] = 1;
	}
}

void config_toggle_depth(void)
{
	if(vgolf_config[CONFIG_DEPTH])
	{
		vgolf_config[CONFIG_DEPTH] = 0;
		set_color_depth(8);
	}
	else
	{
		vgolf_config[CONFIG_DEPTH] = 1;
		int depth = desktop_color_depth();
		if(depth >= 8)
		{
			set_color_depth(depth);
		}
		else
		{
			set_color_depth(8);
		}
	}
	set_gfx_mode(vgolf_config[CONFIG_GFX_MODE], 640, 480, 0, 0);
	set_display_switch_mode(SWITCH_BACKGROUND);
	set_color_depth(8);
	if(vgolf_state == VGOLF_STATE_PAUSE)
	{
		set_palette(game_course->palette);
	}
	else
	{
		set_palette(game_palette);
	}
	set_mouse_sprite(NULL);
}

void config_toggle_upload(void)
{
	if(vgolf_config[CONFIG_UPLOAD])
	{
		vgolf_config[CONFIG_UPLOAD] = 0;
	}
	else
	{
		vgolf_config[CONFIG_UPLOAD] = 1;
	}
}

void config_toggle_reset_huds(void)
{
	if(vgolf_config[CONFIG_RESET_HUDS])
	{
		vgolf_config[CONFIG_RESET_HUDS] = 0;
	}
	else
	{
		vgolf_config[CONFIG_RESET_HUDS] = 1;
	}
}

void config_toggle_commentary(void)
{
	if(vgolf_config[CONFIG_COMMENTARY])
	{
		vgolf_config[CONFIG_COMMENTARY] = 0;
	}
	else
	{
		vgolf_config[CONFIG_COMMENTARY] = 1;
	}
}

void config_toggle_sound_on(void)
{
	if(vgolf_config[CONFIG_SOUND_ON])
	{
		vgolf_config[CONFIG_SOUND_ON] = 0;
		ncds_disable_effects();
	}
	else
	{
		vgolf_config[CONFIG_SOUND_ON] = 1;
		ncds_enable_effects();
	}
}

void config_toggle_music_on(void)
{
	if(vgolf_config[CONFIG_MUSIC_ON])
	{
		vgolf_config[CONFIG_MUSIC_ON] = 0;
		ncds_disable_music();
	}
	else
	{
		vgolf_config[CONFIG_MUSIC_ON] = 1;
		ncds_enable_music();
	}
}

void config_set_sfx_volume_left(void)
{
	if(vgolf_config[CONFIG_SFX_VOLUME] > 0)
	{
		vgolf_config[CONFIG_SFX_VOLUME] -= 5;
		if(vgolf_config[CONFIG_SFX_VOLUME] < 0)
		{
			vgolf_config[CONFIG_SFX_VOLUME] = 0;
		}
		ncds_set_sound_volume(vgolf_config[CONFIG_SFX_VOLUME]);
	}
}

void config_set_sfx_volume_right(void)
{
	if(vgolf_config[CONFIG_SFX_VOLUME] < 100)
	{
		vgolf_config[CONFIG_SFX_VOLUME] += 5;
		if(vgolf_config[CONFIG_SFX_VOLUME] > 100)
		{
			vgolf_config[CONFIG_SFX_VOLUME] = 100;
		}
		ncds_set_sound_volume(vgolf_config[CONFIG_SFX_VOLUME]);
	}
}

void config_set_music_volume_left(void)
{
	if(vgolf_config[CONFIG_MUSIC_VOLUME] > 0)
	{
		vgolf_config[CONFIG_MUSIC_VOLUME] -= 5;
		if(vgolf_config[CONFIG_MUSIC_VOLUME] < 0)
		{
			vgolf_config[CONFIG_MUSIC_VOLUME] = 0;
		}
		ncds_set_music_volume(vgolf_config[CONFIG_MUSIC_VOLUME]);
	}
}

void config_set_music_volume_right(void)
{
	if(vgolf_config[CONFIG_MUSIC_VOLUME] < 100)
	{
		vgolf_config[CONFIG_MUSIC_VOLUME] += 5;
		if(vgolf_config[CONFIG_MUSIC_VOLUME] > 100)
		{
			vgolf_config[CONFIG_MUSIC_VOLUME] = 100;
		}
		ncds_set_music_volume(vgolf_config[CONFIG_MUSIC_VOLUME]);
	}
}

void config_set_commentary_volume_left(void)
{
	if(vgolf_config[CONFIG_COMMENTARY_VOLUME] > 0)
	{
		vgolf_config[CONFIG_COMMENTARY_VOLUME] -= 5;
		if(vgolf_config[CONFIG_COMMENTARY_VOLUME] < 0)
		{
			vgolf_config[CONFIG_COMMENTARY_VOLUME] = 0;
		}
	}
}

void config_set_commentary_volume_right(void)
{
	if(vgolf_config[CONFIG_COMMENTARY_VOLUME] < 100)
	{
		vgolf_config[CONFIG_COMMENTARY_VOLUME] += 5;
		if(vgolf_config[CONFIG_COMMENTARY_VOLUME] > 100)
		{
			vgolf_config[CONFIG_COMMENTARY_VOLUME] = 100;
		}
	}
}

void config_set_commentary_announcer_left(void)
{
	vgolf_config[CONFIG_COMMENTARY]--;
	if(vgolf_config[CONFIG_COMMENTARY] < 0)
	{
		vgolf_config[CONFIG_COMMENTARY] = vgolf_commentaries.count - 1;
	}
}

void config_set_commentary_announcer_right(void)
{
	vgolf_config[CONFIG_COMMENTARY]++;
	if(vgolf_config[CONFIG_COMMENTARY] >= vgolf_commentaries.count)
	{
		vgolf_config[CONFIG_COMMENTARY] = 0;
	}
}

void exit_game_yes(void)
{
	vgolf_end_game();
}

void exit_game_no(void)
{
	hl_sub_menu_back(0, NULL);
}

void give_up_yes(void)
{
	game_player[main_game_vars.cur_player].ball.speed = 0;
	game_player[main_game_vars.cur_player].ball.mode = BALL_MODE_SUNK;
	game_player[main_game_vars.cur_player].ball.visible = 0;
	vgolf_profiles.item[game_player[main_game_vars.cur_player].profile].total.data[VGOLF_PROFILE_DATA_HOLES]++;
	
	//add the latest player tallies to his scorecard
	game_player[main_game_vars.cur_player].scorecard.score[main_game_vars.cur_hole] = game_player[main_game_vars.cur_player].cur_strokes;
	game_player[main_game_vars.cur_player].scorecard.par_score[main_game_vars.cur_hole] = game_course->current_hole.par_score;
	
	/* update profile course info */
	if(vgolf_profiles.item[game_player[main_game_vars.cur_player].profile].course_info[selected_course].score[main_game_vars.cur_hole] == 0 || vgolf_profiles.item[game_player[main_game_vars.cur_player].profile].course_info[selected_course].score[main_game_vars.cur_hole] > game_player[main_game_vars.cur_player].cur_strokes)
	{
		vgolf_profiles.item[game_player[main_game_vars.cur_player].profile].course_info[selected_course].score[main_game_vars.cur_hole] = game_player[main_game_vars.cur_player].cur_strokes;
	}
	vgolf_state = VGOLF_STATE_PLAY;
	do_player_hole_finish(game_player, &game_course->current_hole);
	if(vgolf_player_count > 1)
	{
		int p_no = vgolf_get_next_player();

		if(p_no != -1)
		{
			main_game_vars.cur_player = p_no;
			build_turn_text(p_no);
			add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, temp_text, SCREEN_W / 2 - strlen(temp_text) * game_vfont.w * 2, SCREEN_H / 2 - game_vfont.h * 2, 0, VMSG_TEXT_ENTER_FROMRIGHT, -10, 4, 0, 0, -1);
		}
	}
}

void give_up_no(void)
{
	vgolf_state = VGOLF_STATE_PLAY;
	block_click = 1;
	if(vgolf_player_count > 1)
	{
		int p_no = vgolf_get_next_player();

		if(p_no != -1)
		{
			main_game_vars.cur_player = p_no;
			build_turn_text(p_no);
			add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, temp_text, SCREEN_W / 2 - strlen(temp_text) * game_vfont.w * 2, SCREEN_H / 2 - game_vfont.h * 2, 0, VMSG_TEXT_ENTER_FROMRIGHT, -10, 4, 0, 0, -1);
		}
	}
}

void in_game_resume(void)
{
	vgolf_resume_game();
	block_click = 1;
}

void delete_profile_yes(void)
{
	int i;
	
	for(i = selected_profile; i < vgolf_profiles.items - 1; i++)
	{
		memcpy(&vgolf_profiles.item[i], &vgolf_profiles.item[i + 1], sizeof(PROFILE));
	}
	vgolf_profiles.items--;
	if(selected_profile >= vgolf_profiles.items)
	{
		selected_profile = vgolf_profiles.items - 1;
	}
	create_profile_page(selected_profile);
	vgolf_state = VGOLF_STATE_PROFILE;
}

void delete_profile_no(void)
{
	vgolf_state = VGOLF_STATE_PROFILE;
}

void ball_lost_drop(void)
{
	do_ball_drop(&current_player->ball, current_player, &current_hole->vert[current_player->ball.water_vert], current_hole);
	deactivate_vfont_message(&game_vmessage[0]);
	current_player->pic_mode = 0;             //make player's face content again
	vgolf_state = VGOLF_STATE_PLAY;
	block_click = 1;
}

void ball_lost_rehit(void)
{
	do_ball_rehit(&current_player->ball, current_hole);
	deactivate_vfont_message(&game_vmessage[0]);
	current_player->pic_mode = 0;             //make player's face content again
	vgolf_state = VGOLF_STATE_PLAY;
	block_click = 1;
}

//loads the main images
int load_main_image_bank(MAIN_IMAGE_BANK * ibank)
{
	BITMAP * bp;
	
	credit_image[0] = load_pcx("vgolf.dat#credits1.pcx", NULL);
	credit_image[1] = load_pcx("vgolf.dat#credits2.pcx", NULL);
	credit_image[2] = load_pcx("vgolf.dat#credits3.pcx", NULL);
	credit_image[3] = load_pcx("vgolf.dat#credits4.pcx", NULL);
	credit_image[4] = load_pcx("vgolf.dat#credits5.pcx", NULL);
	credit_image[5] = load_pcx("vgolf.dat#credits6.pcx", NULL);
	credit_image[6] = load_pcx("vgolf.dat#credits7.pcx", NULL);
	credit_image[7] = load_pcx("vgolf.dat#credits8.pcx", NULL);
	logo_image = load_pcx("vgolf.dat#logo_pcx", NULL);
	
	bp = load_pcx(base_filename("c9a00-00_pcx"), game_palette);
	if(bp)
	{
		destroy_bitmap(bp);
	}

	return 1;
}

void free_main_image_bank(void)
{
	destroy_bitmap(credit_image[0]);
	destroy_bitmap(credit_image[1]);
	destroy_bitmap(credit_image[2]);
	destroy_bitmap(credit_image[3]);
	destroy_bitmap(credit_image[4]);
	destroy_bitmap(credit_image[5]);
	destroy_bitmap(credit_image[6]);
	destroy_bitmap(credit_image[7]);
	destroy_bitmap(logo_image);
}

//load the sound effect files
int load_main_game_samples(SOUND_BANK * sbank)
{
	sbank->sfx_swing[0] = load_wav(base_filename("sfx_h000_wav"));
	if(!sbank->sfx_swing[0])
	{
		return 0;
	}
	sbank->sfx_swing[1] = load_wav(base_filename("sfx_h001_wav"));
	if(!sbank->sfx_swing[1])
	{
		return 0;
	}
	sbank->sfx_swing[2] = load_wav(base_filename("sfx_h002_wav"));
	if(!sbank->sfx_swing[2])
	{
		return 0;
	}
	sbank->sfx_ball_collision[0] = load_wav(base_filename("sfx_c000_wav"));
	if(!sbank->sfx_ball_collision[0])
	{
		return 0;
	}
	sbank->sfx_ball_collision[1] = load_wav(base_filename("sfx_c001_wav"));
	if(!sbank->sfx_ball_collision[1])
	{
		return 0;
	}
	sbank->sfx_ball_collision[2] = load_wav(base_filename("sfx_c002_wav"));
	if(!sbank->sfx_ball_collision[2])
	{
		return 0;
	}
	sbank->sfx_ball_holesink = load_wav(base_filename("sfx_hole_wav"));
	if(!sbank->sfx_ball_holesink)
	{
		return 0;
	}
	sbank->sfx_hole_finish[0][0] = load_wav(base_filename("sfx_f000_wav"));
	if(!sbank->sfx_hole_finish[0][0])
	{
		return 0;
	}
	sbank->sfx_hole_finish[0][1] = load_wav(base_filename("sfx_f001_wav"));
	if(!sbank->sfx_hole_finish[0][1])
	{
		return 0;
	}
	sbank->sfx_hole_finish[0][2] = load_wav(base_filename("sfx_f002_wav"));
	if(!sbank->sfx_hole_finish[0][2])
	{
		return 0;
	}
	sbank->sfx_hole_finish[0][3] = load_wav(base_filename("sfx_f003_wav"));
	if(!sbank->sfx_hole_finish[0][3])
	{
		return 0;
	}
	sbank->sfx_hole_finish[1][0] = load_wav(base_filename("sfx_f100_wav"));
	if(!sbank->sfx_hole_finish[1][0])
	{
		return 0;
	}
	sbank->sfx_hole_finish[1][1] = load_wav(base_filename("sfx_f200_wav"));
	if(!sbank->sfx_hole_finish[1][1])
	{
		return 0;
	}
	sbank->sfx_hole_finish[1][2] = load_wav(base_filename("sfx_f201_wav"));
	if(!sbank->sfx_hole_finish[1][2])
	{
		return 0;
	}
	sbank->sfx_hole_finish[1][3] = load_wav(base_filename("sfx_f202_wav"));
	if(!sbank->sfx_hole_finish[1][3])
	{
		return 0;
	}
	sbank->sfx_hole_finish[2][0] = load_wav(base_filename("sfx_f300_wav"));
	if(!sbank->sfx_hole_finish[2][0])
	{
		return 0;
	}
	sbank->sfx_hole_finish[2][1] = load_wav(base_filename("sfx_f301_wav"));
	if(!sbank->sfx_hole_finish[2][1])
	{
		return 0;
	}
	sbank->sfx_hole_finish[2][2] = load_wav(base_filename("sfx_f302_wav"));
	if(!sbank->sfx_hole_finish[2][2])
	{
		return 0;
	}
	sbank->sfx_hole_finish[2][3] = load_wav(base_filename("sfx_f303_wav"));
	if(!sbank->sfx_hole_finish[2][3])
	{
		return 0;
	}
	sbank->sfx_hole_ahhh[0] = load_wav(base_filename("sfx_a000_wav"));
	if(!sbank->sfx_hole_ahhh[0])
	{
		return 0;
	}
	sbank->sfx_hole_ahhh[1] = load_wav(base_filename("sfx_a001_wav"));
	if(!sbank->sfx_hole_ahhh[1])
	{
		return 0;
	}
	sbank->sfx_ball_splash = load_wav(base_filename("sfx_spla_wav"));
	if(!sbank->sfx_ball_splash)
	{
		return 0;
	}
	sbank->sfx_ball_rotate = load_wav(base_filename("sfx_r000_wav"));
	if(!sbank->sfx_ball_rotate)
	{
		return 0;
	}
	sbank->sfx_menu_select = load_wav(base_filename("sfx_m000_wav"));
	if(!sbank->sfx_menu_select)
	{
		return 0;
	}
	sbank->sfx_menu_escape = load_wav(base_filename("sfx_m001_wav"));
	if(!sbank->sfx_menu_escape)
	{
		return 0;
	}
	sbank->sfx_menu_slider = load_wav(base_filename("sfx_m002_wav"));
	if(!sbank->sfx_menu_slider)
	{
		return 0;
	}
	sbank->sfx_menu_itemsel = load_wav(base_filename("sfx_m003_wav"));
	if(!sbank->sfx_menu_itemsel)
	{
		return 0;
	}
	sbank->sfx_coin = load_wav(base_filename("sfx_coin_wav"));
	if(!sbank->sfx_coin)
	{
		return 0;
	}
	sbank->sfx_ball_zapped = load_wav(base_filename("sfx_zap_wav"));
	if(!sbank->sfx_ball_zapped)
	{
		return 0;
	}
	sbank->sfx_ball_teleport = load_wav(base_filename("sfx_tele_wav"));
	if(!sbank->sfx_ball_teleport)
	{
		return 0;
	}
	sbank->sfx_ball_enter = load_wav(base_filename("sfx_pipi.wav"));
	if(!sbank->sfx_ball_enter)
	{
		return 0;
	}
	sbank->sfx_ball_exit = load_wav(base_filename("sfx_pipo.wav"));
	if(!sbank->sfx_ball_exit)
	{
		return 0;
	}
	sbank->sfx_title_swing = load_wav(base_filename("golfswing.wav"));
	if(!sbank->sfx_title_swing)
	{
		return 0;
	}

	return 1;
}

//set the victory screen up for display
void set_victory_screen_vars(MAIN_GAME_VARS * gvars)
{
	int i;

	gvars->vict_pody = SCREEN_H;   //place the victory podium at the bottom of the screen

	for(i = 0; i < 4; i++)
	{        //generate some random values for the background clouds
		gvars->vict_cloud_x[i] = random()%640;
		gvars->vict_cloud_y[i] = random()%200;
	}
	for(i = 0; i < 4; i++)
	{        //place all the trophies offscreen
		gvars->vict_trophy_x[i] = gvars->vict_trophy_y[i] = -100;
		gvars->vict_winner[i] = 0;
	}
}

//set the main global game variables
void set_main_game_vars(void)
{
	int p;
	
	main_game_vars.quit_game = 0;
	main_game_vars.quit_all = 0;
	main_game_vars.quit_title = 0;
	//main_game_vars.game_state = GAMESTATE_START_HOLE;
	main_game_vars.game_state = GAMESTATE_SCOREBOARD_SEGIN;
	main_game_vars.game_state_old = main_game_vars.game_state;
	main_game_vars.cur_vmessage_slot = 0;
	main_game_vars.cur_player = 0;
	set_victory_screen_vars(&main_game_vars);   //reset the victory screen stuff
	main_game_vars.num_players = 2;             //set it to 1 for now..
	main_game_vars.game_mode = GAMEMODE_STROKE_PLAY;  //set it to stroke play as default
	main_game_vars.cur_hole = vgolf_start_hole - 1;                //current hole is 0
	next_hole = vgolf_start_hole;
	main_game_vars.cur_hole = vgolf_start_hole - 1;
	main_game_vars.hole_finished = 0;           //hole is not finished
	main_game_vars.cur_statsheet = -1;          
	main_game_vars.ingame_timer = 0;
	main_game_vars.ingame_clicker = 0;
	main_game_vars.ingame_timer_change = 0;     //timing stuff
	main_game_vars.player_huds_state = 0;
	main_game_vars.got_hiscore = 0;

	main_game_vars.num_player_images = 8;
        
	//give all the players some default settings
	for(p = 0; p < MAX_PLAYERS; p++)
	{
		game_player[p].id = p;
		game_player[p].profile = 0;
		game_player[p].avatar = 0;
	}
}

//calculate poly normals..
void initialize_hole_polys(HOLE * hole)
{
	int i;

	for(i = 0; i < hole->num_polys; i++)
	{
		calc_body_normals(&hole->body[i]);
	}
}

//reset the message queue
void initialize_vmessage_queue(V_MESSAGE vmsg[], int num_messages, FONT * vf)
{
	int i;

	for(i = 0; i < num_messages; i++)
	{
		set_vfont_message(&vmsg[i], vf, "", 0, 0, 0, VMSG_TEXT_PLAIN, 0, 0, 0, 0, 0);
	}
}

//do all the initial vert calculations/settings..
void setup_hole_verts(HOLE * hole)
{
	int v;

	for(v = 0; v < hole->num_verts; v++)
	{
		if(hole->vert[v].flag == VERT_FLAG_ANIMAL_0 || hole->vert[v].flag == VERT_FLAG_ANIMAL_1)
		{
			//refs 4 and 5 define a bounding box that holds the animal inside it
			hole->vert[v].ref_no[4] = hole->s.x+hole->vert[v].s.x;
			hole->vert[v].ref_no[5] = hole->s.y+hole->vert[v].s.y;
			hole->vert[v].ref_no[0] = hole->vert[v].ref_no[1] = hole->vert[v].ref_no[2] = hole->vert[v].ref_no[3] = 0;
		}
		else if(hole->vert[v].flag == VERT_FLAG_POLY_MORPH)
		{
			if(hole->vert[v].ref_no[0] == 0)
			{
				//tag the associated poly to be a rotating poly
				hole->body[hole->vert[v].ref_no[1]].poly.color = POLY_TYPE_MORPH_ROTATE;
			}
		}
		else if(hole->vert[v].flag == VERT_FLAG_POLY_MOVE_HORIZ)
		{
			//tag the associated poly to be a horizontally moving poly
			hole->body[hole->vert[v].ref_no[1]].poly.color = POLY_TYPE_MOTION_HORIZ;
		}
		else if(hole->vert[v].flag == VERT_FLAG_POLY_MOVE_VERT)
		{
			//tag the associated poly to be a vertically moving poly
			hole->body[hole->vert[v].ref_no[1]].poly.color = POLY_TYPE_MOTION_VERT;
		}
		else if(hole->vert[v].flag == VERT_FLAG_HOLE)
		{
			if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT)
			{
				hole->vert[v].flag = VERT_FLAG_NULL;  //no hole in coin collector mode
			}
			hole->vert[v].ref_no[0] = hole->vert[v].ref_no[1] = hole->vert[v].ref_no[2] = hole->vert[v].ref_no[3] = hole->vert[v].ref_no[4] = hole->vert[v].ref_no[5] = 0;   //empty the hole out if needed
		}
		else if(hole->vert[v].flag == VERT_FLAG_LASER_SEND)
		{
			//reset laser active and timers
			hole->vert[v].ref_no[0] = hole->vert[v].ref_no[1] = 0;
			//make the associated poly empty while laser is inactive
			hole->body[hole->vert[v].ref_no[4]].poly.color = POLY_TYPE_EMPTY_WALL;
		}
		else if(hole->vert[v].flag == VERT_FLAG_TELEPORT)
		{
			//reset animation counter
			hole->vert[v].ref_no[5] = 0;
		}
		else if(hole->vert[v].flag == VERT_FLAG_BUBBLE_HORIZ || hole->vert[v].flag == VERT_FLAG_BUBBLE_VERT)
		{
			hole->vert[v].ref_no[0] = hole->vert[v].ref_no[1] = hole->vert[v].ref_no[2] = hole->vert[v].ref_no[3] = hole->vert[v].ref_no[4] = hole->vert[v].ref_no[5] = 0;
		}
		else if(hole->vert[v].flag == VERT_FLAG_CONVEYOR)
		{
			hole->vert[v].ref_no[1] = 1;
			hole->vert[v].ref_no[4] = 0;
			hole->body[hole->vert[v].ref_no[5]].poly.color = POLY_TYPE_CONVEYOR_WALL;
		}
		else if(hole->vert[v].flag == VERT_FLAG_POLY_DOOR_HORIZ)
		{
			//set these polys as walls
			hole->body[hole->vert[hole->vert[v].ref_no[4]].ref_no[4]].poly.color = POLY_TYPE_DOOR_L;
			hole->body[hole->vert[hole->vert[v].ref_no[4]].ref_no[5]].poly.color = POLY_TYPE_DOOR_R;
		}
		else if(hole->vert[v].flag == VERT_FLAG_POLY_DOOR_VERT)
		{
			//set these polys as walls
			hole->body[hole->vert[hole->vert[v].ref_no[4]].ref_no[4]].poly.color = POLY_TYPE_DOOR_U;
			hole->body[hole->vert[hole->vert[v].ref_no[4]].ref_no[5]].poly.color = POLY_TYPE_DOOR_D;
		}
	}
	if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT)
	{
		for(v = hole->num_verts; v < hole->num_verts+hole->num_coins; v++)
		{
			if(hole->vert[v].flag == VERT_FLAG_COIN_0)
			{
				int i;
				
				hole->vert[v].ref_no[0] = 1;   //set coin as active
				hole->vert[v].radius = 7;      //set coin radius to reasonable size
				for(i = 1; i < MAX_VERT_REFS; i++)
				{
					hole->vert[v].ref_no[i] = 0;
				}
			}
		}
	}
}

//get the hole ready for play..
void initialize_hole(HOLE * hole)
{
	hole->deccel_factor = 0.05;
	hole->s.x = SCREEN_W/2;
	hole->s.y = SCREEN_H/2;
	initialize_hole_polys(hole);
	setup_hole_verts(hole);

	trash_bitmap(hole->layer_pic[2]);
	hole->layer_pic[2] = create_bitmap(SCREEN_W, SCREEN_H);
	clear_to_color(hole->layer_pic[2], 0);
	initialize_vmessage_queue(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont2);
	//initialize_game_zbuff(&game_zbuff, hole);
}

//give the course some starter values that show it's empty..
void preinitialize_course(VGOLF_COURSE * cr)
{
	strcpy(cr->name, "");
}

//reset this player's scorecard
void reset_player_scorecard(SCORECARD * sc, PLAYER * pl)
{
	int i;

	for(i = 0; i < 18; i++) sc->score[i] = sc->par_score[i] = sc->match_won[i] = 0;
	sc->num_birdies = sc->num_eagles = sc->num_albo = 0;
	sc->matches_won = 0;

	strcpy(sc->name, vgolf_profiles.item[pl->profile].name);
	sc->coin_score = 0;
	sc->coin_best_combo = 0;
}

//reset the trajectory vertex list
void reset_trajectory_points(VERTEX t[], int num_points)
{
	int i;
	
	for(i = 0; i < num_points; i++)
	{
		t[i].x = t[i].y = 0;
	}
}

//do the first initialization of the player
void preinitialize_player(PLAYER * pl, int plno)
{
	int i;

	pl->ball.s.x = pl->ball.s.y = -100;
	pl->ball.v.x = pl->ball.v.y = 0;
	pl->ball.f.x = pl->ball.f.y = 0;
	pl->ball.s_old = pl->ball.s;
	pl->ball.speed = 0;
	pl->ball.visible = 0;
	pl->ball.cur_vert = 0;
	pl->ball.idle_timer = 0;
	pl->ball.idle_timeout = 75;
	
	pl->cur_hole = main_game_vars.cur_hole;
	pl->club_angle = pl->f_club_angle = 0;
	pl->club_power = 0;
	pl->club_rot_speed = 4;
	pl->f_club_rot_speed = pl->club_rot_speed/128.0;
	pl->club_v[0].x = pl->club_v[0].y = 0;
	pl->club_v[1].x = pl->club_v[1].y = 0;
	pl->club_color = 93;
	pl->club_swing = 0;
	pl->club_width = 20;
	pl->f_club_width = pl->club_width/128.0;
	pl->traj_power = pl->traj_power_max = pl->traj_count = pl->traj_calc_done = pl->traj_active = pl->traj_line_timer = 0;
	pl->ball.radius = BALL_STD_RADIUS;
	pl->ball.s_med = pl->ball.s;
	pl->ball.mirage_visible = 0;        
	pl->ball.mirage_timer = 0;
	pl->ball.mirage_timer_end = 5;
        
	//reset player stroke tally
	pl->cur_strokes = 0;
	//define the ball's polygon
	set_ball_poly(&pl->ball, BALL_STD_RADIUS);
	//load ball's various images
	load_ball_images(&pl->ball);
	
	//ball's hit rectangle should just encompass the ball's diameter
	pl->ball.hit_rect.x1 = pl->ball.hit_rect.y1 = -pl->ball.radius;
	pl->ball.hit_rect.x2 = pl->ball.hit_rect.y2 = pl->ball.radius;
	
	reset_player_scorecard(&pl->scorecard, pl);
	pl->hud.w = 200;
	pl->hud.h = 34;
	//player 1 is on the top left
	if(plno == 0)
	{
		pl->hud.x = 5;
		pl->hud.y = 10;
	}
	//player 2 is on the bottom right
	else if(plno == 1)
	{
		pl->hud.x = 435;
		pl->hud.y = 10;
	}
	//player 3 is on the top right
	else if(plno == 2)
	{
		pl->hud.x = 5;
		pl->hud.y = 440;
	}
	//player 4 is on the bottom left
	else if(plno == 3)
	{
		pl->hud.x = 5;
		pl->hud.y = 440;
	}
	//set avatar to "content" mode
	pl->pic_mode = 0;
	//reset coin counters
	pl->coins_overall = pl->coins_total = pl->coins_current = pl->scorecard.coin_score = 0;
	//reset ball angle
	pl->ball.angle = pl->ball.f_angle = 0;
	//reset trajectory point list
	reset_trajectory_points(pl->traj_v, MAX_TRAJECTORY_POINTS);        
        
	pl->cur_statsheet = -1;       //set to "no data found" statsheet
	for(i = 0; i < 4; i++)
	{
		blit(vgolf_avatars.avatar[pl->avatar]->image[i], pl->pic[i], 0, 0, 0, 0, pl->pic[i]->w, pl->pic[i]->h);
		convert_palette(pl->pic[i], game_palette, game_course->palette);
	}
	if(pl->profile > 0)
	{
		vgolf_profiles.item[pl->profile].avatar = vgolf_avatars.sum[pl->avatar];
	}
}

//initialize all players before game
void preinitialize_players(PLAYER pl[], int num_players)
{
	int i;
	int first = -1;

	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			preinitialize_player(&pl[i], i);
			if(first < 0)
			{
				first = i;
			}
		}
	}
	main_game_vars.cur_player = first;
}

//free memory related to this scorecard
void deinitialize_scorecard(SCORECARD * sc)
{
	if(sc->name != NULL) free(sc->name);
}

//destroy images in the main image bank
void destroy_main_image_bank(MAIN_IMAGE_BANK * ibank)
{
	int i;
	
	for(i = 0; i < 0; i++)
	{
		destroy_bitmap(credit_image[i]);
		credit_image[i] = NULL;
	}
}

//destroy course images
void destroy_course_image_bank(COURSE_IMAGE_BANK * ibank)
{
	int i, j;
	
	for(i = 0; i < MAX_COURSE_VERT_PIC_TYPES; i++)
	{
		for(j = 0; j < MAX_VERT_ANIMS; j++)
		{
			trash_bitmap(ibank->vert_pic[i][j]);
		}
	}
	for(i = 0; i < MAX_COURSE_ANIMAL_TYPES; i++)
	{
		for(j = 0; j < MAX_ANIMAL_ANIMS; j++)
		{
			trash_bitmap(ibank->animal_pic[i][j]);
		}
	}
	for(i = 0; i < MAX_COURSE_OBJECT_TYPES; i++)
	{
		for(j = 0; j < MAX_OBJECT_ANIMS; j++)
		{
			trash_bitmap(ibank->object_pic[i][j]);
		}
	}
	trash_bitmap(ibank->victory_bgpic);
	trash_bitmap(ibank->victory_podiumpic);
	trash_bitmap(ibank->victory_cloudpic[0]);
	trash_bitmap(ibank->victory_cloudpic[1]);
	trash_bitmap(ibank->victory_cloudpic[2]);
	trash_bitmap(ibank->victory_cloudpic[3]);
	trash_bitmap(ibank->victory_trophypic[0]);
	trash_bitmap(ibank->victory_trophypic[1]);
	trash_bitmap(ibank->victory_trophypic[2]);
	trash_bitmap(ibank->iconpic);
	for(i = 0; i < MAX_COIN_PICS; i++) trash_bitmap(ibank->coinpic[i]);
}

//unload scoreboard images
void destroy_course_scoreboard(SCOREBOARD * sb)
{
	int i;
	
	trash_bitmap(sb->bgpic);
	trash_bitmap(sb->gridpic);
	for(i = 0; i < 19; i++) trash_bitmap(sb->numpic[i]);
}

//destroy all the sound effect samples
void destroy_sample_bank(SOUND_BANK * sbank)
{
	int i,j;

	for(i = 0; i < MAX_SFX_SWING_SAMPLES; i++) destroy_sample(sbank->sfx_swing[i]);
	for(i = 0; i < MAX_SFX_COLLISION_SAMPLES; i++) destroy_sample(sbank->sfx_ball_collision[i]);
	destroy_sample(sbank->sfx_ball_holesink);
	for(i = 0; i < 3; i++)
	{
		for(j = 0; j < MAX_SFX_HOLE_FINISH_SAMPLES; j++)
		{
			destroy_sample(sbank->sfx_hole_finish[i][j]);
		}
	}
	for(i = 0; i < MAX_SFX_HOLE_AHHH_SAMPLES; i++) destroy_sample(sbank->sfx_hole_ahhh[i]);
	destroy_sample(sbank->sfx_ball_splash);
	destroy_sample(sbank->sfx_ball_zapped);
	destroy_sample(sbank->sfx_ball_teleport);
	destroy_sample(sbank->sfx_ball_rotate);
	destroy_sample(sbank->sfx_menu_select);
	destroy_sample(sbank->sfx_menu_escape);
	destroy_sample(sbank->sfx_menu_slider);
	destroy_sample(sbank->sfx_menu_itemsel);
	destroy_sample(sbank->sfx_coin);
}

//unload/deinitialize all main game variable stuff
void deinitialize_main_game_vars(void)
{
	destroy_main_image_bank(&main_game_vars.image_bank);
	destroy_sample_bank(&main_game_vars.sound_bank);
	destroy_vgolf_commentary(main_game_vars.commentary);
}

//unload all the ball's images from memory (if necessary)
void unload_ball_images(BALL * ball)
{
	int i,j;

	for(i = 0; i < MAX_BALL_ANIM_MODES; i++)
	{
		for(j = 0; j < ball->anim_def[i].num_frames; j++)
		{
			trash_bitmap(ball->anim_def[i].pic[j]);
		}
	}

	trash_bitmap(ball->shad_pic);
	destroy_bitmap(main_game_vars.image_bank.scorecard);
}

//deinitialize the player..
void deinitialize_player(PLAYER * pl, int plno)
{
	//trash all the ball's images from memory
	unload_ball_images(&pl->ball);
}

//deinitialize all players before game exit
void deinitialize_players(PLAYER pl[], int num_players)
{
	int i;

	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			deinitialize_player(&pl[i], i);
		}
	}
}

void draw_poly2(BITMAP * bp, POLY po, VERTEX c, int col)
{
	int i;
	float ang;
	VERTEX n, m;

	for(i = 0; i < po.num_vertices-1; i++)
	{
		ang = atan2(po.p[i].y-po.p[i+1].y, po.p[i].x-po.p[i+1].x)+M_PI/2;
		m = new_vertex((po.p[i].x+po.p[i+1].x)/2, (po.p[i].y+po.p[i+1].y)/2);
		n = new_vertex(10*cos(ang), 10*sin(ang));

		line(bp, c.x+po.p[i].x, c.y+po.p[i].y, c.x+po.p[i+1].x, c.y+po.p[i+1].y, col);
		line(bp, c.x+m.x, c.y+m.y, c.x+m.x+n.x, c.y+m.y+n.y, col+1);
	}
	line(bp, c.x+po.p[po.num_vertices-1].x, c.y+po.p[po.num_vertices-1].y, c.x+po.p[0].x, c.y+po.p[0].y, col);
}

//tells if a collision has occured between these two bodies
char check_body_collision(BODY * a, BODY * b)
{
	return body_collide(a, b);
}

//checks to see if the ball is within a rectangle that surrounds the line segment endpoints..
char circle_lineseg_rect_collision(VERTEX b_c, float radius, VERTEX v1, VERTEX v2, VERTEX c)
{
	int x1,y1,x2,y2;

	//sort the points
	if(v1.x < v2.x)
	{
		x1 = v1.x;
		x2 = v2.x;
	}
	else
	{
		x2 = v1.x;
		x1 = v2.x;
	}
	if(v1.y < v2.y)
	{
		y1 = v1.y;
		y2 = v2.y;
	}
	else
	{
		y2 = v1.y;
		y1 = v2.y;
	}

	//make sure the rectangle is of a minimum span
	if(x2-x1 < MIN_RECT_SPAN)
	{
		x1 -= MIN_RECT_SPAN/2;
		x2 += MIN_RECT_SPAN/2;
	}
	if(y2-y1 < MIN_RECT_SPAN)
	{
		y1 -= MIN_RECT_SPAN/2;
		y2 += MIN_RECT_SPAN/2;
	}
	return rect_collision_f(-radius, -radius, radius, radius, x1, y1, x2, y2, b_c, c);
}

//tells if this poly is empty
char poly_is_empty(POLY po)
{
	return (po.color == POLY_TYPE_EMPTY_WALL);
}

//tells if this poly is empty
char poly_is_laser(POLY po)
{
	return (po.color == POLY_TYPE_LASER_WALL);
}

//check for collisions between the ball and the hole's walls
void check_ball_hole_collision(BALL * ball, PLAYER * pl, HOLE * hole)
{
	int i,j;
        
	for(i = 0; i < hole->num_polys; i++)
	{
		float r_off;
	        
		/* if it's a conveyor poly skip it */
		if(hole->body[i].poly.color == POLY_TYPE_CONVEYOR_WALL)
		{
		}
		
		/* see if we're going to be in the path of a laser */
		else if(poly_is_empty(hole->body[i].poly))
		{
			if(ball->speed <= 0 && line_circle_collide(ball->s.x, ball->s.y, ball->radius, hole->body[i].poly.p[0].x + hole->s.x, hole->body[i].poly.p[0].y + hole->s.y, hole->body[i].poly.p[1].x + hole->s.x, hole->body[i].poly.p[1].y + hole->s.y))
			{
				if(ball->mode != BALL_MODE_IN_MOTION_AREA)
				{
					ball->mode = BALL_MODE_IN_MOTION_AREA;
					ball->mode2 = 1;
					ball->idle_timer = 0; //reset idle timer
					return;
				}
			}
		}
		else
		{
			for(r_off = 0; r_off < (ball->speed <= 0 ? 0.5 : ball->speed); r_off += 1.0)
			{
				if(poly_is_laser(hole->body[i].poly))
				{
					VERTEX ball_off = new_vertex(r_off*cos(ball->f_angle), r_off*sin(ball->f_angle));
					VERTEX ball_off_pos = vertex_add(ball->s, ball_off);
					if(line_circle_collide(ball_off_pos.x, ball_off_pos.y, ball->radius, hole->body[i].poly.p[0].x + hole->s.x, hole->body[i].poly.p[0].y + hole->s.y, hole->body[i].poly.p[1].x + hole->s.x, hole->body[i].poly.p[1].y + hole->s.y) && ball->mode != BALL_MODE_LASER_ZAPPED)
					{
						do_ball_laser_zap(ball, pl, hole);
						return;
					}
				}
				else
				{
					VERTEX ball_off = new_vertex(r_off*cos(ball->f_angle), r_off*sin(ball->f_angle));
					VERTEX ball_off_pos = vertex_add(ball->s, ball_off);
					for(j = 0; j < hole->body[i].poly.num_vertices-1; j++)
					{
						int num_sides;
						BODY seg_body, ball_offset;
		
						//goto the next two points if the ball isn't within a rectangle defined by the current two points
						if(!circle_lineseg_rect_collision(ball_off_pos, ball->radius, hole->body[i].poly.p[j], hole->body[i].poly.p[j+1], hole->s))
						{
							continue;
						}
		
						//create a new segment body based on these two endpoints..
						seg_body = new_seg_body(hole->body[i].poly.p[j], hole->body[i].poly.p[j+1], hole->s, &num_sides);
		                
						//offset the ball's body by the offset so we can check to see if it will hit a wall segment before its next velocity calculation
						ball_offset = body_offset(ball->body, ball_off);
						
						//check for a collision between the ball body and this segment body..
						if(check_body_collision(&ball_offset, &seg_body))
						{
							//play a random sound when the ball hits a solid wall (don't do this for the trajectory stuff)
							play_ball_wall_hit_sample();
			       				
							if(ball->speed <= 0 && ball->mode == BALL_MODE_IN_MOTION_AREA)
							{
								ball->speed += 1.0;   //nudge the ball a little bit if it's within a motion area and stopped when the poly hits it
								ball->idle_timer = 0; //reset idle timer
							}
                						
							ball->s_med = ball_offset.pos;
							ball->mirage_visible = 1;
							ball->body = ball_offset;
							ball->v = ball->body.vel;
							ball->f_angle = atan2(ball->v.y, ball->v.x);
							ball->v.x = ball->speed*cos(ball->f_angle);
							ball->v.y = ball->speed*sin(ball->f_angle);
                            	    			
							return;
						}
					}
				}
			}    	      
		}
	}
}

//load ball images
int load_ball_images(BALL * ball)
{
	//splash animation..
	ball->anim_def[1].speed = 4;
	ball->anim_def[1].num_frames = 4;
	ball->anim_def[1].cur_frame = 0;
	
	ball->anim_def[1].pic[0] = load_pcx(base_filename("splash00_pcx"), NULL);
	convert_palette(ball->anim_def[1].pic[0], game_palette, game_course->palette);
	ball->anim_def[1].pic[1] = load_pcx(base_filename("splash01_pcx"), NULL);
	convert_palette(ball->anim_def[1].pic[1], game_palette, game_course->palette);
	ball->anim_def[1].pic[2] = load_pcx(base_filename("splash02_pcx"), NULL);
	convert_palette(ball->anim_def[1].pic[2], game_palette, game_course->palette);
	ball->anim_def[1].pic[3] = load_pcx(base_filename("splash03_pcx"), NULL);
	convert_palette(ball->anim_def[1].pic[3], game_palette, game_course->palette);
	ball->anim_def[1].pic[4] = load_pcx(base_filename("splash04_pcx"), NULL);
	convert_palette(ball->anim_def[1].pic[4], game_palette, game_course->palette);
	main_game_vars.image_bank.scorecard = load_pcx("vgolf.dat#scorecard.pcx", NULL);
	convert_palette(main_game_vars.image_bank.scorecard, game_palette, game_course->palette);
	
	//ball shadow
	if(ball->shad_pic == NULL)
	{
		ball->shad_pic = create_bitmap(ball->radius*2, ball->radius*2);
		clear_to_color(ball->shad_pic, 0);
		circlefill(ball->shad_pic, ball->radius, ball->radius, ball->radius, 5);
	}
	
	return 0;
}

//initialize the player before the next hole
void hole_initialize_player(PLAYER * pl, int plno, int cur_hole)
{
	pl->ball.s.x = pl->ball.s.y = -100;
	pl->ball.v.x = pl->ball.v.y = 0;
	pl->ball.f.x = pl->ball.f.y = 0;
	pl->ball.cf.x = pl->ball.cf.y = 0;
	pl->ball.s_old = pl->ball.s;
	pl->ball.speed = 0;
	pl->ball.visible = 0;
	pl->ball.cur_vert = 0;
	pl->ball.idle_timer = 0;
	pl->ball.idle_timeout = 75;
	pl->ball.radius = BALL_STD_RADIUS;
	pl->ball.off_conveyor = 0;
	pl->ball.conveyor_vert = -1;
	pl->ball.decel_type = 0;
	
	pl->ball.angle = pl->ball.f_angle = 0;
	pl->cur_hole = cur_hole;
	pl->club_angle = pl->f_club_angle = 0;
	pl->club_power = 0;
	pl->club_v[0].x = pl->club_v[0].y = 0;
	pl->club_v[1].x = pl->club_v[1].y = 0;
	pl->traj_active = pl->traj_power = pl->traj_count = pl->traj_calc_done = pl->traj_line_timer = 0;
	pl->ball.s_med = pl->ball.s;
	pl->ball.mirage_visible = 0;        
	pl->ball.mirage_timer = 0;
	pl->ball.mirage_timer_end = 5;

	pl->cur_strokes = 0;
	pl->pic_mode = 0;
	pl->coins_total = pl->coins_current = pl->coin_best_combo = 0;

	//reset trajectory point list
	reset_trajectory_points(pl->traj_v, MAX_TRAJECTORY_POINTS);        
}

//initialize all players before hole
void hole_initialize_players(PLAYER pl[], int num_players, int cur_hole)
{
	int i;

	main_game_vars.player_huds_state = 0;
//	main_game_vars.player_huds_hiding = main_game_vars.player_huds_hidden = 0;
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			hole_initialize_player(&pl[i], i, cur_hole);
		}
        pl[0].hud.x = 5;
        pl[0].hud.y = 10;
        pl[1].hud.x = 435;
        pl[1].hud.y = 10;
        pl[2].hud.x = 5;
        pl[2].hud.y = 440;
        pl[3].hud.x = 435;
        pl[3].hud.y = 440;
	}
}

//hit the ball
void do_player_hit_ball(PLAYER * pl)
{
	pl->ball.s_old = pl->ball.s;
	pl->ball.angle = pl->ball.f_angle = pl->f_club_angle;
	pl->ball.mode = BALL_MODE_HIT;
	pl->ball.speed = pl->club_power/8.0f;
	pl->ball.v.x = pl->ball.speed*cos(pl->f_club_angle);
	pl->ball.v.y = pl->ball.speed*sin(pl->f_club_angle);
	pl->ball.body.vel = pl->ball.v;
	pl->cur_strokes++;
	vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_STROKES]++;
	if(main_game_vars.player_huds_state == 2 && vgolf_config[CONFIG_RESET_HUDS])
	{
		main_game_vars.player_huds_state = 3; 
	}
}

//swing the club.. (two keypresses required for ball hit)
void do_player_club_swing(PLAYER * pl)
{
	if(pl->club_swing == 1)
	{
		if(pl->club_power < MAX_CLUB_POWER) pl->club_power++;
		else pl->club_swing = 2;
	}
	if(pl->club_swing == 2)
	{
		if(pl->club_power > 0) pl->club_power--;
		else pl->club_swing = 0;
	}

	pl->club_v[0].x = -pl->club_power/2*cos(pl->f_club_angle+pl->f_club_width);
	pl->club_v[0].y = -pl->club_power/2*sin(pl->f_club_angle+pl->f_club_width);
	pl->club_v[1].x = -pl->club_power/2*cos(pl->f_club_angle-pl->f_club_width);
	pl->club_v[1].y = -pl->club_power/2*sin(pl->f_club_angle-pl->f_club_width);

	if(!(vgolf_mouse_b & 1))
	{
		//hit the ball
		do_player_hit_ball(pl);
		//reset the hit power meter
		pl->club_power = 0;
		//we're not swinging anymore..
		pl->club_swing = 0;
		//reset the club vertices
		pl->club_v[0] = pl->club_v[1] = zero_v();
		//play the "ball hitting club" sample
		play_club_ball_hit_sample();
	}
}

//hide the player huds to get a clear view at the playing area, or bring them back
void hide_player_huds(PLAYER pl[], int num_players, char hidden)
{
	int p;

	//move player huds out of the way..
	switch(main_game_vars.player_huds_state)
	{
		case 1:
		{
			for(p = 0; p < 4; p++)
			{
				if(p == 0 || p == 1)
				{
					if(pl[p].hud.y > -55)
					{
						pl[p].hud.y -= 4;
					}
					else
					{
						main_game_vars.player_huds_state = 2; // they're hidden now
					}
				}
				else
				{
					if(pl[p].hud.y < SCREEN_H+15)
					{
						pl[p].hud.y += 4;
					}
					else
					{
						main_game_vars.player_huds_state = 2; // they're hidden now
					}
				}
			}
			break;
		}
		case 3:
		{
			for(p = 0; p < 4; p++)
			{
				if(p == 0 || p == 1)
				{
					if(pl[p].hud.y < 10)
					{
						pl[p].hud.y += 4;
					}
					else
					{
						pl[p].hud.y = 10;
						main_game_vars.player_huds_state = 0;
					}
				}
				else
				{
					if(pl[p].hud.y > 440)
					{
						pl[p].hud.y -= 4;
					}
					else
					{
						pl[p].hud.y = 440;
						main_game_vars.player_huds_state = 0;
					}
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

//get controller input from exiting, pausing, etc.)
void get_main_keys(void)
{
	if(key[KEY_TILDE] && debug_mode)
	{  //toggle debug mode
		main_game_vars.debug_on = !main_game_vars.debug_on;
		key[KEY_TILDE] = 0;
	}
	else if(key[KEY_F12])
	{
		exit(0);
		key[KEY_F12] = 0;
	}
}

//handle the logic of the animal verts
void handle_animal_vert(VERT * vert, HOLE * hole)
{
	int i;

	vert->s.x += vert->ref_no[1]*ANIMAL_MOVE_FACTOR;
	vert->s.y += vert->ref_no[2]*ANIMAL_MOVE_FACTOR;

	//keep animal within original radius..
	if(distance(v_offset(hole->s, vert->s).x, v_offset(hole->s, vert->s).y, vert->ref_no[4], vert->ref_no[5]) > vert->radius)
	{
		vert->s.x -= vert->ref_no[1]*ANIMAL_MOVE_FACTOR;
		vert->s.y -= vert->ref_no[2]*ANIMAL_MOVE_FACTOR;
	}

	//animate animal..
	vert->ref_no[3]++;   //+= timer_change
	if(!(vert->ref_no[3] % ANIMAL_ANIM_TIME))
	{
		if(vert->ref_no[1] > 0) vert->ref_no[0] = 0;
		else vert->ref_no[0] = 2;
	}
	if(!(vert->ref_no[3] % (ANIMAL_ANIM_TIME*2)))
	{
		if(vert->ref_no[1] > 0) vert->ref_no[0] = 1;
		else vert->ref_no[0] = 3;
	}

	//give animal random movement vector..
	if(vert->ref_no[3] >= (ANIMAL_MOVE_TIME-50)+random()%(ANIMAL_MOVE_TIME))
	{
		vert->ref_no[1] = random() % 3;
		i = random()%2;
		if(i) vert->ref_no[1] *= -1;

		vert->ref_no[2] = random() % 3;
		i = random()%2;
		if(i) vert->ref_no[2] *= -1;

		vert->ref_no[3] = 0;
	}
}

//handle the logic of the bubble vert
void handle_bubble_vert(VERT * vert, HOLE * hole)
{
	//inactive, set speed
	if(!vert->ref_no[0])
	{
		vert->ref_no[0] = 1;
		vert->ref_no[1] = (rand()%4)+2;
		vert->ref_no[2] = 0;
		vert->ref_no[3] = 0;
		vert->ref_no[4] = (rand()%16)+12;  //random perpendicular movement
	}

	vert->s.x -= vert->ref_no[1]*CLOUD_MOVE_FACTOR*(vert->flag == VERT_FLAG_BUBBLE_HORIZ);
	vert->s.y -= vert->ref_no[1]*CLOUD_MOVE_FACTOR*(vert->flag == VERT_FLAG_BUBBLE_VERT);

	//handle swaying motion
	if(vert->flag == VERT_FLAG_BUBBLE_HORIZ)
	{
		if(!vert->ref_no[2])
		{
			vert->s.y -= 0.25;
		}
		else
		{
			vert->s.y += 0.25;
		}

		//wrap when edge of screen hit..
		if(hole->s.x+vert->s.x+vert->radius < 0)
		{
			vert->s.x = SCREEN_W+vert->radius;
			vert->s.y = (rand()%(SCREEN_H/2-vert->radius))*(rand()%2 ? -1 : 1);
		}
	}
	else
	{
		if(!vert->ref_no[2])
		{
			vert->s.x -= 0.25;
		}
		else
		{
			vert->s.x += 0.25;
		}

		//wrap when edge of screen hit..
		if(hole->s.y+vert->s.y+vert->radius < 0)
		{
			vert->s.x = (rand()%(SCREEN_W/2-vert->radius))*(rand()%2 ? -1 : 1);
			vert->s.y = SCREEN_H+vert->radius;
		}
	}
	if(++vert->ref_no[3] >= vert->ref_no[4])
	{
		vert->ref_no[3] = 0;
		vert->ref_no[2] = !vert->ref_no[2];
	}
}

//handle the logic of the cloud vert
void handle_cloud_vert(VERT * vert, HOLE * hole)
{

        //inactive, set speed
        if(!vert->ref_no[0]){
                vert->ref_no[0] = 1;
                vert->ref_no[1] = (rand()%4)+2;
                vert->ref_no[2] = 0;
                vert->ref_no[3] = 0;
                vert->ref_no[4] = (rand()%(int)vert->radius/8);
                }

        vert->s.x += vert->ref_no[1]*CLOUD_MOVE_FACTOR*(vert->flag == VERT_FLAG_CLOUD_HORIZ);
        vert->s.y += vert->ref_no[1]*CLOUD_MOVE_FACTOR*(vert->flag == VERT_FLAG_CLOUD_VERT);

        //wrap when edge of screen hit..
        if(hole->s.x+vert->s.x-vert->radius > SCREEN_W){
                vert->s.x = -SCREEN_W/2-vert->radius;
                vert->s.y = (rand()%(SCREEN_H/2-vert->radius))*(rand()%2 ? -1 : 1);
                }
        else if(hole->s.y+vert->s.y-vert->radius > SCREEN_H){
                vert->s.y = SCREEN_H/2-vert->radius;
                vert->s.x = (rand()%(SCREEN_W/2-vert->radius))*(rand()%2 ? -1 : 1);
                }

}

void handle_blinker_vert(VERT * vert, HOLE * hole)
{
	if(!vert->ref_no[0])
	{  //laser inactive, run timer to delay ceiling
		if(++(vert->ref_no[1]) >= vert->ref_no[2])
		{  //keep counting while the laser is being fired (multiply by two to slow it down a little)
			vert->ref_no[1] = 0;     //reset the delay timer
			vert->ref_no[0] = 1;     //reset the activtiy of the laser send and receive verts
		}
	}
	else
	{   //fire laser
		if(++(vert->ref_no[1]) >= vert->ref_no[3])
		{
			vert->ref_no[1] = 0;    //reset the laser delay timer
			vert->ref_no[0] = 0;    //reset the activity of the laser send and receive verts
		}
	}
}

void handle_door_vert(VERT * vert, HOLE * hole){
        if(!vert->ref_no[0]){  //count delay..
                if(++vert->ref_no[1] >= vert->ref_no[2]){
                        vert->ref_no[1] = 0;
                        vert->ref_no[0] = 1;
                        //tag back to door polys
                        if(vert->flag == VERT_FLAG_POLY_DOOR_HORIZ){
                                hole->body[hole->vert[vert->ref_no[4]].ref_no[4]].poly.color = POLY_TYPE_DOOR_L;
                                hole->body[hole->vert[vert->ref_no[4]].ref_no[5]].poly.color = POLY_TYPE_DOOR_R;
                                }
                        else if(vert->flag == VERT_FLAG_POLY_DOOR_VERT){
                                hole->body[hole->vert[vert->ref_no[4]].ref_no[4]].poly.color = POLY_TYPE_DOOR_U;
                                hole->body[hole->vert[vert->ref_no[4]].ref_no[5]].poly.color = POLY_TYPE_DOOR_D;
                                }
                        }
                }
        else if(vert->ref_no[0] == 1){  //move the doors to the center
                if(vert->ref_no[1] < vert->radius){
                        vert->ref_no[1] += vert->ref_no[3];

                        if(vert->flag == VERT_FLAG_POLY_DOOR_HORIZ){
                                //enlarge left door poly toward center
                                hole->body[vert->ref_no[6]].poly.p[2].x += vert->ref_no[3];
                                hole->body[vert->ref_no[6]].poly.p[3].x += vert->ref_no[3];

                                //enlarge right door poly toward center
                                hole->body[vert->ref_no[7]].poly.p[0].x -= vert->ref_no[3];
                                hole->body[vert->ref_no[7]].poly.p[1].x -= vert->ref_no[3];
                                hole->body[vert->ref_no[7]].poly.p[4].x -= vert->ref_no[3];
                                }
                        else if(vert->flag == VERT_FLAG_POLY_DOOR_VERT){
                                //enlarge top door poly toward center
                                hole->body[vert->ref_no[6]].poly.p[0].y += vert->ref_no[3];
                                hole->body[vert->ref_no[6]].poly.p[3].y += vert->ref_no[3];
                                hole->body[vert->ref_no[6]].poly.p[4].y += vert->ref_no[3];

                                //enlarge bottom door poly toward center
                                hole->body[vert->ref_no[7]].poly.p[1].y -= vert->ref_no[3];
                                hole->body[vert->ref_no[7]].poly.p[2].y -= vert->ref_no[3];
                                }
                        }
                else{
                        //set to normal walls now..
                        hole->body[vert->ref_no[6]].poly.color = POLY_TYPE_STD_WALL;
                        hole->body[vert->ref_no[7]].poly.color = POLY_TYPE_STD_WALL;
                        vert->ref_no[0] = 2;
                        }
                }
        else if(vert->ref_no[0] == 2){  //stay closed for this duration
                if(++vert->ref_no[1] >= DEFAULT_DOOR_DURATION){
                        vert->ref_no[0] = 3;
                        vert->ref_no[1] = 0;
                        }
                }
        else if(vert->ref_no[0] == 3){  //stay closed for this duration
                if(vert->ref_no[1] < vert->radius){
                        vert->ref_no[1] += vert->ref_no[3];

                        if(vert->flag == VERT_FLAG_POLY_DOOR_HORIZ){
                                //enlarge left door poly toward center
                                hole->body[vert->ref_no[6]].poly.p[2].x -= vert->ref_no[3];
                                hole->body[vert->ref_no[6]].poly.p[3].x -= vert->ref_no[3];

                                //enlarge right door poly toward center
                                hole->body[vert->ref_no[7]].poly.p[0].x += vert->ref_no[3];
                                hole->body[vert->ref_no[7]].poly.p[1].x += vert->ref_no[3];
                                hole->body[vert->ref_no[7]].poly.p[4].x += vert->ref_no[3];
                                }
                        else if(vert->flag == VERT_FLAG_POLY_DOOR_VERT){
                                //enlarge top door poly toward center
                                hole->body[vert->ref_no[6]].poly.p[0].y -= vert->ref_no[3];
                                hole->body[vert->ref_no[6]].poly.p[3].y -= vert->ref_no[3];
                                hole->body[vert->ref_no[6]].poly.p[4].y -= vert->ref_no[3];

                                //enlarge bottom door poly toward center
                                hole->body[vert->ref_no[7]].poly.p[1].y += vert->ref_no[3];
                                hole->body[vert->ref_no[7]].poly.p[2].y += vert->ref_no[3];
                                }
                        }
                else{
                        vert->ref_no[0] = 0;
                        vert->ref_no[1] = 0;
                        }
                }
}
/* new stuff]] */

//handle the various vert actions, depending on the vert's flag
void handle_vert_action(VERT * vert, HOLE * hole){
        int i;

        switch(vert->flag){
                case VERT_FLAG_ANIMAL_0:
                        handle_animal_vert(vert, hole);
                        break;
                case VERT_FLAG_ANIMAL_1:
                        handle_animal_vert(vert, hole);
                        break;
                case VERT_FLAG_POLY_MORPH:
	                //rotate polygon..
        	        if(vert->ref_no[0] == 0){
                                POLY tmp_poly = hole->body[vert->ref_no[1]].poly;
                                float rot_angle = PI*vert->ref_no[2]/256.0;
                                
                                for(i = 0; i < tmp_poly.num_vertices; i++){
                                        tmp_poly.p[i].x -= vert->s.x;
                                        tmp_poly.p[i].y -= vert->s.y;
                                        rotate_vertex(&tmp_poly.p[i], rot_angle);
                                        }
                                for(i = 0; i < tmp_poly.num_vertices; i++){
                                        tmp_poly.p[i].x += vert->s.x;
                                        tmp_poly.p[i].y += vert->s.y;
                                        }
                                hole->body[vert->ref_no[1]].poly = tmp_poly;
                                }
                        calc_body_normals(&hole->body[vert->ref_no[1]]);
                	break;
		case VERT_FLAG_POLY_MOVE_HORIZ:  //this vert controls a poly that moves horizontally
                	if(!vert->ref_no[0]){  //move to right endpoint
                        	if(vert->ref_no[2] < vert->ref_no[4]){
                                	vert->ref_no[2] += vert->ref_no[5];
                                	translate_poly_x(&hole->body[vert->ref_no[1]].poly, vert->ref_no[5]);
                                	}
                        	else vert->ref_no[0] = 1;
                        	}
                	else{                  //move to left endpoint
                        	if(vert->ref_no[2] > 0){
                                	vert->ref_no[2] -= vert->ref_no[5];
                                	translate_poly_x(&hole->body[vert->ref_no[1]].poly, -vert->ref_no[5]);
                                	}
                        	else vert->ref_no[0] = 0;
                        	}
			break;
		case VERT_FLAG_POLY_MOVE_VERT:  //this vert controls a poly that moves vertically
	                if(!vert->ref_no[0]){  //move to right endpoint
        	                if(vert->ref_no[2] < vert->ref_no[4]){
                	                vert->ref_no[2] += vert->ref_no[5];
                        	        translate_poly_y(&hole->body[vert->ref_no[1]].poly, vert->ref_no[5]);
                                	}
                        	else vert->ref_no[0] = 1;
                        	}
                	else{  //move back to vert center
                        	if(vert->ref_no[2] > 0){
                                	vert->ref_no[2] -= vert->ref_no[5];
                                	translate_poly_y(&hole->body[vert->ref_no[1]].poly, -vert->ref_no[5]);
                                	}
                        	else vert->ref_no[0] = 0;
                        	}
                        break;
		case VERT_FLAG_TELEPORT:      //this vert acts as a teleport pad
                	vert->ref_no[5] = !vert->ref_no[5];  //2-frame animation loop
                	break;
/* [[new stuff */
                case VERT_FLAG_BUBBLE_HORIZ: //handle bubble motion
                case VERT_FLAG_BUBBLE_VERT:
                        handle_bubble_vert(vert, hole);
                        break;
		case VERT_FLAG_LASER_SEND:  //this vert fires a laser to an end-vert
	                if(!vert->ref_no[0]){  //laser inactive, run timer to delay ceiling
        	                if(vert->ref_no[1] < vert->ref_no[2]*2){  //keep counting while the laser is being fired (multiply by two to slow it down a little)
/* [[new stuff */
                                        //vert->ref_no[1] += t;   //keep counting while laser is being fired (multiply by two to slow it down a bit)
                                        vert->ref_no[1]++;
/* new stuff]] (changed the increment to just 1, t seems to make timing eratic..)*/
                        	        }
                        	else{   //set laser to active
                                	vert->ref_no[1] = 0;     //reset the delay timer
	                                vert->ref_no[0] = hole->vert[vert->ref_no[5]].ref_no[0] = 1;   //reset the activtiy of the laser send and receive verts

        	                        //poly that represents laser is set to detect collisions
                	                hole->body[vert->ref_no[4]].poly.color = POLY_TYPE_LASER_WALL;
                        	        }
	                        }
        	        else{   //fire laser
                	        if(vert->ref_no[1] < vert->ref_no[3]*2){
/* [[new stuff */
                                        //vert->ref_no[1] += t;   //keep counting while laser is being fired (multiply by two to slow it down a bit)
                                        vert->ref_no[1]++;
/* new stuff]] */
                                	}
	                        else{   //deactivate laser
        	                        vert->ref_no[1] = 0;    //reset the laser delay timer
                	                vert->ref_no[0] = hole->vert[vert->ref_no[5]].ref_no[0] = 0;   //reset the activity of the laser send and receive verts

                        	        //poly that represents laser is set to ignore collisions
                                	hole->body[vert->ref_no[4]].poly.color = POLY_TYPE_EMPTY_WALL;
                                	}
                        	}
                	break;
	        case VERT_FLAG_CONVEYOR:   //animate the conveyor belt
                	if(vert->ref_no[1]){
                        	if(vert->ref_no[4] < vert->ref_no[2]*4-1)
                        	{
/* [[new stuff */
                                        //vert->ref_no[4] += t;
                                        vert->ref_no[4]++;
/* new stuff]] */
							}
                        	else
                        	{
	                        	vert->ref_no[4] = 0;
                        	}
                        	}
                	break;
/* [[new stuff */
                case VERT_FLAG_POLY_DOOR_HORIZ:  //this vert controls a dual poly door
                case VERT_FLAG_POLY_DOOR_VERT:  //this vert controls a dual poly door
                        handle_door_vert(vert, hole);
                        break;
                case VERT_FLAG_CLOUD_HORIZ: //handle the cloud motion and anims
                case VERT_FLAG_CLOUD_VERT:  //animate the conveyor belt
                        handle_cloud_vert(vert, hole);
                        break;
                case VERT_FLAG_BLINKER:     //this vert is a blinking light
                        handle_blinker_vert(vert, hole);
                        break;
/* new stuff]] */
                default:
                        break;
                }
}

//vert logic..
void vert_logic(VERT * vert, HOLE * hole){
        handle_vert_action(vert, hole);
}

//hole logic..
void hole_logic(HOLE * hole)
{
	int v;

	for(v = 0; v < hole->num_verts; v++)
	{
		vert_logic(&hole->vert[v], hole);
	}
}

//get main in-game input (for exlayer whose ball is furthest from the hole
int furthest_player_from_hole(PLAYER pl[], int num_players, HOLE * hole)
{
	int p;
	int hole_vert = 0;
	int nearest = -1;

	for(p = 0; p < 4; p++)
	{
		if(vgolf_player_list[p])
		{
			if(pl[p].ball.mode != BALL_MODE_SUNK)
			{
				nearest = p;
				break;
			}
		}
	}
	if(nearest == -1) return nearest;

	for(p = 0; p < hole->num_verts; p++)
	{
		if(hole->vert[p].flag == VERT_FLAG_HOLE)
		{
			hole_vert = p;
			break;
		}
	}
	for(p = 1; p < 4; p++)
	{
		if(vgolf_player_list[p])
		{
			if(v_distance(pl[p].ball.s, v_offset(hole->s, hole->vert[hole_vert].s)) > v_distance(pl[nearest].ball.s, v_offset(hole->s, hole->vert[hole_vert].s)) && pl[p].ball.mode != BALL_MODE_SUNK)
			{
				nearest = p;
			}
		}
	}
	return nearest;
}

/* returns which player is next
   if all players done, return -1 */
int vgolf_get_next_player(void)
{
	int p = main_game_vars.cur_player;
	
	while(1)
	{
		p++;
		if(p > 3)
		{
			p = 0;
		}
		if(p == main_game_vars.cur_player)
		{
			return -1;
		}
		if(vgolf_player_list[p] && game_player[p].ball.mode != BALL_MODE_SUNK)
		{
			return p;
		}
	}
	return -1;
}

/* return which player got the best score */
int vgolf_get_best_player(void)
{
	int best = -1;
	int best_score = 10000;
	int i;
	
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			if(game_player[i].cur_strokes < best_score)
			{
				best = i;
				best_score = game_player[i].cur_strokes;
			}
		}
	}
	
	return best;
}

void build_turn_text(int p)
{
	int r = rand() % 4;
	
	switch(r)
	{
		case 0:
		{
			sprintf(temp_text, "You're up, %s!", vgolf_profiles.item[game_player[p].profile].name);
			break;
		}
		case 1:
		{
			sprintf(temp_text, "%s, it's your turn.", vgolf_profiles.item[game_player[p].profile].name);
			break;
		}
		case 2:
		{
			sprintf(temp_text, "Take your shot, %s!", vgolf_profiles.item[game_player[p].profile].name);
			break;
		}
		case 3:
		{
			sprintf(temp_text, "It's your turn, %s.", vgolf_profiles.item[game_player[p].profile].name);
			break;
		}
	}
}

//finish the current player's turn, go to the next player (if there are others)
void do_player_turn_finish(PLAYER * pl, HOLE * hole)
{
	/* if we're close to the hole play the sounds */
	if(pl->so_close && pl->ball.hole_vert >= 0)
	{
		add_so_close_message();
		play_ahhh_sample();
		pl->so_close = 0;
  	}
	if(vgolf_player_count > 1)
	{
		int p_no = vgolf_get_next_player();

		if(p_no != -1)
		{
			main_game_vars.cur_player = p_no;
			build_turn_text(p_no);
			add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, temp_text, SCREEN_W / 2 - strlen(temp_text) * game_vfont.w * 2, SCREEN_H / 2 - game_vfont.h * 2, 0, VMSG_TEXT_ENTER_FROMRIGHT, -10, 4, 0, 0, -1);
		}
	}
}

//either seg to next hole, or let the next player in line take their turn
void do_player_hole_finish(PLAYER * pl, HOLE * hole)
{
	if(vgolf_player_count == 1)
	{
		if(!main_game_vars.cur_vmessage_slot)
		{
			//begin seg to scoreboard
			main_game_vars.game_state = GAMESTATE_HOLE_FINISHED;
			main_game_vars.hole_finished = 1;
			pl->pic_mode = 0;
			
		}
	}
	else
	{
	  	main_game_vars.game_state = GAMESTATE_PLAYER_FINISHED_HOLE;
	}
}

//move the ball around
void move_player_ball(BALL * ball, PLAYER * pl, HOLE * hole)
{
	if(ball->off_conveyor)
	{
		/* recalculate speed based on velocity vector */
		ball->speed = distance(0, 0, ball->v.x + ball->cf.x, ball->v.y + ball->cf.y);
		ball->f_angle = atan2(ball->v.y + ball->cf.y, ball->v.x + ball->cf.x);
		ball->cf.x = 0;
		ball->cf.y = 0;
	}
	else
	{
		/* recalculate speed based on velocity vector */
		ball->speed = distance(0, 0, ball->v.x, ball->v.y);	
	}
	
	/* decelerate ball */
	//keep slowing the ball down until its speed is 0
	if(ball->speed > 0)
	{
		if(ball->decel_type == 0)
		{
			ball->speed -= hole->deccel_factor;
		}
		else if(ball->decel_type == 1)
		{
			ball->speed -= hole->deccel_factor * 2;
		}
		else if(ball->decel_type == 2)
		{
			ball->speed -= hole->deccel_factor * 3;
		}
		if(ball->speed < 0)
		{
			ball->speed = 0;
			check_ball_hole_collision(ball, pl, hole);  //check for collisions between ball and walls
			if(ball->mode != BALL_MODE_SUNK && ball->mode != BALL_MODE_WATER_SINKING && ball->mode != BALL_MODE_IN_MOTION_AREA && ball->mode != BALL_MODE_LASER_ZAPPED && ball->mode != BALL_MODE_ON_CONVEYOR)
			{
					ball->mode = BALL_MODE_IDLE;
					ball->idle_timer = 0;
					do_player_turn_finish(pl, hole);
			}
		}
	}
	else
	{
		//set speed to 0
		ball->speed = 0;

		check_ball_hole_collision(ball, pl, hole);  //check for collisions between ball and walls
		
		//get ball ready for another hit, finish this player's turn
		if(ball->mode != BALL_MODE_SUNK && ball->mode != BALL_MODE_WATER_SINKING && ball->mode != BALL_MODE_IN_MOTION_AREA && ball->mode != BALL_MODE_LASER_ZAPPED && ball->mode != BALL_MODE_ON_CONVEYOR)
		{
				ball->mode = BALL_MODE_IDLE;
				ball->idle_timer = 0;
				do_player_turn_finish(pl, hole);
		}
		//ball is currently within a motion area, do the idle timer
		else if(ball->mode == BALL_MODE_IN_MOTION_AREA)
		{
			if(ball->mode2 == 0)
			{
				//if(ball->idle_timer < ball->idle_timeout) ball->idle_timer += main_game_vars.ingame_timer_change;
				//make sure ball is within the motion area before handling the idle timer
				if(check_circle_collision(ball->s, v_offset(hole->s, hole->vert[ball->cur_vert].s), ball->radius, hole->vert[ball->cur_vert].radius))
				{
					//handle the idle timer
					if(++ball->idle_timer >= ball->idle_timeout)
					{
						ball->idle_timer = 0;
						ball->mode = BALL_MODE_IDLE;
						do_player_turn_finish(pl, hole);
					}
				}
				else
				{
					ball->idle_timer = 0;
					ball->mode = BALL_MODE_IDLE;
					do_player_turn_finish(pl, hole);
				}
			}
			else if(ball->mode2 == 1)
			{
				if(++ball->idle_timer >= ball->idle_timeout * 2)
				{
					ball->idle_timer = 0;
					ball->mode = BALL_MODE_IDLE;
					do_player_turn_finish(pl, hole);
				}
			}
		}
		//ball is on a conveyor belt..
		else if(ball->mode == BALL_MODE_ON_CONVEYOR)
		{
			if(!check_circle_collision(ball->s, v_offset(hole->s, hole->vert[ball->cur_vert].s), ball->radius, hole->vert[ball->cur_vert].radius))
			{
				ball->mode = BALL_MODE_IDLE;
				do_player_turn_finish(pl, hole);
			}
			else
			{
				ball->mode = BALL_MODE_ON_CONVEYOR;
			}
		}
		else
		{
			ball->idle_timer = 0;
		}
		//reset swing coin counter
		pl->coins_current = 0;
	}
	
	ball->s.x += ball->v.x + ball->cf.x;
	ball->s.y += ball->v.y + ball->cf.y;
	ball->v.x = ball->speed*cos(ball->f_angle) + ball->f.x;
	ball->v.y = ball->speed*sin(ball->f_angle) + ball->f.y;
	if(!ball->off_conveyor)
	{
		ball->f_angle = atan2(ball->v.y, ball->v.x);
	}
	else
	{
		ball->off_conveyor = 0;
	}
	ball->body.pos = ball->s;
	ball->body.vel = ball->v;
    //reset ball's force vector
	ball->f.x = ball->f.y = 0;                                
}

//slow the ball down
//TODO: might need some work..
void decell_ball(BALL * ball, PLAYER * pl, HOLE * hole)
{
}

//hit the ball from the previous hit position
void do_ball_rehit(BALL * ball, HOLE * hole)
{
	ball->s = ball->s_old;        //place the ball at the last position it was hit from
	ball->visible = 1;            //make ball visible
	ball->mode = BALL_MODE_IDLE;  //reset ball mode
	ball->idle_timer = 0;         //reset idle timer if needed
}

//drop the ball to the edge of the offending vert (water, ob, etc)
//TODO: make ball drop near the closest edge..
void do_ball_drop(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole)
{
	int ang = abs(64 - fixtoi(fatan2(ftofix((hole->s.x + vert->s.x) - ball->s.x), ftofix((hole->s.y + vert->s.y) - ball->s.y))) + 256 * (ball->s.x < vert->s.x + hole->s.x && ball->s.y > vert->s.y + hole->s.y));
	ball->s.x = hole->s.x+vert->s.x-(vert->radius+ball->radius)*fixtof(fcos(itofix(ang)));
	ball->s.y = hole->s.y+vert->s.y-(vert->radius+ball->radius)*fixtof(fsin(itofix(ang)));
	ball->visible = 1;                //make ball visible again
	ball->mode = BALL_MODE_IDLE;      //reset ball mode for next hit
	pl->coins_current = 0;            //reset coin counter
	do_player_turn_finish(pl, hole);  //finish the player's turn
}

//drop the ball on the nearest laser respawn pad (after zap)
void do_ball_laser_drop(BALL * ball, PLAYER * pl, HOLE * hole)
{
	int v;
	int dist = 0;
	int cur_slot = 0;
	
	for(v = 0; v < hole->num_verts; v++)
	{
		//check for a laser respawn position vert
		if(hole->vert[v].flag == VERT_FLAG_LASER_RESPAWN)
		{
			//find the distance between the ball and this vert..
			dist = v_distance(ball->s, v_offset(hole->s, hole->vert[v].s));
			//if this is the least distance of the currently found respawn positions, set the current slot to this vert
			if(dist < v_distance(ball->s, v_offset(hole->s, hole->vert[cur_slot].s)))
			{
				cur_slot = v;
			}
		}
	}
	//place the ball at the closest laser respawn position
	ball->s = v_offset(hole->s, hole->vert[cur_slot].s);
	//make ball visible again
	ball->visible = 1;
	//reset ball mode
	ball->mode = BALL_MODE_IDLE;
	//make player content again
	pl->pic_mode = 0;
}

//laser destroys ball
void do_ball_laser_zap(BALL * ball, PLAYER * pl, HOLE * hole)
{
	ball->speed = 0;                      //set ball speed to 0
	ball->v.x = ball->v.y = 0;
	ball->mode = BALL_MODE_LASER_ZAPPED;  //set ball to laser zap mode
	ball->visible = 0;                    //set ball invisible
	pl->cur_strokes++;                    //add another stroke to the player stroke total
	vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_STROKES]++;
	
	//play the zap sound effect
	play_ball_zap_sample();
	
	//add some random message to the queue
	add_ball_zap_message();
	
	pl->pic_mode = 2;                     //set player pic to "ouch" mode
}

/* [[new stuff */
//drop the ball on the nearest smash respawn pad (after smash)
void do_ball_smash_drop(BALL * ball, PLAYER * pl, HOLE * hole)
{
	int v;
	int dist = 0;
	int cur_slot = 0;

	for(v = 0; v < hole->num_verts; v++)
	{
		//check for a smash respawn position vert
		if(hole->vert[v].flag == VERT_FLAG_SMASH_RESPAWN)
		{
			//find the distance between the ball and this vert..
			dist = v_distance(ball->s, v_offset(hole->s, hole->vert[v].s));
			//if this is the least distance of the currently found respawn positions, set the current slot to this vert
			if(dist < v_distance(ball->s, v_offset(hole->s, hole->vert[cur_slot].s)))
			{
				cur_slot = v;
			}
		}
	}
	//place the ball at the closest laser respawn position
	ball->s = v_offset(hole->s, hole->vert[cur_slot].s);
	//make ball visible again
	ball->visible = 1;
	//reset idle timer
	ball->idle_timer = 0;
	//reset ball mode
	ball->mode = BALL_MODE_IDLE;
	//make player content again
	pl->pic_mode = 0;
}

//laser destroys ball
void do_ball_smash(BALL * ball, PLAYER * pl, HOLE * hole)
{
	ball->speed = 0;                      //set ball speed to 0
	ball->v.x = ball->v.y = 0;
	ball->mode = BALL_MODE_DOOR_SMASHED;  //set ball to door smash mode
	ball->visible = 0;                    //set ball invisible
	pl->cur_strokes++;                    //add another stroke to the player stroke total
	vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_STROKES]++;

	//play the zap sound effect
	play_ball_smash_sample();

	//add some random message to the queue
	add_ball_smash_message();

	pl->pic_mode = 2;                     //set player pic to "ouch" mode
}
/* new stuff]] */

//ball sinks into water hazard
void do_ball_water_sink(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole)
{
	ball->speed = 0;                       //set speed to zero
	ball->mode = BALL_MODE_WATER_SINKING;  //ball is sinking (now we can animate it)
	ball->visible = 0;                     //ball is invisible temporarily
	pl->cur_strokes++;                     //add another stroke to the player's current total
	vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_STROKES]++;

	//play the "splash" sample
	play_ball_splash_sample();

	//add the commentary message, and the prompt message
	add_ball_water_sunk_messages();        

	pl->pic_mode = 2;    //player is unhappy
}

//ball enters the teleport pad, send ball to linked teleport pad
void do_ball_teleport(BALL * ball, VERT * vert, HOLE * hole)
{
	//play the teleport sound
	play_ball_teleport_sample();
	
	vert->ref_no[4] = hole->vert[vert->ref_no[0]].ref_no[4] = 0;
	ball->angle = vert->ref_no[1];
	ball->f_angle = PI*ball->angle/128.0f;
	ball->s = v_offset(hole->s, hole->vert[vert->ref_no[0]].s);

	//set ball so it sits outside the pad, to avoid re-teleporting
	ball->s.x += (hole->vert[vert->ref_no[0]].radius+ball->radius)*cos(ball->f_angle);
	ball->s.y += (hole->vert[vert->ref_no[0]].radius+ball->radius)*sin(ball->f_angle);

	//make ball visible again
	ball->visible = 1;
	//set ball activity level to hit mode so it can keep going after re-appearing
	ball->mode = BALL_MODE_HIT;
}

//ball enters the pipe.. start counter, when counter ends, ball pops out other end
void do_ball_pipe_enter(BALL * ball, VERT * vert, HOLE * hole)
{

	//if the ball isn't currently in pipe enter mode, set it
	if(ball->mode != BALL_MODE_PIPE_ENTER)
	{
		ncds_play_sample(main_game_vars.sound_bank.sfx_ball_enter, 128, 1000, 0);
		vert->ref_no[5] = (int)(ball->speed*100);   //record the current speed
		ball->speed = 0;                            //stop the ball temporarily
		ball->mode = BALL_MODE_PIPE_ENTER;
		ball->visible = 0;
		ball->mirage_visible = 1;
		ball->s_med = v_offset(hole->s, vert->s);
	}

	if(++vert->ref_no[2] >= vert->ref_no[3])
	{
		vert->ref_no[2] = 0;
		//ball->mode = BALL_MODE_PIPE_EXIT;
		ball->angle = vert->ref_no[4];
		ball->f_angle = PI*ball->angle/128.0;   //convert the 0-256 angle to 0-2PI angle
		ball->s = v_offset(hole->s, hole->vert[vert->ref_no[1]].s);
		ball->speed = vert->ref_no[5]/100.0;    //get the old speed back
		ball->mode = BALL_MODE_HIT;             //go back to normal hit mode
		ball->visible = 1;
		ncds_play_sample(main_game_vars.sound_bank.sfx_ball_exit, 128, 1000, 0);
	}
}

//ball sinks in the cup..
void do_ball_hole_sink(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole)
{
	ball->speed = 0;
	ball->mode = BALL_MODE_SUNK;
	ball->visible = 0;
	ball->s = v_offset(hole->s, vert->s);
	vert->ref_no[0] = 1;
        
	//play the cup sink sample
	play_ball_holesink_sample();

	//player got a hole-in-one
	if(pl->cur_strokes == 1)
	{
		//play a hole-in-one sample
		play_hole_in_one_sample();
	        
		//add the hole-in-one message to the queue
		add_hole_in_one_message();
		vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_HOLES_IN_ONE]++;
	}
	//player got par
	if(!(hole->par_score-pl->cur_strokes))
	{
		//plays sample for a good finish
		play_good_finish_sample();
	        
		//add the par finish messages to the message queue
		add_par_finish_messages();
	        
		pl->pic_mode = 1;              //player is happy
		vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_PARS]++;
	}
	//player got birdie
	else if(hole->par_score-pl->cur_strokes == 1)
	{
		//plays sample for a good finish
		play_good_finish_sample();
	        
		//add the birdie finish messages to the message queue
		add_birdie_finish_messages();
	        
		pl->scorecard.num_birdies++;  //increment player's birdie count
		pl->pic_mode = 1;             //player is happy
		vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_BIRDIES]++;
	}
	//player got eagle
	else if(hole->par_score-pl->cur_strokes == 2)
	{
		//plays sample for a good finish
		play_good_finish_sample();
	        
		//add the eagle finish messages to the message queue
		add_eagle_finish_messages();
	        
		pl->scorecard.num_eagles++;  //increment player's eagle count
		pl->pic_mode = 1;            //player is happy
		vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_EAGLES]++;
	}
	//player got albatross
	else if(hole->par_score-pl->cur_strokes >= 3)
	{
		//play an "albatross" sample
		play_hole_in_one_sample();
	        
		//add the albatross finish messages to the message queue
		add_albatross_finish_messages();
	        
		pl->scorecard.num_albo++;  //increment player's albatross count
		pl->pic_mode = 1;          //player is happy
		vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_ALBATROSSES]++;
	}
	//player got bogey
	else if(hole->par_score-pl->cur_strokes == -1)
	{
		//plays a "bad finish" sample
		play_poor_finish_sample();
	        
		//add the bogey finish messages to the message queue
		add_bogey_finish_messages();
	        
		pl->pic_mode = 3;          //player is upset..
	}
	//player got worse than bogey
	else if(hole->par_score-pl->cur_strokes < -1)
	{
		//plays a "bad finish" sample
		play_poor_finish_sample();
	        
		//add the worse-than-bogey finish messages to the message queue
		add_under_bogey_finish_messages(hole->par_score-pl->cur_strokes);
	        
		pl->pic_mode = 3;         //player makes an upset face
	}
                
	vgolf_profiles.item[pl->profile].total.data[VGOLF_PROFILE_DATA_HOLES]++;
	
	//add the latest player tallies to his scorecard
	pl->scorecard.score[main_game_vars.cur_hole] = pl->cur_strokes;
	pl->scorecard.par_score[main_game_vars.cur_hole] = hole->par_score;
	
	/* update profile course info */
	if(vgolf_profiles.item[pl->profile].course_info[selected_course].score[main_game_vars.cur_hole] == 0 || vgolf_profiles.item[pl->profile].course_info[selected_course].score[main_game_vars.cur_hole] > pl->cur_strokes)
	{
		vgolf_profiles.item[pl->profile].course_info[selected_course].score[main_game_vars.cur_hole] = pl->cur_strokes;
	}
}

//does the vert's reaction to the ball being nearby
void do_ball_vert_reaction(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole, int v_no)
{
	switch(vert->flag)
	{
		case VERT_FLAG_HOLE:      //the cup..
		{
			//make sure the ball isn't going too fast..
			if(ball->speed < BALL_MAX_HOLEHIT_SPEED)
			{
				//make sure the ball is within a certain distance of the cup's center
				if(v_distance(ball->s, v_offset(hole->s, vert->s)) < ball->radius)
				{
					//drop the ball in the cup
					do_ball_hole_sink(ball, pl, vert, hole);
					pl->so_close = 0;
				}
				else
				{
					pl->so_close = 1;
				}
			}
			//ball is really close to the hole, but not in the cup..
			ball->hole_vert = v_no;
			break;
		}
		case VERT_FLAG_HAZARD_WATER:   //this vert is a circular water hole
		{
			if(v_distance(ball->s, v_offset(hole->s, vert->s)) < vert->radius)
			{
				do_ball_water_sink(ball, pl, vert, hole);
			}
			ball->cur_vert = v_no;
			ball->water_vert = v_no;
			break;
		}
		case VERT_FLAG_PIPE_ENTER:     //this vert just teleports the ball to another vert position after a delay
		{
			if(check_circle_collision(ball->s, v_offset(hole->s, vert->s), ball->radius, vert->radius-2))
			{
				do_ball_pipe_enter(ball, vert, hole);
			}
			break;
		}
		case VERT_FLAG_PIPE_EXIT:
		{
			break;
		}
		case VERT_FLAG_INDENT_HILL:   //this vert is a circular hill that moves the ball away from its center
		{
			//find angle between ball and the hill's center point
			float ang = atan2(ball->s.y - (hole->s.y + vert->s.y), ball->s.x - (hole->s.x + vert->s.x));
			float dist = sqrt((ball->s.x - (hole->s.x + vert->s.x)) * (ball->s.x - (hole->s.x + vert->s.x)) + (ball->s.y - (hole->s.y + vert->s.y)) * (ball->s.y - (hole->s.y + vert->s.y)));
			if(dist < 0.5)
			{
				dist = 0.5;
			}
			float rad = vert->radius;
			float hill_f = dist / rad / 10.0;  //hill force
	                  	
			//add the force to the ball's force vector
			ball->f.x = hill_f * cos(ang);
			ball->f.y = hill_f * sin(ang);
			break;
		}
		case VERT_FLAG_INDENT_RUT:    //this vert is a circular rut that moves the ball toward its center
		{
			//find angle between ball and the hill's center point
			float ang = atan2(ball->s.y - (hole->s.y + vert->s.y), ball->s.x - (hole->s.x + vert->s.x));
			float dist = sqrt((ball->s.x - (hole->s.x + vert->s.x)) * (ball->s.x - (hole->s.x + vert->s.x)) + (ball->s.y - (hole->s.y + vert->s.y)) * (ball->s.y - (hole->s.y + vert->s.y)));
			if(dist < 5)
			{
				dist = 5;
			}
			float rad = vert->radius;
			float hill_f = dist / rad / 10.0;  //hill force
	                  	
			//add the force to the ball's force vector
			ball->f.x = -hill_f*cos(ang);
			ball->f.y = -hill_f*sin(ang);
			break;
		}
		case VERT_FLAG_HAZARD_SAND:   //this vert is a circle of sand that slows the ball down
		{
			if(ball->speed > 0) ball->decel_type = 1;
			break;
		}
		case VERT_FLAG_HAZARD_SLIME:  //this vert is a circle of slime that slows the ball down even more
		{
			if(ball->speed > 0) ball->decel_type = 2;
			break;
		}
		case VERT_FLAG_POLY_MOTION_AREA:   //this vert defines a motion area that surrounds a moving polygon
		{
			ball->cur_vert = v_no;

			//horizontally moving poly
			if(vert->ref_no[0] == 1)
			{
				RECT temp_rect;    //create a temp rect and check to see if ball is touching it
				temp_rect.x1 = vert->s.x-vert->radius;
				temp_rect.y1 = vert->s.y-vert->ref_no[1];
				temp_rect.x2 = vert->s.x+vert->radius;
				temp_rect.y2 = vert->s.y+vert->ref_no[2];

				//if the rects touch, keep the ball "hit" so it stays here until the poly contacts it again (moving it out of the space)
				if(rect_collision(ball->hit_rect, temp_rect, ball->s, hole->s))
				{
					ball->mode = BALL_MODE_IN_MOTION_AREA;
					ball->mode2 = 0;
				}
				else ball->mode = BALL_MODE_HIT;
			}

			//vertically moving poly
			else if(vert->ref_no[0] == 2)
			{
				RECT temp_rect;    //create a temp rect and check to see if ball is touching it
				temp_rect.x1 = vert->s.x-vert->ref_no[1];
				temp_rect.y1 = vert->s.y-vert->radius;
				temp_rect.x2 = vert->s.x+vert->ref_no[2];
				temp_rect.y2 = vert->s.y+vert->radius;

				//if the rects touch, keep the ball "hit" so it stays here until the poly contacts it again (moving it out of the space)
				if(rect_collision(ball->hit_rect, temp_rect, ball->s, hole->s))
				{
					ball->mode = BALL_MODE_IN_MOTION_AREA;
					ball->mode2 = 0;
				}
				else
				{
					ball->mode = BALL_MODE_HIT;
				}
			}

			//horizontally moving door poly
			else if(vert->ref_no[0] == 3)
			{
				//only check when doors are closing..
				if(hole->vert[vert->ref_no[3]].ref_no[0] <= 1)
				{
					RECT temp_rect;    //create a temp rect and check to see if ball is touching it
					temp_rect.x1 = vert->s.x-vert->radius+hole->vert[vert->ref_no[3]].ref_no[1]*hole->vert[vert->ref_no[3]].ref_no[0];
					temp_rect.y1 = vert->s.y-vert->ref_no[2];
					temp_rect.x2 = vert->s.x+vert->radius-hole->vert[vert->ref_no[3]].ref_no[1]*hole->vert[vert->ref_no[3]].ref_no[0];
					temp_rect.y2 = vert->s.y+vert->ref_no[2];

					//if the rects touch, keep the ball "hit" so it stays here until the poly contacts it again (moving it out of the space)
					if(rect_collision(ball->hit_rect, temp_rect, ball->s, hole->s))
					{
						RECT rect_l, rect_r;    //create a temp rect for the left and right doors
						rect_l.x1 = vert->s.x-vert->radius; rect_l.y1 = vert->s.y-vert->ref_no[2];
						rect_l.x2 = vert->s.x-vert->radius+hole->vert[vert->ref_no[3]].ref_no[1]*hole->vert[vert->ref_no[3]].ref_no[0]+hole->vert[vert->ref_no[3]].ref_no[3]+ball->radius; rect_l.y2 = vert->s.y+vert->ref_no[2];
						rect_r.x1 = vert->s.x+vert->radius-hole->vert[vert->ref_no[3]].ref_no[1]*hole->vert[vert->ref_no[3]].ref_no[0]-hole->vert[vert->ref_no[3]].ref_no[3]-ball->radius; rect_r.y1 = vert->s.y-vert->ref_no[2];
						rect_r.x2 = vert->s.x+vert->radius; rect_r.y2 = vert->s.y+vert->ref_no[2];

						//if ball touching both walls, smash..
						if(hole->vert[vert->ref_no[3]].ref_no[0] == 1)
						{
							if(rect_collision(rect_l, ball->hit_rect, hole->s, ball->s) && rect_collision(rect_r, ball->hit_rect, hole->s, ball->s))
							{
								ball->s.x = hole->s.x+vert->s.x;
								do_ball_smash(ball, pl, hole);
							}
							//if ball is touching left wall, cling to left wall while it closes
							else if(rect_collision(rect_l, ball->hit_rect, hole->s, ball->s))
							{
								ball->s.x = hole->s.x+rect_l.x2+ball->radius;
							}
							//if ball is touching rt wall, cling to rt wall while it closes
							else if(rect_collision(rect_r, ball->hit_rect, hole->s, ball->s))
							{
								ball->s.x = hole->s.x+rect_r.x1-ball->radius;
							}
							else
							{
								ball->mode = BALL_MODE_IN_MOTION_AREA;
								ball->mode2 = 0;
							}
						}
						else
						{
							ball->mode = BALL_MODE_IN_MOTION_AREA;
							ball->mode2 = 0;
						}
					}
					else ball->mode = BALL_MODE_HIT;
				}
				else ball->mode = BALL_MODE_HIT;
			}

			//vertically moving door poly
			else if(vert->ref_no[0] == 4)
			{
				//only check when doors are closing..
				if(hole->vert[vert->ref_no[3]].ref_no[0] <= 1)
				{
					RECT temp_rect;    //create a temp rect and check to see if ball is touching it
					temp_rect.x1 = vert->s.x - vert->ref_no[2];
					temp_rect.y1 = vert->s.y - vert->radius + hole->vert[vert->ref_no[3]].ref_no[1] * hole->vert[vert->ref_no[3]].ref_no[0];
					temp_rect.x2 = vert->s.x + vert->ref_no[2];
					temp_rect.y2 = vert->s.y + vert->radius - hole->vert[vert->ref_no[3]].ref_no[1] * hole->vert[vert->ref_no[3]].ref_no[0];

					//if the rects touch, keep the ball "hit" so it stays here until the poly contacts it again (moving it out of the space)
					if(rect_collision(ball->hit_rect, temp_rect, ball->s, hole->s))
					{
						RECT rect_l, rect_r;    //create a temp rect for the left and right doors
						rect_l.x1 = vert->s.x - vert->ref_no[2]; rect_l.y1 = vert->s.y - vert->radius;
						rect_l.x2 = vert->s.x + vert->ref_no[2]; rect_l.y2 = vert->s.y - vert->radius + hole->vert[vert->ref_no[3]].ref_no[1] * hole->vert[vert->ref_no[3]].ref_no[0] + hole->vert[vert->ref_no[3]].ref_no[3] + ball->radius;
						rect_r.x1 = vert->s.x - vert->ref_no[2]; rect_r.y1 = vert->s.y + vert->radius - hole->vert[vert->ref_no[3]].ref_no[1] * hole->vert[vert->ref_no[3]].ref_no[0] - hole->vert[vert->ref_no[3]].ref_no[3] - ball->radius;
						rect_r.x2 = vert->s.x + vert->ref_no[2]; rect_r.y2 = vert->s.y + vert->radius;

						if(hole->vert[vert->ref_no[3]].ref_no[0] == 1)
						{
							//if ball touching both walls, smash..
							if(rect_collision(rect_l, ball->hit_rect, hole->s, ball->s) && rect_collision(rect_r, ball->hit_rect, hole->s, ball->s))
							{
								ball->s.y = hole->s.y+vert->s.y;
								do_ball_smash(ball, pl, hole);
							}
							//if ball is touching left wall, cling to left wall while it closes
							else if(rect_collision(rect_l, ball->hit_rect, hole->s, ball->s))
							{
								ball->s.y = hole->s.y+rect_l.y2+ball->radius;
							}
							//if ball is touching rt wall, cling to rt wall while it closes
							else if(rect_collision(rect_r, ball->hit_rect, hole->s, ball->s))
							{
								ball->s.y = hole->s.y+rect_r.y1-ball->radius;
							}
							else
							{
								ball->mode = BALL_MODE_IN_MOTION_AREA;
								ball->mode2 = 0;
							}
						}
						else
						{
							ball->mode = BALL_MODE_IN_MOTION_AREA;
							ball->mode2 = 0;
						}
					}
					else ball->mode = BALL_MODE_HIT;
				}
				else ball->mode = BALL_MODE_HIT;
			}
			break;
		}
		case VERT_FLAG_POLY_MORPH:
		{
			ball->cur_vert = v_no;
			if(vert->ref_no[0] == 0)
			{
				ball->mode = BALL_MODE_IN_MOTION_AREA;
				ball->mode2 = 0;
			}
			break;
		}
		case VERT_FLAG_TELEPORT:
		{
			if(vert->ref_no[4] == 0)
			{  //if active flag isn't set, do teleport
				if(v_distance(ball->s, v_offset(hole->s, vert->s)) < ball->radius)
				{  //make sure the ball is close enough to the center of the teleport pad
					ball->visible = 0;       //set ball to visible for a second
					do_ball_teleport(ball, vert, hole);  //teleport the ball to the specified teleport pad
				}
			}
			break;
		}
		case VERT_FLAG_CONVEYOR:
		{
			ball->cur_vert = v_no;
			if((ball->conveyor_vert == -1 || ball->conveyor_vert == v_no) && point_in_poly(ball->s, hole->body[vert->ref_no[5]].poly, hole->s))
			{
				//if(rect_collision(ball->hit_rect, temp_rect, ball->s, hole->s)){
				if(vert->ref_no[0] == 0)
				{
					ball->cf.x = 0;
					ball->cf.y = -vert->ref_no[2]/2.0;
					//if(ball->angle < 64) ball->angle++;
					//else if(ball->angle > 64) ball->angle--;
				}
				else if(vert->ref_no[0] == 1)
				{
					ball->cf.x = 0;
					ball->cf.y = vert->ref_no[2]/2.0;
					//if(ball->angle < 192) ball->angle++;
					//else if(ball->angle > 192) ball->angle--;
				}
				else if(vert->ref_no[0] == 2)
				{
					ball->cf.y = 0;
					ball->cf.x = -vert->ref_no[2]/2.0;
					//if(ball->angle > 0) ball->angle--;
				}
				else if(vert->ref_no[0] == 3)
				{
					ball->cf.y = 0;
					ball->cf.x = vert->ref_no[2]/2.0;
					//if(ball->angle < 128) ball->angle++;
					//else if(ball->angle > 128) ball->angle--;
				}
				ball->mode = BALL_MODE_ON_CONVEYOR;
				ball->conveyor_vert = v_no;
			}
			else if(ball->conveyor_vert == v_no)
			{
				/* make sure ball keeps momentum from conveyor belt	after rolling off */
				if(ball->mode == BALL_MODE_ON_CONVEYOR)
				{
					ball->off_conveyor = 1;
					ball->conveyor_vert = -1;
				}
				ball->mode = BALL_MODE_HIT;
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

void undo_ball_vert_reaction(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole, int v_no)
{
	switch(vert->flag)
	{
		case VERT_FLAG_INDENT_HILL:   //this vert is a circular hill that moves the ball away from its center
		case VERT_FLAG_INDENT_RUT:    //this vert is a circular rut that moves the ball toward its center
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

//check to see if the ball is within the radius of the vert..
int check_ball_vert_collision(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole, int v_no)
{
	
	//check for a collision between rectangles surrounding the ball and this vert first (might decrease # of calculations in the long run)
	if(rect_collision_f(-ball->radius, -ball->radius, ball->radius, ball->radius, -vert->radius, -vert->radius, vert->radius, vert->radius, ball->s, v_offset(hole->s, vert->s)))
	{
		//check for a collision between the ball circle and the vert circle
		if(check_circle_collision(ball->s, v_offset(hole->s, vert->s), ball->radius, vert->radius))
		{
			//react according to the vert's flag
			do_ball_vert_reaction(ball, pl, vert, hole, v_no);
			return 1;
		}
	}
	return 0;
}

//check the ball against all verts for possible collisions
void check_ball_vert_collision_all(BALL * ball, PLAYER * pl, HOLE * hole)
{
	int v;
	int reset = 1;

	ball->decel_type = 0;
	ball->hole_vert = -1;
	for(v = 0; v < hole->num_verts; v++)
	{
		if(check_ball_vert_collision(ball, pl, &hole->vert[v], hole, v))
		{
			if(hole->vert[v].flag == VERT_FLAG_INDENT_HILL || hole->vert[v].flag == VERT_FLAG_INDENT_RUT)
			{
				reset = 0;
			}
		}
	}
	if(reset)
	{
		ball->f.x = 0;
		ball->f.y = 0;
	}
}

//find the p player
void get_player_keys(PLAYER * pl, HOLE * hole)
{
	/* line up shot */
	if(!mouse_unused)
	{
		pl->f_club_angle = atan2(pl->ball.s.y - vgolf_mouse_y, pl->ball.s.x - vgolf_mouse_x);
	}
	else
	{
		if(key[KEY_LEFT])
		{
			pl->f_club_angle -= (VGOLF_PI) / 180.0;
			if(pl->f_club_angle < 0)
			{
				pl->f_club_angle += VGOLF_PI * 2.0;
			}
		}
		if(key[KEY_RIGHT])
		{
			pl->f_club_angle += (VGOLF_PI) / 180.0;
			if(pl->f_club_angle >= VGOLF_PI * 2.0)
			{
				pl->f_club_angle -= VGOLF_PI * 2.0;
			}
		}
	}
	
	//only get club swinging keys when ball is idle
	if(!pl->club_swing)
	{ 
		if(pl->ball.mode == BALL_MODE_IDLE && vgolf_state != VGOLF_STATE_BALL_SUNK)
		{
			pl->club_power = vgolf_mouse_z * 5;
			if(pl->club_power < 0)
			{
				pl->club_power = 0;
			}
			else if(pl->club_power > MAX_CLUB_POWER)
			{
				pl->club_power = MAX_CLUB_POWER;
			}
			pl->club_v[0].x = -pl->club_power/2*cos(pl->f_club_angle+pl->f_club_width);
			pl->club_v[0].y = -pl->club_power/2*sin(pl->f_club_angle+pl->f_club_width);
			pl->club_v[1].x = -pl->club_power/2*cos(pl->f_club_angle-pl->f_club_width);
			pl->club_v[1].y = -pl->club_power/2*sin(pl->f_club_angle-pl->f_club_width);

			if((vgolf_mouse_b & 1))
			{
				if(!block_click)
				{
					pl->club_swing = 1;     //swing mode active..
				}
			}
			else
			{
				if(block_click)
				{
					block_click = 0;
				}
			}
			if(vgolf_mouse_b & 2)
			{
				//if trajectory display active, deactivate it before swing begins
				if(pl->traj_active)
				{
					pl->traj_active = 0;
				}
				if(main_game_vars.player_huds_state == 0)
				{         //if the huds aren't already hiding..
					main_game_vars.player_huds_state = 1;  //..hide the huds
				}
				else if(main_game_vars.player_huds_state == 2)
				{         //if the huds aren't already hiding..
					main_game_vars.player_huds_state = 3;  //..hide the huds
				}
			}
		}
			
		//if ball is sunk in the water, wait for "d" or "r" to be pressed to drop or rehit
		else if(pl->ball.mode == BALL_MODE_WATER_SUNK)
		{
			if(main_game_vars.cur_vmessage_slot == 1)    //make sure the current message is the "[d]rop or [r]ehit?" one
			{
				//drop the ball nearby
			}
		}
		if(key[KEY_1] && debug_mode)   //just a test cheat to put the ball in the hole when this key is hit
		{
			
			game_player[main_game_vars.cur_player].ball.s = v_offset(game_course->current_hole.s, game_course->current_hole.vert[1].s);
			game_player[main_game_vars.cur_player].cur_strokes = game_course->current_hole.par_score;
			game_player[main_game_vars.cur_player].ball.mode = BALL_MODE_HIT;
			game_player[main_game_vars.cur_player].ball.speed = game_course->current_hole.deccel_factor+0.05;
			key[KEY_1] = 0;
		}
		if(key[KEY_2] && debug_mode)   // jump to victory screen with random scores
		{
			int i, j;
			
			for(i = 0; i < 4; i++)
			{
				if(vgolf_player_list[i])
				{
					for(j = 0; j < game_course->num_holes; j++)
					{
						game_player[i].scorecard.score[j] = game_course->hole[j].par_score + 1;
					}
				}
	//       		game_player[i].scorecard.score[0] += i;
			}
			if(vgolf_player_count == 1)
			{
				single_player_course_completed_logic(&main_game_vars, game_player, game_course);
			}
			else
			{
				multi_player_course_completed_logic(&main_game_vars, game_player, vgolf_player_count, game_course);
			}
			start_victory_screen_segin(&main_game_vars, game_course, game_player, vgolf_player_count);
			key[KEY_2] = 0;
		}
	}
	//swing the player's club
	else
	{
		do_player_club_swing(pl);
	}
}

//finalize the match
void do_match_finish(HOLE * hole, int cur_hole, PLAYER pl[], int num_players)
{
	int i;
	V_INT p_no, p_total; //p_no contains the player numbers from 0 to num_players
                             //p_total contains the stroke total for each player in p_no
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			p_no.c[i] = i;
			p_total.c[i] = pl[i].cur_strokes;
		}
	}
	
	//sort from smallest to largest score
	v_int_heap_sort_ref(&p_total, &p_no, num_players);
	//p_no should now be sorted from the player with the best score to that with the worst

	//if there are only 2 players, the sort is simple
	if(num_players == 2)
	{
		if(p_total.c[0] < p_total.c[1])
		{
			//the player with least score is tagged the winner
			pl[p_no.c[0]].scorecard.match_won[cur_hole] = 1;
			//other players are tagged losers
			pl[p_no.c[1]].scorecard.match_won[cur_hole] = 0;
			//winning player's win total increments
			pl[p_no.c[0]].scorecard.matches_won++;
		}
		else
		{
			//players are tied
			pl[p_no.c[0]].scorecard.match_won[cur_hole] = pl[p_no.c[1]].scorecard.match_won[cur_hole] = 2;
		}
	}
	else if(num_players == 3)
	{
		//players are tied
		if(p_total.c[0] == p_total.c[1])
		{
			pl[p_no.c[0]].scorecard.match_won[cur_hole] = pl[p_no.c[1]].scorecard.match_won[cur_hole] = pl[p_no.c[2]].scorecard.match_won[cur_hole] = 2;
		}
		else
		{
			//the player with least score is tagged the winner
			pl[p_no.c[0]].scorecard.match_won[cur_hole] = 1;
			//other players are tagged losers
			pl[p_no.c[1]].scorecard.match_won[cur_hole] = pl[p_no.c[2]].scorecard.match_won[cur_hole] = 0;
			//winning player's win total increments
			pl[p_no.c[0]].scorecard.matches_won++;
		}
	}
	else
	{
		//players are tied
		if(p_total.c[0] == p_total.c[1])
		{
			//all players are tagged as tied
			pl[0].scorecard.match_won[cur_hole] = pl[1].scorecard.match_won[cur_hole] = pl[2].scorecard.match_won[cur_hole] = pl[3].scorecard.match_won[cur_hole] = 2;
		}
		else
		{
			//the player with least score is tagged the winner
			pl[p_no.c[0]].scorecard.match_won[cur_hole] = 1;
			//other players are tagged losers
			pl[p_no.c[1]].scorecard.match_won[cur_hole] = pl[p_no.c[2]].scorecard.match_won[cur_hole] = pl[p_no.c[3]].scorecard.match_won[cur_hole] = 0;
			//winning player's win total increments
			pl[p_no.c[0]].scorecard.matches_won++;
		}
	}
}

//logic associated with the translucent ball
void ball_mirage_logic(BALL * ball)
{
	if(ball->mirage_visible)
	{                                    //mirage is visible, continue
		if(++ball->mirage_timer >= ball->mirage_timer_end)
		{  //increment the timer until the end
			ball->mirage_visible = 0;                    //make mirage invisible
			ball->mirage_timer = 0;                      //reset timer for next time
		}
	}
}

//ball logic..
void ball_logic(BALL * ball, PLAYER * pl, HOLE * hole)
{
	
	if(ball->mode == BALL_MODE_HIT || ball->mode == BALL_MODE_IN_MOTION_AREA || ball->mode == BALL_MODE_ON_CONVEYOR)
	{
		check_ball_hole_collision(ball, pl, hole);  //check for collisions between ball and walls
		move_player_ball(ball, pl, hole);               //move the ball around
		decell_ball(ball, pl, hole);                //slow the ball down until it stops
	}
	if(ball->mode == BALL_MODE_HIT || ball->mode == BALL_MODE_PIPE_ENTER || ball->mode == BALL_MODE_IN_MOTION_AREA || ball->mode == BALL_MODE_ON_CONVEYOR)
	{
		//check for collisions with the various verts, and react according to the vert flag
		check_ball_vert_collision_all(ball, pl, hole);  
	}
	if(!ball->visible && ball->mode == BALL_MODE_WATER_SINKING)
	{
		if(do_anim_def_logic(&ball->anim_def[1], ball->s.x-12, ball->s.y-12, 0) >= (ball->anim_def[1].num_frames-1)*ball->anim_def[1].speed)
		{
			ball->anim_def[1].cur_frame = 0;   //reset the anim counter
			ball->mode = BALL_MODE_WATER_SUNK; //ball is now sunk
			current_player = pl;
			current_hole = hole;
			vgolf_ball_sunk();
		}
	}
	if(ball->mode == BALL_MODE_WATER_SUNK)
	{
	}
	else if(ball->mode == BALL_MODE_LASER_ZAPPED)
	{      //ball has been zapped by a laser
		if(!main_game_vars.cur_vmessage_slot)
		{      //wait for the "zap" message to clear before dropping ball into play again
			//bring ball back to the nearest respawn pad
			do_ball_laser_drop(ball, pl, hole);
		}
	}
	else if(ball->mode == BALL_MODE_DOOR_SMASHED)
	{      //ball has been zapped by a laser
		if(!main_game_vars.cur_vmessage_slot)
		{      //wait for the "zap" message to clear before dropping ball into play again
			//bring ball back to the nearest respawn pad
			do_ball_smash_drop(ball, pl, hole);
		}
	}
	if(ball->mode == BALL_MODE_SUNK)
	{                   //ball is in the cup
		do_player_hole_finish(pl, hole);            //finish the hole
	}
	ball_mirage_logic(ball);                            //countdown until the mirage disappears
}

//handle the logic involving the trajectory calculations
void player_trajectory_logic(PLAYER * pl, HOLE * hole)
{
	if(pl->traj_active && !pl->traj_calc_done)
	{
		BALL temp_ball = pl->ball;
		
		temp_ball.angle = temp_ball.f_angle = pl->f_club_angle;
		temp_ball.mode = BALL_MODE_HIT;
		temp_ball.speed = 10.0f;
		temp_ball.v.x = temp_ball.speed*cos(pl->f_club_angle);
		temp_ball.v.y = temp_ball.speed*sin(pl->f_club_angle);
		temp_ball.body.vel = temp_ball.v;
        	
		pl->traj_v[0] = temp_ball.s;
		pl->traj_count++;
		while(temp_ball.speed > 0)
		{
			move_player_ball(&temp_ball, pl, hole);
			check_ball_hole_collision(&temp_ball, pl, hole);
			temp_ball.speed -= hole->deccel_factor;
		}
		pl->traj_calc_done = 1;
	}
	if(pl->traj_active)
	{
		pl->traj_line_timer = !pl->traj_line_timer;    //alternate the dots on the dotted line display
	}
}

//does the player logic for one player
void player_logic(PLAYER * pl, HOLE * hole)
{
	//if the player's club isn't being swung, get key input
	get_player_keys(pl, hole);
	//handle ball logic (movement, collision, etc)
	ball_logic(&pl->ball, pl, hole);
        
	//handle the trajectory logic if necessary
	player_trajectory_logic(pl, hole);
}

//does the player logic for all players in the game
void player_logic_all(PLAYER pl[], int num_players, HOLE * hole)
{
	//handle logic for the current player
	player_logic(&pl[main_game_vars.cur_player], hole);
}

//drop the player's ball on the tee vert spot
void player_start_hole(PLAYER * pl, HOLE * hole)
{
	int i;

	//look through all the verts for the tee vert
	for(i = 0; i < hole->num_verts; i++)
	{
		if(hole->vert[i].flag == VERT_FLAG_TEE)
		{                 //tee flag found..
			pl->ball.s = v_offset(hole->s, hole->vert[i].s); //place ball on tee vert position
			pl->ball.s_old = pl->ball.s;
			break;
		}
	}
	pl->ball.body.pos = pl->ball.s;
	pl->ball.mode = BALL_MODE_IDLE;          //ball needs to be idle to start off
	pl->ball.idle_timer = 0;                 //reset the idle timer
	pl->ball.visible = 1;                    //ball should be visible
	main_game_vars.hole_finished = 0;        //hole isn't finished
	#ifndef PANDORA
		if(!mouse_unused)
		{
			position_mouse(pl->ball.s.x, pl->ball.s.y);
		}
	#endif
}

//handle the in-game message queue..
void game_vmessage_logic(V_MESSAGE msg[], int * cur_slot, FONT * vf)
{
	int i;

	switch(msg[0].active)
	{
		case -1:  //message needs activation
			msg[0].active = 1;
			break;

		case 1:   //message is activated, do the logic
			vfont_message_logic(vf, &msg[0], msg[0].method, 1);
			break;

		case 2:   //message is done, deactivate it
			msg[0].active = 0;
			for(i = 0; i < *cur_slot; i++)
			{  //move all the messages down in the queue
				msg[i] = msg[i+1];
			}
			(*cur_slot)--;
			if(*cur_slot > 0) msg[0].active = 1;
			break;

		default:
			break;
	}
}

//start the seg-out sequence
void start_scoreboard_segout(void)
{
	main_game_vars.game_state = main_game_vars.game_state_old = GAMESTATE_SCOREBOARD_SEGOUT;
}

//start the victory screen seg-in sequence
void start_victory_screen_segin(MAIN_GAME_VARS * gvars, VGOLF_COURSE * cr, PLAYER pl[], int num_players)
{
	int i, j;
	int total[4];
	int low = 100000;
	int li = -1;
	char text[64] = {0};
	
	if(vgolf_config[CONFIG_UPLOAD])
	{
		ncds_pause_music();
		for(i = 0; i < 4; i++)
		{
			if(vgolf_player_list[i] && game_player[i].profile > 0 && strlen(vgolf_profiles.item[game_player[i].profile].name) > 0)
			{
				sprintf(text, "%lu", vgolf_courses.item[selected_course].sum);
				t3net_upload_score("http://www.t3-i.com/leaderboards/poll.php", "vgolf", "1.1", "normal", text, vgolf_profiles.item[game_player[i].profile].name, player_course_stroke_total(game_player[i], game_course->num_holes) * 2 + 'v' + 'g' + 'o' + 'l' + 'f');
			}
		}
		ncds_resume_music();
		gametime_reset();
	}
	if(vgolf_player_count == 1)
	{
		gvars->game_state = gvars->game_state_old = GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGIN;
	}
	else
	{
		gvars->game_state = gvars->game_state_old = GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGIN;
	}

	/* update course profile for all players */
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			total[i] = 0;
			for(j = 0; j < cr->num_holes; j++)
			{
				total[i] += pl[i].scorecard.score[j];
			}
			if(vgolf_profiles.item[pl[i].profile].course_info[selected_course].total_score == 0 || vgolf_profiles.item[pl[i].profile].course_info[selected_course].total_score > total[i])
			{
				vgolf_profiles.item[pl[i].profile].course_info[selected_course].total_score = total[i];
			}
			vgolf_profiles.item[pl[i].profile].course_info[selected_course].completed++;
		}
	}
	
	/* update wins/losses in multiplayer mode */
	if(vgolf_player_count > 1)
	{
		for(i = 0; i < 4; i++)
		{
			if(vgolf_player_list[i])
			{
				if(total[i] < low)
				{
					low = total[i];
					li = i;
				}
			}
		}
		for(i = 0; i < 4; i++)
		{
			if(vgolf_player_list[i])
			{
				if(total[i] == low)
				{
					vgolf_profiles.item[pl[i].profile].course_info[selected_course].won++;
				}
				else
				{
					vgolf_profiles.item[pl[i].profile].course_info[selected_course].lost++;
				}
			}
		}
	}
	
	gvars->cur_hole = vgolf_start_hole - 1;	                       //restart the cur_hole counter
}

//get key input on the scoreboard screen
void get_scoreboard_keys(VGOLF_COURSE * cr, PLAYER pl[], int num_players, int cur_hole)
{
	if(cr->scoreboard.scoreboard_seg_count >= cr->scoreboard.num_scoreboard_segs + 1)
	{
		hyperlink_page_logic(scoreboard_page);
	}
}

//seg-out is done, goto next game state
void finish_scoreboard_segout(void)
{
	//since we're doing the seg-out sequence on the victory screen also, do a check first
	//before going to the "start hole" state
	if(!(main_game_vars.game_state > GAMESTATE_COURSE_COMPLETED_1P && main_game_vars.game_state < GAMESTATE_COIN_COLLECT_FINISHED))
	{
		main_game_vars.game_state = GAMESTATE_START_HOLE;
	}
}

//handles logic related to the scoreboard screen seg'ing out
void course_scoreboard_segout_logic(VGOLF_COURSE * cr)
{
	//increment segment counter until hits the end, then decrease the # of segments to show
	cr->scoreboard.scoreboard_seg_counter -= 16;
	if(cr->scoreboard.scoreboard_seg_counter <= 0)
	{
		cr->scoreboard.scoreboard_seg_counter = cr->scoreboard.scoreboard_seg_counter_end;
		cr->scoreboard.scoreboard_seg_count--;
		if(cr->scoreboard.scoreboard_seg_count <= 0)
		{
			//all segments are removed now, finish seg-out sequence
			finish_scoreboard_segout();
		}
	}
}

//seg-in is done, goto next game state
void finish_scoreboard_segin(void)
{
	main_game_vars.game_state = GAMESTATE_SCOREBOARD_SEGMID;
}

//handles logic related to the scoreboard screen seg'ing in
void course_scoreboard_segin_logic(VGOLF_COURSE * cr)
{
	//increment segment counter until hits the end, then increase the # of segments to show
	cr->scoreboard.scoreboard_seg_counter += 16;
	if(cr->scoreboard.scoreboard_seg_counter >= cr->scoreboard.scoreboard_seg_counter_end)
	{
		cr->scoreboard.scoreboard_seg_counter = 0;
		cr->scoreboard.scoreboard_seg_count++;
		if(cr->scoreboard.scoreboard_seg_count >= cr->scoreboard.num_scoreboard_segs + 1)
		{
			//all segments are visible now, finish the seg-in sequence
			finish_scoreboard_segin();
		}
	}
}


//handles main logic related to the scoreboard screen
void course_scoreboard_main_logic(VGOLF_COURSE * cr, PLAYER pl[], int num_players, int cur_hole)
{
	get_scoreboard_keys(cr, pl, num_players, cur_hole);
}

//do the single player logic upon course completion
void single_player_course_completed_logic(MAIN_GAME_VARS * gvars, PLAYER * pl, VGOLF_COURSE * cr)
{
	//start the victory screen seg-in sequence
	gvars->game_state = GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGIN;
}

//do the multi-player logic upon course completion
void multi_player_course_completed_logic(MAIN_GAME_VARS * gvars, PLAYER pl[], int num_players, VGOLF_COURSE * cr)
{
	//if there are only two players, sorting will be a little easier..
	V_INT p_total, p_no;
	int i;

	switch(gvars->game_mode)
	{
		case GAMEMODE_STROKE_PLAY:
		{
			for(i = 0; i < 4; i++)
			{
				p_no.c[i] = i;
				if(vgolf_player_list[i])
				{
					p_total.c[i] = player_course_stroke_total(pl[i], cr->num_holes);
				}
				else
				{
					p_total.c[i] = 100000;
				}
			}

			//sort from smallest to largest score
			v_int_heap_sort_ref(&p_total, &p_no, 4);

			//set winner array
			for(i = 0; i < 4; i++)
			{
				gvars->vict_winner[i] = p_no.c[i];
			}

			//check for tie
			if(p_total.c[gvars->vict_winner[0]] == p_total.c[gvars->vict_winner[1]])
			{
//				gvars->vict_winner[0] = gvars->vict_winner[1] = gvars->vict_winner[2] = -1;
			}
			break;
		}
		case GAMEMODE_MATCH_PLAY:
		{
			for(i = 0; i < 4; i++)
			{
				p_no.c[i] = i;
				if(vgolf_player_list[i])
				{
					p_total.c[i] = pl[i].scorecard.matches_won;
				}
				else
				{
					p_total.c[i] = 0;
				}
			}

			//sort from largest to smallest score
			v_int_heap_sort_ref_rev(&p_total, &p_no, vgolf_player_count);

			//set winner array
			for(i = 0; i < 4; i++)
			{
				gvars->vict_winner[i] = p_no.c[i];
			}

			//check for tie
			if(p_total.c[gvars->vict_winner[0]] == p_total.c[gvars->vict_winner[1]])
			{
				gvars->vict_winner[0] = gvars->vict_winner[1] = gvars->vict_winner[2] = -1;
			}
			break;
		}
		default:
		{
			break;
		}
	}
	
	//start the victory screen seg-in sequence
	gvars->game_state = GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGIN;
}

//move the clouds on the victory screen
void victory_screen_cloud_logic(MAIN_GAME_VARS * gvars)
{
	int i;
	
	for(i = 0; i < 4; i++)
	{
		gvars->vict_cloud_x[i] -= 0.5*(1+i);  //make the further clouds move slower
		if(gvars->vict_cloud_x[i] <= -250)
		{
			gvars->vict_cloud_x[i] = SCREEN_W+random()%150;  //respawn clouds at the edge of the screen when they go off the other edge
			gvars->vict_cloud_y[i] = random()%200;
		}
	}
}

//do all the victory screen stuff
void victory_screen_segin_logic(MAIN_GAME_VARS * gvars, VGOLF_COURSE * cr)
{
	//move podium
	if(gvars->vict_pody > 480 - cr->image_bank.victory_podiumpic->h + 1)
	{
		gvars->vict_pody -= 2;
	}
	else
	{
		//seg-in is done, go to next state
		if(vgolf_player_count == 1) gvars->game_state = gvars->game_state_old = GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGMID;
		else gvars->game_state = gvars->game_state_old = GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGMID;
	}
	//move clouds
	victory_screen_cloud_logic(gvars);
        
	//do the scoreboard disappearance
	course_scoreboard_segout_logic(cr);
}

//get keys for the victory screen
void get_victory_screen_keys(MAIN_GAME_VARS * gvars, VGOLF_COURSE * cr)
{
	hyperlink_page_logic(game_complete_page);
}

//victory screen logic for single player game
void victory_screen_single_player_logic_main(MAIN_GAME_VARS * gvars, PLAYER * pl, VGOLF_COURSE * cr)
{
	//move clouds
	victory_screen_cloud_logic(gvars);
	//get keys
	get_victory_screen_keys(gvars, cr);
}

//victory screen logic for multiplayer game
void victory_screen_multi_player_logic_main(MAIN_GAME_VARS * gvars, PLAYER pl[], int num_players, VGOLF_COURSE * cr)
{
	//move clouds
	victory_screen_cloud_logic(gvars);
	//get keys
	get_victory_screen_keys(gvars, cr);
}

//the main logic stuff
void main_game_logic(PLAYER pl[], int num_players, VGOLF_COURSE * cr)
{
	int i;

	switch(main_game_vars.game_state)
	{
		case GAMESTATE_SCOREBOARD_SEGIN:   //scoreboard is coming onscreen
			hole_logic(&cr->current_hole);
			course_scoreboard_segin_logic(cr);
			break;
		case GAMESTATE_SCOREBOARD_SEGMID:  //scoreboard is onscreen
			course_scoreboard_main_logic(cr, pl, num_players, main_game_vars.cur_hole);
			break;
		case GAMESTATE_SCOREBOARD_SEGOUT:  //scoreboard is disappearing
			hole_logic(&cr->current_hole);
			course_scoreboard_segout_logic(cr);
			break;
		case GAMESTATE_START_HOLE:         //hole needs to be set up before playing
			for(i = 0; i < 4; i++)
			{
				if(vgolf_player_list[i])
				{
					player_start_hole(&pl[i], &cr->current_hole);
				}
			}
			main_game_vars.game_state = main_game_vars.game_state_old = GAMESTATE_READY_TOPLAY;

			sprintf(temp_text, "Tee off, %s!", vgolf_profiles.item[game_player[main_game_vars.cur_player].profile].name);
			add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, temp_text, SCREEN_W, SCREEN_H / 2, 0, VMSG_TEXT_ENTER_FROMRIGHT, -10, 1, 0, 0, 99999);

			break;
		case GAMESTATE_READY_TOPLAY:       //we're ready to play golf
			hole_logic(&cr->current_hole);
			player_logic_all(pl, num_players, &cr->current_hole);
			hide_player_huds(pl, num_players, 0);
			get_main_keys();
			if(main_game_vars.cur_vmessage_slot > 0)
			{
				game_vmessage_logic(game_vmessage, &main_game_vars.cur_vmessage_slot, gfont);
			}

			break;
		case GAMESTATE_PLAYER_FINISHED_HOLE:
		{
			hole_logic(&cr->current_hole);
			if(main_game_vars.cur_vmessage_slot > 0)
			{
				game_vmessage_logic(game_vmessage, &main_game_vars.cur_vmessage_slot, gfont);
			}
			else
			{
				int p = vgolf_get_next_player();
                
				if(p != -1) // all players finished when -1
				{
					int v, vert_ref = 0;
	
					//find the vert for the hole
					for(v = 0; v < game_course->current_hole.num_verts; v++)
					{
						if(game_course->current_hole.vert[v].flag == VERT_FLAG_HOLE)
						{
							vert_ref = v;
							break;
						}
					}
					game_course->current_hole.vert[vert_ref].ref_no[0] = 0;  //empty the hole out for the next player..
					
					/* find next player */
					if(main_game_vars.cur_player != p)
					{
						main_game_vars.cur_player = p;
						build_turn_text(p);
						add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, gfont, &main_game_vars.cur_vmessage_slot, temp_text, SCREEN_W / 2 - strlen(temp_text) * game_vfont.w * 2, SCREEN_H / 2 - game_vfont.h * 2, 0, VMSG_TEXT_ENTER_FROMRIGHT, -10, 4, 0, 0, -1);
					}
					main_game_vars.game_state = GAMESTATE_READY_TOPLAY;
				}
				else
				{
					//begin seg to scoreboard
					//set current player to winner of previous hole..
					main_game_vars.game_state = GAMESTATE_HOLE_FINISHED;
					main_game_vars.hole_finished = 1;
					main_game_vars.cur_player = vgolf_get_best_player();
				}
				pl->pic_mode = 0;
			}
			break;
		}
		case GAMESTATE_HOLE_FINISHED:      //hole is finished..
			//wait for messages to finish
			if(main_game_vars.cur_vmessage_slot == 0)
			{
				if(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY)
				{
					//finalize the match
					do_match_finish(&cr->current_hole, main_game_vars.cur_hole, pl, num_players);
				}
				else if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT)
				{
				}
				//start the scoreboard seg-in
				main_game_vars.game_state = GAMESTATE_SCOREBOARD_SEGIN;
				next_hole = main_game_vars.cur_hole + 1;
			}
			else
			{
				game_vmessage_logic(game_vmessage, &main_game_vars.cur_vmessage_slot, gfont);
			}

			break;
		case GAMESTATE_COURSE_COMPLETED_1P:  //course is completed in 1-player mode
			single_player_course_completed_logic(&main_game_vars, &pl[0], cr);
			break;
		case GAMESTATE_COURSE_COMPLETED_XP:  //course is completed in multiplayer mode
			multi_player_course_completed_logic(&main_game_vars, pl, num_players, cr);
			break;
		case GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGIN:
			//victory screen is coming onscreen
			victory_screen_segin_logic(&main_game_vars, cr);
			break;
		case GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGIN:
			//victory screen is coming onscreen
			victory_screen_segin_logic(&main_game_vars, cr);
			break;
		case GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGMID:
			//victory screen is in place
			victory_screen_single_player_logic_main(&main_game_vars, &pl[0], cr);
			break;
		case GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGMID:
			//victory screen is in place
			victory_screen_multi_player_logic_main(&main_game_vars, pl, num_players, cr);
			break;
		default:
			//get main keyboard input (for pausing, exiting, etc.)
			get_main_keys();

			break;
	}
	
	/* keep time for all players */
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			vgolf_profiles.item[pl[i].profile].total.time++;
		}
	}
}

//draw background layer..
void draw_hole_bkg(BITMAP * bp, HOLE * hole)
{
	clear_to_color(bp, 10);
	if(hole->layer_pic[0] != NULL) masked_blit(hole->layer_pic[0], bp, 0, 0, 0, 0, hole->layer_pic[0]->w, hole->layer_pic[0]->h);
}               

//draw foreground layer..
void draw_hole_fg(BITMAP * bp, HOLE * hole)
{
	if(hole->layer_pic[1] != NULL) masked_blit(hole->layer_pic[1], bp, 0, 0, 0, 0, hole->layer_pic[1]->w, hole->layer_pic[1]->h);
}

//draw the "morphing poly" layer..
void draw_hole_polymorph_layer(BITMAP * bp, HOLE * hole)
{
	if(hole->layer_pic[2] != NULL) masked_blit(hole->layer_pic[2], bp, 0, 0, 0, 0, hole->layer_pic[2]->w, hole->layer_pic[2]->h);
}

//hole foreground display stuff
void hole_fg_display(BITMAP * bp, HOLE * hole)
{
	//draw the polygons if there are any..
	draw_hole_polymorph_layer(bp, hole);

	//draw the foreground if it's there
	draw_hole_fg(bp, hole);
}

//draws a laser between the endpoints
void laser_display(BITMAP * bp, int x1, int y1, int x2, int y2, int color)
{
	//use the two-step line algorithm so we can add some noise outside the line
	draw_line_twostep(bp, x1, y1, x2, y2, color, 3);
}

//display one vert
void hole_vert_display(BITMAP * bp, VERT * vert, HOLE * hole, COURSE_IMAGE_BANK i_bank)
{
	int i;

	switch(vert->flag)
	{
		case VERT_FLAG_TEE:
			break;

		case VERT_FLAG_HOLE:        //draw the cup
			masked_blit(i_bank.vert_pic[vert->flag][vert->ref_no[0]], bp, 0, 0, hole->s.x+vert->s.x-i_bank.vert_pic[vert->flag][vert->ref_no[0]]->w/2, hole->s.y+vert->s.y-i_bank.vert_pic[vert->flag][vert->ref_no[0]]->h/2, i_bank.vert_pic[vert->flag][vert->ref_no[0]]->w, i_bank.vert_pic[vert->flag][vert->ref_no[0]]->h);
			break;

		case VERT_FLAG_TREE_0:      //draw the various tree types
			masked_blit(i_bank.object_pic[0][0], bp, 0, 0, hole->s.x+vert->s.x-i_bank.object_pic[0][0]->w/2, hole->s.y+vert->s.y-i_bank.object_pic[0][0]->h/2, i_bank.object_pic[0][0]->w, i_bank.object_pic[0][0]->h);
			break;

		case VERT_FLAG_TREE_1:
			masked_blit(i_bank.object_pic[1][0], bp, 0, 0, hole->s.x+vert->s.x-i_bank.object_pic[1][0]->w/2, hole->s.y+vert->s.y-i_bank.object_pic[1][0]->h/2, i_bank.object_pic[1][0]->w, i_bank.object_pic[1][0]->h);
			break;

		case VERT_FLAG_ANIMAL_0:    //draw the various animal types
			masked_blit(i_bank.animal_pic[0][vert->ref_no[0]], bp, 0, 0, hole->s.x+vert->s.x-i_bank.animal_pic[0][vert->ref_no[0]]->w/2, hole->s.y+vert->s.y-i_bank.animal_pic[0][vert->ref_no[0]]->h/2, i_bank.animal_pic[0][vert->ref_no[0]]->w, i_bank.animal_pic[0][vert->ref_no[0]]->h);
			break;

		case VERT_FLAG_ANIMAL_1:
			masked_blit(i_bank.animal_pic[1][vert->ref_no[0]], bp, 0, 0, hole->s.x+vert->s.x-i_bank.animal_pic[1][vert->ref_no[0]]->w/2, hole->s.y+vert->s.y-i_bank.animal_pic[1][vert->ref_no[0]]->h/2, i_bank.animal_pic[1][vert->ref_no[0]]->w, i_bank.animal_pic[1][vert->ref_no[0]]->h);
			break;

		case VERT_FLAG_POLY_MORPH:  //draw the rotating polygon to the poly layer
			draw_polygon(hole->layer_pic[2], hole->body[vert->ref_no[1]].poly, hole->s.x, hole->s.y);
			floodfill(hole->layer_pic[2], hole->s.x+vert->s.x, hole->s.y+vert->s.y, vert->ref_no[3]);
			break;

		case VERT_FLAG_POLY_MOVE_HORIZ:  //this vert controls a poly that moves horizontally
			draw_polygon(hole->layer_pic[2], hole->body[vert->ref_no[1]].poly, hole->s.x, hole->s.y);
			floodfill(hole->layer_pic[2], hole->s.x+vert->s.x+vert->ref_no[2], hole->s.y+vert->s.y, vert->ref_no[3]);
			break;

		case VERT_FLAG_POLY_MOVE_VERT:  //this vert controls a poly that moves horizontally
			draw_polygon(hole->layer_pic[2], hole->body[vert->ref_no[1]].poly, hole->s.x, hole->s.y);
			floodfill(hole->layer_pic[2], hole->s.x+vert->s.x, hole->s.y+vert->s.y+vert->ref_no[2], vert->ref_no[3]);
			break;

		case VERT_FLAG_LASER_SEND:
			//draw the laser sender/receiver
			masked_blit(i_bank.object_pic[2][vert->ref_no[0]], bp, 0, 0, hole->s.x+vert->s.x-i_bank.object_pic[2][0]->w/2, hole->s.y+vert->s.y-i_bank.object_pic[2][0]->h/2, i_bank.object_pic[2][0]->w, i_bank.object_pic[2][0]->h);
			//if the laser is active, draw the laser beam between the vert endpoints
			if(vert->ref_no[0])
			{
				//laser_display(hole->layer_pic[2], hole->s.x+vert->s.x, hole->s.y+vert->s.y, hole->s.x+hole->vert[vert->ref_no[5]].s.x, hole->s.y+hole->vert[vert->ref_no[5]].s.y, hole->vert[vert->ref_no[5]].ref_no[1]);
				line(hole->layer_pic[2], hole->s.x + vert->s.x, hole->s.y + vert->s.y, hole->s.x + hole->vert[vert->ref_no[5]].s.x, hole->s.y + hole->vert[vert->ref_no[5]].s.y, hole->vert[vert->ref_no[5]].ref_no[1]);
			}
			break;

		case VERT_FLAG_LASER_RECV:
			//draw the laser sender/receiver
			masked_blit(i_bank.object_pic[2][vert->ref_no[0]], bp, 0, 0, hole->s.x+vert->s.x-i_bank.object_pic[2][0]->w/2, hole->s.y+vert->s.y-i_bank.object_pic[2][0]->h/2, i_bank.object_pic[2][0]->w, i_bank.object_pic[2][0]->h);
			break;

		case VERT_FLAG_TELEPORT:   //draw the teleport pad
			masked_blit(i_bank.object_pic[3][vert->ref_no[5]], bp, 0, 0, hole->s.x+vert->s.x-i_bank.object_pic[3][0]->w/2, hole->s.y+vert->s.y-i_bank.object_pic[3][0]->h/2, i_bank.object_pic[3][0]->w, i_bank.object_pic[3][0]->h);
			break;

		case VERT_FLAG_CONVEYOR:   //draw the conveyor belt
			if(vert->ref_no[0] < 2)
			{
				for(i = 0; i < (vert->radius*2)/i_bank.object_pic[4][0]->h; i++)
				{
					if(vert->ref_no[0] == 0) masked_blit(i_bank.object_pic[4][vert->ref_no[4]/vert->ref_no[2]], bp, 0, 0, hole->s.x+vert->s.x-i_bank.object_pic[4][0]->w/2, hole->s.y+vert->s.y-vert->radius+i*i_bank.object_pic[4][0]->h, i_bank.object_pic[4][0]->w, i_bank.object_pic[4][0]->h);
					else draw_sprite_v_flip(bp, i_bank.object_pic[4][vert->ref_no[4]/vert->ref_no[2]], hole->s.x+vert->s.x-i_bank.object_pic[4][0]->w/2, hole->s.y+vert->s.y-vert->radius+i*i_bank.object_pic[4][0]->h);
				}
			}
			else
			{
				for(i = 0; i < (vert->radius*2)/i_bank.object_pic[4][0]->h; i++)
				{
					if(vert->ref_no[0] == 3) rotate_sprite(bp, i_bank.object_pic[4][vert->ref_no[4]/vert->ref_no[2]], hole->s.x+vert->s.x-vert->radius+i*i_bank.object_pic[4][0]->h, hole->s.y+vert->s.y-i_bank.object_pic[4][0]->w/2, itofix(64));
					else rotate_sprite_v_flip(bp, i_bank.object_pic[4][vert->ref_no[4]/vert->ref_no[2]], hole->s.x+vert->s.x-vert->radius+i*i_bank.object_pic[4][0]->h, hole->s.y+vert->s.y-i_bank.object_pic[4][0]->w/2, itofix(64));
				}
			}
			break;

		case VERT_FLAG_CLOUD_HORIZ:  //draw the cloud to the poly layer (so it flies overhead..)
		case VERT_FLAG_CLOUD_VERT:  //draw the cloud to the poly layer (so it flies overhead..)
			circlefill(hole->layer_pic[2], hole->s.x+vert->s.x, hole->s.y+vert->s.y, vert->radius, vert->ref_no[5]);
			circle(hole->layer_pic[2], hole->s.x+vert->s.x, hole->s.y+vert->s.y, vert->radius, vert->ref_no[5]-1);
			break;

		case VERT_FLAG_BLINKER:     //draw the flashing blinker light..
			if(vert->ref_no[0])
			{
				if(vert->ref_no[4]) rectfill(bp, hole->s.x+vert->s.x-vert->radius, hole->s.y+vert->s.y-vert->radius, hole->s.x+vert->s.x+vert->radius, hole->s.y+vert->s.y+vert->radius, vert->ref_no[5]);
				else circlefill(bp, hole->s.x+vert->s.x, hole->s.y+vert->s.y, vert->radius, vert->ref_no[5]);
			}
			break;

		case VERT_FLAG_POLY_DOOR_HORIZ:  //draw the moving doors
		case VERT_FLAG_POLY_DOOR_VERT:  //draw the moving doors
		{
			int lp_no = vert->ref_no[6];
			int rp_no = vert->ref_no[7];
			rectfill(bp, hole->s.x+hole->body[lp_no].poly.p[1].x, hole->s.y+hole->body[lp_no].poly.p[1].y, hole->s.x+hole->body[lp_no].poly.p[3].x, hole->s.y+hole->body[lp_no].poly.p[3].y, vert->ref_no[5]);
			rect(bp, hole->s.x+hole->body[lp_no].poly.p[1].x, hole->s.y+hole->body[lp_no].poly.p[1].y, hole->s.x+hole->body[lp_no].poly.p[3].x, hole->s.y+hole->body[lp_no].poly.p[3].y, 1);
			rectfill(bp, hole->s.x+hole->body[rp_no].poly.p[1].x, hole->s.y+hole->body[rp_no].poly.p[1].y, hole->s.x+hole->body[rp_no].poly.p[3].x, hole->s.y+hole->body[rp_no].poly.p[3].y, vert->ref_no[5]);
			rect(bp, hole->s.x+hole->body[rp_no].poly.p[1].x, hole->s.y+hole->body[rp_no].poly.p[1].y, hole->s.x+hole->body[rp_no].poly.p[3].x, hole->s.y+hole->body[rp_no].poly.p[3].y, 1);
			break;
		}

		case VERT_FLAG_BUBBLE_HORIZ:
		case VERT_FLAG_BUBBLE_VERT:
			//drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
			circle(hole->layer_pic[2], hole->s.x+vert->s.x, hole->s.y+vert->s.y, vert->radius, makecol(255, 255, 255));
			circlefill(hole->layer_pic[2], hole->s.x+vert->s.x-vert->radius/2, hole->s.y+vert->s.y-vert->radius/2, vert->radius/5, makecol(255, 255, 255));
			//solid_mode();
			break;
		case VERT_FLAG_COIN_0:
			if(vert->ref_no[0] > 0)
			{
				masked_blit(i_bank.coinpic[vert->ref_no[3]/2], bp, 0, 0, hole->s.x+vert->s.x-i_bank.coinpic[vert->ref_no[3]/2]->w/2, hole->s.y+vert->s.y-i_bank.coinpic[vert->ref_no[3]/2]->h/2, i_bank.coinpic[vert->ref_no[3]/2]->w, i_bank.coinpic[vert->ref_no[3]/2]->h);
			}
			else
			{
				if(vert->ref_no[4] > 0)
				{
					vfont_printf(bp, gfont, hole->s.x+vert->s.x, hole->s.y+vert->s.y, 0, "%i", vert->ref_no[5]);
				}
			}
			break;

		default:
			break;
	}
}

//go through the display code for all verts in this hole
void hole_vert_display_all(BITMAP * bp, HOLE * hole, COURSE_IMAGE_BANK i_bank)
{
	int i;

	for(i = 0; i < hole->num_verts+hole->num_coins*(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT); i++)
	{
		hole_vert_display(bp, &hole->vert[i], hole, i_bank);
	}
}

//debugging stuff
void draw_hole_debug(BITMAP * bp, HOLE * hole)
{
	int p;
	
	for(p = 0; p < hole->num_polys; p++)
	{
		draw_poly2(bp, hole->body[p].poly, hole->s, 11);
	}
}

//hole background display stuff
void hole_bkg_display(BITMAP * bp, HOLE * hole, COURSE_IMAGE_BANK i_bank)
{

	//clear this layer off
	clear_to_color(hole->layer_pic[2], 0);

	//draw the hole's bitmap
	draw_hole_bkg(bp, hole);

	//draw the verts
	hole_vert_display_all(bp, hole, i_bank);
        
	//debug stuff..
	if(main_game_vars.debug_on) draw_hole_debug(bp, hole);
}

//draw the ball "mirage" effect
void ball_mirage_display(BITMAP * bp, BALL * ball)
{
	//set to transparent drawing mode
	color_map = &trans_table;
	drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
	//draw the circle primitive
	circlefill(bp, ball->s_med.x, ball->s_med.y, ball->radius, BALL_COLOR);
	//return to normal drawing mode
	solid_mode();		
}

void ball_shadow_display(BITMAP * bp, BALL * ball)
{
	//set to transparent drawing mode
	color_map = &trans_table;
	drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
	//draw the circle primitive
	circlefill(bp, ball->s.x+1, ball->s.y+3, ball->radius, BALL_SHADOW_COLOR);
	//return to normal drawing mode
	solid_mode();	
}

//draw this player's ball
void player_ball_display(BITMAP * bp, BALL * ball, char active, char shadows_on)
{
	if(active)
	{
		if(ball->visible)
		{
			if(shadows_on)
			{
				ball_shadow_display(bp, ball);
			}
			//draw the ball circle
			circlefill(bp, ball->s.x, ball->s.y, ball->radius, BALL_COLOR);
			
			//draw the idle timer if it's active
			if(ball->idle_timer > 0)
			{
				if(ball->mode == 0)
				{
					//change this to transparency later..
					vfont_printf(bp, gfont, ball->s.x, ball->s.y-ball->radius*2-ball->idle_timer/25, 0, "%i", (ball->idle_timeout+1-ball->idle_timer)/19+1);
				}
				else
				{
					vfont_printf(bp, gfont, ball->s.x, ball->s.y-ball->radius*2-ball->idle_timer/25, 0, "%i", ((ball->idle_timeout * 2)+1-ball->idle_timer)/19+1);
				}
			}
		}
		else
		{
			//draw the splash animation if the ball is sinking
			if(ball->mode == BALL_MODE_WATER_SINKING)
			{
				do_anim_def(bp, &ball->anim_def[1], ball->s.x-12, ball->s.y-12, 0);
			}
		}
		//draw the translucent ball if necessary
		if(ball->mirage_visible)
		{
			ball_mirage_display(bp, ball);
		}
		else
		{
			//draw the ball as a smaller circle if it's inactive
			if(ball->visible)
			{
				circlefill(bp, ball->s.x, ball->s.y, ball->radius/3, BALL_COLOR);
			}
		}
	}
}

//do the ball display code for all players' balls
void player_ball_display_all(BITMAP * bp, PLAYER pl[], int num_players, char shadows_on)
{
	int p;

	for(p = 0; p < 4; p++)
	{
		if(vgolf_player_list[p])
		{
			player_ball_display(bp, &pl[p].ball, (p == main_game_vars.cur_player), shadows_on);
		}
	}
}

//draw one player's club
void player_club_display(BITMAP * bp, PLAYER * pl)
{
	if(pl->ball.mode == BALL_MODE_IDLE)
	{
		//set to transparent drawing mode
		color_map = &trans_table;
		drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
		//draw the triangle primitives that define the club area
		//draw a darker outline around the triangle in case we're on a lighter backdrop
		//return to normal drawing mode
		solid_mode();
		triangle(bp, pl->ball.s.x, pl->ball.s.y, pl->ball.s.x-(MAX_CLUB_POWER/2)*cos(pl->f_club_angle+pl->f_club_width), pl->ball.s.y-(MAX_CLUB_POWER/2)*sin(pl->f_club_angle+pl->f_club_width), pl->ball.s.x-(MAX_CLUB_POWER/2)*cos(pl->f_club_angle-pl->f_club_width), pl->ball.s.y-(MAX_CLUB_POWER/2)*sin(pl->f_club_angle-pl->f_club_width), makecol(255, 255, 255));
		triangle(bp, pl->ball.s.x, pl->ball.s.y, pl->ball.s.x+pl->club_v[0].x, pl->ball.s.y+pl->club_v[0].y, pl->ball.s.x+pl->club_v[1].x, pl->ball.s.y+pl->club_v[1].y, pl->club_color);
		line(bp, pl->ball.s.x, pl->ball.s.y, pl->ball.s.x-(MAX_CLUB_POWER/2)*cos(pl->f_club_angle+pl->f_club_width), pl->ball.s.y-(MAX_CLUB_POWER/2)*sin(pl->f_club_angle+pl->f_club_width), makecol(0, 0, 0));
		line(bp, pl->ball.s.x, pl->ball.s.y, pl->ball.s.x-(MAX_CLUB_POWER/2)*cos(pl->f_club_angle-pl->f_club_width), pl->ball.s.y-(MAX_CLUB_POWER/2)*sin(pl->f_club_angle-pl->f_club_width), makecol(0, 0, 0));
		line(bp, pl->ball.s.x-(MAX_CLUB_POWER/2)*cos(pl->f_club_angle+pl->f_club_width), pl->ball.s.y-(MAX_CLUB_POWER/2)*sin(pl->f_club_angle+pl->f_club_width), pl->ball.s.x-(MAX_CLUB_POWER/2)*cos(pl->f_club_angle-pl->f_club_width), pl->ball.s.y-(MAX_CLUB_POWER/2)*sin(pl->f_club_angle-pl->f_club_width), makecol(0, 0, 0));

		/* draw crosshair */
		line(bp, pl->ball.s.x + 48.0 * cos(pl->f_club_angle) - 3 - 1, pl->ball.s.y + 48.0 * sin(pl->f_club_angle) + 1, pl->ball.s.x + 48.0 * cos(pl->f_club_angle) + 3 - 1, pl->ball.s.y + 48.0 * sin(pl->f_club_angle) + 1, makecol(0, 0, 0));
		line(bp, pl->ball.s.x + 48.0 * cos(pl->f_club_angle) - 1, pl->ball.s.y + 48.0 * sin(pl->f_club_angle) - 3 + 1, pl->ball.s.x + 48.0 * cos(pl->f_club_angle) - 1, pl->ball.s.y + 48.0 * sin(pl->f_club_angle) + 3 + 1, makecol(0, 0, 0));
		line(bp, pl->ball.s.x + 48.0 * cos(pl->f_club_angle) - 3, pl->ball.s.y + 48.0 * sin(pl->f_club_angle), pl->ball.s.x + 48.0 * cos(pl->f_club_angle) + 3, pl->ball.s.y + 48.0 * sin(pl->f_club_angle), makecol(255, 255, 255));
		line(bp, pl->ball.s.x + 48.0 * cos(pl->f_club_angle), pl->ball.s.y + 48.0 * sin(pl->f_club_angle) - 3, pl->ball.s.x + 48.0 * cos(pl->f_club_angle), pl->ball.s.y + 48.0 * sin(pl->f_club_angle) + 3, makecol(255, 255, 255));
	}
}

//draw all players' clubs
void player_club_display_all(BITMAP * bp, PLAYER pl[], int num_players, int cur_player)
{
	player_club_display(bp, &pl[cur_player]);
}

//draws the player's trajectory line
void player_trajectory_display(BITMAP * bp, PLAYER * pl, HOLE * hole){
	//only display when the trajectory is activated
	if(pl->traj_active){		
		int i;

		//set to transparent drawing mode
		color_map = &trans_table;
		drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
		
		for(i = 0; i < pl->traj_count-1; i++){
			float ang1 = atan2(pl->traj_v[i+1].y-pl->traj_v[i].y, pl->traj_v[i+1].x-pl->traj_v[i].x)-M_PI/2;
			float x_comp1 = cos(ang1), y_comp1 = sin(ang1);
			//draw the line primitive
			//line(bp, pl->traj_v[i].x, pl->traj_v[i].y, pl->traj_v[i+1].x, pl->traj_v[i+1].y, 95-i/4);
			draw_dotted_line(bp, (int)(pl->traj_v[i].x+x_comp1), (int)(pl->traj_v[i].y+y_comp1), (int)(pl->traj_v[i+1].x+x_comp1), (int)(pl->traj_v[i+1].y+y_comp1), 95-i/4, pl->traj_line_timer, 2);
			draw_dotted_line(bp, (int)pl->traj_v[i].x, (int)pl->traj_v[i].y, (int)pl->traj_v[i+1].x, (int)pl->traj_v[i+1].y, 95-i/4, pl->traj_line_timer, 2);
			}
		//return to normal drawing mode
		solid_mode();	
		}
}

//draw this player's hud
void player_hud_display(BITMAP * bp, PLAYER * pl, V_FONT * vft, HOLE * hole, int plno)
{

	if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT)
	{
		vfont_printf(bp, gfont, pl->hud.x+vft->w*3, pl->hud.y, 0, "%s", vgolf_profiles.item[pl->profile].name);
		vfont_printf(bp, gfont, pl->hud.x+vft->w*3+vft->w*strlen("Abe Lincoln"), pl->hud.y, 0, "coins:%i/%i", pl->coins_total, hole->num_coins);
		vfont_printf(bp, gfont, pl->hud.x+vft->w*3+vft->w*strlen("Abe Lincoln coins:00/00   "), pl->hud.y, 0, "strokes:%i", pl->cur_strokes);
		vfont_printf(bp, gfont, pl->hud.x+vft->w*3, pl->hud.y+vft->h, 0, "score:%i", pl->scorecard.coin_score);
		masked_blit(pl->pic[pl->pic_mode], bp, 0, 0, pl->hud.x-5, pl->hud.y-6, pl->pic[pl->pic_mode]->w, pl->pic[pl->pic_mode]->h);
	}
	else
	{
		int tcol, wcol;

		if(plno == main_game_vars.cur_player)
		{
			tcol = makecol(0, 255, 0);
			wcol = makecol(255, 255, 255);
		}
		else
		{
			tcol = makecol(0, 128, 0);
			wcol = makecol(128, 128, 128);
		}
		//print the current stroke tally
		outline_text_shadow(bp, gfont2, pl->hud.x + 58, pl->hud.y - 8, -2, 2, wcol, makecol(0, 0, 0), makecol(0, 0, 0), vgolf_profiles.item[pl->profile].name);
		sprintf(text_buffer, "Strokes: %i", pl->cur_strokes);
		outline_text_shadow(bp, gfont2, pl->hud.x + 58, pl->hud.y + 24 - 8, -2, 2, tcol, makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);

		//check to see if the char pic overlays any important stuff (hole, ball, etc)
		if(plno == 0 || plno == 1)
		{
			masked_blit(pl->pic[pl->pic_mode], bp, 0, 0, pl->hud.x-5, pl->hud.y-6, pl->pic[pl->pic_mode]->w, pl->pic[pl->pic_mode]->h);
		}
		else
		{
			masked_blit(pl->pic[pl->pic_mode], bp, 0, 0, pl->hud.x-5, pl->hud.y-vft->h, pl->pic[pl->pic_mode]->w, pl->pic[pl->pic_mode]->h);
		}
	}
}

//go through and display all player huds..
void player_hud_display_all(BITMAP * bp, PLAYER pl[], int num_players, V_FONT * vf, HOLE * hole)
{
	int p;

	for(p = 0; p < 4; p++)
	{
		if(vgolf_player_list[p])
		{
			player_hud_display(bp, &pl[p], vf, hole, p);
		}
	}
}

//display misc. hole info
void hole_hud_display(BITMAP * bp, V_FONT * vf, HOLE * hole, int mode)
{
	//draw par score
	if(mode != GAMEMODE_COIN_COLLECT)
	{
		sprintf(text_buffer, "Par %i", hole->par_score);
		outline_text_shadow_center(bp, gfont2, SCREEN_W / 2, SCREEN_H - 24, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
	}
}

//handle the in-game message queue..
void game_vmessage_display(BITMAP * bp, V_MESSAGE msg, FONT * vf)
{

//		textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), makecol(0, 0, 0), "%f, %f", -vmsg->length * vmsg->attrib_ref[1], vmsg->s.x);
        switch(msg.active){
                case 1:  //only draw the message when it's active
                        vfont_message_textout(bp, vf, &msg, msg.method, main_game_vars.ingame_timer_change);
                        break;

                default:
                        break;
                }
}

//draw the victory screen background image
void course_victory_screen_bg_display(BITMAP * bp, VGOLF_COURSE * cr)
{
	masked_blit(cr->image_bank.victory_bgpic, bp, 0, 0, 0, 0, cr->image_bank.victory_bgpic->w, cr->image_bank.victory_bgpic->h);
}

//draw the victory screen clouds
void course_victory_screen_cloud_display(BITMAP * bp, VGOLF_COURSE * cr, MAIN_GAME_VARS * gvars)
{
	int i;
	
	for(i = 3; i >= 0; i--)
	{
		masked_blit(cr->image_bank.victory_cloudpic[i/2], bp, 0, 0, gvars->vict_cloud_x[i], gvars->vict_cloud_y[i], cr->image_bank.victory_cloudpic[i/2]->w, cr->image_bank.victory_cloudpic[i/2]->h);
	}
}

//display the victory message
void victory_screen_text_display(BITMAP * bp, V_FONT * vf, int x, int y, PLAYER * pl, VGOLF_COURSE * cr, MAIN_GAME_VARS * gvars, char tied)
{
	int i;
	char completed = 1;
                
	switch(gvars->game_mode)
	{
		case GAMEMODE_STROKE_PLAY:
			sprintf(text_buffer, "Congratulations, %s.", tied ? "players" : vgolf_profiles.item[pl->profile].name);
			outline_text_shadow_center(bp, gfont2, x, y, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
			sprintf(text_buffer, "Here %s courtesy of", tied ? "are some trophies" : "is a trophy");
			outline_text_shadow_center(bp, gfont2, x, y + 30, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
			sprintf(text_buffer, "the %s club membership!", cr->name);
			outline_text_shadow_center(bp, gfont2, x, y + 60, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
				
			if(gvars->got_hiscore)
			{
				sprintf(text_buffer, "You got a high score!");
				outline_text_shadow_center(bp, gfont2, x, y + 90, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
			}
			if(vgolf_player_count == 1)
			{
				sprintf(text_buffer, "Do come again!");
				outline_text_shadow_center(bp, gfont2, x, SCREEN_H - 100, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
			}
			break;
			case GAMEMODE_COIN_COLLECT:

				for(i = 0; i < cr->num_holes; i++)
				{
					if(!pl->scorecard.match_won[i]) completed = 0;
				}
				if(completed)
				{
					vfont_printf_center(bp, vf, x, y, 0, "Coin collector mode completed.");
					vfont_printf_center(bp, vf, x, y+30, 0, "Nicely done, %s!", vgolf_profiles.item[pl->profile].name);
				}
				if(main_game_vars.got_hiscore)
				{
					vfont_printf_center(bp, vf, x, y+60, 0, "You got a high score!");
				}
				break;
			default:
				break;
	}
}

//draw the player podium and trophy stuff
void victory_screen_player_podium_display(BITMAP * bp, int x, int y, V_FONT * vf, VGOLF_COURSE * cr, MAIN_GAME_VARS * gvars, PLAYER pl, int plno, int place, int spot)
{
	int dy = 0, dx = 0;
	char * ptext[3] = {"1st", "2nd", "3rd"};
	
	//the lower the place, the higher the podium sits
	if(vgolf_player_count == 1)
	{
		dy = (gvars->vict_pody / (vgolf_player_count + 1)) * spot;
		//podium sits in the center of the screen in single player mode
		dx = 0;
	}
	else if(vgolf_player_count == 2)
	{
		dy = (gvars->vict_pody / (vgolf_player_count + 1)) * spot;
		//podiums sit side by side centered in the middle of the screen
		if(!spot) dx = -cr->image_bank.victory_podiumpic->w / 2;
		else dx = cr->image_bank.victory_podiumpic->w / 2;
	}
	else if(vgolf_player_count >= 3)
	{
		dy = (gvars->vict_pody / 6) * spot;
		//podiums sit in cross-pattern in the middle of the screen
		if(!spot) dx = -cr->image_bank.victory_podiumpic->w / 2 - 48;
		else if(spot == 1) dx = cr->image_bank.victory_podiumpic->w / 2 + 48;
		else dx = 0;
	}
	
	//draw place string (1st, 2nd, or 3rd)
//    sprintf(text_buffer, "Strokes: %i", pl->cur_strokes);
	if(place >= 0 && place < 3)
	{
		//draw podium, trophy, and player character image
		masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, x + dx - cr->image_bank.victory_podiumpic->w / 2, gvars->vict_pody + dy, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
		masked_blit(cr->image_bank.victory_trophypic[place], bp, 0, 0, x + dx - cr->image_bank.victory_trophypic[0]->w / 2, gvars->vict_pody + dy - cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
		masked_blit(pl.pic[1], bp, 0, 0, x + dx - pl.pic[1]->w / 2, gvars->vict_pody + dy + 55, pl.pic[1]->w, pl.pic[1]->h);
	
		outline_text_shadow_center(bp, gfont, x + dx - 3, gvars->vict_pody + dy + 20, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), ptext[place]);
	}
	
	switch(gvars->game_mode)
	{
		case GAMEMODE_STROKE_PLAY:			
			break;
		case GAMEMODE_MATCH_PLAY:
			vfont_printf_center(bp, vf, x + dx, gvars->vict_pody + dy + 70, 0, "%i", pl.scorecard.matches_won);
			break;
		default:
			break;
	}
}

//draws the victory screen
void victory_screen_display(BITMAP * bp, V_FONT * vf, VGOLF_COURSE * cr, MAIN_GAME_VARS * gvars, PLAYER pl[], int num_players)
{
	int i;
	int place[4] = {0, 1, 2, 3};

	course_victory_screen_bg_display(bp, cr);            //draw background layer
	course_victory_screen_cloud_display(bp, cr, gvars);  //draw clouds
	
	switch(vgolf_player_count)
	{
		case 1:    //single player mode..
			victory_screen_player_podium_display(bp, SCREEN_W/2, SCREEN_H/2, vf, cr, gvars, pl[gvars->cur_player], 0, 0, 0);
			//draw the text last..
			victory_screen_text_display(bp, vf, SCREEN_W/2, 50, &pl[0], cr, gvars, 0);
			break;
		case 2:    //multi-player mode..
		case 3:    //multi-player mode..
		case 4:    //multi-player mode..
			if(gvars->vict_winner[0] != -1)
			{ //not tied, do standard screen
				//draw victory screen for all players
				if(player_course_stroke_total(pl[gvars->vict_winner[0]], cr->num_holes) == player_course_stroke_total(pl[gvars->vict_winner[1]], cr->num_holes) && player_course_stroke_total(pl[gvars->vict_winner[1]], cr->num_holes) == player_course_stroke_total(pl[gvars->vict_winner[2]], cr->num_holes))
				{
					place[0] = 0;
					place[1] = 0;
					place[2] = 0;
				}
				else if(player_course_stroke_total(pl[gvars->vict_winner[0]], cr->num_holes) == player_course_stroke_total(pl[gvars->vict_winner[1]], cr->num_holes))
				{
					place[0] = 0;
					place[1] = 0;
					place[2] = 1;
				}
				else if(player_course_stroke_total(pl[gvars->vict_winner[1]], cr->num_holes) == player_course_stroke_total(pl[gvars->vict_winner[2]], cr->num_holes))
				{
					place[0] = 0;
					place[1] = 1;
					place[2] = 1;
				}
				for(i = 0; i < 4; i++)
				{
					if(gvars->vict_winner[i] >= 0 && gvars->vict_winner[i] < 4 && vgolf_player_list[gvars->vict_winner[i]])
					{
						victory_screen_player_podium_display(bp, SCREEN_W/2, SCREEN_H/2, vf, cr, gvars, pl[gvars->vict_winner[i]], i, place[gvars->vict_winner[i]], gvars->vict_winner[i]);
					}
				}
				//draw the text last..
				victory_screen_text_display(bp, vf, SCREEN_W/2, 100, &pl[gvars->vict_winner[0]], cr, gvars, (gvars->vict_winner[0] != -1));
			}
			else
			{
			}
			break;
		default:
			break;
	}
	switch(main_game_vars.game_state)
	{
		case GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGMID: //victory screen is set in place
		case GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGMID:
		{
			hyperlink_page_render(game_complete_page, bp);
			break;
		}
	}
}


//draws the course scoreboard
void course_scoreboard_display(BITMAP * bp, V_FONT * vf, VGOLF_COURSE * cr, PLAYER pl[], int num_players, int cur_hole)
{
	int s, i, seg_h = bp->h/cr->scoreboard.num_scoreboard_segs;
	char out_text[256];
	int tc;
	
	masked_blit(cr->scoreboard.bgpic, bp, 0, 0, 0, (cr->scoreboard.scoreboard_seg_count - 1) * seg_h + cr->scoreboard.scoreboard_seg_counter - 48, cr->scoreboard.bgpic->w, cr->scoreboard.bgpic->h);
	for(s = 0; s < cr->scoreboard.scoreboard_seg_count - 1; s++)
	{
		masked_blit(cr->scoreboard.bgpic, bp, 0, 0, 0, s*seg_h, cr->scoreboard.bgpic->w, cr->scoreboard.bgpic->h);
	}

	if(cr->scoreboard.scoreboard_seg_count >= 10)
	{
		for(i = 0; i < 4; i++)
		{
			int n, p_total, c_total;
		        
			if(vgolf_player_list[i]) 
			{
	
		        //only draw this stuff when it appears within the visible scoreboard area
		        if(cr->scoreboard.y+cr->scoreboard.grid_y[i] <= cr->scoreboard.scoreboard_seg_count*seg_h)
		        {
			        if(show_records)
			        {
						//current totals
						p_total = vgolf_profiles.item[pl[i].profile].course_info[selected_course].total_score;
						c_total = vgolf_courses.item[selected_course].par;
//						sprintf(text_buffer, "%d - %d", p_total, c_total);
//						outline_text_shadow_center(bp, gfont2, 32, 0, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
		                                                                
						draw_character_ex(bp, main_game_vars.image_bank.scorecard, cr->scoreboard.x + cr->scoreboard.grid_x[i] - 2, cr->scoreboard.y + cr->scoreboard.grid_y[i] + 2, makecol(0, 0, 0), makecol(0, 0, 0));
						masked_blit(main_game_vars.image_bank.scorecard, bp, 0, 0, cr->scoreboard.x + cr->scoreboard.grid_x[i], cr->scoreboard.y + cr->scoreboard.grid_y[i], main_game_vars.image_bank.scorecard->w, main_game_vars.image_bank.scorecard->h);
						if(vgolf_profiles.item[pl[i].profile].course_info[selected_course].completed <= 0)
						{
							sprintf(text_buffer, "P%i: %s (N/A)", i+1, vgolf_profiles.item[pl[i].profile].name);
						}
						else if(p_total-c_total == 0)
						{
							sprintf(text_buffer, "P%i: %s (E)", i+1, vgolf_profiles.item[pl[i].profile].name);
						}
						else if(p_total-c_total > 0)
						{
							sprintf(text_buffer, "P%i: %s (+%d)", i+1, vgolf_profiles.item[pl[i].profile].name, p_total - c_total);
						}
						else
						{
							sprintf(text_buffer, "P%i: %s (%d)", i+1, vgolf_profiles.item[pl[i].profile].name, p_total - c_total);
						}
						outline_text_shadow_center(bp, gfont2, cr->scoreboard.grid_x[i] + 110, cr->scoreboard.y + cr->scoreboard.grid_y[i] - 2, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
			        }
			        else
		        	{
						//current totals
						p_total = player_course_stroke_total(pl[i], cur_hole+1);
						c_total = course_stroke_total(pl[i], cur_hole+1, cr->num_holes);
		                                                                
						draw_character_ex(bp, main_game_vars.image_bank.scorecard, cr->scoreboard.x + cr->scoreboard.grid_x[i] - 2, cr->scoreboard.y + cr->scoreboard.grid_y[i] + 2, makecol(0, 0, 0), makecol(0, 0, 0));
						masked_blit(main_game_vars.image_bank.scorecard, bp, 0, 0, cr->scoreboard.x + cr->scoreboard.grid_x[i], cr->scoreboard.y + cr->scoreboard.grid_y[i], main_game_vars.image_bank.scorecard->w, main_game_vars.image_bank.scorecard->h);
						if(p_total-c_total == 0)
						{
							sprintf(text_buffer, "P%i: %s (E)", i+1, vgolf_profiles.item[pl[i].profile].name);
						}
						else if(p_total-c_total > 0)
						{
							sprintf(text_buffer, "P%i: %s (+%d)", i+1, vgolf_profiles.item[pl[i].profile].name, p_total - c_total);
						}
						else
						{
							sprintf(text_buffer, "P%i: %s (%d)", i+1, vgolf_profiles.item[pl[i].profile].name, p_total - c_total);
						}
						outline_text_shadow_center(bp, gfont2, cr->scoreboard.grid_x[i] + 110, cr->scoreboard.y + cr->scoreboard.grid_y[i] - 2, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
					}
				}
	                
				switch(main_game_vars.game_mode)
				{
					case GAMEMODE_STROKE_PLAY:
					
					    //only draw this stuff when it appears within the visible scoreboard area
				        if(cr->scoreboard.y+cr->scoreboard.grid_y[i] > cr->scoreboard.scoreboard_seg_count*seg_h) break;
					        
						//draw the numbers for the holes
						for(n = 0; n < 9; n++)
						{
							sprintf(out_text, "%d", n + 1);
							outline_text_shadow_center(buffer, gfont2, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*25 + 13, cr->scoreboard.y+cr->scoreboard.grid_y[i]+1 + 21, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), out_text);
	
							//if there are more than 9 holes, draw the numbers for the back 9
							if(cr->num_holes > 9)
							{
								sprintf(out_text, "%d", n + 10);
								outline_text_shadow_center(buffer, gfont2, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*25 + 13, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3+1 + 18, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), out_text);
							}
						}
						/* draw the scores */
				        if(show_records && vgolf_profiles.item[pl[i].profile].course_info[selected_course].completed <= 0) break;
						for(n = 0; n < 9; n++)
						{
							if(n < next_hole || show_records)
							{
								sprintf(out_text, "%d", show_records ? vgolf_profiles.item[pl[i].profile].course_info[selected_course].score[n]: pl[i].scorecard.score[n]);
								tc = makecol(0, 192, 0);
								outline_text_shadow_center(buffer, gfont2, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*25 + 13, cr->scoreboard.y+cr->scoreboard.grid_y[i] + cr->scoreboard.num_h+1 + 28, -2, 2, tc, makecol(0, 0, 0), makecol(0, 0, 0), out_text);
							}
	
							//if there are more than 9 holes, draw the numbers for the back 9
							if(cr->num_holes > 9)
							{
								if(n+9 < next_hole || show_records)
								{
									sprintf(out_text, "%d", show_records ? vgolf_profiles.item[pl[i].profile].course_info[selected_course].score[n + 9]: pl[i].scorecard.score[n + 9]);
									tc = makecol(0, 192, 0);
									outline_text_shadow_center(buffer, gfont2, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*25 + 13, cr->scoreboard.y+cr->scoreboard.grid_y[i] + cr->scoreboard.num_h+1 + 28 + 50 - 2, -2, 2, tc, makecol(0, 0, 0), makecol(0, 0, 0), out_text);
								}
							}
						}
						//draw individual player scores
						break;
						
					case GAMEMODE_MATCH_PLAY:
					
						//only draw this stuff when it appears within the visible scoreboard area
				        if(cr->scoreboard.y+cr->scoreboard.grid_y[i] > cr->scoreboard.scoreboard_seg_count*seg_h) break;
				        
						//current totals
						p_total = player_course_stroke_total(pl[i], cur_hole);
						c_total = course_stroke_total(pl[i], cur_hole, cr->num_holes);
						if(p_total-c_total == 0) vfont_printf(bp, gfont3, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "e");
						else if(p_total-c_total > 0) vfont_printf(bp, gfont3, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "+%i", p_total-c_total);
						else vfont_printf(bp, gfont3, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "%i", p_total-c_total);
	
						vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2, 0, "%i", pl[i].scorecard.matches_won);
	
						for(n = 0; n < 9; n++)
						{
							masked_blit(cr->scoreboard.numpic[n+1], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
	
							if(n <= main_game_vars.cur_hole)
							{
								//print stroke total
								if(pl[i].scorecard.score[n] < 10)
								{
									masked_blit(cr->scoreboard.numpic[pl[i].scorecard.score[n]], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
								}
								else vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+1, 0, "X");
	
								//print win-loss statistic
								if(pl[i].scorecard.match_won[n] == 0)
								{
									vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+1, 0, "L");
								}
								else if(pl[i].scorecard.match_won[n] == 1)
								{
									vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+1, 0, "W");
								}
								else
								{
									vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+1, 0, "T");
								}
							}
	
							if(cr->num_holes > 9)
							{
								masked_blit(cr->scoreboard.numpic[n+10], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
								if(n+9 <= main_game_vars.cur_hole)
								{
									//print stroke total
									if(pl[i].scorecard.score[n+9] < 10)
									{
										masked_blit(cr->scoreboard.numpic[pl[i].scorecard.score[n+9]], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
									}
									else vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+1, 3*vf->w/4, "X");
									
									//print win-loss statistic
									if(pl[i].scorecard.match_won[n+9] == 0)
									{
										vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+1, 0, "L");
									}
									else if(pl[i].scorecard.match_won[n+9] == 1)
									{
										vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+1, 0, "W");
									}
									else
									{
										vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+1, 0, "T");
									}
								}
							}
						}
						//draw individual player scores
						if(main_game_vars.hole_finished)
						{
							if(pl[i].scorecard.match_won[main_game_vars.cur_hole] == 1)
							{
								vfont_printf_center(bp, vf, SCREEN_W/2, SCREEN_H/2-40, 0, "Match goes to %s!", vgolf_profiles.item[pl->profile].name);
							}
							else if(pl[i].scorecard.match_won[main_game_vars.cur_hole] == 2)
							{
								vfont_printf_center(bp, vf, SCREEN_W/2, SCREEN_H/2-40, 0, "It's a tie..");
							}
						}                
	                	break;
		                	
	                case GAMEMODE_COIN_COLLECT:
	                
		        		//only draw this stuff when it appears within the visible scoreboard area
		        		if(cr->scoreboard.y+cr->scoreboard.grid_y[i] > cr->scoreboard.scoreboard_seg_count*seg_h) break;
			                
		        		//current totals
						vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "%i", pl[i].scorecard.coin_score);
	
						for(n = 0; n < 9; n++)
						{
							masked_blit(cr->scoreboard.numpic[n+1], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
							if(n <= main_game_vars.cur_hole)
							{
								rectfill(bp, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+1, cr->scoreboard.x+cr->scoreboard.grid_x[i]+(n+1)*cr->scoreboard.num_w-1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2-1, 14+(29+50*(pl[i].scorecard.score[n] != pl[i].scorecard.par_score[n]))*(pl[i].scorecard.par_score[n] != 0));
								vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+3, 2*vf->w/3, "%i", pl[i].scorecard.score[n]/10);
								vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1+vf->w/2, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+3, 2*vf->w/3, "%i", pl[i].scorecard.score[n]%10);
								vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+3, 2*vf->w/3, "%i", pl[i].scorecard.par_score[n]/10);
								vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1+vf->w/2, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+3, 2*vf->w/3, "%i", pl[i].scorecard.par_score[n]%10);
							}
	
							if(cr->num_holes > 9)
							{
								masked_blit(cr->scoreboard.numpic[n+10], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
								if(n+9 <= main_game_vars.cur_hole)
								{
									rectfill(bp, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+1, cr->scoreboard.x+cr->scoreboard.grid_x[i]+(n+1)*cr->scoreboard.num_w-1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5-1, 14+(29+50*(pl[i].scorecard.score[n+9] != pl[i].scorecard.par_score[n+9]))*(pl[i].scorecard.par_score[n+9] != 0));
									vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+3, 2*vf->w/3, "%i", pl[i].scorecard.score[n+9]/10);
									vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1+vf->w/2, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+3, 2*vf->w/3, "%i", pl[i].scorecard.score[n+9]%10);
									vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+3, 2*vf->w/3, "%i", pl[i].scorecard.par_score[n+9]/10);
									vfont_printf(bp, gfont, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1+vf->w/2, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+3, 2*vf->w/3, "%i", pl[i].scorecard.par_score[n+9]%10);
								}
							}
						}
						break;
						
		               	default:
		               	
		               		break;
				}
			}
		}
		
		//identify course name (only display if it lies within the visible scoreboard area)
		if(cr->scoreboard.y+cr->scoreboard.cname_y <= cr->scoreboard.scoreboard_seg_count*seg_h)
		{
			sprintf(text_buffer, "Course: %s", cr->name);
			outline_text_shadow(bp, gfont2, cr->scoreboard.x+cr->scoreboard.cname_x, cr->scoreboard.y+cr->scoreboard.cname_y - 6, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
			if(next_hole < 18)
			{
		        sprintf(text_buffer, "Next hole: %i", next_hole + 1);
				outline_text_shadow(bp, gfont2, 640 - text_length(gfont2, text_buffer) - 18, cr->scoreboard.y+cr->scoreboard.cname_y - 6, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
			}
			else
			{
		        sprintf(text_buffer, "Next: Awards Ceremony");
				outline_text_shadow(bp, gfont2, 640 - text_length(gfont2, text_buffer) - 18, cr->scoreboard.y+cr->scoreboard.cname_y - 6, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
			}
		}
		if(cr->scoreboard.scoreboard_seg_count >= cr->scoreboard.num_scoreboard_segs + 1)
		{
			hyperlink_page_render(scoreboard_page, bp);
		}
	}
}

//the main display stuff
void main_game_display(BITMAP * bp, PLAYER pl[], int num_players, VGOLF_COURSE * cr)
{

	//clear the screen buffer before doing anything
	clear_to_color(bp, 0);

	if(main_game_vars.cur_hole >= 0)
	{  //make sure the current hole isn't negative before doing anything
		//draw the hole image
		hole_bkg_display(bp, &cr->current_hole, cr->image_bank);

		//draw all player balls
		player_ball_display_all(bp, pl, num_players, vgolf_config[CONFIG_FX_ON]);

		//draw the foreground layer
		hole_fg_display(bp, &cr->current_hole);

		//draw the player clubs
		if(vgolf_state != VGOLF_STATE_BALL_SUNK)
		{
			player_club_display_all(bp, pl, num_players, main_game_vars.cur_player);
		}
        	
		//draw the current player's trajectory if active
		player_trajectory_display(bp, &pl[main_game_vars.cur_player], &cr->current_hole);
		
		//draw the player heads up displays
		player_hud_display_all(bp, pl, num_players, &game_vfont, &cr->current_hole);

		//draw the hole heads up display
		hole_hud_display(bp, &game_vfont, &cr->current_hole, main_game_vars.game_mode);
	}
        	
	switch(main_game_vars.game_state)
	{
		case GAMESTATE_SCOREBOARD_SEGIN:                   //scoreboard is segging-in
			course_scoreboard_display(bp, &game_vfont, cr, pl, num_players, main_game_vars.cur_hole);
			break;
		case GAMESTATE_SCOREBOARD_SEGMID:                  //scoreboard is set in place
			course_scoreboard_display(bp, &game_vfont, cr, pl, num_players, main_game_vars.cur_hole);
			break;
		case GAMESTATE_SCOREBOARD_SEGOUT:                  //scoreboard is leaving
			course_scoreboard_display(bp, &game_vfont, cr, pl, num_players, main_game_vars.cur_hole);
			break;
		case GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGIN:  //victory screen is segging-in
		case GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGIN:
			victory_screen_display(bp, &game_vfont, cr, &main_game_vars, pl, num_players);
			course_scoreboard_display(bp, &game_vfont, cr, pl, num_players, main_game_vars.cur_hole);
			break;
		case GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGMID: //victory screen is set in place
		case GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGMID:
			victory_screen_display(bp, &game_vfont, cr, &main_game_vars, pl, num_players);
			break;
		default:
			break;
	}

		
	//draw the message found in slot 0 (this is the current message)
	game_vmessage_display(bp, game_vmessage[0], gfont);
	if(!mouse_hidden)
	{
		draw_sprite(bp, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
	}
}

//this is the sequence that occurs in the transition between the title and the game
void do_title_game_transition(void)
{
	main_game_vars.quit_game = 0;                       //quit game = false
	main_game_vars.quit_title = 1;                      //quit title = true
	main_game_vars.game_state = GAMESTATE_SCOREBOARD_SEGIN;    //we're ready for the scoreboard seg-in sequence
	main_game_vars.game_state_old = main_game_vars.game_state;
	main_game_vars.cur_vmessage_slot = 0;                      //reset the message slot to 0
	main_game_vars.cur_player = 0;                             //give player 1 the first shot
	set_victory_screen_vars(&main_game_vars);                  //set the victory screen up
	main_game_vars.cur_hole = vgolf_start_hole - 1;                              //current hole is -1 (means we're just starting up)
}
	
//main deinitialization routine
void deinitialize(void){
	//unload everything we allocated from memory
//	deinitialize_main_game_vars();
//        deinitialize_game_menus();
        deinitialize_players(game_player, vgolf_player_count);
        
        //unload allegro keyboard and joystick handlers
        remove_keyboard();
        remove_joystick();
        
        //save the current configuration settings to file
        save_config("vgolf.cfg");
        
        //stop any music currently playing
        ncds_stop_music();

        //exit allegro
        allegro_exit();
}

void game_settings_update(void)
{
}

void game_options_update(void)
{
	if(vgolf_config[CONFIG_COMMENTARY] == 1)
	{
		sprintf(current_newmenu->vtext[0], "Audio+Visual");
	}
	else
	{
		sprintf(current_newmenu->vtext[0], "Visual");
	}
	sprintf(current_newmenu->vtext[1], "%s", vgolf_config[CONFIG_RESET_HUDS] ? "On" : "Off");
}

void online_options_update(void)
{
	if(vgolf_config[CONFIG_UPLOAD])
	{
		sprintf(current_newmenu->vtext[0], "Yes");
	}
	else
	{
		sprintf(current_newmenu->vtext[0], "No");
	}
}

void config_set_nick(void)
{
	strcpy(old_vgolf_nick, vgolf_nick);
	strcpy(entered_text, "");
	entered_text_pos = strlen("");
	entering_text = 2;
	newmenu_disable_controls();
	clear_keybuf();
}

void network_options_update(void)
{
	if(entering_text == 2)
	{
		sprintf(current_newmenu->vtext[0], "%s", entered_text);
	}
	else
	{
		sprintf(current_newmenu->vtext[0], "%s", vgolf_nick);
	}
	sprintf(current_newmenu->vtext[1], "%s", vgolf_config[CONFIG_NET_MODE] == 0 ? "Responsive" : "Smooth");
}

void video_options_update(void)
{
	sprintf(current_newmenu->vtext[0], "%s", vgolf_config[CONFIG_GFX_MODE] == GFX_AUTODETECT_WINDOWED ? "Windowed" : "Fullscreen");
	sprintf(current_newmenu->vtext[1], "%s", vgolf_config[CONFIG_DEPTH] ? "8-Bit" : "Desktop");
	sprintf(current_newmenu->vtext[2], "%s", vgolf_config[CONFIG_VSYNC] ? "On" : "Off");
}

void audio_options_update(void)
{
	sprintf(current_newmenu->vtext[0], "%d%%", vgolf_config[CONFIG_MUSIC_VOLUME]);
	sprintf(current_newmenu->vtext[1], "%d%%", vgolf_config[CONFIG_SFX_VOLUME]);
	sprintf(current_newmenu->vtext[2], "%d%%", vgolf_config[CONFIG_COMMENTARY_VOLUME]);
	sprintf(current_newmenu->vtext[3], "%s", vgolf_commentaries.name[vgolf_config[CONFIG_COMMENTARY]]);
}

void commentary_options_update(void)
{
	sprintf(current_newmenu->vtext[0], "%d%%", vgolf_config[CONFIG_COMMENTARY_VOLUME]);
	sprintf(current_newmenu->vtext[1], "%s", vgolf_commentaries.name[vgolf_config[CONFIG_COMMENTARY]]);
}

void profile_name_update(void)
{
	sprintf(current_newmenu->vtext[0], "%s", entered_text);
}

int get_course(unsigned long sum)
{
	int i;
	
	for(i = 0; i < vgolf_courses.items; i++)
	{
		if(vgolf_courses.item[i].sum == sum)
		{
			return i;
		}
	}
	return -1;
}

void vgolf_start_game(void)
{
	char text[256];
	int pick;
	int i;
	
	pick = selected_course;
	ncds_stop_music();
	if(game_course)
	{
		vgolf_destroy_course(game_course);
	}
	if(pick >= 0)
	{
		game_course = vgolf_load_course(vgolf_courses.item[pick].filename);
	}
	if(!game_course)
	{
		sprintf(text, "Cannot load course!");
		allegro_message("%s", text);
		exit(0);
	}
//	vgolf_save_course(game_course, vgolf_courses.item[pick].filename);
	if(main_game_vars.commentary)
	{
		destroy_vgolf_commentary(main_game_vars.commentary);
	}
	main_game_vars.commentary = load_vgolf_commentary(vgolf_commentaries.filename[vgolf_config[CONFIG_COMMENTARY]]);
	if(!main_game_vars.commentary)
	{
		allegro_message("Unable to load commentary!\n");
		exit(0);
	}
	
	/* count the players */
	vgolf_player_count = 0;
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
//			vgolf_player_list[vgolf_player_count] = i;
			vgolf_player_count++;
		}
	}
	
	create_trans_table(&trans_table, game_course->palette, 115, 115, 115, NULL);
	create_color_table(&shadow_table, game_course->palette, (void *)myblend, NULL);
	ncds_play_music(ncds_get_music(vgolf_courses.item[pick].filename), 1);

	do_title_game_transition();

	preinitialize_players(game_player, vgolf_player_count);
	game_speed_counter = 0;

	for(i = 0; i < GAME_VMESSAGE_QUEUE_SIZE; i++)
	{
		deactivate_vfont_message(&game_vmessage[i]);
	}
	main_game_vars.cur_vmessage_slot = 0;
	next_hole = vgolf_start_hole;
	memcpy(&game_course->current_hole, &game_course->hole[vgolf_start_hole], sizeof(HOLE));
	initialize_hole(&game_course->current_hole);       //initialize the next hole before we start it
	hole_initialize_players(game_player, vgolf_player_count, main_game_vars.cur_hole);   //initialize the players before starting the next hole
	vgolf_state = VGOLF_STATE_PLAY;
	fx_fade_start(white_palette, game_course->palette, 2, NULL);
	select_palette(game_course->palette);
	game_complete_page->element[0].d1 = VGOLF_COLOR_MENU_MAIN_DARK;
	game_complete_page->element[0].d2 = VGOLF_COLOR_MENU_MAIN;
	game_complete_page->element[0].d3 = makecol(0, 0, 0);
	scoreboard_page->element[0].d1 = VGOLF_COLOR_MENU_MAIN_DARK;
	scoreboard_page->element[0].d2 = VGOLF_COLOR_MENU_MAIN;
	scoreboard_page->element[0].d3 = makecol(0, 0, 0);
	scoreboard_page->element[1].d1 = VGOLF_COLOR_MENU_MAIN_DARK;
	scoreboard_page->element[1].d2 = VGOLF_COLOR_MENU_MAIN;
	scoreboard_page->element[1].d3 = makecol(0, 0, 0);
	scoreboard_page->element[2].d1 = VGOLF_COLOR_MENU_MAIN_DARK;
	scoreboard_page->element[2].d2 = VGOLF_COLOR_MENU_MAIN;
	scoreboard_page->element[2].d3 = makecol(0, 0, 0);
	for(i = 0; i < nmenu[5]->items; i++)
	{
		if(nmenu[5]->item[i].type == NEWMENU_ITEM_TYPE_HEADER)
		{
			nmenu[5]->item[i].color = VGOLF_COLOR_MENU_OPTION;
			nmenu[5]->item[i].acolor = VGOLF_COLOR_MENU_OPTION;
		}
		else
		{
			nmenu[5]->item[i].color = VGOLF_COLOR_MENU_MAIN_DARK;
			nmenu[5]->item[i].acolor = VGOLF_COLOR_MENU_MAIN;
		}
	}
	for(i = 0; i < nmenu[6]->items; i++)
	{
		if(nmenu[6]->item[i].type == NEWMENU_ITEM_TYPE_HEADER)
		{
			nmenu[6]->item[i].color = VGOLF_COLOR_MENU_OPTION;
			nmenu[6]->item[i].acolor = VGOLF_COLOR_MENU_OPTION;
		}
		else
		{
			nmenu[6]->item[i].color = VGOLF_COLOR_MENU_MAIN_DARK;
			nmenu[6]->item[i].acolor = VGOLF_COLOR_MENU_MAIN;
		}
	}
	for(i = 0; i < nmenu[13]->items; i++)
	{
		if(nmenu[13]->item[i].type == NEWMENU_ITEM_TYPE_HEADER)
		{
			nmenu[13]->item[i].color = VGOLF_COLOR_MENU_OPTION;
			nmenu[13]->item[i].acolor = VGOLF_COLOR_MENU_OPTION;
		}
		else
		{
			nmenu[13]->item[i].color = VGOLF_COLOR_MENU_MAIN_DARK;
			nmenu[13]->item[i].acolor = VGOLF_COLOR_MENU_MAIN;
		}
		if(nmenu[13]->item[i].children)
		{
			((NEWMENU_ITEM *)(nmenu[13]->item[i].child_item[0]))->color = VGOLF_COLOR_MENU_MAIN_DARK;
			((NEWMENU_ITEM *)(nmenu[13]->item[i].child_item[0]))->acolor = VGOLF_COLOR_MENU_MAIN_DARK;
		}
	}
	for(i = 0; i < nmenu[14]->items; i++)
	{
		if(nmenu[14]->item[i].type == NEWMENU_ITEM_TYPE_HEADER)
		{
			nmenu[14]->item[i].color = VGOLF_COLOR_MENU_OPTION;
			nmenu[14]->item[i].acolor = VGOLF_COLOR_MENU_OPTION;
		}
		else
		{
			nmenu[14]->item[i].color = VGOLF_COLOR_MENU_MAIN_DARK;
			nmenu[14]->item[i].acolor = VGOLF_COLOR_MENU_MAIN;
		}
		if(nmenu[14]->item[i].children)
		{
			((NEWMENU_ITEM *)(nmenu[14]->item[i].child_item[0]))->color = VGOLF_COLOR_MENU_MAIN_DARK;
			((NEWMENU_ITEM *)(nmenu[14]->item[i].child_item[0]))->acolor = VGOLF_COLOR_MENU_MAIN_DARK;
		}
	}
	for(i = 0; i < nmenu[15]->items; i++)
	{
		if(nmenu[15]->item[i].type == NEWMENU_ITEM_TYPE_HEADER)
		{
			nmenu[15]->item[i].color = VGOLF_COLOR_MENU_OPTION;
			nmenu[15]->item[i].acolor = VGOLF_COLOR_MENU_OPTION;
		}
		else
		{
			nmenu[15]->item[i].color = VGOLF_COLOR_MENU_MAIN_DARK;
			nmenu[15]->item[i].acolor = VGOLF_COLOR_MENU_MAIN;
		}
		if(nmenu[15]->item[i].children)
		{
			((NEWMENU_ITEM *)(nmenu[15]->item[i].child_item[0]))->color = VGOLF_COLOR_MENU_MAIN_DARK;
			((NEWMENU_ITEM *)(nmenu[15]->item[i].child_item[0]))->acolor = VGOLF_COLOR_MENU_MAIN_DARK;
		}
	}
	sub_menu_page->element[0].d1 = VGOLF_COLOR_MENU_MAIN_DARK;
	sub_menu_page->element[0].d2 = VGOLF_COLOR_MENU_MAIN;
	sub_menu_page->element[0].d3 = makecol(0, 0, 0);
	set_mouse_sprite(NULL);
	gametime_reset();
}

void end_game_helper(void)
{
	if(vgolf_config[CONFIG_UPLOAD])
	{
		ncds_stop_music();
		vgolf_state = VGOLF_STATE_LEADERBOARDS;
		profile_course = selected_course;
		leaderboard_mode = 1;
		select_palette(game_palette);
		create_leaderboard_page();
		strcpy(leaderboard_menu_text, "< Menu");
	}
	else
	{
		vgolf_state = VGOLF_STATE_MENU;
		set_current_newmenu(nmenu[0]);
		ncds_play_music("vgolf.dat#menu.xm", 1);
	}
	gametime_reset();
	fx_fade_start(black_palette, game_palette, 2, NULL);
	select_palette(game_palette);
	sub_menu_page->element[0].d1 = VGOLF_COLOR_MENU_MAIN_DARK;
	sub_menu_page->element[0].d2 = VGOLF_COLOR_MENU_MAIN;
	sub_menu_page->element[0].d3 = makecol(0, 0, 0);
	set_mouse_sprite(NULL);
}

void vgolf_end_game(void)
{
	fx_fade_start(game_course->palette, black_palette, 2, end_game_helper);
}

void vgolf_pause_game(void)
{
	int i;
	
	for(i = 0; i < nmenu[10]->items; i++)
	{
		if(nmenu[10]->item[i].type == NEWMENU_ITEM_TYPE_HEADER)
		{
			nmenu[10]->item[i].color = VGOLF_COLOR_MENU_OPTION;
			nmenu[10]->item[i].acolor = VGOLF_COLOR_MENU_OPTION;
		}
		else if(nmenu[10]->item[i].type == NEWMENU_ITEM_TYPE_OPTION)
		{
			nmenu[10]->item[i].color = VGOLF_COLOR_MENU_MAIN_DARK;
			nmenu[10]->item[i].acolor = VGOLF_COLOR_MENU_MAIN;
		}
	}
	set_current_newmenu(nmenu[5]);
	last_commentary = vgolf_config[CONFIG_COMMENTARY];
	vgolf_state = VGOLF_STATE_PAUSE;
}

void vgolf_resume_game(void)
{
	if(last_commentary != vgolf_config[CONFIG_COMMENTARY])
	{
		destroy_vgolf_commentary(main_game_vars.commentary);
		main_game_vars.commentary = load_vgolf_commentary(vgolf_commentaries.filename[vgolf_config[CONFIG_COMMENTARY]]);
		if(!main_game_vars.commentary)
		{
			allegro_message("Unable to load commentary!\n");
			exit(0);
		}
	}
	vgolf_state = VGOLF_STATE_PLAY;
}

void vgolf_ball_sunk(void)
{
	int i;
	
	for(i = 0; i < nmenu[11]->items; i++)
	{
		if(nmenu[11]->item[i].type == NEWMENU_ITEM_TYPE_HEADER)
		{
			nmenu[11]->item[i].color = VGOLF_COLOR_MENU_OPTION;
			nmenu[11]->item[i].acolor = VGOLF_COLOR_MENU_OPTION;
		}
		else if(nmenu[11]->item[i].type == NEWMENU_ITEM_TYPE_OPTION)
		{
			nmenu[11]->item[i].color = VGOLF_COLOR_MENU_MAIN_DARK;
			nmenu[11]->item[i].acolor = VGOLF_COLOR_MENU_MAIN;
		}
	}
	set_current_newmenu(nmenu[11]);
	vgolf_state = VGOLF_STATE_BALL_SUNK;
}

void pop_done(void)
{
	reset_letters();
	newmenu_enable_controls();
	vgolf_start_game();
}

void letter_logic(TITLE_LETTER * lp)
{
	if(lp->state != 2)
	{
		lp->x += lp->vx;
		lp->y += lp->vy;
		lp->z += lp->vz;
		if(lp->z < itofix(-500) && lp->active)
		{
			lp->active = 0;
			fx_fade_start(game_palette, white_palette, 6, pop_done);
		}
		lp->vy += ftofix(0.25);
		if(lp->state == 0)
		{
			if(lp->vy > itofix(3))
			{
				lp->vy = itofix(-3);
			}
		}
		else
		{
			if(lp->vy > itofix(100))
			{
				lp->vy = itofix(100);
			}
		}
		lp->z += lp->vz;
	}
}

void reset_letters(void)
{
	int i;
	
	letter[0].x = itofix(98);
	letter[0].y = itofix(16);
	letter[0].z = itofix(64);
	letter[0].vx = itofix(0);
	letter[0].vy = itofix(0);
	letter[0].vz = itofix(0);
	letter[0].state = 0;
	letter[1].x = itofix(200);
	letter[1].y = itofix(64);
	letter[1].z = itofix(0);
	letter[1].vx = itofix(0);
	letter[1].vy = itofix(0);
	letter[1].vz = itofix(0);
	letter[1].state = 0;
	letter[2].x = itofix(640 / 2 - letter[2].bp->w / 2);
	letter[2].y = itofix(64);
	letter[2].z = itofix(0);
	letter[2].vx = itofix(0);
	letter[2].vy = itofix(0);
	letter[2].vz = itofix(0);
	letter[2].state = 0;
	letter[3].x = itofix(354);
	letter[3].y = itofix(64);
	letter[3].z = itofix(0);
	letter[3].vx = itofix(0);
	letter[3].vy = itofix(0);
	letter[3].vz = itofix(0);
	letter[3].state = 0;
	letter[4].x = itofix(410);
	letter[4].y = itofix(64);
	letter[4].z = itofix(0);
	letter[4].vx = itofix(0);
	letter[4].vy = itofix(0);
	letter[4].vz = itofix(0);
	letter[4].state = 0;
	for(i = 0; i < 5; i++)
	{
		if(letter[i].active)
		{
			letter_logic(&letter[1]);
		}
	}
	for(i = 0; i < 10; i++)
	{
		letter_logic(&letter[2]);
	}
	for(i = 0; i < 15; i++)
	{
		letter_logic(&letter[3]);
	}
	for(i = 0; i < 20; i++)
	{
		letter_logic(&letter[4]);
	}
	letter[2].state = 2;
}

void fade_quit(void)
{
	vgolf_state = VGOLF_STATE_FIN;
}

void menu_quit(void)
{
	fx_fade_start(game_palette, black_palette, 2, fade_quit);
}

void play_proc(void)
{
	int i;
	
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			break;
		}
	}
	if(i == 4)
	{
		ncdmsg_add(&vgolf_messages, "No Players!", makecol(255, 255, 255), 300);
		return;
	}
	for(i = 0; i < 5; i++)
	{
		letter[i].vz = fdiv(itofix(rand() % 27),itofix(9)) - itofix(6);
//		letter[i].vz = itofix(-3);
//		letter[i].vx = itofix(fdiv(itofix(rand() % 27),itofix(6)) - 3);
		letter[i].state = 1;
	}
	letter[2].vz = ftofix(-5.7);
//	letter[2].vx = ftofix(2.7);
	letter[2].vy = -ftofix(4.0);
	letter[2].active = 1;
//	fade_level = 0;
//	fade_dir = 1;
//	fade_type = 1;
	newmenu_disable_controls();
}

void player_setup_proc(void)
{
	int i;
	
	vgolf_state = VGOLF_STATE_PLAYER_SETUP;
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			player_page_pointer[i] = player_page[i];
			player_page[i]->element[1].data = vgolf_avatars.avatar[game_player[i].avatar]->image[0];
		}
		else
		{
			player_page_pointer[i] = player_start_page[i];
		}
	}
//	draw_sprite(buffer, main_game_vars.image_bank.charpic[joynet_controller[i].settings.lp[2]][0], px[i] + 160 - main_game_vars.image_bank.charpic[joynet_controller[i].settings.lp[2]][0]->w / 2, 30 + py[i] + 40 + 8 + 16 + 24);
	bfade_level = 255;
	bfade_target = 192;
	bfade_speed = -8;
}

void credits_proc_helper(void)
{
	vgolf_state = VGOLF_STATE_CREDITS;
	vgolf_credits.y = 480;
	vgolf_credits.vy = 1;
	vgolf_credits.fade_level = 0;
	vgolf_credits.fade_dir = 0;
	vgolf_credits.current_image = 0;
	vgolf_credits.done = 0;
	clear_to_color(screen, getpixel(letter[0].bp, 0, 0));
	set_palette(game_palette);
	newmenu_enable_controls();
}

void credits_proc(void)
{
	newmenu_disable_controls();
	fx_fade_start(game_palette, blue_palette, 2, credits_proc_helper);
}

void course_select_proc(void)
{
	vgolf_state = VGOLF_STATE_COURSE_SELECT;
	bfade_level = 255;
	bfade_target = 192;
	bfade_speed = -8;
}

void control_edit_proc(void)
{
	ncdmsg_add(&vgolf_messages, "Controls cannot be customized in this demo!", makecol(255, 255, 255), 300);
}

void profile_view_proc(void)
{
	vgolf_state = VGOLF_STATE_PROFILE;
	selected_profile = 0;
	profile_course = -1;
	profile_show_details = 0;
	create_profile_page(selected_profile);
}

void leaderboards_view_proc(void)
{
	vgolf_state = VGOLF_STATE_LEADERBOARDS;
	strcpy(leaderboard_menu_text, "< Back");
	profile_course = 0;
	leaderboard_mode = 0;
	create_leaderboard_page();
}

void profile_new_proc(void)
{
	strcpy(entered_text, "");
	entered_text_pos = 0;
	entering_text = 3;
	newmenu_disable_controls();
	clear_keybuf();
}

unsigned long check_sum(const char * fn)
{
	PACKFILE * fp;
	unsigned long sum = 0;
	
	if(exists(fn))
	{
		fp = pack_fopen(fn, "r");
		if(fp)
		{
			sum = 0;
			while(!pack_feof(fp))
			{
				sum += pack_getc(fp);
			}
			pack_fclose(fp);
		}
	}
	return sum;
}

int vgolf_add_course(const char * fn, int attrib, void * param)
{
	VGOLF_COURSE * temp_course;
	
	/* store filename */
	strcpy(vgolf_courses.item[vgolf_courses.items].filename, fn);
	
	temp_course = vgolf_load_course_info(fn);
	if(temp_course)
	{
		strcpy(vgolf_courses.item[vgolf_courses.items].name, temp_course->name);
        vgolf_courses.item[vgolf_courses.items].par = temp_course->course_par;
        vgolf_courses.item[vgolf_courses.items].holes = temp_course->num_holes;
		vgolf_courses.item[vgolf_courses.items].preview = temp_course->image_bank.iconpic;
		convert_palette(vgolf_courses.item[vgolf_courses.items].preview, temp_course->palette, game_palette);
		vgolf_courses.item[vgolf_courses.items].sum = check_sum(fn);
		vgolf_courses.items++;
	}
	
	return 0;
}

int vgolf_add_avatar(const char * fn, int attrib, void * param)
{
	vgolf_avatars.avatar[vgolf_avatars.avatars] = vgolf_load_avatar(fn, game_palette);
	vgolf_avatars.sum[vgolf_avatars.avatars] = check_sum(fn);
	vgolf_avatars.avatars++;
	return 0;
}

int vgolf_add_commentary(const char * fn, int attrib, void * param)
{
	VGOLF_COMMENTARY * commentary_info;
	
	commentary_info = load_vgolf_commentary_info(fn);
	if(commentary_info)
	{
		strcpy(vgolf_commentaries.filename[vgolf_commentaries.count], fn);
		strcpy(vgolf_commentaries.name[vgolf_commentaries.count], commentary_info->name);
		strcpy(vgolf_commentaries.author[vgolf_commentaries.count], commentary_info->author);
		strcpy(vgolf_commentaries.comment[vgolf_commentaries.count], commentary_info->comment);
		vgolf_commentaries.count++;
		destroy_vgolf_commentary(commentary_info);
	}
	return 0;
}

/* draw shadow boxes */
void shadow_box(BITMAP * bp, int x, int y, int dx, int dy, int lw, int oc, int bc)
{
	int i;
	
	color_map = &shadow_map;
	drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	rectfill(buffer, x, y, dx, dy, bc);
	for(i = 0; i < lw; i++)
	{
		rect(buffer, x + i, y + i, dx - i, dy - i, oc);
	}
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	color_map = &trans_table;			
}

int hl_player_back(int p, void * pp)
{
	int i;
	
	play_esc_sample();
	for(i = 0; i < 4; i++)
	{
		vgolf_player_list[i] = 0;
	}
	vgolf_state = VGOLF_STATE_MENU;
	return 1;
}

int hl_player_next(int p, void * pp)
{
	int i;
	
	play_select_sample();
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			break;
		}
	}
	if(i == 4)
	{
		ncdmsg_add(&vgolf_messages, "No Players!", makecol(255, 255, 255), 300);
		return 0;
	}
	
	vgolf_state = VGOLF_STATE_COURSE;
	return 1;
}

int hl_player_cancel(int p, void * pp)
{
	play_esc_sample();
	vgolf_state = VGOLF_STATE_PLAYER_SETUP;
	return 1;
}

int hl_player_name(int p, void * pp)
{
	int i = -1;
	play_select_sample();
	if(pp == player_page[0])
	{
		i = 0;
	}
	else if(pp == player_page[1])
	{
		i = 1;
	}
	else if(pp == player_page[2])
	{
		i = 2;
	}
	else if(pp == player_page[3])
	{
		i = 3;
	}
	if(i >= 0)
	{
		vgolf_player_choosing = i;
		vgolf_state = VGOLF_STATE_PLAYER_ACCOUNT;
	}
	player_account_selected_page[i] = 0;
	create_account_page(i);
	return 1;
}

int hl_add_player(int p, void * pp)
{
	int i = -1;
	play_select_sample();
	if(pp == player_start_page[0])
	{
		i = 0;
	}
	else if(pp == player_start_page[1])
	{
		i = 1;
	}
	else if(pp == player_start_page[2])
	{
		i = 2;
	}
	else if(pp == player_start_page[3])
	{
		i = 3;
	}
	if(i >= 0)
	{
		if(!vgolf_player_list[i])
		{
			vgolf_player_list[i] = 1;
			game_player[i].profile = 0;
			game_player[i].avatar = 0;
			player_page_pointer[i] = player_page[i];
			player_page_pointer[i]->element[1].data = vgolf_avatars.avatar[0]->image[0];
			sprintf(player_page_text_buffer[i], "%s", vgolf_profiles.item[game_player[i].profile].name);
		}
	}
	return 1;
}

int hl_player_avatar(int p, void * pp)
{
	int i = -1;
	play_select_sample();
	if(pp == player_page[0])
	{
		i = 0;
	}
	else if(pp == player_page[1])
	{
		i = 1;
	}
	else if(pp == player_page[2])
	{
		i = 2;
	}
	else if(pp == player_page[3])
	{
		i = 3;
	}
	if(i >= 0)
	{
		vgolf_player_choosing = i;
		vgolf_avatar_selected_page = 0;
		create_avatar_page();
		vgolf_state = VGOLF_STATE_PLAYER_AVATAR;
	}
	return 1;
}

int hl_player_avatar_pick(int p, void * pp)
{
	int ai;
	int cap;
	
	if(vgolf_avatars.avatars <= 20)
	{
		play_select_sample();
		game_player[vgolf_player_choosing].avatar = ((HYPERLINK_PAGE *)(pp))->hover_element - 1;
		player_page_pointer[vgolf_player_choosing]->element[1].data = vgolf_avatars.avatar[game_player[vgolf_player_choosing].avatar]->image[0];
		vgolf_state = VGOLF_STATE_PLAYER_SETUP;
	}
	else
	{
		if(vgolf_avatars.avatars <= 19 + vgolf_avatar_selected_page * 18)
		{
			cap = 19;
		}
		else
		{
			cap = 18;
		}
		if(vgolf_avatar_selected_page == 0)
		{
			if(((HYPERLINK_PAGE *)(pp))->hover_element - 1 == 19)
			{
				play_select_sample();
				vgolf_avatar_selected_page++;
				create_avatar_page();
			}
			else
			{
				play_select_sample();
				game_player[vgolf_player_choosing].avatar = ((HYPERLINK_PAGE *)(pp))->hover_element - 1;
				player_page_pointer[vgolf_player_choosing]->element[1].data = vgolf_avatars.avatar[game_player[vgolf_player_choosing].avatar]->image[0];
				vgolf_state = VGOLF_STATE_PLAYER_SETUP;
			}
		}
		else
		{
			ai = 19 + (vgolf_avatar_selected_page - 1) * 18 - 1;
			if(((HYPERLINK_PAGE *)(pp))->hover_element - 1 == 0)
			{
				play_select_sample();
				vgolf_avatar_selected_page--;
				create_avatar_page();
			}
			else
			{
				if(cap == 18 && ((HYPERLINK_PAGE *)(pp))->hover_element - 1 == 19)
				{
					play_select_sample();
					vgolf_avatar_selected_page++;
					create_avatar_page();
				}
				else
				{
					play_select_sample();
					game_player[vgolf_player_choosing].avatar = ((HYPERLINK_PAGE *)(pp))->hover_element - 1 + ai;
					player_page_pointer[vgolf_player_choosing]->element[1].data = vgolf_avatars.avatar[game_player[vgolf_player_choosing].avatar]->image[0];
					vgolf_state = VGOLF_STATE_PLAYER_SETUP;
				}
			}
		}
	}
	if(game_player[vgolf_player_choosing].profile > 0)
	{
		vgolf_profiles.item[game_player[vgolf_player_choosing].profile].avatar = vgolf_avatars.sum[game_player[vgolf_player_choosing].avatar];
	}
	return 1;
}

int hl_player_account_pick(int p, void * pp)
{
	int i;
	
	play_select_sample();
	if(((HYPERLINK_PAGE *)(pp))->hover_element - 1 == 0)
	{
		strcpy(entered_text, "");
		entered_text_pos = 0;
		entering_text = 3;
		newmenu_disable_controls();
		clear_keybuf();
		vgolf_state = VGOLF_STATE_PLAYER_NEW;
	}
	else
	{
		game_player[vgolf_player_choosing].profile = ((HYPERLINK_PAGE *)(pp))->hover_element - 2 + player_account_selected_page[vgolf_player_choosing] * 5;
		strcpy(player_page_pointer[vgolf_player_choosing]->element[0].data, vgolf_profiles.item[game_player[vgolf_player_choosing].profile].name);
		vgolf_state = VGOLF_STATE_PLAYER_SETUP;
		if(game_player[vgolf_player_choosing].profile > 0)
		{
			for(i = 0; i < vgolf_avatars.avatars; i++)
			{
				if(vgolf_avatars.sum[i] == vgolf_profiles.item[game_player[vgolf_player_choosing].profile].avatar)
				{
					game_player[vgolf_player_choosing].avatar = i;
					player_page_pointer[vgolf_player_choosing]->element[1].data = vgolf_avatars.avatar[i]->image[0];
					break;
				}
			}
		}
	}
	return 1;
}

int hl_player_account_next_page(int p, void * pp)
{
	play_select_sample();
	player_account_selected_page[vgolf_player_choosing]++;
	create_account_page(vgolf_player_choosing);
	return 1;
}
int hl_player_account_previous_page(int p, void * pp)
{
	play_select_sample();
	player_account_selected_page[vgolf_player_choosing]--;
	create_account_page(vgolf_player_choosing);
	return 1;
}

int hl_profile_back(int p, void * pp)
{
	play_esc_sample();
	if(vgolf_state == VGOLF_STATE_LEADERBOARDS && leaderboard_mode == 1)
	{
		ncds_play_music("vgolf.dat#menu.xm", 1);
		gametime_reset();
	}
	vgolf_state = VGOLF_STATE_MENU;
	set_current_newmenu(nmenu[0]);
	return 1;
}

int hl_profile_delete(int p, void * pp)
{
	if(selected_profile > 0)
	{
		play_select_sample();
		set_current_newmenu(nmenu[12]);
		vgolf_state = VGOLF_STATE_PROFILE_DELETE;
	}
	else
	{
		play_select_sample();
		ncdmsg_add(&vgolf_messages, "Cannot delete guest account!", makecol(255, 255, 255), 300);
	}
	return 1;
}

int hl_sub_menu_back(int p, void * pp)
{
	int i;
	if(current_newmenu->parent)
	{
		current_newmenu = (NEWMENU *)current_newmenu->parent;
		for(i = 0; i < current_newmenu->items; i++)
		{
			if(current_newmenu->item[i].type == NEWMENU_ITEM_TYPE_OPTION)
			{
				current_newmenu->item[i].tz = itofix(0);
				current_newmenu->item[i].tvz = itofix(0);
			}
			else
			{
				current_newmenu->item[i].tz = -itofix(100);
				current_newmenu->item[i].tvz = itofix(0);
			}
		}
		current_newmenu->item[current_newmenu->current_item].tz = -itofix(100);
		current_newmenu->item[current_newmenu->current_item].tvz = itofix(0);
		current_newmenu->current_item = -1;
		play_esc_sample();
	}
	return 1;
}

int hl_profile_previous(int p, void * pp)
{
	play_select_sample();
	selected_profile--;
	create_profile_page(selected_profile);
	return 1;
}

int hl_profile_next(int p, void * pp)
{
	play_select_sample();
	selected_profile++;
	create_profile_page(selected_profile);
	return 1;
}

int hl_leaderboard_previous(int p, void * pp)
{
	play_select_sample();
	profile_course--;
	create_leaderboard_page();
	return 1;
}

int hl_leaderboard_next(int p, void * pp)
{
	play_select_sample();
	profile_course++;
	create_leaderboard_page();
	return 1;
}

int hl_profile_course_previous(int p, void * pp)
{
	play_select_sample();
	profile_course--;
	create_profile_page(selected_profile);
	return 1;
}

int hl_profile_course_next(int p, void * pp)
{
	play_select_sample();
	profile_course++;
	create_profile_page(selected_profile);
	return 1;
}

int hl_course_select_previous(int p, void * pp)
{
	play_select_sample();
	selected_course--;
	create_course_page();
	return 1;
}

int hl_course_select_next(int p, void * pp)
{
	play_select_sample();
	selected_course++;
	create_course_page();
	return 1;
}

int hl_game_complete_finished(int p, void * pp)
{
	play_select_sample();
	main_game_vars.quit_game = 1;
	main_game_vars.quit_title = 0;
	vgolf_end_game();
	return 1;
}

int hl_game_next_hole(int p, void * pp)
{
	play_select_sample();
	if(main_game_vars.cur_hole < game_course->num_holes - 1)
	{
		//increment to next hole
		main_game_vars.cur_hole++;

		//load hole data
		memcpy(&game_course->current_hole, &game_course->hole[main_game_vars.cur_hole], sizeof(HOLE));

		initialize_hole(&game_course->current_hole);       //initialize the next hole before we start it
		hole_initialize_players(game_player, vgolf_player_count, main_game_vars.cur_hole);   //initialize the players before starting the next hole
		start_scoreboard_segout();                //start the seg-out sequence
		gametime_reset();
	}
	else
	{
		//last hole has been passed, go to victory screen
		if(vgolf_player_count == 1)
		{
			single_player_course_completed_logic(&main_game_vars, game_player, game_course);
		}
		else
		{
			multi_player_course_completed_logic(&main_game_vars, game_player, vgolf_player_count, game_course);
		}
		start_victory_screen_segin(&main_game_vars, game_course, game_player, vgolf_player_count);
	}
	return 1;
}

int hl_game_toggle_records(int p, void * pp)
{
	play_select_sample();
	if(show_records)
	{
		show_records = 0;
	}
	else
	{
		show_records = 1;
	}
	create_scoreboard_page();
	return 1;
}

int hl_game_quit(int p, void * pp)
{
	play_esc_sample();
	vgolf_pause_game();
	return 1;
}

int hl_course_back(int p, void * pp)
{
	int i;
	
	play_esc_sample();
	for(i = 0; i < 4; i++)
	{
		if(vgolf_player_list[i])
		{
			player_page_pointer[i] = player_page[i];
		}
		else
		{
			player_page_pointer[i] = player_start_page[i];
		}
	}
	vgolf_state = VGOLF_STATE_PLAYER_SETUP;
	
	return 1;
}

int hl_course_change(int p, void * pp)
{
	play_select_sample();
	vgolf_state = VGOLF_STATE_COURSE_SELECT;
	selected_course = ((HYPERLINK_PAGE *)(pp))->hover_element;
	return 1;
}

int hl_course_pick(int p, void * pp)
{
	play_select_sample();
	selected_course = ((HYPERLINK_PAGE *)(pp))->hover_element;
	course_page->element[1].data = vgolf_courses.item[selected_course].preview;
	strcpy(course_page_name, vgolf_courses.item[selected_course].name);
	sprintf(course_page_par, "Par: %d", vgolf_courses.item[selected_course].par);
	sprintf(course_page_holes, "Holes: %d", vgolf_courses.item[selected_course].holes);
	vgolf_state = VGOLF_STATE_COURSE;
	return 1;
}

int hl_course_next(int p, void * pp)
{
	ncds_play_sample(main_game_vars.sound_bank.sfx_title_swing, 128, 1000, 0);
	play_proc();
	return 1;
}

int hl_profile_toggle_details(int p, void * pp)
{
	play_select_sample();
	if(profile_show_details)
	{
		profile_show_details = 0;
	}
	else
	{
		profile_show_details = 1;
	}
	create_profile_page(selected_profile);
	return 1;
}

void create_account_page(int player)
{
	int i;
	int start_item = player_account_selected_page[player] * 5;
	
	if(player_account_page)
	{
		hyperlink_page_destroy(player_account_page);
	}
	player_account_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(player_account_page, NULL, "Select Profile", gfont, 320 + 2, 92, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(player_account_page, hl_player_account_pick, "New Player", gfont, 320 + 2, 92 + 64, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	for(i = start_item; i < start_item + 5 && i < vgolf_profiles.items; i++)
	{
		hyperlink_page_add_element_text(player_account_page, hl_player_account_pick, vgolf_profiles.item[i].name, gfont, 320 + 2, 92 + 64 + (i - start_item + 1) * 36, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	if(player_account_selected_page[player] > 0)
	{
		hyperlink_page_add_element_text(player_account_page, hl_player_account_previous_page, "<", gfont, 320 + 2 - 120, 92 + 64 + (5) * 36, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	if(start_item + 5 < vgolf_profiles.items)
	{
		hyperlink_page_add_element_text(player_account_page, hl_player_account_next_page, ">", gfont, 320 + 2 + 120, 92 + 64 + (5) * 36, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	hyperlink_page_add_element_text(player_account_page, hl_player_cancel, "< Back", gfont, 2, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
}

void create_avatar_page(void)
{
	int i;
	int ai;
	int ali;
	int cap;
	
	if(player_avatar_page)
	{
		hyperlink_page_destroy(player_avatar_page);
	}
	player_avatar_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(player_avatar_page, NULL, "Select Avatar", gfont, 320 + 2, 92, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	
	/* if there is only one page, show up to 20 avatars */
	if(vgolf_avatars.avatars <= 20)
	{
		for(i = 0; i < vgolf_avatars.avatars; i++)
		{
			hyperlink_page_add_element_image(player_avatar_page, hl_player_avatar_pick, vgolf_avatars.avatar[i]->image[0], 160 + (i % 5) * 64, 132 + (i / 5) * 64, HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
	}
	else
	{
		if(vgolf_avatar_selected_page == 0)
		{
			for(i = 0; i < 19; i++)
			{
				hyperlink_page_add_element_image(player_avatar_page, hl_player_avatar_pick, vgolf_avatars.avatar[i]->image[0], 160 + (i % 5) * 64, 132 + (i / 5) * 64, HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
			}
			hyperlink_page_add_element_text(player_avatar_page, hl_player_avatar_pick, ">", gfont, 160 + (19 % 5) * 64 + 32, 132 + (19 / 5) * 64 + 8, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
		else
		{
			if(vgolf_avatars.avatars <= 19 + vgolf_avatar_selected_page * 18)
			{
				cap = 19;
			}
			else
			{
				cap = 18;
			}
			ai = 19 + (vgolf_avatar_selected_page - 1) * 18;
			ali = ai - 1;
			hyperlink_page_add_element_text(player_avatar_page, hl_player_avatar_pick, "<", gfont, 160 + 32, 132 + 8, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
			for(i = ai; (i < vgolf_avatars.avatars && (i - ai < cap)); i++)
			{
				hyperlink_page_add_element_image(player_avatar_page, hl_player_avatar_pick, vgolf_avatars.avatar[i]->image[0], 160 + ((i - ali) % 5) * 64, 132 + ((i - ali) / 5) * 64, HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
			}
			if(cap == 18)
			{
				hyperlink_page_add_element_text(player_avatar_page, hl_player_avatar_pick, ">", gfont, 160 + (19 % 5) * 64 + 32, 132 + (19 / 5) * 64 + 8, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
			}
		}
	}
}

void create_profile_page(int profile)
{
	if(profile_page)
	{
		hyperlink_page_destroy(profile_page);
	}
	profile_page = hyperlink_page_create(0, 0);
	
	/* name of profile */
	hyperlink_page_add_element_text(profile_page, NULL, vgolf_profiles.item[profile].name, gfont, 320 + 2, 80, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	if(selected_profile > 0)
	{
		hyperlink_page_add_element_text(profile_page, hl_profile_previous, "<", gfont, 320 + 2 - 120, 80, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	if(selected_profile < vgolf_profiles.items - 1)
	{
		hyperlink_page_add_element_text(profile_page, hl_profile_next, ">", gfont, 320 + 2 + 120, 80, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	
	/* name of selected course */
	if(profile_course < 0)
	{
		/* total stats */
		hyperlink_page_add_element_text(profile_page, hl_profile_course_next, ">", gfont, 320 + 2 + 152, 80 + 64, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
		hyperlink_page_add_element_text(profile_page, NULL, "Statistics", gfont, 320 + 2, 80 + 64, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		sprintf(profile_text[5], "Strokes: %d", vgolf_profiles.item[profile].total.data[VGOLF_PROFILE_DATA_STROKES]);
		hyperlink_page_add_element_text(profile_page, NULL, profile_text[5], gfont, 320 + 2, 80 + 128, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		sprintf(profile_text[6], "Pars: %d", vgolf_profiles.item[profile].total.data[VGOLF_PROFILE_DATA_PARS]);
		hyperlink_page_add_element_text(profile_page, NULL, profile_text[6], gfont, 320 + 2, 80 + 160, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		sprintf(profile_text[7], "Birdies: %d", vgolf_profiles.item[profile].total.data[VGOLF_PROFILE_DATA_BIRDIES]);
		hyperlink_page_add_element_text(profile_page, NULL, profile_text[7], gfont, 320 + 2, 80 + 192, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		sprintf(profile_text[8], "Eagles: %d", vgolf_profiles.item[profile].total.data[VGOLF_PROFILE_DATA_EAGLES]);
		hyperlink_page_add_element_text(profile_page, NULL, profile_text[8], gfont, 320 + 2, 80 + 224, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		sprintf(profile_text[9], "Albatrosses: %d", vgolf_profiles.item[profile].total.data[VGOLF_PROFILE_DATA_ALBATROSSES]);
		hyperlink_page_add_element_text(profile_page, NULL, profile_text[9], gfont, 320 + 2, 80 + 256, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		sprintf(profile_text[10], "Holes In One: %d", vgolf_profiles.item[profile].total.data[VGOLF_PROFILE_DATA_HOLES_IN_ONE]);
		hyperlink_page_add_element_text(profile_page, NULL, profile_text[10], gfont, 320 + 2, 80 + 288, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	else
	{
		sprintf(profile_text[0], "%s", vgolf_courses.item[profile_course].name);
		hyperlink_page_add_element_text(profile_page, NULL, profile_text[0], gfont, 320 + 2, 80 + 64, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		if(profile_course >= 0)
		{
			hyperlink_page_add_element_text(profile_page, hl_profile_course_previous, "<", gfont, 320 + 2 - 152 - text_length(gfont, "<"), 80 + 64, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
		if(profile_course < vgolf_courses.items - 1)
		{
			hyperlink_page_add_element_text(profile_page, hl_profile_course_next, ">", gfont, 320 + 2 + 152, 80 + 64, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
		
		if(profile_show_details)
		{
			sprintf(profile_text[4], "View Overall");
			hyperlink_page_add_element_text(profile_page, hl_profile_toggle_details, profile_text[4], gfont, 320 + 2, 80 + 288, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
		else
		{
			/* course stats */
			sprintf(profile_text[1], "Best Score: %d", vgolf_profiles.item[profile].course_info[profile_course].total_score);
			hyperlink_page_add_element_text(profile_page, NULL, profile_text[1], gfont, 320 + 2, 80 + 128, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
			sprintf(profile_text[2], "Wins: %d", vgolf_profiles.item[profile].course_info[profile_course].won);
			hyperlink_page_add_element_text(profile_page, NULL, profile_text[2], gfont, 320 + 2, 80 + 160, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
			sprintf(profile_text[3], "Losses: %d", vgolf_profiles.item[profile].course_info[profile_course].lost);
			hyperlink_page_add_element_text(profile_page, NULL, profile_text[3], gfont, 320 + 2, 80 + 192, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
			sprintf(profile_text[4], "Total Played: %d", vgolf_profiles.item[profile].course_info[profile_course].completed);
			hyperlink_page_add_element_text(profile_page, NULL, profile_text[4], gfont, 320 + 2, 80 + 224, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
			hyperlink_page_add_element_text(profile_page, hl_profile_toggle_details, "View Details", gfont, 320 + 2, 80 + 288, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
	}
}

void create_leaderboard_page(void)
{
	char text[64] = {0};
	int i, j, score;
	int top[4] = {-1, -1, -1, -1};
	int color = 0;
	
	sprintf(text, "%lu", vgolf_courses.item[profile_course].sum);
	if(leaderboard)
	{
		t3net_destroy_leaderboard(leaderboard);
	}
	ncds_pause_music();
	leaderboard = t3net_get_leaderboard("http://www.t3-i.com/leaderboards/query.php", "vgolf", "1.1", "normal", text, 10, 1);
	ncds_resume_music();
	gametime_reset();
	if(!leaderboard)
	{
		vgolf_state = VGOLF_STATE_MENU;
		return;
	}
	if(leaderboard_page)
	{
		hyperlink_page_destroy(leaderboard_page);
	}
	leaderboard_page = hyperlink_page_create(0, 0);
	
	if(leaderboard_mode == 1)
	{
		for(i = 0; i < 4; i++)
		{
			if(vgolf_player_list[i])
			{
				for(j = 0; j < 10; j++)
				{
					if(!stricmp(vgolf_profiles.item[game_player[i].profile].name, leaderboard->entry[j]->name) && (player_course_stroke_total(game_player[i], game_course->num_holes) * 2 + 'v' + 'g' + 'o' + 'l' + 'f') == leaderboard->entry[j]->score)
					{
						top[i] = j;
						break;
					}
				}
			}
		}
	}
	
	/* course */
	hyperlink_page_add_element_text(leaderboard_page, NULL, vgolf_courses.item[profile_course].name, gfont, 320 + 2, 60, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	if(leaderboard_mode == 0)
	{
		if(profile_course > 0)
		{
			hyperlink_page_add_element_text(leaderboard_page, hl_leaderboard_previous, "<", gfont, 320 + 2 - 152 - text_length(gfont, "<"), 60, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
		if(profile_course < vgolf_courses.items - 1)
		{
			hyperlink_page_add_element_text(leaderboard_page, hl_leaderboard_next, ">", gfont, 320 + 2 + 152, 60, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
	}
	
	/* name of selected course */
	for(i = 0; i < 10; i++)
	{
		color = VGOLF_COLOR_MENU_OPTION;
		
		/* highlight top scores is in mode 1 */
		if(leaderboard_mode == 1)
		{
			for(j = 0; j < 4; j++)
			{
				if(i == top[j])
				{
					color = makecol(255, 255, 0);
					break;
				}
			}
		}
		if(strlen(leaderboard->entry[i]->name) > 0)
		{
			hyperlink_page_add_element_text(leaderboard_page, NULL, leaderboard->entry[i]->name, gfont2, 160 + 2, 60 + 64 + 32 * i, color, color, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_SHADOW);
			score = (leaderboard->entry[i]->score - 'f' - 'l' - 'o' - 'g' - 'v') / 2;
			if(score == vgolf_courses.item[profile_course].par)
			{
				sprintf(profile_text[i], "E");
			}
			else if(score > vgolf_courses.item[profile_course].par)
			{
				sprintf(profile_text[i], "+%d", score - vgolf_courses.item[profile_course].par);
			}
			else
			{
				sprintf(profile_text[i], "%d", score - vgolf_courses.item[profile_course].par);
			}
			hyperlink_page_add_element_text(leaderboard_page, NULL, profile_text[i], gfont2, 480 - text_length(gfont2, profile_text[i]) + 2, 60 + 64 + 32 * i, color, color, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_SHADOW);
		}
		else
		{
			hyperlink_page_add_element_text(leaderboard_page, NULL, "---", gfont2, 320 + 2, 60 + 64 + 32 * i, color, color, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_SHADOW | HYPERLINK_ELEMENT_FLAG_CENTER);
		}
	}
}

void create_scoreboard_page(void)
{
	if(scoreboard_page)
	{
		hyperlink_page_destroy(scoreboard_page);
	}
	scoreboard_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(scoreboard_page, hl_game_next_hole, "Next >", gfont2, 640 - text_length(gfont2, "Next >") - 4 - 8, 480 - text_height(gfont) + 6, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(scoreboard_page, hl_game_quit, "Menu", gfont2, 2 + 8, 480 - text_height(gfont) + 6, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	if(show_records)
	{
		hyperlink_page_add_element_text(scoreboard_page, hl_game_toggle_records, "View Scores", gfont2, 320 - text_length(gfont2, "View Scores") / 2 - 4, 480 - text_height(gfont) + 6 - 480 / 10, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	else
	{
		hyperlink_page_add_element_text(scoreboard_page, hl_game_toggle_records, "View Records", gfont2, 320 - text_length(gfont2, "View Records") / 2 - 4, 480 - text_height(gfont) + 6 - 480 / 10, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
}

void create_course_page(void)
{
	if(course_page)
	{
		hyperlink_page_destroy(course_page);
	}
	course_page = hyperlink_page_create(0, 0);
	sprintf(course_page_name, "%s", vgolf_courses.item[selected_course].name);
	sprintf(course_page_par, "Par: %d", vgolf_courses.item[selected_course].par);
	sprintf(course_page_holes, "Holes: %d", vgolf_courses.item[selected_course].holes);
	hyperlink_page_add_element_text(course_page, NULL, course_page_name, gfont, 320 + 2, 240 - 2 - vgolf_courses.item[selected_course].preview->h / 2 - text_height(gfont) - 32, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	if(selected_course > 0)
	{
		hyperlink_page_add_element_text(course_page, hl_course_select_previous, "<", gfont, 320 + 2 - 152 - text_length(gfont, "<"), 240 - 2 - vgolf_courses.item[selected_course].preview->h / 2 - text_height(gfont) - 32, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	if(selected_course < vgolf_courses.items - 1)
	{
		hyperlink_page_add_element_text(course_page, hl_course_select_next, ">", gfont, 320 + 2 + 152, 240 - 2 - vgolf_courses.item[selected_course].preview->h / 2 - text_height(gfont) - 32, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	hyperlink_page_add_element_image(course_page, NULL, vgolf_courses.item[selected_course].preview, 320, 240, HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW | HYPERLINK_ELEMENT_FLAG_BLIT);
	hyperlink_page_add_element_text(course_page, NULL, course_page_par, gfont, 320 + 2, 240 + vgolf_courses.item[selected_course].preview->h / 2, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(course_page, NULL, course_page_holes, gfont, 320 + 2, 240 + vgolf_courses.item[selected_course].preview->h / 2 + text_height(gfont), VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(course_page, hl_course_next, "Play >", gfont, 640 - text_length(gfont, "Play >") - 4, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(course_page, hl_course_back, "< Back", gfont, 2, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
}

int hl_player_add_account(int p, void * pp)
{
	int i;
	
	play_select_sample();
	strcpy(vgolf_profiles.item[vgolf_profiles.items].name, entered_text);
	vgolf_profiles.items++;
	create_account_page(vgolf_player_choosing);
	vgolf_state = VGOLF_STATE_PLAYER_ACCOUNT;
	newmenu_enable_controls();
	for(i = 0; i < vgolf_courses.items; i++)
	{
		vgolf_profiles.item[vgolf_profiles.items - 1].course_info[i].id = vgolf_courses.item[i].sum;
	}
	return 1;
}

int hl_player_cancel_account(int p, void * pp)
{
	play_esc_sample();
	vgolf_state = VGOLF_STATE_PLAYER_ACCOUNT;
	newmenu_enable_controls();
	return 1;
}

int hl_remove_player(int p, void * pp)
{
	int i = -1;
	play_esc_sample();
	if(pp == player_page[0])
	{
		i = 0;
	}
	else if(pp == player_page[1])
	{
		i = 1;
	}
	else if(pp == player_page[2])
	{
		i = 2;
	}
	else if(pp == player_page[3])
	{
		i = 3;
	}
	if(i >= 0)
	{
		if(vgolf_player_list[i])
		{
			vgolf_player_list[i] = 0;
			player_page_pointer[i] = player_start_page[i];
		}
	}
	return 1;
}

void set_up_menus(void)
{
	/* set up menus */
	nmenu[12] = create_newmenu("Delete Profile", NULL, 0, 240 - (text_height(gfont) * 3) / 2);
	add_newmenu_item(nmenu[12], "Delete Profile?", NEWMENU_ITEM_TYPE_HEADER, 0, 0, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[12], "Yes", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, delete_profile_yes, NULL, NULL);
	add_newmenu_item(nmenu[12], "No", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, delete_profile_no, NULL, NULL);
	
	nmenu[11] = create_newmenu("Ball Lost", NULL, 0, 240 - (text_height(gfont) * 3) / 2);
	add_newmenu_item(nmenu[11], "Ball Lost", NEWMENU_ITEM_TYPE_HEADER, 0, 0, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[11], "Drop", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, ball_lost_drop, NULL, NULL);
	add_newmenu_item(nmenu[11], "Rehit", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, ball_lost_rehit, NULL, NULL);
	
	nmenu[10] = create_newmenu("Exit Game", NULL, 0, 240 - (text_height(gfont) * 3) / 2);
	add_newmenu_item(nmenu[10], "Exit Game?", NEWMENU_ITEM_TYPE_HEADER, 0, 0, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[10], "Yes", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, exit_game_yes, NULL, NULL);
	add_newmenu_item(nmenu[10], "No", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, exit_game_no, NULL, NULL);
	
	nmenu[17] = create_newmenu("Take Gimme", NULL, 0, 240 - (text_height(gfont) * 3) / 2);
	add_newmenu_item(nmenu[17], "Give Up?", NEWMENU_ITEM_TYPE_HEADER, 0, 0, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[17], "Yes", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, give_up_yes, NULL, NULL);
	add_newmenu_item(nmenu[17], "No", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, give_up_no, NULL, NULL);
	
	nmenu[9] = create_newmenu("Enter Name", profile_name_update, 0, 320 - (text_height(gfont) * 1) / 2);
	add_newmenu_item(nmenu[9], "Enter Name", NEWMENU_ITEM_TYPE_HEADER, 0, 0, gfont, VGOLF_COLOR_MENU_HEADER, NULL, VGOLF_COLOR_MENU_HEADER, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[9], nmenu[9]->vtext[0], 0, text_height(gfont) * 1, gfont, makecol(192, 192, 192), NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	
	nmenu[8] = create_newmenu("Audio Options", audio_options_update, 0, 320 - (text_height(gfont) * 4) / 2 - 15 - 32);
	add_newmenu_item(nmenu[8], "Audio Options", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[8], "Music Volume", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 1, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[8], nmenu[8]->vtext[0], 0, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[8], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "100%") / 2 - 32, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_music_volume_left, NULL, NULL);
	add_newmenu_item(nmenu[8], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "100%") / 2 + 32, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_music_volume_right, NULL, NULL);
	add_newmenu_item(nmenu[8], "Sound Volume", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 3, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[8], nmenu[8]->vtext[1], 0, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[8], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "100%") / 2 - 32, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_sfx_volume_left, NULL, NULL);
	add_newmenu_item(nmenu[8], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "100%") / 2 + 32, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_sfx_volume_right, NULL, NULL);
	add_newmenu_item(nmenu[8], "Commentary Volume", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 5, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[8], nmenu[8]->vtext[2], 0, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[8], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "100%") / 2 - 32, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_commentary_volume_left, NULL, NULL);
	add_newmenu_item(nmenu[8], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "100%") / 2 + 32, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_commentary_volume_right, NULL, NULL);
	add_newmenu_item(nmenu[8], "Announcer", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 7, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[8], nmenu[8]->vtext[3], 0, text_height(gfont2) * 8, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[8], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "Default Voice") / 2 - 32, text_height(gfont2) * 8, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_commentary_announcer_left, NULL, NULL);
	add_newmenu_item(nmenu[8], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "Default Voice") / 2 + 32, text_height(gfont2) * 8, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_commentary_announcer_right, NULL, NULL);
	
	nmenu[7] = create_newmenu("Video Options", video_options_update, 0, 320 - (text_height(gfont) * 4) / 2 - 15);
	add_newmenu_item(nmenu[7], "Video Options", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[7], "Screen", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 1, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[7], nmenu[7]->vtext[0], 0, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[7], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "Fullscreen") / 2 - 32, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_gfx_mode, NULL, NULL);
	add_newmenu_item(nmenu[7], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "Fullscreen") / 2 + 32, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_gfx_mode, NULL, NULL);
	add_newmenu_item(nmenu[7], "Color Depth", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 3, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[7], nmenu[7]->vtext[1], 0, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[7], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "Desktop") / 2 - 32, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_depth, NULL, NULL);
	add_newmenu_item(nmenu[7], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "Desktop") / 2 + 32, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_depth, NULL, NULL);
	add_newmenu_item(nmenu[7], "Monitor Sync", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 5, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[7], nmenu[7]->vtext[2], 0, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[7], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "Off") / 2 - 32, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_vsync, NULL, NULL);
	add_newmenu_item(nmenu[7], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "Off") / 2 + 32, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_vsync, NULL, NULL);
	
	nmenu[15] = create_newmenu("Audio Options", audio_options_update, 0, 240 - (text_height(gfont) * 4) / 2 - 15);
	add_newmenu_item(nmenu[15], "Audio Options", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[15], "Music Volume", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 1, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[15], nmenu[15]->vtext[0], 0, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[15], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "100%") / 2 - 32, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_music_volume_left, NULL, NULL);
	add_newmenu_item(nmenu[15], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "100%") / 2 + 32, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_music_volume_right, NULL, NULL);
	add_newmenu_item(nmenu[15], "Sound Volume", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 3, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[15], nmenu[15]->vtext[1], 0, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[15], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "100%") / 2 - 32, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_sfx_volume_left, NULL, NULL);
	add_newmenu_item(nmenu[15], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "100%") / 2 + 32, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_sfx_volume_right, NULL, NULL);
	add_newmenu_item(nmenu[15], "Commentary Volume", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 5, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[15], nmenu[15]->vtext[2], 0, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[15], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "100%") / 2 - 32, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_commentary_volume_left, NULL, NULL);
	add_newmenu_item(nmenu[15], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "100%") / 2 + 32, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_commentary_volume_right, NULL, NULL);
	add_newmenu_item(nmenu[15], "Announcer", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 7, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[15], nmenu[15]->vtext[3], 0, text_height(gfont2) * 8, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[15], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "Default Voice") / 2 - 32, text_height(gfont2) * 8, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_commentary_announcer_left, NULL, NULL);
	add_newmenu_item(nmenu[15], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "Default Voice") / 2 + 32, text_height(gfont2) * 8, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_set_commentary_announcer_right, NULL, NULL);
	
	nmenu[14] = create_newmenu("Video Options", video_options_update, 0, 240 - (text_height(gfont) * 5) / 2 - 15 + 32);
	add_newmenu_item(nmenu[14], "Video Options", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[14], "Screen", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 1, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[14], nmenu[14]->vtext[0], 0, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[14], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "Fullscreen") / 2 - 32, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_gfx_mode, NULL, NULL);
	add_newmenu_item(nmenu[14], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "Fullscreen") / 2 + 32, text_height(gfont2) * 2, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_gfx_mode, NULL, NULL);
	add_newmenu_item(nmenu[14], "Color Depth", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 3, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[14], nmenu[14]->vtext[1], 0, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[14], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "Desktop") / 2 - 32, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_depth, NULL, NULL);
	add_newmenu_item(nmenu[14], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "Desktop") / 2 + 32, text_height(gfont2) * 4, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_depth, NULL, NULL);
	add_newmenu_item(nmenu[14], "Monitor Sync", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont2) * 5, gfont2, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[14], nmenu[14]->vtext[2], 0, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[14], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont2, "Off") / 2 - 32, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_vsync, NULL, NULL);
	add_newmenu_item(nmenu[14], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont2, "Off") / 2 + 32, text_height(gfont2) * 6, gfont2, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_vsync, NULL, NULL);
	
	nmenu[13] = create_newmenu("Online Options", online_options_update, 0, 320 - (text_height(gfont) * 2) / 2 - 15);
	add_newmenu_item(nmenu[13], "Online Options", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[13], "Upload Scores", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[13], nmenu[13]->vtext[0], 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[13], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont, "Yes") / 2 - 32, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_upload, NULL, NULL);
	add_newmenu_item(nmenu[13], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont, "Yes") / 2 + 32, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_upload, NULL, NULL);
	
	nmenu[16] = create_newmenu("Online Options", online_options_update, 0, 320 - (text_height(gfont) * 2) / 2 - 15);
	add_newmenu_item(nmenu[16], "Online Options", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[16], "Upload Scores", NEWMENU_ITEM_TYPE_HEADER, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_child_variable(nmenu[16], nmenu[16]->vtext[0], 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN_DARK);
	add_newmenu_item(nmenu[16], "<", NEWMENU_ITEM_TYPE_VOPTION, -text_length(gfont, "Yes") / 2 - 32, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_upload, NULL, NULL);
	add_newmenu_item(nmenu[16], ">", NEWMENU_ITEM_TYPE_VOPTION, text_length(gfont, "Yes") / 2 + 32, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, NULL, config_toggle_upload, NULL, NULL);
	
	nmenu[6] = create_newmenu("Options", NULL, 0, 240 - (text_height(gfont) * 3) / 2 + 32);
	add_newmenu_item(nmenu[6], "Options", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[6], "Video", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, nmenu[14], NULL, NULL, NULL);
	add_newmenu_item(nmenu[6], "Audio", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, nmenu[15], NULL, NULL, NULL);
	add_newmenu_item(nmenu[6], "Online", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 3, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, nmenu[13], NULL, NULL, NULL);
	
	nmenu[4] = create_newmenu("Options", NULL, 0, 300 - (text_height(gfont) * 3) / 2);
	add_newmenu_item(nmenu[4], "Options", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[4], "Video", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, nmenu[7], NULL, NULL, NULL);
	add_newmenu_item(nmenu[4], "Audio", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, nmenu[8], NULL, NULL, NULL);
	add_newmenu_item(nmenu[4], "Online", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 3, gfont, VGOLF_COLOR_MENU_MAIN_DARK, NULL, VGOLF_COLOR_MENU_MAIN, nmenu[16], NULL, NULL, NULL);
	
	nmenu[5] = create_newmenu("In-Game Options", game_options_update, 0, 240 - (text_height(gfont) * 3) / 2);
	add_newmenu_item(nmenu[5], "In-Game Menu", NEWMENU_ITEM_TYPE_HEADER, 0, -text_height(gfont) / 2, gfont, VGOLF_COLOR_MENU_OPTION, NULL, VGOLF_COLOR_MENU_OPTION, NULL, NULL, NULL, NULL);
	add_newmenu_item(nmenu[5], "Resume Game", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, in_game_resume, NULL, NULL);
	add_newmenu_item(nmenu[5], "Options", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, nmenu[6], NULL, NULL, NULL);
	add_newmenu_item(nmenu[5], "Quit", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 3, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, nmenu[10], NULL, NULL, NULL);
	
	nmenu[0] = create_newmenu("Main Menu", NULL, 0, 320 - (text_height(gfont) * 6) / 2);
	add_newmenu_item(nmenu[0], "Play Game", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 0, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, player_setup_proc, NULL, NULL);
	add_newmenu_item(nmenu[0], "Profiles", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 1, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, profile_view_proc, NULL, NULL);
	add_newmenu_item(nmenu[0], "Leaderboards", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 2, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, leaderboards_view_proc, NULL, NULL);
	add_newmenu_item(nmenu[0], "Options", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 3, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, nmenu[4], NULL, NULL, NULL);
	add_newmenu_item(nmenu[0], "Credits", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 4, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, credits_proc, NULL, NULL);
	add_newmenu_item(nmenu[0], "Quit", NEWMENU_ITEM_TYPE_OPTION, 0, text_height(gfont) * 5, gfont, VGOLF_COLOR_MENU_MAIN_DARK, gfont, VGOLF_COLOR_MENU_MAIN, NULL, menu_quit, NULL, NULL);
}

void set_up_credits(void)
{
	vgolf_credits.items = 0;
	credits_add_item(&vgolf_credits, "Game Design", gfont, text_height(gfont) * 0, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Travis Cope", gfont, text_height(gfont) * 1, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Programming", gfont, text_height(gfont) * 6, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Travis Cope", gfont, text_height(gfont) * 7, makecol(255, 255, 255));
	credits_add_item(&vgolf_credits, "Todd Cope", gfont, text_height(gfont) * 8, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Game Engine", gfont, text_height(gfont) * 13, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Travis Cope", gfont, text_height(gfont) * 14, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Course Design", gfont, text_height(gfont) * 19, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Travis Cope", gfont, text_height(gfont) * 20, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Interface", gfont, text_height(gfont) * 25, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Todd Cope", gfont, text_height(gfont) * 26, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Graphics & Art", gfont, text_height(gfont) * 31, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Travis Cope", gfont, text_height(gfont) * 32, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Additional Graphics", gfont, text_height(gfont) * 37, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Todd Cope", gfont, text_height(gfont) * 38, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Music", gfont, text_height(gfont) * 43, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Travis Cope", gfont, text_height(gfont) * 44, makecol(255, 255, 255));
	credits_add_item(&vgolf_credits, "Todd Cope", gfont, text_height(gfont) * 45, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Sound", gfont, text_height(gfont) * 50, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Travis Cope", gfont, text_height(gfont) * 51, makecol(255, 255, 255));
	credits_add_item(&vgolf_credits, "Todd Cope", gfont, text_height(gfont) * 52, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Commentary", gfont, text_height(gfont) * 57, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Todd Cope", gfont, text_height(gfont) * 58, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Web Support", gfont, text_height(gfont) * 63, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Grady O'Connell", gfont, text_height(gfont) * 64, makecol(255, 255, 255));
	
	credits_add_item(&vgolf_credits, "Testing & Feedback", gfont, text_height(gfont) * 69, makecol(0, 255, 0));
	credits_add_item(&vgolf_credits, "Todd Cope", gfont, text_height(gfont) * 70, makecol(255, 255, 255));
	credits_add_item(&vgolf_credits, "Travis Cope", gfont, text_height(gfont) * 71, makecol(255, 255, 255));
	credits_add_item(&vgolf_credits, "Brandy Cope", gfont, text_height(gfont) * 72, makecol(255, 255, 255));
	credits_add_item(&vgolf_credits, "Grady O'Connell", gfont, text_height(gfont) * 73, makecol(255, 255, 255));
	credits_add_item(&vgolf_credits, "Mark Oates", gfont, text_height(gfont) * 74, makecol(255, 255, 255));
	
	vgolf_credits.images = 0;
	credits_add_image(&vgolf_credits, credit_image[0], buffer->w / 4, 240);
	credits_add_image(&vgolf_credits, credit_image[1], buffer->w / 4, 240 - CREDITS_SPACE * 1);
	credits_add_image(&vgolf_credits, credit_image[2], buffer->w / 4, 240 - CREDITS_SPACE * 2);
	credits_add_image(&vgolf_credits, credit_image[3], buffer->w / 4, 240 - CREDITS_SPACE * 3);
	credits_add_image(&vgolf_credits, credit_image[4], buffer->w / 4, 240 - CREDITS_SPACE * 4);
	credits_add_image(&vgolf_credits, credit_image[5], buffer->w / 4, 240 - CREDITS_SPACE * 5);
	credits_add_image(&vgolf_credits, credit_image[6], buffer->w / 4, 240 - CREDITS_SPACE * 6);
	credits_add_image(&vgolf_credits, thanks_pic, buffer->w / 2, 240 - CREDITS_SPACE * 7 - 160);
}

void vgolf_init(void)
{
	int i, j;
	int px[4] = {0, 320, 0, 320};
	int py[4] = {0, 0, 240, 240};
	int cx[4] = {114 - 2, 320 + 2, 114 - 2, 320 + 2};
	int cy[4] = {34 - 2, 34 - 2, 240 + 2, 240 + 2};
	
	allegro_init();
	set_window_title("vGolf");
	buffer = create_bitmap(640, 480);
	scratch_pad = create_bitmap(48, 48);
	vfscratch = scratch_pad;
	thanks_pic = create_bitmap(640, 480);
//	scbuff = create_bitmap(SCREEN_W, SCREEN_H);
//	trans_scbuff = create_bitmap(SCREEN_W, SCREEN_H);
//	menu_scbuff = create_bitmap(SCREEN_W, SCREEN_H);

	install_keyboard();
	install_mouse();
	install_timer();
	ncds_install();
	srand(time(0));
	fx_particle_system_init();
	
	//set the main game variables up
	set_main_game_vars();

	//set the framerate timers up
	initialize_ingame_timer();

	//load the vfont from the specified bitmap
	load_vfont_bmp(&game_vfont, 16, 16, base_filename("font_pcx"));
	//load the main image bank up
	if(!load_main_image_bank(&main_game_vars.image_bank))
	{
		allegro_message("Unable to load %s!", fname_buffer);
		exit(0);
	}
	//load the main sample bank up
	if(!load_main_game_samples(&main_game_vars.sound_bank))
	{
		allegro_message("Unable to load %s!", fname_buffer);
		exit(0);
	}

        //set the palette
        set_palette(game_palette);
        
        //load the game menus into menu and set them up for use
//        initialize_game_menus();

        //set the players up..
        preinitialize_players(game_player, MAX_PLAYERS);
        
	/* load images */
	letter[1].bp = load_pcx("vgolf.dat#title_g.pcx", NULL);
//	set_palette(mypalette);
	letter[0].bp = load_pcx("vgolf.dat#title_v.pcx", NULL);
	letter[2].bp = load_pcx("vgolf.dat#title_o.pcx", NULL);
	letter[3].bp = load_pcx("vgolf.dat#title_l.pcx", NULL);
	letter[4].bp = load_pcx("vgolf.dat#title_f.pcx", NULL);
	gfont = load_font("vgolf.dat#fonts/menufont1.pcx", NULL, NULL);
	gfont2 = load_font("vgolf.dat#fonts/menufont2.pcx", NULL, NULL);
	gfont3 = load_font("vgolf.dat#fonts/menufont3.pcx", NULL, NULL);
	gfont4 = load_font("vgolf.dat#fonts/menufont4.pcx", NULL, NULL);
	clear_to_color(thanks_pic, getpixel(letter[0].bp, 0, 0));
	draw_sprite(thanks_pic, credit_image[7], thanks_pic->w / 2 - credit_image[7]->w / 2, thanks_pic->h / 2 - text_height(gfont) / 2 - credit_image[7]->h / 2);
	int epx = 320 - text_length(gfont, "Thank You For Playing") / 2;
	int epy = thanks_pic->h / 2 - text_height(gfont) / 2 - credit_image[7]->h / 2 + credit_image[7]->h;
	outline_text_shadow(thanks_pic, gfont, epx + 2, epy - 2, -4, 4, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), "Thank You For Playing");
//	textprintf_centre_ex(thanks_pic, gfont, 320, thanks_pic->h / 2 - text_height(gfont) / 2 - credit_image[7]->h / 2 + credit_image[7]->h, mak
	scorecard_pic = load_pcx("vgolf.dat#scorecard.pcx", NULL);
	
	create_color_table(&shadow_map, game_palette, (void *)myblend, NULL);
	color_map = &trans_table;
	create_light_table(&fade_map, game_palette, game_palette[getpixel(letter[0].bp, 0, 0)].r, game_palette[getpixel(letter[0].bp, 0, 0)].g, game_palette[getpixel(letter[0].bp, 0, 0)].b, NULL);
	
	reset_letters();
	for(i = 0; i < 256; i++)
	{
		white_palette[i].r = 63;
		white_palette[i].g = 63;
		white_palette[i].b = 63;
	}
	for(i = 0; i < 256; i++)
	{
		blue_palette[i].r = game_palette[getpixel(letter[0].bp, 0, 0)].r;
		blue_palette[i].g = game_palette[getpixel(letter[0].bp, 0, 0)].g;
		blue_palette[i].b = game_palette[getpixel(letter[0].bp, 0, 0)].b;
	}
	
        //load the game message from file
//        main_game_vars.commentary = load_vgolf_commentary(vgolf_commentaries.filename[0]);
    set_palette(game_palette);
    
	vgolf_courses.items = 0;
	for_each_file_ex("courses/*.crs", 0, FA_DIREC | FA_LABEL | FA_SYSTEM, vgolf_add_course, 0);
	
	vgolf_avatars.avatars = 0;
	vgolf_add_avatar("vgolf.dat#phantom.pcx", FA_DIREC, NULL);
	for_each_file_ex("avatars/*.pcx", 0, FA_DIREC | FA_LABEL | FA_SYSTEM, vgolf_add_avatar, 0);
	
	vgolf_commentaries.count = 0;
	for_each_file_ex("commentaries/*.vc", 0, FA_DIREC | FA_LABEL | FA_SYSTEM, vgolf_add_commentary, 0);
	
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			game_player[i].pic[j] = create_bitmap(vgolf_avatars.avatar[0]->image[0]->w, vgolf_avatars.avatar[0]->image[0]->h);
		}
	}
	
	//try loading the configuration file..
	if(!load_config("vgolf.cfg"))
	{
		//if none found, use the default configuration settings
		set_default_config();
	}

	if(!load_profiles(&vgolf_profiles, "vgolf.pdt"))
	{
		memset(&vgolf_profiles, 0, sizeof(PROFILE_LIST));
		strcpy(vgolf_profiles.item[0].name, "Guest");
		for(i = 0; i < vgolf_courses.items; i++)
		{
			vgolf_profiles.item[0].course_info[i].id = vgolf_courses.item[i].sum;
		}
		vgolf_profiles.items = 1;
	}
	
	/* create hyperlink pages */
	for(i = 0; i < 4; i++)
	{
		player_page[i] = hyperlink_page_create(px[i], py[i]);
		hyperlink_page_add_element_text(player_page[i], hl_player_name, player_page_text_buffer[i], gfont, 160, 40 + 8, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		hyperlink_page_add_element_image(player_page[i], hl_player_avatar, NULL, 160, 122, HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
		hyperlink_page_add_element_text(player_page[i], hl_remove_player, "Remove Player", gfont, 160, 40 + 8 + 24 + 84, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	
		player_start_page[i] = hyperlink_page_create(px[i], py[i]);
		hyperlink_page_add_element_text(player_start_page[i], hl_add_player, "Add Player", gfont, 160, 120 - text_height(gfont) / 2, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	}
	
	player_global_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(player_global_page, hl_player_next, "Next >", gfont, 640 - text_length(gfont, "Next >") - 4, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(player_global_page, hl_player_back, "< Back", gfont, 2, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	
	sub_menu_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(sub_menu_page, hl_sub_menu_back, "< Back", gfont, 2, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	
	create_scoreboard_page();
	
	profile_global_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(profile_global_page, hl_profile_back, "< Back", gfont, 2, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(profile_global_page, hl_profile_delete, "Delete Profile", gfont, 640 - text_length(gfont, "Delete Profile") - 4, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	create_profile_page(0);
	
	leaderboard_global_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(leaderboard_global_page, hl_profile_back, leaderboard_menu_text, gfont, 2, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	create_profile_page(0);
	
	create_account_page(0);
	
	game_complete_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(game_complete_page, hl_game_complete_finished, "Finished >", gfont, 640 - text_length(gfont, "Finished >") - 4, 480 - text_height(gfont) + 8, VGOLF_COLOR_MENU_MAIN_DARK, VGOLF_COLOR_MENU_MAIN, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_SHADOW);
	
	player_new_account_page = hyperlink_page_create(0, 0);
	hyperlink_page_add_element_text(player_new_account_page, NULL, "Enter Name", gfont, 320 + 2, 92, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(player_new_account_page, NULL, entered_text, gfont, 320 + 2, 92 + 36, VGOLF_COLOR_MENU_OPTION, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(player_new_account_page, hl_player_add_account, "Okay", gfont, 320 + 2, 92 + 36 + 64, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	hyperlink_page_add_element_text(player_new_account_page, hl_player_cancel_account, "Cancel", gfont, 320 + 2, 92 + 36 + 64 + 36, VGOLF_COLOR_MENU_OPTION_DARK, VGOLF_COLOR_MENU_OPTION, makecol(0, 0, 0), HYPERLINK_ELEMENT_FLAG_CLICKABLE | HYPERLINK_ELEMENT_FLAG_CENTER | HYPERLINK_ELEMENT_FLAG_SHADOW);
	
	create_course_page();
	
	course_select_page = hyperlink_page_create(0, 0);
	for(i = 0; i < vgolf_courses.items && i < 4; i++)
	{
		hyperlink_page_add_element_image(course_select_page, hl_course_pick, vgolf_courses.item[i].preview, cx[i], cy[i], HYPERLINK_ELEMENT_FLAG_SHADOW | HYPERLINK_ELEMENT_FLAG_BLIT);
	}
	set_palette(game_palette);
	set_up_menus();
	set_up_credits();
	vgolf_color_black = makecol(0, 0, 0);
	vgolf_color_white = makecol(255, 255, 255);
	if(vgolf_config[CONFIG_DEPTH])
	{
		set_color_depth(8);
	}
	else
	{
		int depth = desktop_color_depth();
		if(depth >= 8)
		{
			set_color_depth(depth);
		}
		else
		{
			set_color_depth(8);
		}
	}
	set_gfx_mode(vgolf_config[CONFIG_GFX_MODE], 640, 480, 0, 0);
	set_display_switch_mode(SWITCH_BACKGROUND);
	set_color_depth(8);
	set_mouse_sprite(NULL);
	
	/* system mouse is always visible on Mac OS X so hide Allegro's cursor */
	#ifdef ALLEGRO_MACOSX
		mouse_hidden = 1;
	#endif
	
	/* curl on OpenPandora doesn't fail when the network is not available so it hangs,
	 * disable uploading by default, new users will probably go straight to the game,
	 * if they later enable uploading and experience hanging they are likely to know
	 * where the problem is */
	#ifdef PANDORA
		vgolf_config[CONFIG_UPLOAD] = 0;
	#endif
	hyperlink_link_mouse(&vgolf_mouse_x, &vgolf_mouse_y, &vgolf_mouse_z, &vgolf_mouse_b);
}

void vgolf_exit(void)
{
	int i, j;
	
	ncds_stop_music();
	
	/* save data */
	save_profiles(&vgolf_profiles, "vgolf.pdt");
	save_config("vgolf.cfg");
	
	/* free resources */
	destroy_bitmap(buffer);
	destroy_bitmap(scratch_pad);
	destroy_bitmap(thanks_pic);
	destroy_bitmap(letter[1].bp);
	destroy_bitmap(letter[0].bp);
	destroy_bitmap(letter[2].bp);
	destroy_bitmap(letter[3].bp);
	destroy_bitmap(letter[4].bp);
	destroy_font(gfont);
	destroy_font(gfont2);
	destroy_font(gfont3);
	destroy_font(gfont4);
	destroy_bitmap(scorecard_pic);
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			destroy_bitmap(game_player[i].pic[j]);
		}
	}
	for(i = 0; i < 4; i++)
	{
		hyperlink_page_destroy(player_page[i]);
		hyperlink_page_destroy(player_start_page[i]);
	}
	hyperlink_page_destroy(player_global_page);
	hyperlink_page_destroy(sub_menu_page);
	hyperlink_page_destroy(scoreboard_page);
	hyperlink_page_destroy(profile_global_page);
	hyperlink_page_destroy(leaderboard_global_page);
	hyperlink_page_destroy(profile_page);
	hyperlink_page_destroy(player_account_page);
	hyperlink_page_destroy(game_complete_page);
	hyperlink_page_destroy(player_new_account_page);
	hyperlink_page_destroy(course_page);
	hyperlink_page_destroy(course_select_page);
	
	if(game_course)
	{
		vgolf_destroy_course(game_course);
	}
	
	destroy_main_image_bank(&main_game_vars.image_bank);
	destroy_sample_bank(&main_game_vars.sound_bank);
	if(main_game_vars.commentary)
	{
		destroy_vgolf_commentary(main_game_vars.commentary);
	}
	
	if(leaderboard)
	{
		t3net_destroy_leaderboard(leaderboard);
	}
}

void credits_escape_helper(void)
{
	fx_fade_start(blue_palette, game_palette, 2, NULL);
	vgolf_state = VGOLF_STATE_MENU;
}

void intro_helper(void)
{
	vgolf_state = VGOLF_STATE_MENU;
	ncds_play_music("vgolf.dat#menu.xm", 1);
	fx_fade_start(black_palette, game_palette, 2, NULL);
	select_palette(game_palette);
	set_mouse_sprite(NULL);
	gametime_reset();
}

void vgolf_logic(void)
{
	int i;
	
	/* handle commentary sample start */
	if(cvoice >= 0)
	{
		if(!voice_check(cvoice))
		{
			ncds_set_music_volume(oldmvol);
		}
	}
	if(cvtime > 0 && csample)
	{
		cvtime--;
		if(cvtime <= 0)
		{
			cvoice = ncds_play_sample_ex(csample, vgolf_config[CONFIG_COMMENTARY_VOLUME], 127, -1, -1);
			csample = NULL;
		}
	}
	if(entering_text)
	{
		text_enter_logic();
	}
	
	/* try keyboard input */
	if(key[KEY_LEFT] || key[KEY_RIGHT])
	{
		mouse_unused = 1;
	}
	
	vgolf_old_mouse_x = vgolf_mouse_x;
	vgolf_old_mouse_y = vgolf_mouse_y;
	vgolf_mouse_x = mouse_x;
	vgolf_mouse_y = mouse_y;
	
	/* try mouse input, enable mouse if mouse moved */
	if(vgolf_old_mouse_x != vgolf_mouse_x || vgolf_old_mouse_y != vgolf_mouse_y)
	{
		mouse_unused = 0;
	}	
		
	/* limit wheel to range (0-20) */
	if(mouse_z < 0)
	{
		position_mouse_z(0);
	}
	else if(mouse_z > 20)
	{
		position_mouse_z(20);
	}
	vgolf_mouse_z = mouse_z;
	
	/* update mouse */
	vgolf_mouse_b = mouse_b;
	if(key[KEY_HOME])
	{
		vgolf_mouse_b |= 1;
	}
	if(key[KEY_PGUP])
	{
		vgolf_mouse_b |= 2;
	}
	
	if(vgolf_mouse_b & 1)
	{
		left_click++;
	}
	else
	{
		left_click = 0;
	}
	if(vgolf_mouse_b & 2)
	{
		right_click++;
	}
	else
	{
		right_click = 0;
	}
	switch(vgolf_state)
	{
		case VGOLF_STATE_INTRO:
		{
			vgolf_frames++;
			if(key[KEY_ESC] || key[KEY_ENTER] || key[KEY_SPACE] || vgolf_frames == 300 || left_click == 1)
			{
				if(fx_fade_speed == 0)
				{
					fx_fade_start(game_palette, black_palette, 2, intro_helper);
					key[KEY_ESC] = key[KEY_ENTER] = key[KEY_SPACE] = 0;
				}
			}
			break;
		}
		case VGOLF_STATE_PLAY:
		{
			main_game_logic(game_player, vgolf_player_count, game_course);
			if(key[KEY_ESC])
			{
				vgolf_pause_game();
				key[KEY_ESC] = 0;
			}
			break;
		}
		case VGOLF_STATE_BALL_SUNK:
		{
			main_game_logic(game_player, vgolf_player_count, game_course);
			newmenu_logic();
			break;
		}
		case VGOLF_STATE_PAUSE:
		{
			if(fx_fade_level >= 64)
			{
				newmenu_logic();
				if(current_newmenu != nmenu[5])
				{
					hyperlink_page_logic(sub_menu_page);
				}
			}
			break;
		}
		case VGOLF_STATE_PROFILE:
		{
			if(key[KEY_ESC])
			{
				vgolf_state = VGOLF_STATE_MENU;
				key[KEY_ESC] = 0;
			}
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			hyperlink_page_logic(profile_page);
			hyperlink_page_logic(profile_global_page);
			break;
		}
		case VGOLF_STATE_PROFILE_DELETE:
		{
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			newmenu_logic();
			break;
		}
		case VGOLF_STATE_PLAYER_SETUP:
		{
			if(key[KEY_ESC])
			{
				vgolf_state = VGOLF_STATE_MENU;
				key[KEY_ESC] = 0;
			}
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			for(i = 0; i < 4; i++)
			{
				hyperlink_page_logic(player_page_pointer[i]);
			}
			hyperlink_page_logic(player_global_page);
			break;
		}
		case VGOLF_STATE_PLAYER_AVATAR:
		{
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			hyperlink_page_logic(player_avatar_page);
			break;
		}
		case VGOLF_STATE_PLAYER_ACCOUNT:
		{
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			hyperlink_page_logic(player_account_page);
			break;
		}
		case VGOLF_STATE_PLAYER_NEW:
		{
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			hyperlink_page_logic(player_new_account_page);
			break;
		}
		case VGOLF_STATE_COURSE:
		{
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			hyperlink_page_logic(course_page);
			break;
		}
		case VGOLF_STATE_COURSE_SELECT:
		{
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			hyperlink_page_logic(course_select_page);
			break;
		}
		case VGOLF_STATE_CREDITS:
		{
			credits_logic(&vgolf_credits);
			if(vgolf_credits.done)
			{
				credits_escape_helper();
			}
			else if((key[KEY_ESC] || left_click == 1) && fx_fade_speed == 0)
			{
				fx_fade_start(game_palette, blue_palette, 2, credits_escape_helper);
				key[KEY_ESC] = 0;
			}
			break;
		}
		case VGOLF_STATE_LEADERBOARDS:
		{
			if(key[KEY_ESC])
			{
				play_esc_sample();
				vgolf_state = VGOLF_STATE_MENU;
				if(leaderboard_mode == 1)
				{
					set_current_newmenu(nmenu[0]);
					ncds_play_music("vgolf.dat#menu.xm", 1);
					gametime_reset();
				}
				key[KEY_ESC] = 0;
			}
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			hyperlink_page_logic(leaderboard_page);
			hyperlink_page_logic(leaderboard_global_page);
			break;
		}
		default:
		{
			newmenu_logic();
			for(i = 1; i < 5; i++)
			{
				letter_logic(&letter[i]);
			}
			if(bfade_speed < 0)
			{
				bfade_level += bfade_speed;
				if(bfade_level < bfade_target)
				{
					bfade_level = bfade_target;
					bfade_speed = 0;
				}
			}
			else if(bfade_speed > 0)
			{
				bfade_level += bfade_speed;
				if(bfade_level > bfade_target)
				{
					bfade_level = bfade_target;
					bfade_speed = 0;
				}
			}
			if(current_newmenu != nmenu[0])
			{
				hyperlink_page_logic(sub_menu_page);
			}
			break;
		}
	}
	ncdmsg_logic(&vgolf_messages);
	fx_fade_logic();
	vgolf_ticks++;
	rest(0);
}

void vgolf_render_logo(void)
{
	int i;
	
	draw_sprite(buffer, letter[0].bp, fixtoi(letter[0].x), fixtoi(letter[0].y));
	for(i = 4; i >= 1; i--)
	{
		if(i != 2)
		{
			fx_draw_3d_sprite(buffer, letter[i].bp, fixtoi(letter[i].x), fixtoi(letter[i].y), fixtoi(letter[i].z), 320, 240);
		}
	}
	fx_draw_3d_sprite(buffer, letter[2].bp, fixtoi(letter[2].x), fixtoi(letter[2].y), fixtoi(letter[2].z), 320, 240);
}

void vgolf_render(void)
{
	int i;
	int px[4] = {0, 320, 0, 320};
	int py[4] = {0, 0, 240, 240};
	char textbuffer[256] = {0};
	
	switch(vgolf_state)
	{
		case VGOLF_STATE_INTRO:
		{
			blit(logo_image, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
			break;
		}
		case VGOLF_STATE_PLAY:
		{
			main_game_display(buffer, game_player, vgolf_player_count, game_course);
			break;
		}
		case VGOLF_STATE_BALL_SUNK:
		{
			main_game_display(buffer, game_player, vgolf_player_count, game_course);
			drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
			rectfill(buffer, 0, 0, buffer->w - 1, buffer->h - 1, makecol(128, 128, 128));
			solid_mode();
			newmenu_render(buffer);
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		case VGOLF_STATE_PAUSE:
		{
			main_game_display(buffer, game_player, vgolf_player_count, game_course);
			color_map = &shadow_table;
			drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
			rectfill(buffer, 0, 0, buffer->w - 1, buffer->h - 1, makecol(128, 128, 128));
			solid_mode();
			if(fx_fade_level >= 64)
			{
				newmenu_render(buffer);
				if(current_newmenu != nmenu[5])
				{
					hyperlink_page_render(sub_menu_page, buffer);
				}
			}
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		case VGOLF_STATE_PROFILE:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			vgolf_render_logo();
			shadow_box(buffer, 132, 64, 640 - 132 - 1, 64 + 352, 2, makecol(192, 192, 192), makecol(192, 192, 192));
			shadow_box(buffer, 132 + 8, 64 + 8, 640 - 132 - 8 - 1, 64 + 352 - 8, 1, makecol(192, 192, 192), makecol(192, 192, 192));
			hyperlink_page_render(profile_page, buffer);
			hyperlink_page_render(profile_global_page, buffer);
			
			/* draw detailed scores */
			if(profile_show_details && profile_course >= 0)
			{
				draw_character_ex(buffer, scorecard_pic, 320 - scorecard_pic->w / 2 - 2, 80 + 128 + 2, makecol(0, 0, 0), makecol(0, 0, 0));
				draw_sprite(buffer, scorecard_pic, 320 - scorecard_pic->w / 2, 80 + 128);

				/* draw the scorecard header */
				if(vgolf_profiles.item[selected_profile].course_info[profile_course].completed <= 0)
				{
					sprintf(text_buffer, "Score: (N/A)");
				}
				else if(vgolf_profiles.item[selected_profile].course_info[profile_course].total_score - vgolf_courses.item[profile_course].par == 0)
				{
					sprintf(text_buffer, "Score: (E)");
				}
				else if(vgolf_profiles.item[selected_profile].course_info[profile_course].total_score - vgolf_courses.item[profile_course].par > 0)
				{
					sprintf(text_buffer, "Score: +%d", vgolf_profiles.item[selected_profile].course_info[profile_course].total_score - vgolf_courses.item[profile_course].par);
				}
				else
				{
					sprintf(text_buffer, "Score: %d", vgolf_profiles.item[selected_profile].course_info[profile_course].total_score - vgolf_courses.item[profile_course].par);
				}
				outline_text_shadow_center(buffer, gfont2, 320 - scorecard_pic->w / 2 + 110, 80 + 128 - 2, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
				
				/* draw the scorecard hole numbers */
				for(i = 0; i < 9; i++)
				{
					sprintf(text_buffer, "%d", i + 1);
					outline_text_shadow_center(buffer, gfont2, 320 - scorecard_pic->w / 2 + i * 25 + 13, 80 + 128 + 1 + 21, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
					sprintf(text_buffer, "%d", i + 10);
					outline_text_shadow_center(buffer, gfont2, 320 - scorecard_pic->w / 2 + i * 25 + 13, 80 + 128 + 18 + 17 * 3 + 1, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
				}
				
				/* draw the scores */
				if(vgolf_profiles.item[selected_profile].course_info[profile_course].completed > 0)
				{
					for(i = 0; i < 9; i++)
					{
						sprintf(text_buffer, "%d", vgolf_profiles.item[selected_profile].course_info[profile_course].score[i]);
						outline_text_shadow_center(buffer, gfont2, 320 - scorecard_pic->w / 2 + i * 25 + 13, 80 + 128 + 18 + 17 + 1 + 10, -2, 2, makecol(0, 192, 0), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
	
						sprintf(text_buffer, "%d", vgolf_profiles.item[selected_profile].course_info[profile_course].score[i + 9]);
						outline_text_shadow_center(buffer, gfont2, 320 - scorecard_pic->w / 2 + i * 25 + 13, 80 + 128 + 18 + 17 * 4 + 1 + 7, -2, 2, makecol(0, 192, 0), makecol(0, 0, 0), makecol(0, 0, 0), text_buffer);
					}
				}
			}
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		case VGOLF_STATE_PLAYER_SETUP:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			vgolf_render_logo();
			for(i = 0; i < 4; i++)
			{
				shadow_box(buffer, px[i] + 32, py[i] + 32, px[i] + 288 - 1, py[i] + 208 - 1, 2, makecol(192, 192, 192), makecol(192, 192, 192));
				shadow_box(buffer, px[i] + 40, py[i] + 40, px[i] + 280 - 1, py[i] + 200 - 1, 1, makecol(192, 192, 192), makecol(192, 192, 192));
				hyperlink_page_render(player_page_pointer[i], buffer);
			}
			hyperlink_page_render(player_global_page, buffer);
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		case VGOLF_STATE_PLAYER_AVATAR:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			vgolf_render_logo();
			shadow_box(buffer, 144, 80, 144 + 320 + 24 - 1, 80 + 256 + 24 + 24 + 8 - 1, 2, makecol(192, 192, 192), makecol(192, 192, 192));
			shadow_box(buffer, 152, 88, 144 + 320 + 24 - 8 - 1, 80 + 256 + 24 + 24 + 8 - 8 - 1, 1, makecol(192, 192, 192), makecol(192, 192, 192));
			hyperlink_page_render(player_avatar_page, buffer);
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		case VGOLF_STATE_PLAYER_ACCOUNT:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			vgolf_render_logo();
			shadow_box(buffer, 144, 80, 144 + 320 + 24 - 1, 80 + 256 + 24 + 24 + 8 - 1, 2, makecol(192, 192, 192), makecol(192, 192, 192));
			shadow_box(buffer, 152, 88, 144 + 320 + 24 - 8 - 1, 80 + 256 + 24 + 24 + 8 - 8 - 1, 1, makecol(192, 192, 192), makecol(192, 192, 192));
			hyperlink_page_render(player_account_page, buffer);
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		case VGOLF_STATE_PLAYER_NEW:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			vgolf_render_logo();
			shadow_box(buffer, 144, 80, 144 + 320 + 24 - 1, 80 + 256 + 24 + 24 + 8 - 1, 2, makecol(192, 192, 192), makecol(192, 192, 192));
			shadow_box(buffer, 152, 88, 144 + 320 + 24 - 8 - 1, 80 + 256 + 24 + 24 + 8 - 8 - 1, 1, makecol(192, 192, 192), makecol(192, 192, 192));
			hyperlink_page_render(player_new_account_page, buffer);
			if(vgolf_ticks % 30 < 15)
			{
				outline_text_shadow(buffer, gfont, 320 + 2 + text_length(gfont, entered_text) / 2 + 4, 96 + 32 - 2, -2, 2, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), "_");
			}
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		case VGOLF_STATE_COURSE:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			vgolf_render_logo();
			if(newmenu_controls_active)
			{
				shadow_box(buffer, buffer->w / 2 - vgolf_courses.item[selected_course].preview->w / 2 - 32 - 64, 32, buffer->w / 2 + vgolf_courses.item[selected_course].preview->w / 2 + 32 + 64 - 1, buffer->h - 32 - 1, 2, makecol(192, 192, 192), makecol(192, 192, 192));
				shadow_box(buffer, buffer->w / 2 - vgolf_courses.item[selected_course].preview->w / 2 - 24 - 64, 32 + 8, buffer->w / 2 + vgolf_courses.item[selected_course].preview->w / 2 + 24 + 64 - 1, buffer->h - 32 - 8 - 1, 1, makecol(192, 192, 192), makecol(192, 192, 192));
				hyperlink_page_render(course_page, buffer);
				if(!mouse_hidden)
				{
					draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
				}
        	}
			break;
		}
		case VGOLF_STATE_COURSE_SELECT:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			vgolf_render_logo();
			shadow_box(buffer, buffer->w / 2 - vgolf_courses.item[selected_course].preview->w / 2 - 32 - 64, 32, buffer->w / 2 + vgolf_courses.item[selected_course].preview->w / 2 + 32 + 64 - 1, buffer->h - 32 - 1, 2, makecol(192, 192, 192), makecol(192, 192, 192));
			shadow_box(buffer, buffer->w / 2 - vgolf_courses.item[selected_course].preview->w / 2 - 24 - 64, 32 + 8, buffer->w / 2 + vgolf_courses.item[selected_course].preview->w / 2 + 24 + 64 - 1, buffer->h - 32 - 8 - 1, 1, makecol(192, 192, 192), makecol(192, 192, 192));
			hyperlink_page_render(course_select_page, buffer);
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		case VGOLF_STATE_CREDITS:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			credits_render(buffer, &vgolf_credits);
			break;
		}
		case VGOLF_STATE_LEADERBOARDS:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			vgolf_render_logo();
			shadow_box(buffer, 132, 32, 640 - 132 - 1, 32 + 384 + 32, 2, makecol(192, 192, 192), makecol(192, 192, 192));
			shadow_box(buffer, 132 + 8, 32 + 8, 640 - 132 - 8 - 1, 32 + 384 + 32 - 8, 1, makecol(192, 192, 192), makecol(192, 192, 192));
			hyperlink_page_render(leaderboard_page, buffer);
			hyperlink_page_render(leaderboard_global_page, buffer);
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
		default:
		{
			clear_to_color(buffer, getpixel(letter[0].bp, 0, 0));
			newmenu_render(buffer);
			vgolf_render_logo();
			if(current_newmenu != nmenu[0])
			{
				hyperlink_page_render(sub_menu_page, buffer);
			}
			if(!mouse_hidden)
			{
				draw_sprite(buffer, mouse_sprite, vgolf_mouse_x, vgolf_mouse_y);
			}
			break;
		}
	}
	for(i = 0; i < vgolf_messages.messages; i++)
	{
		outline_text_shadow(buffer, gfont2, 1, (i) * text_height(gfont2) + 1, -1, 1, vgolf_messages.message[i].color, makecol(0, 0, 0), makecol(0, 0, 0), vgolf_messages.message[i].text);
	}
	if(entering_text == 1)
	{
		outline_text_shadow(buffer, gfont2, 1, buffer->h - text_height(gfont2) - 1, -1, 1, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), entered_text);
	}
	if(debug_mode)
	{
		sprintf(textbuffer, "vmessage_slot = %d", main_game_vars.cur_vmessage_slot);
		outline_text_shadow(buffer, gfont2, 1, 1, -1, 1, makecol(255, 255, 255), makecol(0, 0, 0), makecol(0, 0, 0), textbuffer);
	}
	fx_fade_render();
	if(vgolf_config[CONFIG_VSYNC] && fx_fade_speed == 0)
	{
		#ifdef ALLEGRO_WINDOWS
			IdleUntilVSync();
		#endif
		vsync();
		#ifdef ALLEGRO_WINDOWS
			DoneVSync();
		#endif
	}
	blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

int main(int argc, char * argv[])
{
	int i;
	int updated = 0;
	
	if(argc > 1)
	{
		for(i = 1; i < argc; i++)
		{
			if(!stricmp(argv[i], "-tune"))
			{
				if(i + 1 < argc)
				{
					vgolf_config[CONFIG_FINE_TUNE] = atoi(argv[i + 1]);
				}
				ncds_set_fine_tune(vgolf_config[CONFIG_FINE_TUNE]);
			}
			else if(!stricmp(argv[i], "-mix"))
			{
				if(i + 1 < argc)
				{
					vgolf_config[CONFIG_MIX] = atoi(argv[i + 1]);
				}
				ncds_set_mix_frequency(vgolf_config[CONFIG_MIX]);
			}
		}
	}
	vgolf_init();
	
	nmenu[0]->current_item = 0;
	set_current_newmenu(nmenu[0]);
	
	gametime_init(60);
	vgolf_state = VGOLF_STATE_INTRO;
	set_palette(black_palette);
	vgolf_render();
	fx_fade_start(black_palette, game_palette, 2, NULL);
	select_palette(game_palette);
	set_mouse_sprite(NULL);
	vgolf_frames = 0;
	#ifdef ALLEGRO_WINDOWS
		PrepVSyncIdle();
	#endif
	while(vgolf_state != VGOLF_STATE_FIN)
	{
		if(key[KEY_TAB] && debug_mode)
		{
			vgolf_logic();
			vgolf_render();
			vgolf_render();
			gametime_reset();
		}
		else
		{
			while(gametime_get_frames() - gametime_tick > 0)
			{
				vgolf_logic();
				updated = 0;
				++gametime_tick;
			}
		}
		ncds_update_music();
		if(!updated)
		{
			vgolf_render();
			updated = 1;
		}
	}
	ncds_stop_music();
	vgolf_exit();
	
	return 0;
}
END_OF_MAIN()
