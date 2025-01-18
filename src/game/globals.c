#include <allegro.h>
#include "defines.h"

BITMAP * images[MAX_IMAGES] = NULL;
SAMPLE * sound[MAX_SOUNDS] = NULL;

//MAIN_IMAGE_BANK image_bank; //main image structure
CONTROLLER controller[MAX_CONTROLLER_TYPES];//preset control configs
STATSHEET player_statsheet[MAX_PLAYER_SAVED_STATS]; //all currently-loaded stats
//SOUND_BANK sound_bank;  //samples
float vict_cloud_x[4];  //coordinates for floating clouds
float vict_cloud_y[4];
float vict_trophy_x[3]; //coordinates for trophies
float vict_trophy_y[3];
char game_mode; //stroke or match play?
char hole_finished;     //is hole finished?
char quit_game; //exit game?
char quit_title;//exit title/frontend?
char quit_all;  //exit everything?
char debug_on;  //toggles debug mode
int game_state; //current game state
int game_state_old;     //old game state for restoring
int cur_vmessage_slot;  //current message queue slot
int num_players;//# players currently playing
int cur_player; //current player in use
int cur_hole;   //current hole being played (0-17)
int vict_pody;  //vertical placement for podiums on victory screen
int vict_winner[3];     //winner refs
int cur_statsheet;      //current statsheet to display (used on player setup screen)
int ingame_timer;       //ingame timer
int ingame_clicker;
char ingame_timer_change;
char player_huds_hidden; //are the player huds currently hidden?
char player_huds_hiding; //tells when the player huds are disappearing from view
char got_hiscore;//tells when a player has gotten a high score (on the victory screen)
short save_stats;//save stats to file?
short num_player_images; //# player character images

int f_index; //# of course filenames currently found
//char * f_packfile[MAX_PACKFILES];
char f_packfile[MAX_PACKFILES][32];  //list of course filenames to select from
