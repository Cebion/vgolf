#ifndef VSTRUCTS_H
#define VSTRUCTS_H

#include "poly.h"
#include "defines.h"
#include "hole.h"
#include "commentary.h"

typedef struct
{
	BITMAP * pic[MAX_ANIMDEF_FRAMES];    //images in this animation def
	short speed;              //animation speed
	short num_frames;         //# of frames
	short cur_frame;          //current frame
} ANIM_DEF;

typedef struct
{
	VERTEX s,v,s_old,s_med;   //position, velocity, old position, hit position vectors
	VERTEX f, cf;                 //vector for additional forces
	BODY body;                //ball body (defines poly and collision stuff)
	ANIM_DEF anim_def[MAX_BALL_ANIM_MODES];  //ball animations where applicable
	RECT hit_rect;            //hit rectangle that surrounds the ball circle
	BITMAP * shad_pic;        //shadow image
	
	float speed;              //current ball speed
	char mode;                //current ball mode
	char visible;             //is ball visible?
	char mirage_visible;      //is the translucent ball visible?
	char in_polymorph;        //is ball inside a polymorph poly?
	int angle;                //ball angle (int)
	float f_angle;            //ball angle (float)
	float radius;             //ball radius
	int cur_vert;             //current vert being interacted with
	int idle_timeout;         //max time to wait before resetting back to idle
	int idle_timer;           //idle timer..
	short mirage_timer;       //timer for displaying translucent ball
	short mirage_timer_end;   //timer end for displaying translucent ball
	int off_conveyor; // tell us when the ball has just rolled of a conveyor belt (ugh, whatever works)
	int conveyor_vert;
	int decel_type;
    int hole_vert;
    int water_vert;
    int lazer_poly;
    char mode2;
} BALL;

typedef struct{
        int key_left;             //turn the club left
        int key_right;            //turn the club right
        int key_up;               //up key (for menus?)
        int key_down;             //down key (for menus?)
        int key_swing_pri;        //club swing key
        int key_swing_alt;        //hide huds key
        int key_show_traj;        //key for displaying trajectory
        char type;                //mouse, keyboard, gamepad, etc.
        }CONTROLLER;

typedef struct{
        char name[32];
        int score[18];
        int par_score[18];
        char match_won[18];
        int num_birdies;
        int num_eagles;
        int num_albo;
        int matches_won;
        int coin_best_combo;
        long coin_score;
        short pic_ref;
        }SCORECARD;

typedef struct{
        int x, y;         //hud display coords
        int w, h;         //hud width and height
        }HUD;

typedef struct{
        char name[32];                //player name saved on this statsheet
        short pic_ref;                //player avatar ref on this statsheet
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
        }STATSHEET;

typedef struct{
        CONTROLLER controller;    //input controller
        SCORECARD scorecard;      //current scorecard (tells total score,# birdies, eagles, etc.)
        STATSHEET statsheet;      //statsheet for saving player stats

        BALL ball;                //player ball
        VERTEX club_v[2];         //vertices defining the club for drawing it onscreen
        VERTEX traj_v[MAX_TRAJECTORY_POINTS];  //vertices representing trajectory calculations
        HUD hud;                  //player heads up display
        char club_swing;          //is the club being swung?
        int cur_strokes;          //current # of strokes on this hole
        int club_angle;           //the angle the club is facing
        float f_club_angle;
        int club_power;           //the hit meter power of the club
        int club_rot_speed;       //speed of club rotation
        float f_club_rot_speed;
        int club_color;           //color to draw the club in
        int club_width;           //width of the club onscreen
        float f_club_width;
        char traj_calc_done;      //are the trajectory calculations finished?
        short traj_count;         //trajectory counter (used to count # of collision points)
        short traj_power;         //current trajectory power (tells how far to show trajectory)
        short traj_power_max;     //max trajectory distance
        short traj_line_timer;    //a timer to make the dots on the dotted line move
        char traj_active;         //is the trajectory active?
        int cur_hole;             //current hole being played (0-17)

	int cur_statsheet;        //reference to saved statsheet
        int coins_overall;        //coins amounted this round
        int coins_total;          //coins amounted this hole
        int coins_current;        //coins totaled on this swing
        int coin_best_combo;      //best string of coins collected

		int profile;
		int avatar;
//        char name[32];            //player name
		BITMAP * pic[4];
//        short pic_ref;            //tells which pic to use for this player's character image
        short pic_mode;           //pic emotion state
        int id;
        int so_close;
        }PLAYER;

