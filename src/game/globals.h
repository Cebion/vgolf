#ifndef GLOBALS_H
#define GLOBALS_H

#include "defines.h"

extern BITMAP * images[MAX_IMAGES];
extern SAMPLE * sound[MAX_SOUNDS];

//MAIN_IMAGE_BANK image_bank; //main image structure
//extern CONTROLLER controller[MAX_CONTROLLER_TYPES];//preset control configs
//extern STATSHEET player_statsheet[MAX_PLAYER_SAVED_STATS]; //all currently-loaded stats
//SOUND_BANK sound_bank;  //samples
extern COLOR_MAP fade_map;
extern float vict_cloud_x[4];  //coordinates for floating clouds
extern float vict_cloud_y[4];
extern float vict_trophy_x[3]; //coordinates for trophies
extern float vict_trophy_y[3];
extern char game_mode; //stroke or match play?
extern char hole_finished;     //is hole finished?
extern char quit_game; //exit game?
extern char quit_title;//exit title/frontend?
extern char quit_all;  //exit everything?
extern char debug_on;  //toggles debug mode
extern int game_state; //current game state
extern int game_state_old;     //old game state for restoring
extern int cur_vmessage_slot;  //current message queue slot
extern int num_players;//# players currently playing
extern int cur_player; //current player in use
extern int cur_hole;   //current hole being played (0-17)
extern int vict_pody;  //vertical placement for podiums on victory screen
extern int vict_winner[3];     //winner refs
extern int cur_statsheet;      //current statsheet to display (used on player setup screen)
extern int ingame_timer;       //ingame timer
extern int ingame_clicker;
extern char ingame_timer_change;
extern char player_huds_hidden; //are the player huds currently hidden?
extern char player_huds_hiding; //tells when the player huds are disappearing from view
extern char got_hiscore;//tells when a player has gotten a high score (on the victory screen)
extern short save_stats;//save stats to file?
extern short num_player_images; //# player character images

extern int f_index; //# of course filenames currently found
//char * f_packfile[MAX_PACKFILES];
extern char f_packfile[MAX_PACKFILES][32];  //list of course filenames to select from

#endif
