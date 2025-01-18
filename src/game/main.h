#ifndef MAIN_H
#define MAIN_H

#include <allegro.h>
#include "structs.h"
#include "avatar.h"
#include "course.h"
#include "commentary.h"

#define VGOLF_PI 3.14159265358979323846

#define VGOLF_VERSION_STRING "vGolf v1.24"

#define VGOLF_STATE_INTRO           0
#define VGOLF_STATE_MENU            1
#define VGOLF_STATE_SCORE_CARD      2

#define VGOLF_STATE_PLAY            3

	#define VGOLF_PLAY_STATE_SCORES 0
	#define VGOLF_PLAY_STATE_GAME   1
	
#define VGOLF_STATE_PLAYER_SETUP    4
#define VGOLF_STATE_PLAYER_ACCOUNT  5
#define VGOLF_STATE_PLAYER_AVATAR   6
#define VGOLF_STATE_PLAYER_NEW      7
#define VGOLF_STATE_COURSE          8
#define VGOLF_STATE_COURSE_SELECT   9
#define VGOLF_STATE_CREDITS        10
#define VGOLF_STATE_PAUSE          11
#define VGOLF_STATE_BALL_SUNK      12
#define VGOLF_STATE_PROFILE        13
#define VGOLF_STATE_PROFILE_DELETE 14
#define VGOLF_STATE_LEADERBOARDS   15

#define VGOLF_STATE_FIN       99

#define VGOLF_MAX_COURSES     32

#define VGOLF_COLOR_MENU_HEADER makecol(0, 255, 0)
#define VGOLF_COLOR_MENU_OPTION makecol(255, 255, 255)
#define VGOLF_COLOR_MENU_OPTION_DARK makecol(192, 192, 192)
#define VGOLF_COLOR_MENU_MAIN makecol(0, 255, 0)
#define VGOLF_COLOR_MENU_MAIN_DARK makecol(0, 192, 0)

typedef struct
{
	
	BITMAP * bp;
	fixed x, y, z;
	fixed vx, vy, vz;
	int active;
	int state;
	
} TITLE_LETTER;

typedef struct
{
	
	BITMAP * preview;
	char filename[1024];
	char name[256];
	char author[256];
	char comment[1024];
	int score[3];
	char score_name[3][64];
	int par;
	int holes;
	unsigned long sum;
	
} VGOLF_COURSE_INFO;

typedef struct
{
	
	VGOLF_COURSE_INFO item[256];
	int items;
	
} VGOLF_COURSE_DATABASE;

typedef struct
{
	
	VGOLF_AVATAR * avatar[VGOLF_MAX_AVATARS];
	unsigned long sum[VGOLF_MAX_AVATARS];
	int avatars;
	
} VGOLF_AVATAR_DATABASE;

typedef struct
{
	
	char filename[256][1024];
	char name[256][256];
	char author[256][256];
	char comment[1024][256];
	int count;
	
} VGOLF_COMMENTARY_DATABASE;

typedef struct
{
	
	char name[64];
	unsigned long avatar;
	int total_strokes;            //total course strokes for this player
	int total_par;                //total par score played against for this player
	int total_rounds_won;         //total wins (stroke play)
	int total_matches_won;        //total wins (match play)
	int total_rounds_played;      //total stroke rounds played in
	int total_matches_played;     //total match rounds played in
	int total_holes_played;       //total holes played
	int total_courses_completed;  //# of different courses successfully completed
	int total_top_scores;         //total high scores
	char course_name[MAX_PLAYER_COURSES_COMPLETED][128];  //names of the courses this player has completed
	char course_top_name[3][128]; //name of courses for which this player has top scores
	int course_top_num_holes[3];  //# of holes in the top score courses
	SCORECARD top_scorecard[3];   //top scorecards
	
} VGOLF_PLAYER_PROFILE;

typedef struct
{
	
	VGOLF_PLAYER_PROFILE item[256];
	int items;
	
} VGOLF_PLAYER_DATABASE;

extern unsigned long left_click;
extern unsigned long right_click;
extern VGOLF_COURSE_DATABASE vgolf_courses;
extern char fname_buffer[1024];
extern PALETTE game_palette;
extern int vgolf_mouse_x, vgolf_mouse_y, vgolf_mouse_z, vgolf_mouse_start_z, vgolf_mouse_b;

extern char * course_filename(const char * str, char * filename);

void vgolf_pause_game(void);
void vgolf_resume_game(void);
void vgolf_end_game(void);
void play_select_sample(void);
void play_esc_sample(void);
void play_slider_sample(void);
void play_item_select(void);
void single_player_course_completed_logic(MAIN_GAME_VARS * gvars, PLAYER * pl, VGOLF_COURSE * cr);
void multi_player_course_completed_logic(MAIN_GAME_VARS * gvars, PLAYER pl[], int num_players, VGOLF_COURSE * cr);

void create_account_page(int player);
void create_profile_page(int profile);
void create_leaderboard_page(void);
void create_avatar_page(void);
void create_scoreboard_page(void);
void create_course_page(void);
int hl_player_add_account(int p, void * pp);
void start_victory_screen_segin(MAIN_GAME_VARS * gvars, VGOLF_COURSE * cr, PLAYER pl[], int num_players);

void build_turn_text(int p);
int vgolf_get_next_player(void);
int check_ball_collision(BALL * ball, HOLE * hole);
void do_player_hole_finish(PLAYER * pl, HOLE * hole);
void do_ball_wall_collision(BALL * ball, V_INT seg, HOLE * hole, int poly_no);
void do_ball_ball_collision(BALL * ball, V_INT seg, HOLE * hole, int poly_no);
void do_ball_drop(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole);
void do_ball_rehit(BALL * ball, HOLE * hole);
void vgolf_ball_sunk(void);
void reset_letters(void);
int save_player_profile(VGOLF_PLAYER_PROFILE * pp, char * fn);
int load_player_profile(VGOLF_PLAYER_PROFILE * pp, char * fn);
int hl_sub_menu_back(int p, void * pp);
void play_item_select_sample(void);
int load_ball_images(BALL * ball);

#endif