typedef struct
{
	
	BITMAP * charpic[MAX_GAME_CHARS][4];   //images for the various avatars (4 emotion states)
	BITMAP * statbarpic;                   //image for the stat bar used on the high-score screens
	BITMAP * scorecard;
	
} MAIN_IMAGE_BANK;

typedef struct{
        SAMPLE * sfx_swing[MAX_SFX_SWING_SAMPLES];    //club swing sounds
        SAMPLE * sfx_ball_collision[MAX_SFX_COLLISION_SAMPLES];    //ball-wall collision sounds
        SAMPLE * sfx_ball_holesink;                   //sound of ball hitting cup
        SAMPLE * sfx_hole_finish[4][MAX_SFX_HOLE_FINISH_SAMPLES];  //sounds played when hole is completed (0:bad, 1:good, 2:very good)
        SAMPLE * sfx_hole_ahhh[MAX_SFX_HOLE_AHHH_SAMPLES];         //crowd "ahhh" sounds for really close moments
        SAMPLE * sfx_ball_splash;                     //sound of ball hitting water
        SAMPLE * sfx_ball_zapped;                     //sound of ball being zapped by laser
        SAMPLE * sfx_ball_teleport;                   //sound of ball teleporting
        SAMPLE * sfx_ball_rotate;                     //sound of club rotating
        SAMPLE * sfx_menu_select;                     //sound of pressing enter on a menu item
        SAMPLE * sfx_menu_escape;                     //sound of esc'ing out of a menu
        SAMPLE * sfx_menu_slider;                     //sound of changing a slider's value
        SAMPLE * sfx_menu_itemsel;                    //sound of selecting a new menu item
        SAMPLE * sfx_coin;                            //sound of gathering a coin
        SAMPLE * sfx_ball_enter;                      //sound of gathering a coin
        SAMPLE * sfx_ball_exit;                       //sound of gathering a coin
        SAMPLE * sfx_title_swing;
        }SOUND_BANK;

typedef struct{
        float vict_cloud_x[4];  //coordinates for floating clouds
        float vict_cloud_y[4];
        float vict_trophy_x[4]; //coordinates for trophies
        float vict_trophy_y[4];
        char game_mode;         //stroke or match play?
        char hole_finished;     //is hole finished?
        char quit_game;         //exit game?
        char quit_title;        //exit title/frontend?
        char quit_all;          //exit everything?
        char debug_on;          //toggles debug mode
        int game_state;         //current game state
        int game_state_old;     //old game state for restoring
        int cur_vmessage_slot;  //current message queue slot
        int num_players;        //# players currently playing
        int cur_player;         //current player in use
        int cur_hole;           //current hole being played (0-17)
        int vict_pody;          //vertical placement for podiums on victory screen
        int vict_winner[4];     //winner refs
        int cur_statsheet;      //current statsheet to display (used on player setup screen)
        int ingame_timer;       //ingame timer
        int ingame_clicker;
        char ingame_timer_change;
//        char player_huds_hidden; //are the player huds currently hidden?
//        char player_huds_hiding; //tells when the player huds are disappearing from view
        int player_huds_state;
        char got_hiscore;        //tells when a player has gotten a high score (on the victory screen)
        short save_stats;        //save stats to file?
        short num_player_images; //# player character images

        MAIN_IMAGE_BANK image_bank;                         //main image structure
        CONTROLLER controller[MAX_CONTROLLER_TYPES];        //preset control configs
        STATSHEET player_statsheet[MAX_PLAYER_SAVED_STATS]; //all currently-loaded stats
        SOUND_BANK sound_bank;  //samples
        VGOLF_COMMENTARY * commentary;    //game quotes/messages
        }MAIN_GAME_VARS;

#endif
