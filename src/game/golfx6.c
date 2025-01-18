#include <stdio.h>
#include <allegro.h>
#include <aldumb.h>
#include <math.h>
#include "defines.h"
#include "structs.h"
#include "body.h"
#include "vfont.h"
#include "menu.h"

#define MAX_POLY_VERTICES 128
#define MAX_POLY_EDGES 32
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

GAME_CONFIG game_config;
COURSE game_course;
PALETTE game_palette;
COLOR_MAP trans_table;
V_FONT game_vfont;
V_MESSAGE game_vmessage[GAME_VMESSAGE_QUEUE_SIZE];
PLAYER game_player[MAX_PLAYERS];
MAIN_GAME_VARS main_game_vars;
CMENU * title_menu;
CMENU ** cur_menu;
MENU_CURSOR main_menu_cursor;
BITMAP * titlepic;
BITMAP * titlelogopic;
DUH * game_song;
AL_DUH_PLAYER * game_song_player;

char * key_names[112] = {" ", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Escape", "Tilde", "Minus", "Equals", "Backspace", "Tab", "Left Brace", "Right Brace", "Enter", "Semicolon", "Quote", "Backslash", "Backslash", "Comma", "Stop", "Slash", "Space", "Insert", "Delete", "Home", "End", "Page Up", "Page Down", "Left", "Right", "Up", "Down", "Page Down", "Left", "Right", "Up", "Down", "Page Down", "Left", "Right", "Up", "Down", "Enter", "Print Screen", "Pause", "Yen", "Yen", "Kana", "Henkan", "Muhenkan", "Left Shift", "Right Shift", "Left Control", "Right Control", "Alt", "Alt", "Windows", "Windows", "Menu", "Scroll Lock", "Number Lock", "Caps Lock"};

void do_ball_laser_zap(BALL * ball, PLAYER * pl, HOLE * hole);


//poll the currently-playing music
void poll_music(void){
	if(game_config.sound_on) al_poll_duh(game_song_player);
}

//stop the currently-playing song
void stop_music(void){
	if(game_config.sound_on){
		//stop the previous song
                if(game_song_player != NULL) al_stop_duh(game_song_player);
        	//remove the old song from memory
        	if(game_song != NULL) unload_duh(game_song);
		}
}

//stop an already playing song, load a new song from file and start playing it
void start_new_music(char * filename){
	int s_stereo = !(game_config.sound_quality%2);          //is stereo on?
	long s_freq = ((game_config.sound_quality+2)/2)*11025;  //what's the frequency, kenneth?

	if(game_config.sound_on){
		//stop the previous song
		stop_music();
		                
                //load the new song
                game_song = dumb_load_xm(filename);
                
                //start the song
                game_song_player = al_start_duh(game_song, s_stereo, 0, game_config.sound_music_volume/100.0, 4096, s_freq);
                }
}

//play a random sound when the ball hits a wall
void play_ball_wall_hit_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_ball_collision[random()%MAX_SFX_COLLISION_SAMPLES], game_config.sound_sfx_volume, 128, 1000, 0);
                }
}


//plays the crowd's "ahh" sound
void play_ahhh_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_hole_ahhh[random()%MAX_SFX_HOLE_AHHH_SAMPLES], game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//plays a "bad job" sound effect
void play_poor_finish_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_hole_finish[0][random()%MAX_SFX_HOLE_FINISH_SAMPLES], game_config.sound_sfx_volume, 128, 1000, 0);
                }
}                
                
//plays a "good job" sound effect
void play_good_finish_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_hole_finish[1][random()%MAX_SFX_HOLE_FINISH_SAMPLES], game_config.sound_sfx_volume, 128, 1000, 0);
                }
}
                        
//play a random "hole-in-one" sample
void play_hole_in_one_sample(void){
        if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_hole_finish[2][random()%MAX_SFX_HOLE_FINISH_SAMPLES], game_config.sound_sfx_volume, 128, 1000, 0);
		}
}
                        
//play the "ball hitting cup" sample
void play_ball_holesink_sample(void){
        if(game_config.sound_on){
                play_sample(main_game_vars.sound_bank.sfx_ball_holesink, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}                
                
//play the teleport sample
void play_ball_teleport_sample(void){
        if(game_config.sound_on){
                play_sample(main_game_vars.sound_bank.sfx_ball_teleport, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//play the "splash" sound
void play_ball_splash_sample(void){
	if(game_config.sound_on){
                play_sample(main_game_vars.sound_bank.sfx_ball_splash, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//plays the "zap" sample
void play_ball_zap_sample(void){
        if(game_config.sound_on){
	        play_sample(main_game_vars.sound_bank.sfx_ball_zapped, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//plays the club hitting ball sample
void play_club_ball_hit_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_swing[random()%MAX_SFX_SWING_SAMPLES], game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//plays the club rotation sample
void play_club_rotate_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_ball_rotate, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//plays the sample associated with pressing enter and selecting a menu item
void play_select_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_menu_select, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//plays the sample associated with hitting the esc key
void play_esc_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_menu_escape, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//plays the sample associated with a menu slider changing value
void play_slider_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_menu_slider, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//plays the sample associated with changing the current menu item selection
void play_item_select_sample(void){
	if(game_config.sound_on){
        	play_sample(main_game_vars.sound_bank.sfx_menu_itemsel, game_config.sound_sfx_volume, 128, 1000, 0);
                }
}

//add the hole-in-one message to the queue
void add_hole_in_one_message(void){
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Hole in one!", SCREEN_W/2-strlen("Hole in one!")*game_vfont.w*2, SCREEN_H/2-game_vfont.h*2, 0, VMSG_TEXT_ENTER_FROMRIGHT_SCALED, -20, 64, 0, 0, -1);
}

//add the messages to the queue when a player gets albatross
void add_albatross_finish_messages(void){
	int max_msgs = 9;
	int rand_no = random()%max_msgs;

	//add a random message to spice things up
	if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Unprecedented!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Legendary performance!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "How did you do that?!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 3) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "I don't believe it!!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Worthy of a green jacket!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 5) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "I can't believe this!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 6) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "The stuff of legends!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 7) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "This is insane!!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 8) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "You are a golfing god!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);

        //add the score message
        add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Albatross!!", SCREEN_W/2-strlen("Albatross!!")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add the messages to the queue when a player gets eagle
void add_eagle_finish_messages(void){
	int max_msgs = 9;
	int rand_no = random()%max_msgs;

	//add a random message to spice things up
	if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Awesome!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Superb!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Masterful work!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 3) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Whoa!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "It's gotta be the shoes!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 5) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Tiger? Is that you?", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 6) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Played like a pro!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 7) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Yes!!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 8) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Boom!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);

        //add the score message
        add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Eagle!!", SCREEN_W/2-strlen("Eagle!!")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}


//add the messages to the queue when a player gets birdie
void add_birdie_finish_messages(void){
	int max_msgs = 9;
	int rand_no = random()%max_msgs;

	//add a random message to spice things up
	if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Great!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Nicely done!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Well played!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 3) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Great job!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Excellent!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 5) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Yay!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 6) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Woohoo!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 7) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Now we're talkin'!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 8) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Very nice!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);

        //add the score message
        add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Birdie!", SCREEN_W/2-strlen("Birdie!")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add the messages to the queue when a player gets par
void add_par_finish_messages(void){
	int max_msgs = 9;
	int rand_no = random()%max_msgs;

	//add a random message to spice things up
        if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Not bad..", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Respectable", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Just right", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 3) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Lookin' good.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Good", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 5) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "An average performance", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 6) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Riding the line", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 7) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "So-So", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 8) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "That'll do...", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);

        //add the score message
        add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Par", SCREEN_W/2-strlen("Par")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add the messages to the queue when a player gets bogey
void add_bogey_finish_messages(void){
	int max_msgs = 9;
	int rand_no = random()%max_msgs;

	//add a random message to spice things up
	if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Disappointing.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Argh!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Got it.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 3) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Bagged it", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Decent.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 5) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Ahem..", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 6) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Good.. I guess..", SCREEN_W/2-strlen("Good.. I guess..")*game_vfont.w/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 7) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "And so it goes", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 8) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "A learning experience", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);

        //add the score message
        add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Bogey", SCREEN_W/2-strlen("Bogey")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add the messages to the queue when a player scores worse than bogey
void add_under_bogey_finish_messages(int diff){
	int max_msgs = 9;
	int rand_no = random()%max_msgs;

	//add a random message to spice things up
	if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Ouch", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Finally", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Shameful..", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 3) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "You can do better..", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "You should fire your caddie", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 5) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Maybe golf isn't for you.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 6) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "It's about time..", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 7) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Terrible", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 8) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Ugly performance", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);

        //add score message
        if(diff == -2){       //double bogey..
        	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Double bogey", SCREEN_W/2-strlen("Double bogey")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
		}
	else if(diff == -3){  //triple bogey..
        	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Triple bogey", SCREEN_W/2-strlen("Triple bogey")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
                }
	//X-bogey
	else if(diff == -4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "+4", SCREEN_W/2-strlen("+4")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
        else if(diff == -5) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "+5", SCREEN_W/2-strlen("+5")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
        else if(diff == -6) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "+6", SCREEN_W/2-strlen("+6")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
        else if(diff == -7) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "+7", SCREEN_W/2-strlen("+7")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
        else if(diff == -8) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "+8", SCREEN_W/2-strlen("+8")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
        else if(diff == -9) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "+9", SCREEN_W/2-strlen("+9")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
        else if(diff == -10) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "+10", SCREEN_W/2-strlen("+10")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -16, 16, -1);
        else add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "+X", SCREEN_W/2-strlen("+X")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_ROTATE_ALL, 0, 0, -32, 32, -1);
}

//add a "so close..." message to the queue
void add_so_close_message(void){
	int max_msgs = 3;
	int rand_no = random()%max_msgs;
                                
	//add the message to the queue..
        if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "That's gotta hurt.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "So close.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
	else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Oooohh.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
}

//add a "zapped" message to the queue
void add_ball_zap_message(void){
	int max_msgs = 5;
	int rand_no = random()%max_msgs;
                                
	//add the message to the queue..
        if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, &game_vfont, "Zapped!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Bzzzzt", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Disintegrated", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 3) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Ashes to ashes..", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Ow!!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
}

//add the messages related to the ball water sink to the queue
void add_ball_water_sunk_messages(void){
	int max_msgs = 5;
	int rand_no = random()%max_msgs;
                                
	//add the comment message to the queue..
        if(rand_no == 0) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Ouch.", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 1) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "In the drink", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 2) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Splashdown", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 3) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "You're all wet", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        else if(rand_no == 4) add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Stupid water!", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_SCALE_ALL_CENTERED, 0, 0, 0, 24, -1);
        
        //add the "rehit, drop?" message
        add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "[D]rop or [R]ehit?", SCREEN_W/2-strlen("[D]rop or [R]ehit?")*game_vfont.w/2, SCREEN_H/2-game_vfont.h/2, 0, VMSG_TEXT_FLOAT, 0, 0, -32, 32, -1);
}

//add the quit prompt message to the queue
void add_quit_prompt_message(void){
	add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Quit? [Y / N]", SCREEN_W/2, SCREEN_H/2, 0, VMSG_TEXT_FLOAT_CENTERED, 0, 0, -32, 32, -1);
}

//draw this polygon to the bitmap centered at this point
void draw_polygon(BITMAP * bp, POLY po, int cx, int cy){
        int i;

        for(i = 0; i < po.num_vertices-1; i++){
                line(bp, cx+po.p[i].x, cy+po.p[i].y, cx+po.p[i+1].x, cy+po.p[i+1].y, 1);
                }
}

//try and free a bitmap from memory..
int trash_bitmap(BITMAP * bp){
        if(bp != NULL){
                destroy_bitmap(bp);
                }
        else{
                return -1;
                }
        return 0;
}

/*modified from cs source to use allegro key handler*/
char * get_string(V_FONT * vf, int len, int x, int y, int col){
        char * TempString;
        int TempKey;
        int CurrentChar = 0;
        char Extended;
        int i;

        /* allocate string */
        TempString = malloc(256);
        memset(TempString, 0, 256);

        while((TempKey & 0xff) != 0xD && (TempKey & 0xff) != 0x1B){
                poll_music();
                rectfill(screen, x, y, x+16*len, y+16, col);
                vfont_printf(screen, vf, x, y, 0, TempString);
                vfont_printf(screen, vf, x + strlen((TempString))*16, y, 0, "_");
                vsync();

                TempKey = readkey();
                if((TempKey & 0xff) == 0x0){
                        TempKey = readkey();
                        Extended = 1;
                        }
                else{
                        Extended = 0;
                        }

                if((TempKey & 0xff) == 0x8){
                        TempString[CurrentChar] = '\0';
                        if(CurrentChar > 0){
                                CurrentChar--;
                                TempString[CurrentChar] = '\0';
                                }
                        }
                /* finish up string if enter pressed */
                else if((TempKey & 0xff) != 0xD && (TempKey & 0xff) != 0x1B && !Extended && CurrentChar < 20){
                        TempString[CurrentChar] = (TempKey & 0xff);
                        CurrentChar++;
                        TempString[CurrentChar] = '\0';
                        }
                }

        /* return the string */
        return TempString;
}

//draw this animation def
void draw_anim_def(BITMAP * bp, ANIM_DEF * an, int x, int y){
        masked_blit(an->pic[an->cur_frame/an->speed], bp, 0, 0, x, y, an->pic[an->cur_frame/an->speed]->w, an->pic[an->cur_frame/an->speed]->h);
}

//handle the animation def
int do_anim_def(BITMAP * bp, ANIM_DEF * an, int x, int y, char loop){
        draw_anim_def(bp, an, x, y);   //draw the current frame to the bitmap
        if(an->cur_frame < an->num_frames*an->speed) an->cur_frame++;  //increment the current frame
        else{
                if(loop) an->cur_frame = 0;     //if we're looping, start back at 0
                }
        return an->cur_frame;
}

//returns the filename to the course datfile so we can load the file.. (ie. "vgolf.dat#blah.pcx")
//TODO: deallocate temp_str
char * course_filename(const char * str, char * filename){
        char * temp_str = malloc(256);
        memset(temp_str, 0, 256);

        strcpy(temp_str, str);
        temp_str[strlen(temp_str)] = '#';

        strcat(temp_str, filename);

        return temp_str;
}

//returns the filename to the datfile so we can load the file.. (ie. "vgolf.dat#blah.pcx")
//TODO: deallocate temp_str
char * base_filename(char * filename){
        char * temp_str = malloc(256);
        memset(temp_str, 0, 256);

        strcpy(temp_str, "vgolf.dat#");
        temp_str[strlen(temp_str)] = '\0';

        strcat(temp_str, filename);

        return temp_str;
}

//gives the hole filename for use in the file loading routine from the datfile
void hole_fname(const char * str, char * str_b, int num){
        sprintf(str_b, "%s%s%s%i%i_hol", str, ".crs#", str, num/10, num%10);
}

//give the stat sheets default values if no saved stats are loaded..
int set_default_statsheets(MAIN_GAME_VARS * gvars){
        int i,j,k;

        for(i = 0; i < MAX_PLAYER_SAVED_STATS; i++){
                strcpy(gvars->player_statsheet[i].name, "");
                gvars->player_statsheet[i].pic_ref = 0;
                gvars->player_statsheet[i].total_strokes = 0;
                gvars->player_statsheet[i].total_par = 0;
                gvars->player_statsheet[i].total_rounds_won = 0;
                gvars->player_statsheet[i].total_matches_won = 0;
                gvars->player_statsheet[i].total_rounds_played = 0;
                gvars->player_statsheet[i].total_matches_played = 0;
                gvars->player_statsheet[i].total_holes_played = 0;
                gvars->player_statsheet[i].total_courses_completed = 0;
                for(j = 0; j < MAX_PLAYER_COURSES_COMPLETED; j++){
                        strcpy(gvars->player_statsheet[i].course_name[j], "");
                        }
                gvars->player_statsheet[i].total_top_scores = 0;
                for(j = 0; j < 3; j++){
                        gvars->player_statsheet[i].top_scorecard[j].pic_ref = 0;
                        gvars->player_statsheet[i].course_top_num_holes[j] = 0;
                        strcpy(gvars->player_statsheet[i].top_scorecard[j].name, "");
                        for(k = 0; k < 18; k++){
                                gvars->player_statsheet[i].top_scorecard[j].score[k] = 0;
                                gvars->player_statsheet[i].top_scorecard[j].par_score[k] = 0;
                                gvars->player_statsheet[i].top_scorecard[j].match_won[k] = 0;
                                }
                        gvars->player_statsheet[i].top_scorecard[j].num_birdies = 0;
                        gvars->player_statsheet[i].top_scorecard[j].num_eagles = 0;
                        gvars->player_statsheet[i].top_scorecard[j].num_albo = 0;
                        }
                }
}

//load saved statsheets
int load_statsheets(MAIN_GAME_VARS * gvars, char * filename){
        int i,j,k;
        PACKFILE * in_file;
        char temp_char;

        in_file = pack_fopen(filename, "r");
        if(in_file == NULL) return 0;

        //read header
        temp_char = pack_getc(in_file);
        if(temp_char != 'C') return 0;
        temp_char = pack_getc(in_file);
        if(temp_char != 'S') return 0;
        temp_char = pack_getc(in_file);
        if(temp_char != 'T') return 0;
        temp_char = pack_getc(in_file);
        if(temp_char != 14) return 0;

        for(i = 0; i < MAX_PLAYER_SAVED_STATS; i++){
                for(j = 0; j < 32; j++) gvars->player_statsheet[i].name[j] = pack_getc(in_file);
                gvars->player_statsheet[i].pic_ref = pack_igetw(in_file);
                gvars->player_statsheet[i].total_strokes = pack_igetw(in_file);
                gvars->player_statsheet[i].total_par = pack_igetw(in_file);
                gvars->player_statsheet[i].total_rounds_won = pack_igetw(in_file);
                gvars->player_statsheet[i].total_matches_won = pack_igetw(in_file);
                gvars->player_statsheet[i].total_rounds_played = pack_igetw(in_file);
                gvars->player_statsheet[i].total_matches_played = pack_igetw(in_file);
                gvars->player_statsheet[i].total_holes_played = pack_igetw(in_file);
                gvars->player_statsheet[i].total_courses_completed = pack_igetw(in_file);
                for(j = 0; j < gvars->player_statsheet[i].total_courses_completed; j++){
                        for(k = 0; k < 128; k++) gvars->player_statsheet[i].course_name[j][k] = pack_getc(in_file);
                        }
                gvars->player_statsheet[i].total_top_scores = pack_igetw(in_file);
                for(j = 0; j < gvars->player_statsheet[i].total_top_scores; j++){
                        strcpy(gvars->player_statsheet[i].top_scorecard[j].name, gvars->player_statsheet[i].name);
                        gvars->player_statsheet[i].top_scorecard[j].pic_ref = gvars->player_statsheet[i].pic_ref;
                        gvars->player_statsheet[i].course_top_num_holes[j] = pack_igetw(in_file);
                        for(k = 0; k < 32; k++) gvars->player_statsheet[i].top_scorecard[j].name[k] = pack_getc(in_file);
                        for(k = 0; k < gvars->player_statsheet[i].course_top_num_holes[j]; k++){
                                gvars->player_statsheet[i].top_scorecard[j].score[k] = pack_igetw(in_file);
                                gvars->player_statsheet[i].top_scorecard[j].par_score[k] = pack_igetw(in_file);
                                gvars->player_statsheet[i].top_scorecard[j].match_won[k] = pack_getc(in_file);
                                }
                        gvars->player_statsheet[i].top_scorecard[j].num_birdies = pack_igetw(in_file);
                        gvars->player_statsheet[i].top_scorecard[j].num_eagles = pack_igetw(in_file);
                        gvars->player_statsheet[i].top_scorecard[j].num_albo = pack_igetw(in_file);
                        }
                }
        pack_fclose(in_file);

        return 1;
}

//save player stats
int save_statsheets(MAIN_GAME_VARS * gvars, char * filename){
        int i,j,k;
        PACKFILE * out_file;

        out_file = pack_fopen(filename, "w");

        //write header
        pack_putc('C', out_file);
        pack_putc('S', out_file);
        pack_putc('T', out_file);
        pack_putc(14, out_file);

        for(i = 0; i < MAX_PLAYER_SAVED_STATS; i++){
                for(j = 0; j < 32; j++) pack_putc(gvars->player_statsheet[i].name[j], out_file);
                pack_iputw(gvars->player_statsheet[i].pic_ref, out_file);
                pack_iputw(gvars->player_statsheet[i].total_strokes, out_file);
                pack_iputw(gvars->player_statsheet[i].total_par, out_file);
                pack_iputw(gvars->player_statsheet[i].total_rounds_won, out_file);
                pack_iputw(gvars->player_statsheet[i].total_matches_won, out_file);
                pack_iputw(gvars->player_statsheet[i].total_rounds_played, out_file);
                pack_iputw(gvars->player_statsheet[i].total_matches_played, out_file);
                pack_iputw(gvars->player_statsheet[i].total_holes_played, out_file);
                pack_iputw(gvars->player_statsheet[i].total_courses_completed, out_file);
                for(j = 0; j < gvars->player_statsheet[i].total_courses_completed; j++){
                        for(k = 0; k < 128; k++) pack_putc(gvars->player_statsheet[i].course_name[j][k], out_file);
                        }
                pack_iputw(gvars->player_statsheet[i].total_top_scores , out_file);
                for(j = 0; j < gvars->player_statsheet[i].total_top_scores; j++){
                        pack_iputw(gvars->player_statsheet[i].course_top_num_holes[j], out_file);
                        for(k = 0; k < 32; k++) pack_putc(gvars->player_statsheet[i].top_scorecard[j].name[k], out_file);
                        for(k = 0; k < gvars->player_statsheet[i].course_top_num_holes[j]; k++){
                                pack_iputw(gvars->player_statsheet[i].top_scorecard[j].score[k], out_file);
                                pack_iputw(gvars->player_statsheet[i].top_scorecard[j].par_score[k], out_file);
                                pack_putc(gvars->player_statsheet[i].top_scorecard[j].match_won[k], out_file);
                                }
                        pack_iputw(gvars->player_statsheet[i].top_scorecard[j].num_birdies, out_file);
                        pack_iputw(gvars->player_statsheet[i].top_scorecard[j].num_eagles, out_file);
                        pack_iputw(gvars->player_statsheet[i].top_scorecard[j].num_albo, out_file);
                        }
                }
        pack_fclose(out_file);

        return 1;
}

//gives the top score list default values if none are loaded
int set_default_course_top_scores(COURSE * cr){
        int i,j;

        //normal scorecard
        for(i = 0; i < MAX_COURSE_TOPSCORES; i++){
                strcpy(cr->top_scorecard[i].name, "T^3 software");
                cr->top_scorecard[i].pic_ref = 0;
                for(j = 0; j < cr->num_holes; j++){
                        cr->top_scorecard[i].score[j] = 5;
                        cr->top_scorecard[i].par_score[j] = 5;
                        cr->top_scorecard[i].match_won[j] = 0;
                        }
                cr->top_scorecard[i].num_birdies = 0;
                cr->top_scorecard[i].num_eagles = 0;
                cr->top_scorecard[i].num_albo = 0;
                }
        //coin collector scorecard
        for(i = 0; i < MAX_COURSE_TOPSCORES; i++){
                strcpy(cr->coin_top_scorecard[i].name, "T^3 software");
                cr->coin_top_scorecard[i].pic_ref = 0;
                for(j = 0; j < cr->num_holes; j++){
                        cr->coin_top_scorecard[i].score[j] = 25;
                        cr->coin_top_scorecard[i].match_won[j] = 0;
                        }
                cr->coin_top_scorecard[i].coin_score = 1000;
                cr->coin_top_scorecard[i].coin_best_combo = 8;
                cr->coin_top_scorecard[i].matches_won = 0;
                }

        return 1;
}

//load the top scores for this course (.crh)
int load_course_top_scores(COURSE * cr, char * filename){
        int i,j;
        char temp_char;
        PACKFILE * in_file;

        in_file = pack_fopen(filename, "r");
        if(in_file == NULL) return 0;

        //read header..
        temp_char = pack_getc(in_file);
        if(temp_char != 'C') return 0;
        temp_char = pack_getc(in_file);
        if(temp_char != 'R') return 0;
        temp_char = pack_getc(in_file);
        if(temp_char != 'H') return 0;
        temp_char = pack_getc(in_file);
        if(temp_char != 36) return 0;

        //load regular game scorecard
        for(i = 0; i < MAX_COURSE_TOPSCORES; i++){
                for(j = 0; j < 32; j++) cr->top_scorecard[i].name[j] = pack_getc(in_file);
                cr->top_scorecard[i].pic_ref = pack_igetw(in_file);
                for(j = 0; j < cr->num_holes; j++){
                        cr->top_scorecard[i].score[j] = pack_igetw(in_file);
                        cr->top_scorecard[i].par_score[j] = pack_igetw(in_file);
                        cr->top_scorecard[i].match_won[j] = pack_getc(in_file);
                        }
                cr->top_scorecard[i].num_birdies = pack_igetw(in_file);
                cr->top_scorecard[i].num_eagles = pack_igetw(in_file);
                cr->top_scorecard[i].num_albo = pack_igetw(in_file);
                }
        if(pack_feof(in_file)){
                pack_fclose(in_file);
                return 1;
                }
        //load coin collector scorecard
        for(i = 0; i < MAX_COURSE_TOPSCORES; i++){
                for(j = 0; j < 32; j++) cr->coin_top_scorecard[i].name[j] = pack_getc(in_file);
                cr->coin_top_scorecard[i].pic_ref = pack_igetw(in_file);
                for(j = 0; j < cr->num_holes; j++){
                        cr->coin_top_scorecard[i].score[j] = pack_igetw(in_file);
                        cr->coin_top_scorecard[i].match_won[j] = pack_getc(in_file);
                        }
                cr->coin_top_scorecard[i].coin_score = pack_igetw(in_file);
                cr->coin_top_scorecard[i].coin_best_combo = pack_igetw(in_file);
                cr->coin_top_scorecard[i].matches_won = pack_igetw(in_file);
                }

        pack_fclose(in_file);

        return 1;
}

//save the top score data to file..
int save_course_top_scores(COURSE * cr, char * filename){
        int i,j;
        PACKFILE * out_file;

        out_file = pack_fopen(filename, "w");
        
        //write header..
        pack_putc('C', out_file);
        pack_putc('R', out_file);
        pack_putc('H', out_file);
        pack_putc(36, out_file);

        //save normal top scores
        for(i = 0; i < MAX_COURSE_TOPSCORES; i++){
                for(j = 0; j < 32; j++) pack_putc(cr->top_scorecard[i].name[j], out_file);
                pack_iputw(cr->top_scorecard[i].pic_ref, out_file);
                for(j = 0; j < cr->num_holes; j++){
                        pack_iputw(cr->top_scorecard[i].score[j], out_file);
                        pack_iputw(cr->top_scorecard[i].par_score[j], out_file);
                        pack_putc(cr->top_scorecard[i].match_won[j], out_file);
                        }
                pack_iputw(cr->top_scorecard[i].num_birdies, out_file);
                pack_iputw(cr->top_scorecard[i].num_eagles, out_file);
                pack_iputw(cr->top_scorecard[i].num_albo, out_file);
                }
        //save coin collector scorecard
        for(i = 0; i < MAX_COURSE_TOPSCORES; i++){
                for(j = 0; j < 32; j++) pack_putc(cr->coin_top_scorecard[i].name[j], out_file);
                pack_iputw(cr->coin_top_scorecard[i].pic_ref, out_file);
                for(j = 0; j < cr->num_holes; j++){
                        pack_iputw(cr->coin_top_scorecard[i].score[j], out_file);
                        pack_putc(cr->coin_top_scorecard[i].match_won[j], out_file);
                        }
                pack_iputw(cr->coin_top_scorecard[i].coin_score, out_file);
                pack_iputw(cr->coin_top_scorecard[i].coin_best_combo, out_file);
                pack_iputw(cr->coin_top_scorecard[i].matches_won, out_file);
                }

        pack_fclose(out_file);

        return 1;
}

//load the course info data (.crn)
int load_course_info(COURSE * cr, char * filename){
        int i;
        PACKFILE * in_file;

        in_file = pack_fopen(filename, "r");

        for(i = 0; i < 128; i++) cr->name[i] = pack_getc(in_file);
        cr->course_par = pack_igetw(in_file);
        cr->num_holes = pack_igetw(in_file);
        cr->empty_color = pack_igetw(in_file);

        pack_fclose(in_file);
}

//load up relevant info so we can see the basics of the course in the course selection screen
int load_course_preview(COURSE * cr, char * filename){
        int i;
        char temp_str[64], temp_str2[64];

        //icon
        cr->image_bank.iconpic = load_pcx(course_filename(filename, "crs_icon_pcx"), game_palette);

        strcpy(temp_str2, filename);
        strcpy(temp_str, filename);
        temp_str2[strlen(filename)] = '#';
        temp_str2[strlen(filename)+1] = '\0';
        temp_str[strlen(filename)-4] = '_';
        temp_str[strlen(filename)-3] = 'c';
        temp_str[strlen(filename)-2] = 'r';
        temp_str[strlen(filename)-1] = 'n';
        temp_str[strlen(filename)] = '\0';
        strcat(temp_str2, temp_str);

        //load the course info
        load_course_info(cr, temp_str2);

        //load the top scores
        strcpy(temp_str, filename);
        temp_str[strlen(temp_str)-3] = 'c';
        temp_str[strlen(temp_str)-2] = 'r';
        temp_str[strlen(temp_str)-1] = 'h';
        temp_str[strlen(temp_str)] = '\0';
        if(!load_course_top_scores(cr, temp_str)){
                set_default_course_top_scores(cr);
                }
        return 1;
}

//load a course from file
int load_course(COURSE * cr, char * filename){
        int i;
        char temp_str[64], temp_str2[64];

        strcpy(cr->fname, filename);
        strcpy(cr->fname_prefix, filename);
        cr->fname_prefix[strlen(filename)] = '#';
        for(i = 0; i < strlen(filename)-4; i++) temp_str[i] = filename[i];
        strcat(cr->fname_prefix, temp_str);

        cr->fname_prefix[strlen(cr->fname_prefix)-1] = '\0';

        strcat(cr->fname_prefix, "00_hol");
        cr->fname[strlen(filename)-4] = '\0';

        strcpy(temp_str2, filename);
        strcat(temp_str2, "#");
        strcat(temp_str2, cr->fname);
        strcat(temp_str2, "_crn\0");

        //load the course info
        load_course_info(cr, temp_str2);

        //course preview icon
        cr->image_bank.iconpic = load_pcx(course_filename(filename, "crs_icon_pcx"), game_palette);

        //tee
        cr->image_bank.vert_pic[0][0] = load_pcx(course_filename(filename, "vpic0000_pcx"), NULL);

        //hole
        cr->image_bank.vert_pic[1][0] = load_pcx(course_filename(filename, "vpic0100_pcx"), NULL);
        cr->image_bank.vert_pic[1][1] = load_pcx(course_filename(filename, "vpic0101_pcx"), NULL);

        //trees
        cr->image_bank.object_pic[0][0] = load_pcx(course_filename(filename, "obj00-00_pcx"), NULL);
        cr->image_bank.object_pic[1][0] = load_pcx(course_filename(filename, "obj01-00_pcx"), NULL);

        //laser/wall
        cr->image_bank.object_pic[2][0] = load_pcx(course_filename(filename, "obj02-00_pcx"), NULL);
        cr->image_bank.object_pic[2][1] = load_pcx(course_filename(filename, "obj02-01_pcx"), NULL);

        //teleport pad
        cr->image_bank.object_pic[3][0] = load_pcx(course_filename(filename, "obj03-00_pcx"), NULL);
        cr->image_bank.object_pic[3][1] = load_pcx(course_filename(filename, "obj03-01_pcx"), NULL);

        //conveyor belt
        cr->image_bank.object_pic[4][0] = load_pcx(course_filename(filename, "obj04-00_pcx"), NULL);
        cr->image_bank.object_pic[4][1] = load_pcx(course_filename(filename, "obj04-01_pcx"), NULL);
        cr->image_bank.object_pic[4][2] = load_pcx(course_filename(filename, "obj04-02_pcx"), NULL);
        cr->image_bank.object_pic[4][3] = load_pcx(course_filename(filename, "obj04-03_pcx"), NULL);

        //animals
        cr->image_bank.animal_pic[0][0] = load_pcx(course_filename(filename, "an0000_pcx"), NULL);
        cr->image_bank.animal_pic[0][1] = load_pcx(course_filename(filename, "an0001_pcx"), NULL);
        cr->image_bank.animal_pic[0][2] = load_pcx(course_filename(filename, "an0002_pcx"), NULL);
        cr->image_bank.animal_pic[0][3] = load_pcx(course_filename(filename, "an0003_pcx"), NULL);
        cr->image_bank.animal_pic[1][0] = load_pcx(course_filename(filename, "an0100_pcx"), NULL);
        cr->image_bank.animal_pic[1][1] = load_pcx(course_filename(filename, "an0101_pcx"), NULL);
        cr->image_bank.animal_pic[1][2] = load_pcx(course_filename(filename, "an0102_pcx"), NULL);
        cr->image_bank.animal_pic[1][3] = load_pcx(course_filename(filename, "an0103_pcx"), NULL);

        //load victory screen images
        cr->image_bank.victory_bgpic = load_pcx(course_filename(filename, "victbg_pcx"), NULL);
        cr->image_bank.victory_podiumpic = load_pcx(course_filename(filename, "victpodm_pcx"), NULL);
        cr->image_bank.victory_cloudpic[0] = load_pcx(course_filename(filename, "victcld0_pcx"), NULL);
        cr->image_bank.victory_cloudpic[1] = load_pcx(course_filename(filename, "victcld1_pcx"), NULL);
        cr->image_bank.victory_trophypic[0] = load_pcx(course_filename(filename, "victtr00_pcx"), NULL);
        cr->image_bank.victory_trophypic[1] = load_pcx(course_filename(filename, "victtr01_pcx"), NULL);
        cr->image_bank.victory_trophypic[2] = load_pcx(course_filename(filename, "victtr02_pcx"), NULL);

        //load coin images
        cr->image_bank.coinpic[0] = load_pcx(course_filename(filename, "coin00_pcx"), NULL);
        cr->image_bank.coinpic[1] = load_pcx(course_filename(filename, "coin01_pcx"), NULL);
        cr->image_bank.coinpic[2] = load_pcx(course_filename(filename, "coin02_pcx"), NULL);
        cr->image_bank.coinpic[3] = load_pcx(course_filename(filename, "coin03_pcx"), NULL);

        //load scoreboard images
        //cr->scoreboard.bgpic = load_pcx(course_filename(filename, "scorebd_pcx"), NULL);
        cr->scoreboard.bgpic = load_pcx(course_filename(filename, "scrbdseg_pcx"), NULL);
        cr->scoreboard.gridpic = load_pcx(course_filename(filename, "scoregd_pcx"), NULL);

        //images for the numbers on the scoreboard (00-18)
        cr->scoreboard.numpic[0] = load_pcx(course_filename(filename, "scoreh00_pcx"), NULL);
        cr->scoreboard.numpic[1] = load_pcx(course_filename(filename, "scoreh01_pcx"), NULL);
        cr->scoreboard.numpic[2] = load_pcx(course_filename(filename, "scoreh02_pcx"), NULL);
        cr->scoreboard.numpic[3] = load_pcx(course_filename(filename, "scoreh03_pcx"), NULL);
        cr->scoreboard.numpic[4] = load_pcx(course_filename(filename, "scoreh04_pcx"), NULL);
        cr->scoreboard.numpic[5] = load_pcx(course_filename(filename, "scoreh05_pcx"), NULL);
        cr->scoreboard.numpic[6] = load_pcx(course_filename(filename, "scoreh06_pcx"), NULL);
        cr->scoreboard.numpic[7] = load_pcx(course_filename(filename, "scoreh07_pcx"), NULL);
        cr->scoreboard.numpic[8] = load_pcx(course_filename(filename, "scoreh08_pcx"), NULL);
        cr->scoreboard.numpic[9] = load_pcx(course_filename(filename, "scoreh09_pcx"), NULL);
        cr->scoreboard.numpic[10] = load_pcx(course_filename(filename, "scoreh10_pcx"), NULL);
        cr->scoreboard.numpic[11] = load_pcx(course_filename(filename, "scoreh11_pcx"), NULL);
        cr->scoreboard.numpic[12] = load_pcx(course_filename(filename, "scoreh12_pcx"), NULL);
        cr->scoreboard.numpic[13] = load_pcx(course_filename(filename, "scoreh13_pcx"), NULL);
        cr->scoreboard.numpic[14] = load_pcx(course_filename(filename, "scoreh14_pcx"), NULL);
        cr->scoreboard.numpic[15] = load_pcx(course_filename(filename, "scoreh15_pcx"), NULL);
        cr->scoreboard.numpic[16] = load_pcx(course_filename(filename, "scoreh16_pcx"), NULL);
        cr->scoreboard.numpic[17] = load_pcx(course_filename(filename, "scoreh17_pcx"), NULL);
        cr->scoreboard.numpic[18] = load_pcx(course_filename(filename, "scoreh18_pcx"), NULL);

        //width and height of the numbers on the scoreboard
        cr->scoreboard.num_w = cr->scoreboard.numpic[0]->w+1;
        cr->scoreboard.num_h = cr->scoreboard.numpic[0]->h+1;
        //x,y values of the scoreboard for positioning on the screen
        cr->scoreboard.x = 0;
        cr->scoreboard.y = 0;
        //x,y offset values of the course name on the scoreboard
        cr->scoreboard.cname_x = 20;
        cr->scoreboard.cname_y = 15;
        //x,y offset values of the menu items on the scoreboard
        cr->scoreboard.menu_x[0] = 130;
        cr->scoreboard.menu_y[0] = 440;
        cr->scoreboard.menu_x[1] = 420;
        cr->scoreboard.menu_y[1] = 440;
        //x,y offset values of the grids on the scoreboard (player 1-4)
        cr->scoreboard.grid_x[0] = 20;
        cr->scoreboard.grid_y[0] = 50;
        cr->scoreboard.grid_x[1] = 20;
        cr->scoreboard.grid_y[1] = 250;
        cr->scoreboard.grid_x[2] = 640-cr->scoreboard.gridpic->w-20;
        cr->scoreboard.grid_y[2] = 50;
        cr->scoreboard.grid_x[3] = 640-cr->scoreboard.gridpic->w-20;
        cr->scoreboard.grid_y[3] = 250;
	//# of segments on scoreboard and # of active ones
	cr->scoreboard.num_scoreboard_segs = 10;
	cr->scoreboard.scoreboard_seg_count = 0;
	cr->scoreboard.scoreboard_seg_counter = 0;
	cr->scoreboard.scoreboard_seg_counter_end = 4;
	        
        //successful
        return 1;
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
int player_course_stroke_total(PLAYER pl, int cur_hole, int num_holes){
        int c, sum = 0;

        //make sure we don't go over the # of holes
        //cur_hole = (cur_hole < num_holes ? cur_hole : num_holes-1);
        for(c = 0; c < cur_hole; c++){
                sum += pl.scorecard.score[c];
                }
        return sum;
}

//total strokes by player in course so far..
int course_stroke_total(PLAYER pl, int cur_hole, int num_holes){
        int c, sum = 0;

        //make sure we don't go over the # of holes
        //cur_hole = (cur_hole < num_holes ? cur_hole : num_holes-1);
        for(c = 0; c < cur_hole; c++){
                sum += pl.scorecard.par_score[c];
                }
        return sum;
}

//total strokes on final scorecard
int scorecard_stroke_total(SCORECARD * sc, int num_holes){
        int c, sum = 0;

        for(c = 0; c < num_holes; c++){
                sum += sc->score[c];
                }
        return sum;
}

//total par on final scorecard
int scorecard_par_total(SCORECARD * sc, int num_holes){
        int c, sum = 0;

        for(c = 0; c < num_holes; c++){
                sum += sc->par_score[c];
                }
        return sum;
}


//check to see if there was a high score set in this round..
int round_hiscore_check(COURSE * cr, PLAYER pl[], int num_players){
        int p,s,cur_slot,got_one = 0;

        switch(main_game_vars.game_mode){
        	case GAMEMODE_COIN_COLLECT: //check for a high score in coin collector mode..
                	cur_slot = -1;                  //-1 means no slot found..
                	got_one = 0;
                	for(s = MAX_COURSE_TOPSCORES-1; s >= 0; s--){  //go through all the top scores and see if player 1's score is better than any in the top score list
                        	if(pl[0].scorecard.coin_score > cr->coin_top_scorecard[s].coin_score){
                                	cur_slot = s;   //tag the slot of the score p1 has beaten
                                	got_one = 1;    //we got one..
                                	}
                        	}
                	//found a slot
                	if(cur_slot != -1){
                        	for(s = cur_slot+1; s < MAX_COURSE_TOPSCORES-1; s++){        //go through and move all the other scores down on the list
                                	cr->coin_top_scorecard[s+1] = cr->coin_top_scorecard[s];
                                	}
                        	cr->coin_top_scorecard[cur_slot] = pl[0].scorecard; //put p1's score in the slot of the score he's beaten
                        	}
			break;
		default:  //check for a high score in all other modes..
                        for(p = 0; p < num_players; p++){        //check the top score list against the scores of all the players
        	                cur_slot = -1;
                	        got_one = 0;
                        	for(s = MAX_COURSE_TOPSCORES-1; s >= 0; s--){   //go through all the top scores..
                                	if(scorecard_stroke_total(&pl[p].scorecard, cr->num_holes) < scorecard_stroke_total(&cr->top_scorecard[s], cr->num_holes)){
                                        	cur_slot = s;   //player p's stroke total is less than that of a top score.. tag this top score slot
                                        	got_one = 1;    //we got one..
                                        	}
	                                }
        	                //found a slot
                	        if(cur_slot != -1){
                                	for(s = cur_slot; s < MAX_COURSE_TOPSCORES-2; s++){
                                	        cr->top_scorecard[s+1] = cr->top_scorecard[s];   //move all the other top scores down one slot
                                        	}
                                	cr->top_scorecard[cur_slot] = pl[p].scorecard;  //place this player's score in the top score list at the slot of the score it's beaten
                                	}
                        	}
			break;
		}
		
        if(!got_one) return 1;  //no top scores found..
        return 0;               //a top score was found
}

//checks against the saved top scores for this player
int round_statsheet_hiscore_check(MAIN_GAME_VARS * gvars, PLAYER * pl, COURSE * cr, int s_slot){
        int s,cur_slot,got_one = 0;

        cur_slot = -1;
        for(s = 2; s >= 0; s--){
                if(scorecard_stroke_total(&pl->scorecard, game_course.num_holes)-scorecard_par_total(&pl->scorecard, game_course.num_holes) < scorecard_stroke_total(&gvars->player_statsheet[s_slot].top_scorecard[s], gvars->player_statsheet[s_slot].course_top_num_holes[s])-scorecard_par_total(&gvars->player_statsheet[s_slot].top_scorecard[s], gvars->player_statsheet[s_slot].course_top_num_holes[s])){
                        cur_slot = s;
                        got_one = 1;
                        }
                }
        //found a slot
        if(cur_slot != -1){
                for(s = cur_slot+1; s < 2; s++){
                        gvars->player_statsheet[s_slot].top_scorecard[s+1] = gvars->player_statsheet[s_slot].top_scorecard[s];
                        gvars->player_statsheet[s_slot].course_top_num_holes[s+1] = gvars->player_statsheet[s_slot].course_top_num_holes[s];
                        }
                gvars->player_statsheet[s_slot].top_scorecard[cur_slot] = pl->scorecard;
                strcpy(gvars->player_statsheet[s_slot].top_scorecard[cur_slot].name, cr->name);
                gvars->player_statsheet[s_slot].course_top_num_holes[cur_slot] = cr->num_holes;
                }
        if(!got_one) return 1;
        return 0;
}

//update the players' statsheets with latest dats
void update_statsheets(MAIN_GAME_VARS * gvars, COURSE * cr){
        int i,j,k,l,slot = 0;
        char got_one = 0, got_slot = 0;

        if(gvars->game_mode == GAMEMODE_COIN_COLLECT) return;

        for(i = 0; i < gvars->num_players; i++){
                slot = 0;
                got_slot = 0;
                for(j = 0; j < MAX_PLAYER_SAVED_STATS; j++){
                        if(!strcmp(gvars->player_statsheet[j].name, game_player[i].name)){
                                slot = j;
                                got_slot = 1;
                                }
                        }
                //if entry not found, add a new entry for this player's stats
                if(!got_slot){
                        for(j = 0; j < MAX_PLAYER_SAVED_STATS; j++){
                                if(!strcmp(gvars->player_statsheet[j].name, "")){
                                        slot = j;
                                        }
                                }
                        }
                strcpy(gvars->player_statsheet[slot].name, game_player[i].name);
                gvars->player_statsheet[slot].pic_ref = game_player[i].pic_ref;
                gvars->player_statsheet[slot].total_strokes += scorecard_stroke_total(&game_player[i].scorecard, cr->num_holes);
                gvars->player_statsheet[slot].total_par += scorecard_par_total(&game_player[i].scorecard, cr->num_holes);
                gvars->player_statsheet[slot].total_holes_played += cr->num_holes;
                if(main_game_vars.num_players > 1){
                        if(main_game_vars.game_mode == GAMEMODE_STROKE_PLAY){
                                gvars->player_statsheet[slot].total_rounds_played++;
                                if(gvars->vict_winner[0] == i) gvars->player_statsheet[slot].total_rounds_won++;
                                }
                        else if(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY){
                                gvars->player_statsheet[slot].total_matches_played += cr->num_holes;
                                gvars->player_statsheet[slot].total_matches_won += game_player[i].scorecard.matches_won;
                                }
                        }

                //add this course to the completed list if it's not already there
                got_one = 1;
                for(j = 0; j < gvars->player_statsheet[slot].total_courses_completed; j++){
                        if(!strcmp(gvars->player_statsheet[slot].course_name[j], cr->name)){
                                got_one = 0;
                                break;
                                }
                        }
                if(got_one){
                        strcpy(gvars->player_statsheet[slot].course_name[gvars->player_statsheet[slot].total_courses_completed], cr->name);
                        gvars->player_statsheet[slot].total_courses_completed++;
                        }
                if(gvars->player_statsheet[slot].total_top_scores < 3) gvars->player_statsheet[slot].total_top_scores++;
                //for(j = 0; j < 3; j++){
                //        gvars->player_statsheet[slot].top_scorecard[j].pic_ref = game_player[i].pic_ref;
                //        strcpy(gvars->player_statsheet[slot].top_scorecard[j].name, cr->name);
                //        }
                round_statsheet_hiscore_check(gvars, &game_player[i], cr, slot);
                }
}

//checks to see if this player has an entry in the stat database
int player_statsheet_check(char * name){
        int s;

        for(s = 0; s < MAX_PLAYER_SAVED_STATS; s++){
                if(!strcmp(name, main_game_vars.player_statsheet[s].name)){
                        return s;   //if a match is found, return the slot number of the saved stat sheet
                        }
                }

        return -1;      //no match was found
}


//clears the key buffer for a specified key
void controller_clear_keybuf(CONTROLLER co, char c_key){
        if(co.type != CONTROLLER_MOUSE){
                if(c_key == 0) key[co.key_swing_pri] = 0;
                else if(c_key == 1) key[co.key_left] = 0;
                else if(c_key == 2) key[co.key_right] = 0;
                else if(c_key == 3) key[co.key_show_traj] = 0;
                }
}

//determines if the trajectory key has been pressed
char controller_key_traj_pressed(CONTROLLER co){
        if(co.type != CONTROLLER_MOUSE){
                if(co.type < CONTROLLER_JOYSTICK_1) return key[co.key_show_traj];
                //else{
                //        return joy[CONTROLLER_JOYSTICK_1-co.type].button[0].b;
                //        }
                }
        return 0;
}

//determines if the swing key has been pressed to start swing
char controller_key_swing_pressed(CONTROLLER co){
        if(co.type != CONTROLLER_MOUSE){
                if(co.type < CONTROLLER_JOYSTICK_1) return key[co.key_swing_pri];
                else{
                        return joy[CONTROLLER_JOYSTICK_1-co.type].button[0].b;
                        }
                }
        else return (mouse_b & 1);
                
}

//determines if the "clear" key has been pressed to clear huds from the screen
char controller_key_clearhud_pressed(CONTROLLER co){
        if(co.type != CONTROLLER_MOUSE){
                if(co.type < CONTROLLER_JOYSTICK_1) return key[co.key_swing_alt];
                else{
                        return joy[CONTROLLER_JOYSTICK_1-co.type].button[1].b;
                        }
                }
        else return (mouse_b & 2);
                
}

//determines if the swing key has been released to end swing
char controller_key_swing_released(CONTROLLER co){
        if(co.type != CONTROLLER_MOUSE){
                if(co.type < CONTROLLER_JOYSTICK_1) return key[co.key_swing_pri];
                else{
                        return !joy[CONTROLLER_JOYSTICK_1-co.type].button[0].b;
                        }
                }
        else return !(mouse_b & 1);
                
}

//determines if the left key is pressed
char controller_key_left_pressed(CONTROLLER co){
        if(co.type != CONTROLLER_MOUSE){
                if(co.type < CONTROLLER_JOYSTICK_1) return key[co.key_left];
                else{
                        return joy[CONTROLLER_JOYSTICK_1-co.type].stick[0].axis[0].d1;
                        }
                }
        return 0;                
}

//determines if the right key is pressed
char controller_key_right_pressed(CONTROLLER co){
        if(co.type != CONTROLLER_MOUSE){
                if(co.type < CONTROLLER_JOYSTICK_1) return key[co.key_right];
                else{
                        return joy[CONTROLLER_JOYSTICK_1-co.type].stick[0].axis[0].d2;
                        }
                }
        return 0;                
}

//a wrapper to add a message to the game message queue
void add_game_message(char * txt, int x, int y){
}

//set the ball polygon (just an arbitrary number of points)
void set_ball_poly(BALL * ball, float rad){
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
        for(i = 0; i < 32; i++){
                ball->body.poly.p[i].x = ball->radius*cos(i*M_PI/16.0f);
                ball->body.poly.p[i].y = ball->radius*sin(i*M_PI/16.0f);
                }
}

void body_move(BODY * bo){
        if(!bo->moveable){
                bo->vel.x = bo->vel.y = 0;
                return;
                }
        bo->pos.x += bo->vel.x;
        bo->pos.y += bo->vel.y;
}

//function to keep the in-game running consistently on different machines..
void increment_ingame_timer(void){
        main_game_vars.ingame_timer++;
}END_OF_FUNCTION(increment_ingame_timer);

//initialize the framerate timer
void initialize_ingame_timer(void){
        main_game_vars.ingame_timer = 0;
        LOCK_VARIABLE(main_game_vars.ingame_timer);
        LOCK_FUNCTION(increment_ingame_timer);

        install_int_ex(increment_ingame_timer, BPS_TO_TIMER(60));
}

//save configuration to file
int save_config(GAME_CONFIG * cfg, char * filename){
        PACKFILE * out_file;

        out_file = pack_fopen(filename, "w");

        //write header
        pack_putc('C', out_file);
        pack_putc('F', out_file);
        pack_putc('G', out_file);
        pack_putc(14, out_file);

        pack_iputw(cfg->gfx_mode, out_file);
        pack_iputw(cfg->screen_w, out_file);
        pack_iputw(cfg->screen_h, out_file);
        pack_putc(cfg->fx_on, out_file);
        pack_putc(cfg->reset_huds, out_file);
        pack_putc(cfg->sound_on, out_file);
        pack_iputw(cfg->sound_quality, out_file);
        pack_iputw(cfg->sound_sfx_volume, out_file);
        pack_iputw(cfg->sound_music_volume, out_file);

        pack_fclose(out_file);

        return 1;
}

//load saved configuration
int load_config(GAME_CONFIG * cfg, char * filename){
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

        cfg->gfx_mode = pack_igetw(in_file);
        cfg->screen_w = pack_igetw(in_file);
        cfg->screen_h = pack_igetw(in_file);
        cfg->fx_on = pack_getc(in_file);
        cfg->reset_huds = pack_getc(in_file);
        cfg->sound_on = pack_getc(in_file);
        cfg->sound_quality = pack_igetw(in_file);
        cfg->sound_sfx_volume = pack_igetw(in_file);
        cfg->sound_music_volume = pack_igetw(in_file);

        cfg->stat_save = 1;

        pack_fclose(in_file);

        return 1;
}

//set a default configuration
int set_default_config(GAME_CONFIG * cfg){
        cfg->gfx_mode = GFX_AUTODETECT_WINDOWED;
        cfg->screen_w = 640;
        cfg->screen_h = 480;

        {
                int i;
                for(i = 0; i < 8; i++){
                        cfg->fx_on |= (int)pow(2, i);
                        }
                }
        cfg->stat_save = 1;
        cfg->reset_huds = 1;
        cfg->sound_on = 1;
        cfg->sound_quality = 2;
        cfg->sound_sfx_volume = 100;
        cfg->sound_music_volume = 50;

        return 0;
}

//loads the main images
void load_main_image_bank(MAIN_IMAGE_BANK * ibank){
        //these are the character pics (each character has a few different emotions)
        ibank->charpic[0][0] = load_pcx(base_filename("c9a00-00_pcx"), game_palette);
        ibank->charpic[0][1] = ibank->charpic[0][2] = ibank->charpic[0][3] = ibank->charpic[0][0];

        ibank->charpic[1][0] = load_pcx(base_filename("c0a00-00_pcx"), NULL);
        ibank->charpic[1][1] = load_pcx(base_filename("c0a01-00_pcx"), NULL);
        ibank->charpic[1][2] = load_pcx(base_filename("c0a02-00_pcx"), NULL);
        ibank->charpic[1][3] = load_pcx(base_filename("c0a03-00_pcx"), NULL);

        ibank->charpic[2][0] = load_pcx(base_filename("c1a00-00_pcx"), NULL);
        ibank->charpic[2][1] = load_pcx(base_filename("c1a01-00_pcx"), NULL);
        ibank->charpic[2][2] = load_pcx(base_filename("c1a02-00_pcx"), NULL);
        ibank->charpic[2][3] = load_pcx(base_filename("c1a03-00_pcx"), NULL);

        ibank->charpic[3][0] = load_pcx(base_filename("c2a00-00_pcx"), NULL);
        ibank->charpic[3][1] = load_pcx(base_filename("c2a01-00_pcx"), NULL);
        ibank->charpic[3][2] = load_pcx(base_filename("c2a02-00_pcx"), NULL);
        ibank->charpic[3][3] = load_pcx(base_filename("c2a03-00_pcx"), NULL);

        ibank->charpic[4][0] = load_pcx(base_filename("c3a00-00_pcx"), NULL);
        ibank->charpic[4][1] = load_pcx(base_filename("c3a01-00_pcx"), NULL);
        ibank->charpic[4][2] = load_pcx(base_filename("c3a02-00_pcx"), NULL);
        ibank->charpic[4][3] = load_pcx(base_filename("c3a03-00_pcx"), NULL);

        ibank->charpic[5][0] = load_pcx(base_filename("c4a00-00_pcx"), NULL);
        ibank->charpic[5][1] = load_pcx(base_filename("c4a01-00_pcx"), NULL);
        ibank->charpic[5][2] = load_pcx(base_filename("c4a02-00_pcx"), NULL);
        ibank->charpic[5][3] = load_pcx(base_filename("c4a03-00_pcx"), NULL);

        ibank->charpic[6][0] = load_pcx(base_filename("c5a00-00_pcx"), NULL);
        ibank->charpic[6][1] = load_pcx(base_filename("c5a01-00_pcx"), NULL);
        ibank->charpic[6][2] = load_pcx(base_filename("c5a02-00_pcx"), NULL);
        ibank->charpic[6][3] = load_pcx(base_filename("c5a03-00_pcx"), NULL);
                                           
        ibank->charpic[7][0] = load_pcx(base_filename("c6a00-00_pcx"), NULL);
        ibank->charpic[7][1] = load_pcx(base_filename("c6a01-00_pcx"), NULL);
        ibank->charpic[7][2] = load_pcx(base_filename("c6a02-00_pcx"), NULL);
        ibank->charpic[7][3] = load_pcx(base_filename("c6a03-00_pcx"), NULL);

        ibank->charpic[8][0] = load_pcx(base_filename("c7a00-00_pcx"), NULL);
        ibank->charpic[8][1] = load_pcx(base_filename("c7a01-00_pcx"), NULL);
        ibank->charpic[8][2] = load_pcx(base_filename("c7a02-00_pcx"), NULL);
        ibank->charpic[8][3] = load_pcx(base_filename("c7a03-00_pcx"), NULL);

        ibank->charpic[9][0] = load_pcx(base_filename("c8a00-00_pcx"), NULL);
        ibank->charpic[9][1] = load_pcx(base_filename("c8a01-00_pcx"), NULL);
        ibank->charpic[9][2] = load_pcx(base_filename("c8a02-00_pcx"), NULL);
        ibank->charpic[9][3] = load_pcx(base_filename("c8a03-00_pcx"), NULL);

        ibank->charpic[10][0] = load_pcx(base_filename("caa00-00_pcx"), NULL);
        ibank->charpic[10][1] = load_pcx(base_filename("caa01-00_pcx"), NULL);
        ibank->charpic[10][2] = load_pcx(base_filename("caa02-00_pcx"), NULL);
        ibank->charpic[10][3] = load_pcx(base_filename("caa03-00_pcx"), NULL);

        ibank->charpic[11][0] = load_pcx(base_filename("cba00-00_pcx"), NULL);
        ibank->charpic[11][1] = load_pcx(base_filename("cba01-00_pcx"), NULL);
        ibank->charpic[11][2] = load_pcx(base_filename("cba02-00_pcx"), NULL);
        ibank->charpic[11][3] = load_pcx(base_filename("cba03-00_pcx"), NULL);

        ibank->charpic[12][0] = load_pcx(base_filename("cca00-00_pcx"), NULL);
        ibank->charpic[12][1] = load_pcx(base_filename("cca01-00_pcx"), NULL);
        ibank->charpic[12][2] = load_pcx(base_filename("cca02-00_pcx"), NULL);
        ibank->charpic[12][3] = load_pcx(base_filename("cca03-00_pcx"), NULL);

        ibank->charpic[13][0] = load_pcx(base_filename("cda00-00_pcx"), NULL);
        ibank->charpic[13][1] = load_pcx(base_filename("cda01-00_pcx"), NULL);
        ibank->charpic[13][2] = load_pcx(base_filename("cda02-00_pcx"), NULL);
        ibank->charpic[13][3] = load_pcx(base_filename("cda03-00_pcx"), NULL);

        //this is the status bar image
        ibank->statbarpic = load_pcx(base_filename("statbar_pcx"), NULL);
}

//load the sound effect files
void load_main_game_samples(SOUND_BANK * sbank){
        sbank->sfx_swing[0] = load_wav(base_filename("sfx_h000_wav"));
        sbank->sfx_swing[1] = load_wav(base_filename("sfx_h001_wav"));
        sbank->sfx_swing[2] = load_wav(base_filename("sfx_h002_wav"));
        sbank->sfx_ball_collision[0] = load_wav(base_filename("sfx_c000_wav"));
        sbank->sfx_ball_collision[1] = load_wav(base_filename("sfx_c001_wav"));
        sbank->sfx_ball_collision[2] = load_wav(base_filename("sfx_c002_wav"));
        sbank->sfx_ball_holesink = load_wav(base_filename("sfx_hole_wav"));
        sbank->sfx_hole_finish[0][0] = load_wav(base_filename("sfx_f000_wav"));
        sbank->sfx_hole_finish[0][1] = load_wav(base_filename("sfx_f001_wav"));
        sbank->sfx_hole_finish[0][2] = load_wav(base_filename("sfx_f002_wav"));
        sbank->sfx_hole_finish[0][3] = load_wav(base_filename("sfx_f003_wav"));
        sbank->sfx_hole_finish[1][0] = load_wav(base_filename("sfx_f100_wav"));
        sbank->sfx_hole_finish[1][1] = load_wav(base_filename("sfx_f200_wav"));
        sbank->sfx_hole_finish[1][2] = load_wav(base_filename("sfx_f201_wav"));
        sbank->sfx_hole_finish[1][3] = load_wav(base_filename("sfx_f202_wav"));
        sbank->sfx_hole_finish[2][0] = load_wav(base_filename("sfx_f300_wav"));
        sbank->sfx_hole_finish[2][1] = load_wav(base_filename("sfx_f301_wav"));
        sbank->sfx_hole_finish[2][2] = load_wav(base_filename("sfx_f302_wav"));
        sbank->sfx_hole_finish[2][3] = load_wav(base_filename("sfx_f303_wav"));
        sbank->sfx_hole_ahhh[0] = load_wav(base_filename("sfx_a000_wav"));
        sbank->sfx_hole_ahhh[1] = load_wav(base_filename("sfx_a001_wav"));
        sbank->sfx_ball_splash = load_wav(base_filename("sfx_spla_wav"));
        sbank->sfx_ball_rotate = load_wav(base_filename("sfx_r000_wav"));
        sbank->sfx_menu_select = load_wav(base_filename("sfx_m000_wav"));
        sbank->sfx_menu_escape = load_wav(base_filename("sfx_m001_wav"));
        sbank->sfx_menu_slider = load_wav(base_filename("sfx_m002_wav"));
        sbank->sfx_menu_itemsel = load_wav(base_filename("sfx_m003_wav"));
        sbank->sfx_coin = load_wav(base_filename("sfx_coin_wav"));
        sbank->sfx_ball_zapped = load_wav(base_filename("sfx_zap_wav"));
        sbank->sfx_ball_teleport = load_wav(base_filename("sfx_tele_wav"));
}

//set the predefined controls for each player
void set_main_player_controls(MAIN_GAME_VARS * gvars){
        //setup 0
        gvars->controller[0].key_up = KEY_UP;
        gvars->controller[0].key_down = KEY_DOWN;
        gvars->controller[0].key_left = KEY_LEFT;
        gvars->controller[0].key_right = KEY_RIGHT;
        gvars->controller[0].key_swing_pri = KEY_Z;
        gvars->controller[0].key_swing_alt = KEY_F1;
        gvars->controller[0].key_show_traj = KEY_X;
        gvars->controller[0].type = CONTROLLER_KEYBOARD_A;

        //setup 1
        gvars->controller[1].key_up = KEY_I;
        gvars->controller[1].key_down = KEY_K;
        gvars->controller[1].key_left = KEY_J;
        gvars->controller[1].key_right = KEY_L;
        gvars->controller[1].key_swing_pri = KEY_U;
        gvars->controller[1].key_swing_alt = KEY_F1;
        gvars->controller[1].key_show_traj = KEY_O;
        gvars->controller[1].type = CONTROLLER_KEYBOARD_B;

        //setup 2
        gvars->controller[2].key_up = KEY_HOME;
        gvars->controller[2].key_down = KEY_END;
        gvars->controller[2].key_left = KEY_DEL;
        gvars->controller[2].key_right = KEY_PGDN;
        gvars->controller[2].key_swing_pri = KEY_PGUP;
        gvars->controller[2].key_swing_alt = KEY_F1;
        gvars->controller[2].key_show_traj = KEY_INSERT;
        gvars->controller[2].type = CONTROLLER_KEYBOARD_C;

        //setup 3
        gvars->controller[3].key_up = KEY_J;
        gvars->controller[3].key_down = KEY_N;
        gvars->controller[3].key_left = KEY_B;
        gvars->controller[3].key_right = KEY_M;
        gvars->controller[3].key_swing_pri = KEY_H;
        gvars->controller[3].key_swing_alt = KEY_F1;
        gvars->controller[3].key_show_traj = KEY_K;
        gvars->controller[3].type = CONTROLLER_KEYBOARD_D;

        //setup 4
        gvars->controller[4].key_up = 0;
        gvars->controller[4].key_down = 0;
        gvars->controller[4].key_left = 0;
        gvars->controller[4].key_right = 0;
        gvars->controller[4].key_swing_pri = 0;
        gvars->controller[4].key_swing_alt = 0;
	gvars->controller[4].key_show_traj = 0;
	gvars->controller[4].type = CONTROLLER_MOUSE;
}

//set the victory screen up for display
void set_victory_screen_vars(MAIN_GAME_VARS * gvars){
        int i;

        gvars->vict_pody = SCREEN_H;   //place the victory podium at the bottom of the screen

        for(i = 0; i < 4; i++){        //generate some random values for the background clouds
                gvars->vict_cloud_x[i] = random()%640;
                gvars->vict_cloud_y[i] = random()%200;
                }
        for(i = 0; i < 3; i++){        //place all the trophies offscreen
                gvars->vict_trophy_x[i] = gvars->vict_trophy_y[i] = -100;
                gvars->vict_winner[i] = 0;
                }
}

/*add a file to the file list (from pp source)*/
void add_course_file(const char * fn, int attrib, int param){
        //main_game_vars.f_packfile[main_game_vars.f_index] = malloc(strlen(fn)+1);
        strcpy(main_game_vars.f_packfile[main_game_vars.f_index], fn);
        main_game_vars.f_index++;
}

/*search for all course files (.CRS) in the game directory (from pp source)*/
void build_packfile_list(void){
	//reset the # of files in the list
        main_game_vars.f_index = 0;
        //add all files ending in .crs to the list
        for_each_file("*.crs", FA_ARCH | FA_RDONLY, add_course_file, 0);
}

/*clear the music list from memory (from pp source)*/
void destroy_packfile_list(void){
	int i;

        for(i = 0; i < main_game_vars.f_index; i++){
                free(main_game_vars.f_packfile[i]);
                }
}

//set the main global game variables
void set_main_game_vars(void){
        int p;

        main_game_vars.quit_game = 0;
        main_game_vars.quit_all = 0;
        main_game_vars.quit_title = 0;
        //main_game_vars.game_state = GAMESTATE_START_HOLE;
        main_game_vars.game_state = GAMESTATE_SCOREBOARD_SEGIN;
        main_game_vars.game_state_old = main_game_vars.game_state;
        main_game_vars.cur_vmessage_slot = 0;
        main_game_vars.cur_player = 0;
        set_main_player_controls(&main_game_vars);  //give a default controller setting for everyone
        set_victory_screen_vars(&main_game_vars);   //reset the victory screen stuff
        main_game_vars.num_players = 1;             //set it to 1 for now..
        main_game_vars.game_mode = GAMEMODE_STROKE_PLAY;  //set it to stroke play as default
        main_game_vars.cur_hole = -1;                //current hole is 0
        main_game_vars.hole_finished = 0;           //hole is not finished
        main_game_vars.cur_statsheet = -1;          
        main_game_vars.ingame_timer = 0;
        main_game_vars.ingame_clicker = 0;
        main_game_vars.ingame_timer_change = 0;     //timing stuff
        main_game_vars.player_huds_hidden = main_game_vars.player_huds_hiding = 0;
        main_game_vars.got_hiscore = 0;

        if(game_config.stat_save == 2) main_game_vars.save_stats = -1;
        else main_game_vars.save_stats = game_config.stat_save;

        main_game_vars.num_player_images = 8;
        
        //give all the players some default settings
        for(p = 0; p < MAX_PLAYERS; p++){
                game_player[p].controller = main_game_vars.controller[p];
                game_player[p].pic_ref = 0;
                }
        strcpy(game_player[0].name, "Player 1");
        strcpy(game_player[1].name, "Player 2");
        strcpy(game_player[2].name, "Player 3");
        strcpy(game_player[3].name, "Player 4");
        
        //load all course filenames into the packlist so we can select them via menu..
	build_packfile_list();
}

//calculate poly normals..
void initialize_hole_polys(HOLE * hole){
        int i;

        for(i = 0; i < hole->num_polys; i++){
                calc_body_normals(&hole->body[i].poly);
                }
}

//reset the message queue
void initialize_vmessage_queue(V_MESSAGE vmsg[], int num_messages, V_FONT * vf){
        int i;

        for(i = 0; i < num_messages; i++){
                set_vfont_message(&vmsg[i], vf, "", 0, 0, 0, VMSG_TEXT_PLAIN, 0, 0, 0, 0, 0);
                }
}

//do all the initial vert calculations/settings..
void setup_hole_verts(HOLE * hole){
        int v;

        for(v = 0; v < hole->num_verts; v++){
                if(hole->vert[v].flag == VERT_FLAG_ANIMAL_0 || hole->vert[v].flag == VERT_FLAG_ANIMAL_1){
	                //refs 4 and 5 define a bounding box that holds the animal inside it
                        hole->vert[v].ref_no[4] = hole->s.x+hole->vert[v].s.x;
                        hole->vert[v].ref_no[5] = hole->s.y+hole->vert[v].s.y;
                        hole->vert[v].ref_no[0] = hole->vert[v].ref_no[1] = hole->vert[v].ref_no[2] = hole->vert[v].ref_no[3] = 0;
                        }
                else if(hole->vert[v].flag == VERT_FLAG_POLY_MORPH){
                        if(hole->vert[v].ref_no[0] == 0){
	                        //tag the associated poly to be a rotating poly
                                hole->body[hole->vert[v].ref_no[1]].poly.color = POLY_TYPE_MORPH_ROTATE;
                                }
                        }
                else if(hole->vert[v].flag == VERT_FLAG_POLY_MOVE_HORIZ){
			//tag the associated poly to be a horizontally moving poly
                        hole->body[hole->vert[v].ref_no[1]].poly.color = POLY_TYPE_MOTION_HORIZ;
                        }
                else if(hole->vert[v].flag == VERT_FLAG_POLY_MOVE_VERT){
			//tag the associated poly to be a vertically moving poly
                        hole->body[hole->vert[v].ref_no[1]].poly.color = POLY_TYPE_MOTION_VERT;
                        }
                else if(hole->vert[v].flag == VERT_FLAG_HOLE){
                        if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
                                hole->vert[v].flag = VERT_FLAG_NULL;  //no hole in coin collector mode
                                }
                        hole->vert[v].ref_no[0] = hole->vert[v].ref_no[1] = hole->vert[v].ref_no[2] = hole->vert[v].ref_no[3] = hole->vert[v].ref_no[4] = hole->vert[v].ref_no[5] = 0;   //empty the hole out if needed
                        }
                else if(hole->vert[v].flag == VERT_FLAG_LASER_SEND){
	                //reset laser active and timers
                        hole->vert[v].ref_no[0] = hole->vert[v].ref_no[1] = 0;
                        //make the associated poly empty while laser is inactive
			hole->body[hole->vert[v].ref_no[4]].poly.color = POLY_TYPE_EMPTY_WALL;
                        }
                else if(hole->vert[v].flag == VERT_FLAG_TELEPORT){
	                //reset animation counter
                        hole->vert[v].ref_no[5] = 0;
                        }
                else if(hole->vert[v].flag == VERT_FLAG_CONVEYOR){
                        hole->vert[v].ref_no[1] = 1;
                        hole->vert[v].ref_no[4] = 0;
                        hole->body[hole->vert[v].ref_no[5]].poly.color = POLY_TYPE_CONVEYOR_WALL;
                        }
                }
        if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
                for(v = hole->num_verts; v < hole->num_verts+hole->num_coins; v++){
                        if(hole->vert[v].flag == VERT_FLAG_COIN_0){
                                int i;

                                hole->vert[v].ref_no[0] = 1;   //set coin as active
                                hole->vert[v].radius = 7;      //set coin radius to reasonable size

                                for(i = 1; i < MAX_VERT_REFS; i++){
                                        hole->vert[v].ref_no[i] = 0;
                                        }
                                }
                        }
                }
}

//get the hole ready for play..
void initialize_hole(HOLE * hole){
        hole->deccel_factor = 0.05;
        hole->s.x = SCREEN_W/2;
        hole->s.y = SCREEN_H/2;
        initialize_hole_polys(hole);
        setup_hole_verts(hole);

        trash_bitmap(hole->layer_pic[2]);
        hole->layer_pic[2] = create_bitmap(SCREEN_W, SCREEN_H);
        clear_to_color(hole->layer_pic[2], 0);
        initialize_vmessage_queue(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont);
        //initialize_game_zbuff(&game_zbuff, hole);
}

//the initialization routines for the game menus..
void initialize_game_menus(void){
	int i;
	
        title_menu = create_menu(250, 150, SCREEN_W/2-125, SCREEN_H/2-75, "main menu", NULL);

        //the main menu
        menu_add_item(title_menu, create_menu_item(15, 25, MENUITEM_GOTO_SUBMENU, "play game", "start playing", 1));
        set_menu_item_refs(title_menu->menu_item[0], 0, 0, 0);
        menu_add_item(title_menu, create_menu_item(15, 50, MENUITEM_GOTO_SUBMENU, "options", "configure game options", 1));
        set_menu_item_refs(title_menu->menu_item[1], 1, 0, 0);
        menu_add_item(title_menu, create_menu_item(15, 75, MENUITEM_GOTO_SUBMENU, "credits", "game credits", 0));
        set_menu_item_refs(title_menu->menu_item[2], 3, 0, 0);
        menu_add_item(title_menu, create_menu_item(15, 100, MENUITEM_GOTO_SUBMENU, "quit", "bye!", 1));
        set_menu_item_refs(title_menu->menu_item[3], 2, 0, 0);

        //the "play game" menu
        menu_add_submenu(title_menu, create_menu(300, 300, SCREEN_W/2-150, SCREEN_H/2-150, "play game menu", title_menu));
        menu_add_item(title_menu->submenu[0], create_menu_item(20, 25, MENUITEM_FROMLIST, "players:", "choose number of players", 1));
        set_menu_item_reflist(title_menu->submenu[0]->menu_item[0], "1", "2", "3", "4", "");
        set_menu_item_refs(title_menu->submenu[0]->menu_item[0], MENU_CHANGE_NUMBER_OF_PLAYERS, 0, 0);
        menu_add_item(title_menu->submenu[0], create_menu_item(20, 80, MENUITEM_FROMLIST, "game type", "choose game type", 1));
        set_menu_item_reflist(title_menu->submenu[0]->menu_item[1], "stroke play", "match play", "coin collector", "");
        set_menu_item_refs(title_menu->submenu[0]->menu_item[1], MENU_CHANGE_GAME_MODE, 0, 0);
        menu_add_item(title_menu->submenu[0], create_menu_item(20, 140, MENUITEM_GOTO_SUBMENU, "configure keys", "configure player input", 1));
	set_menu_item_refs(title_menu->submenu[0]->menu_item[2], 0, 0, 0);
	menu_add_item(title_menu->submenu[0], create_menu_item(20, 175, MENUITEM_GOTO_SUBMENU, "configure players", "configure player information", 1));
	set_menu_item_refs(title_menu->submenu[0]->menu_item[3], 1, 0, 0);
	menu_add_item(title_menu->submenu[0], create_menu_item(20, 210, MENUITEM_GOTO_SUBMENU, "course selection", "select your course", 1));
	set_menu_item_refs(title_menu->submenu[0]->menu_item[4], 2, 0, 0);

        //options menu 1
	menu_add_submenu(title_menu, create_menu(290, 280, SCREEN_W/2-145, SCREEN_H/2-140, "game options", title_menu));
        menu_add_item(title_menu->submenu[1], create_menu_item(15, 35, MENUITEM_FROMLIST, "sound quality:", "select your sonic fidelity", 1));
        set_menu_item_reflist(title_menu->submenu[1]->menu_item[0], "11025kHz, mono", "11025kHz, stereo", "22050kHz, mono", "22050kHz, stereo", "44100kHz, mono", "44100kHz, stereo", "disable sound", "");
	set_menu_item_refs(title_menu->submenu[1]->menu_item[0], MENU_CHANGE_SOUND_QUALITY, 0, 0);
        menu_add_item(title_menu->submenu[1], create_menu_item(15, 85, MENUITEM_SLIDER, "music volume:", "loud enough for you?", 1));
	set_menu_item_refs(title_menu->submenu[1]->menu_item[1], MENU_CHANGE_SOUND_MUSIC_VOLUME, game_config.sound_music_volume, 100);
        menu_add_item(title_menu->submenu[1], create_menu_item(15, 120, MENUITEM_SLIDER, "sfx volume:", "boom, Boom, or BOOM?", 1));
	set_menu_item_refs(title_menu->submenu[1]->menu_item[2], MENU_CHANGE_SOUND_SFX_VOLUME, game_config.sound_sfx_volume, 100);
        menu_add_item(title_menu->submenu[1], create_menu_item(15, 155, MENUITEM_FROMLIST, "fx: shadows", "enable/disable shadow effect", 1));
        set_menu_item_reflist(title_menu->submenu[1]->menu_item[3], "on", "off", "");
	set_menu_item_refs(title_menu->submenu[1]->menu_item[3], MENU_CHANGE_FX_SHADOWS, !game_config.fx_on, 0);
        menu_add_item(title_menu->submenu[1], create_menu_item(15, 205, MENUITEM_FROMLIST, "reset HUDs?", "snap HUDs back after swing", 1));
        set_menu_item_reflist(title_menu->submenu[1]->menu_item[4], "yes", "no", "");
	set_menu_item_refs(title_menu->submenu[1]->menu_item[4], MENU_CHANGE_RESET_HUDS, !game_config.reset_huds, 0);

	//the player key configuration main menu
        menu_add_submenu(title_menu->submenu[0], create_menu(280, 280, SCREEN_W/2-140, SCREEN_H/2-140, "key configuration", title_menu->submenu[0]));
        menu_add_item(title_menu->submenu[0]->submenu[0], create_menu_item(25, 35, MENUITEM_FROMLIST, "player 1", "player 1 configuration", 1));
        menu_add_item(title_menu->submenu[0]->submenu[0], create_menu_item(25, 75, MENUITEM_FROMLIST, "player 2", "player 2 configuration", 1));
        menu_add_item(title_menu->submenu[0]->submenu[0], create_menu_item(25, 115, MENUITEM_FROMLIST, "player 3", "player 3 configuration", 1));
        menu_add_item(title_menu->submenu[0]->submenu[0], create_menu_item(25, 155, MENUITEM_FROMLIST, "player 4", "player 4 configuration", 1));
	if(num_joysticks == 1){
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[0], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[1], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[2], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[3], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Custom", "");
		}
	else if(num_joysticks == 2){
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[0], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[1], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[2], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[3], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Custom", "");
		}
	else if(num_joysticks == 3){
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[0], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Joystick 3", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[1], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Joystick 3", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[2], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Joystick 3", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[3], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Joystick 3", "Custom", "");
		}
	else if(num_joysticks == 4){
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[0], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Joystick 3", "Joystick 4", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[1], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Joystick 3", "Joystick 4", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[2], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Joystick 3", "Joystick 4", "Custom", "");
	        set_menu_item_reflist(title_menu->submenu[0]->submenu[0]->menu_item[3], "KBD: left, right, Z, F1", "KBD: J, L, U, F1", "KBD: del, pgdn, pgup, F1", "KBD: B, M, H, F1", "Mouse", "Joystick 1", "Joystick 2", "Joystick 3", "Joystick 4", "Custom", "");
		}
	for(i = 0; i < MAX_PLAYERS; i++){
		set_menu_item_refs(title_menu->submenu[0]->submenu[0]->menu_item[i], MENU_DEFINE_KEYS_PLAYER_1-i, i, 0);
		}

	//this menu lets players set a custom keyboard configuration
        menu_add_submenu(title_menu->submenu[0]->submenu[0], create_menu(270, 150, SCREEN_W/2-135, SCREEN_H/2-75, "set your keys", title_menu->submenu[0]->submenu[0]));
        menu_add_item(title_menu->submenu[0]->submenu[0]->submenu[0], create_menu_item(20, 30, MENUITEM_KEYDEFINE, "rotate left:", "press enter to change key", 1));
        set_menu_item_refs(title_menu->submenu[0]->submenu[0]->submenu[0]->menu_item[0], MENU_CHANGE_KEYS, game_player[0].controller.key_left, 0);
        menu_add_item(title_menu->submenu[0]->submenu[0]->submenu[0], create_menu_item(20, 50, MENUITEM_KEYDEFINE, "rotate right:", "press enter to change key", 1));
	set_menu_item_refs(title_menu->submenu[0]->submenu[0]->submenu[0]->menu_item[1], MENU_CHANGE_KEYS, game_player[0].controller.key_right, 0);
        menu_add_item(title_menu->submenu[0]->submenu[0]->submenu[0], create_menu_item(20, 70, MENUITEM_KEYDEFINE, "swing club:", "press enter to change key", 1));
	set_menu_item_refs(title_menu->submenu[0]->submenu[0]->submenu[0]->menu_item[2], MENU_CHANGE_KEYS, game_player[0].controller.key_swing_pri, 0);
        menu_add_item(title_menu->submenu[0]->submenu[0]->submenu[0], create_menu_item(20, 90, MENUITEM_KEYDEFINE, "hide huds:", "press enter to change key", 1));
	set_menu_item_refs(title_menu->submenu[0]->submenu[0]->submenu[0]->menu_item[3], MENU_CHANGE_KEYS, game_player[0].controller.key_swing_alt, 0);
				
	//the player selection/setup screen
        menu_add_submenu(title_menu->submenu[0], create_menu(560, 400, SCREEN_W/2-280, SCREEN_H/2-200, "player setup menu", title_menu->submenu[0]));
        //player 1
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(20, 35, MENUITEM_CHARNAME, "name:", "enter your name, player 1", 1));
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(40, 55, MENUITEM_CHARNAME, "Player 1", "enter your name, player 1", 0));
	set_menu_item_refs(title_menu->submenu[0]->submenu[1]->menu_item[0], MENU_CHANGE_PLAYER_1_NAME, 0, 0);
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(20, 75, MENUITEM_CHARPIC_SELECT, "picture:", "choose your avatar", 1));
	set_menu_item_refs(title_menu->submenu[0]->submenu[1]->menu_item[2], MENU_CHANGE_PLAYER_1_PIC, 0, main_game_vars.num_player_images);
        //player 2
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(20, 120, MENUITEM_CHARNAME, "name:", "enter your name, player 2", 1));
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(40, 140, MENUITEM_CHARNAME, "Player 2", "enter your name, player 1", 0));
	set_menu_item_refs(title_menu->submenu[0]->submenu[1]->menu_item[3], MENU_CHANGE_PLAYER_2_NAME, 0, 0);
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(20, 160, MENUITEM_CHARPIC_SELECT, "picture:", "choose your avatar", 1));
	set_menu_item_refs(title_menu->submenu[0]->submenu[1]->menu_item[5], MENU_CHANGE_PLAYER_2_PIC, 0, main_game_vars.num_player_images);
        //player 3
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(20, 205, MENUITEM_CHARNAME, "name:", "enter your name, player 3", 1));
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(40, 225, MENUITEM_CHARNAME, "Player 3", "enter your name, player 1", 0));
	set_menu_item_refs(title_menu->submenu[0]->submenu[1]->menu_item[6], MENU_CHANGE_PLAYER_3_NAME, 0, 0);
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(20, 245, MENUITEM_CHARPIC_SELECT, "picture:", "choose your avatar", 1));
	set_menu_item_refs(title_menu->submenu[0]->submenu[1]->menu_item[8], MENU_CHANGE_PLAYER_3_PIC, 0, main_game_vars.num_player_images);
        //player 4
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(20, 290, MENUITEM_CHARNAME, "name:", "enter your name, player 4", 1));
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(40, 310, MENUITEM_CHARNAME, "Player 4", "enter your name, player 1", 0));
	set_menu_item_refs(title_menu->submenu[0]->submenu[1]->menu_item[9], MENU_CHANGE_PLAYER_4_NAME, 0, 0);
        menu_add_item(title_menu->submenu[0]->submenu[1], create_menu_item(20, 330, MENUITEM_CHARPIC_SELECT, "picture:", "choose your avatar", 1));
	set_menu_item_refs(title_menu->submenu[0]->submenu[1]->menu_item[11], MENU_CHANGE_PLAYER_4_PIC, 0, main_game_vars.num_player_images);
        
	//the course selection/setup screen
        menu_add_submenu(title_menu->submenu[0], create_menu(630, 470, SCREEN_W/2-315, SCREEN_H/2-235, "course selection", title_menu->submenu[0]));
        for(i = 0; i < main_game_vars.f_index; i++){
	        menu_add_item(title_menu->submenu[0]->submenu[2], create_menu_item(25, 55+(20)*(i-1), MENUITEM_LOAD_COURSE, main_game_vars.f_packfile[i], main_game_vars.f_packfile[i], 1));
	        set_menu_item_refs(title_menu->submenu[0]->submenu[2]->menu_item[i], MENU_LOAD_COURSE, 0, 0);
        	}
      
        //the "quit game" menu
        menu_add_submenu(title_menu, create_menu(200, 100, SCREEN_W/2-100, SCREEN_H/2-50, "really quit?", title_menu));
        menu_add_item(title_menu->submenu[2], create_menu_item(20, 25, MENUITEM_QUITFE, "yes", "see ya", 1));
        menu_add_item(title_menu->submenu[2], create_menu_item(20, 60, MENUITEM_GOTO_ROOTMENU, "no", "get back to golfing", 1));
	

        //cur_menu is a pointer to the title menu pointer (should contain the address of title_menu)	
        cur_menu = &title_menu;

        //set up the menu cursor
	main_menu_cursor.x = 0;
        main_menu_cursor.y = 0;
        main_menu_cursor.active = 1;
        main_menu_cursor.offset_counter = 0;
        main_menu_cursor.color = 15;
        main_menu_cursor.radius = 10;
        main_menu_cursor.cur_menu = -1;
        main_menu_cursor.cur_submenu = 0;
}

//the initialization routines for the game menus..
void deinitialize_game_menus(void){
        if(title_menu != NULL) destroy_menu(title_menu);
        //if(cur_menu != NULL) destroy_menu(cur_menu);
}

//give the course some starter values that show it's empty..
void preinitialize_course(COURSE * cr){
	strcpy(cr->name, "");
}

//reset this player's scorecard
void reset_player_scorecard(SCORECARD * sc, PLAYER * pl){
        int i;

        for(i = 0; i < 18; i++) sc->score[i] = sc->par_score[i] = sc->match_won[i] = 0;
        sc->num_birdies = sc->num_eagles = sc->num_albo = 0;
        sc->matches_won = 0;

        strcpy(sc->name, pl->name);
        sc->pic_ref = pl->pic_ref;
        sc->coin_score = 0;
        sc->coin_best_combo = 0;
}

//reset the trajectory vertex list
void reset_trajectory_points(VERTEX t[], int num_points){
	int i;
	
	for(i = 0; i < num_points; i++){
		t[i].x = t[i].y = 0;
		}
}

//do the first initialization of the player
void preinitialize_player(PLAYER * pl, int plno){
        int c_type = pl->controller.type;

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
        if(plno == 0){
                pl->hud.x = 5;
                pl->hud.y = 10;
                }
        //player 2 is on the bottom left
        else if(plno == 1){
                pl->hud.x = 5;
                pl->hud.y = 440;
                }
        //player 3 is on the top right
	else if(plno == 2){
                pl->hud.x = 435;
                pl->hud.y = 10;
                }
        //player 4 is on the bottom right
        else if(plno == 3){
                pl->hud.x = 435;
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
}

//initialize all players before game
void preinitialize_players(PLAYER pl[], int num_players){
        int i;

        for(i = 0; i < num_players; i++){
                preinitialize_player(&pl[i], i);
                }
        main_game_vars.cur_player = 0;
}

//give the hole some predefined values
void preinitialize_hole(HOLE * hole){
        hole->s.x = SCREEN_W/2;
        hole->s.y = SCREEN_H/2;
        hole->deccel_factor = 0.05f;
}

//free memory related to this scorecard
void deinitialize_scorecard(SCORECARD * sc){
	if(sc->name != NULL) free(sc->name);
}

//free memory related to this statsheet
void deinitialize_statsheet(STATSHEET * st){
	int i;
	
	if(st->name != NULL) free(st->name);
	for(i = 0; i < MAX_PLAYER_COURSES_COMPLETED; i++){
		if(st->course_name[i] != NULL) free(st->course_name[i]);
		}
	for(i = 0; i < 3; i++){
		if(st->course_top_name[i] != NULL) free(st->course_top_name[i]);
		}
	for(i = 0; i < 3; i++){
		deinitialize_scorecard(&st->top_scorecard[i]);
		}
}

//destroy images in the main image bank
void destroy_main_image_bank(MAIN_IMAGE_BANK * ibank){
	int i, j, num_anims = 4;
	
	for(i = 0; i < MAX_GAME_CHARS; i++){
		for(j = 0; j < num_anims; j++) trash_bitmap(ibank->charpic[i][j]);
		}
	trash_bitmap(ibank->statbarpic);
}

//destroy course images
void destroy_course_image_bank(COURSE_IMAGE_BANK * ibank){
	int i, j;
	
	for(i = 0; i < MAX_COURSE_VERT_PIC_TYPES; i++){
		for(j = 0; j < MAX_VERT_ANIMS; j++){
			trash_bitmap(ibank->vert_pic[i][j]);
			}
		}
	for(i = 0; i < MAX_COURSE_ANIMAL_TYPES; i++){
		for(j = 0; j < MAX_ANIMAL_ANIMS; j++){
			trash_bitmap(ibank->animal_pic[i][j]);
			}
		}
	for(i = 0; i < MAX_COURSE_OBJECT_TYPES; i++){
		for(j = 0; j < MAX_OBJECT_ANIMS; j++){
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
void destroy_course_scoreboard(SCOREBOARD * sb){
	int i;
	
	trash_bitmap(sb->bgpic);
        trash_bitmap(sb->gridpic);
        for(i = 0; i < 19; i++) trash_bitmap(sb->numpic[i]);
        
}

//deinitialize the current hole
void deinitialize_hole(HOLE * hole){
	int i;
	
	for(i = 0; i < MAX_HOLE_PIC_LAYERS; i++){
		trash_bitmap(hole->layer_pic[i]);
		}
	if(hole->name != NULL) free(hole->name);
}
	
//deinitialize the game course
void deinitialize_course(COURSE * cr){
	int i;
	
	destroy_course_image_bank(&cr->image_bank);
	destroy_course_scoreboard(&cr->scoreboard);
	deinitialize_hole(&cr->current_hole);
	for(i = 0; i < MAX_COURSE_TOPSCORES; i++){
		deinitialize_scorecard(&cr->top_scorecard[i]);
		deinitialize_scorecard(&cr->coin_top_scorecard[i]);
		}
	
	if(cr->name != NULL) free(cr->name);
	if(cr->fname != NULL) free(cr->fname);
	if(cr->fname_prefix != NULL) free(cr->fname_prefix);
}

//destroy all the sound effect samples
void destroy_sample_bank(SOUND_BANK * sbank){
        int i,j;

        for(i = 0; i < MAX_SFX_SWING_SAMPLES; i++) destroy_sample(sbank->sfx_swing[i]);
        for(i = 0; i < MAX_SFX_COLLISION_SAMPLES; i++) destroy_sample(sbank->sfx_ball_collision[i]);
        destroy_sample(sbank->sfx_ball_holesink);
        for(i = 0; i < 3; i++){
                for(j = 0; j < MAX_SFX_HOLE_FINISH_SAMPLES; j++){
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
void deinitialize_main_game_vars(void){
	int i;
	
	destroy_main_image_bank(&main_game_vars.image_bank);
	destroy_packfile_list();
	for(i = 0; i < MAX_PLAYER_SAVED_STATS; i++){
		deinitialize_statsheet(&main_game_vars.player_statsheet[i]);
		}
	destroy_sample_bank(&main_game_vars.sound_bank);
}

//unload all the ball's images from memory (if necessary)
void unload_ball_images(BALL * ball){
        int i,j;

        for(i = 0; i < MAX_BALL_ANIM_MODES; i++){
                for(j = 0; j < ball->anim_def[i].num_frames; j++){
                        trash_bitmap(ball->anim_def[i].pic[j]);
                        }
                }

        trash_bitmap(ball->shad_pic);
}

//deinitialize the player..
void deinitialize_player(PLAYER * pl, int plno){
        //trash all the ball's images from memory
        unload_ball_images(&pl->ball);
        if(pl->name != NULL) free(pl->name);
        deinitialize_scorecard(&pl->scorecard);
        deinitialize_statsheet(&pl->statsheet);
}

//deinitialize all players before game exit
void deinitialize_players(PLAYER pl[], int num_players){
        int i;

        for(i = 0; i < num_players; i++){
                deinitialize_player(&pl[i], i);
                }
}

void draw_poly2(BITMAP * bp, POLY po, VERTEX c, int col){
        int i;
        float ang;
        VERTEX n, m;

        for(i = 0; i < po.num_vertices-1; i++){
                ang = atan2(po.p[i].y-po.p[i+1].y, po.p[i].x-po.p[i+1].x)+M_PI/2;
                m = new_vertex((po.p[i].x+po.p[i+1].x)/2, (po.p[i].y+po.p[i+1].y)/2);
                n = new_vertex(10*cos(ang), 10*sin(ang));

                line(bp, c.x+po.p[i].x, c.y+po.p[i].y, c.x+po.p[i+1].x, c.y+po.p[i+1].y, col);
                line(bp, c.x+m.x, c.y+m.y, c.x+m.x+n.x, c.y+m.y+n.y, col+1);
                }
        line(bp, c.x+po.p[po.num_vertices-1].x, c.y+po.p[po.num_vertices-1].y, c.x+po.p[0].x, c.y+po.p[0].y, col);
}

//tells if a collision has occured between these two bodies
char check_body_collision(BODY * a, BODY * b){
        return body_collide(a, b);
}

//checks to see if the ball is within a rectangle that surrounds the line segment endpoints..
char circle_lineseg_rect_collision(VERTEX b_c, float radius, VERTEX v1, VERTEX v2, VERTEX c){
        int x1,y1,x2,y2;

        //sort the points
        if(v1.x < v2.x){
                x1 = v1.x;
                x2 = v2.x;
                }
        else{
                x2 = v1.x;
                x1 = v2.x;
                }
        if(v1.y < v2.y){
                y1 = v1.y;
                y2 = v2.y;
                }
        else{
                y2 = v1.y;
                y1 = v2.y;
                }

        //make sure the rectangle is of a minimum span
        if(x2-x1 < MIN_RECT_SPAN){
                x1 -= MIN_RECT_SPAN/2;
                x2 += MIN_RECT_SPAN/2;
                }
        if(y2-y1 < MIN_RECT_SPAN){
                y1 -= MIN_RECT_SPAN/2;
                y2 += MIN_RECT_SPAN/2;
                }
        return rect_collision_f(-radius, -radius, radius, radius, x1, y1, x2, y2, b_c, c);
}

//tells if this poly is empty
char poly_is_empty(POLY po){
	return (po.color == POLY_TYPE_EMPTY_WALL);
}

//tells if this poly is empty
char poly_is_laser(POLY po){
	return (po.color == POLY_TYPE_LASER_WALL);
}

//check for collisions between the ball and the hole's walls
void check_ball_hole_collision(BALL * ball, PLAYER * pl, HOLE * hole){
        int i,j;
        
        for(i = 0; i < hole->num_polys; i++){
                float r_off;
	        int count = 0;
	        
                for(r_off = 0; r_off < (ball->speed <= 0 ? 0.5 : ball->speed); r_off += 1.0){
	                VERTEX ball_off = new_vertex(r_off*cos(ball->f_angle), r_off*sin(ball->f_angle));
	                VERTEX ball_off_pos = vertex_add(ball->s, ball_off);
                	//if(ball->speed > 1) printf("%i: (%f, %f)->(%f, %f)..", count++, ball->s.x, ball->s.y, ball_off_pos.x, ball_off_pos.y);
	                for(j = 0; j < hole->body[i].poly.num_vertices-1; j++){
	                       	int num_sides;
    	                    	BODY seg_body, ball_offset;

	        	        //goto the next two points if the ball isn't within a rectangle defined by the current two points
    	        	        if(!circle_lineseg_rect_collision(ball_off_pos, ball->radius, hole->body[i].poly.p[j], hole->body[i].poly.p[j+1], hole->s)){
        	        	        continue;
            	        	        }

	            	        //create a new segment body based on these two endpoints..
    	            	        seg_body = new_seg_body(hole->body[i].poly.p[j], hole->body[i].poly.p[j+1], hole->s, &num_sides);
                
				//offset the ball's body by the offset so we can check to see if it will hit a wall segment before its next velocity calculation
    	            	        ball_offset = body_offset(ball->body, ball_off);
				
                    	        //check for a collision between the ball body and this segment body..
            	            	//if(check_body_collision(&ball->body, &seg_body)){
            	            	if(check_body_collision(&ball_offset, &seg_body)){
	            	            	if(poly_is_laser(hole->body[i].poly)){
            	            	                if(!pl->traj_active){
	            	            	                do_ball_laser_zap(ball, pl, hole);
            	            	                	}
            	            	                return;
	            	            		}
					else if(poly_is_empty(hole->body[i].poly)){
                				if(!pl->traj_active && ball->speed <= 0 && ball->mode != BALL_MODE_ON_CONVEYOR){
	                				ball->mode = BALL_MODE_IN_MOTION_AREA;
	                				return;
                					}
                				else continue;
                				}
	       				else{
		       				//play a random sound when the ball hits a solid wall (don't do this for the trajectory stuff)
	       			      	    	if(!pl->traj_active){
	       	       					play_ball_wall_hit_sample();
       	       						}
		       				
                				if(ball->speed <= 0 && ball->mode == BALL_MODE_IN_MOTION_AREA){
	                				ball->speed += 1.0;   //nudge the ball a little bit if it's within a motion area and stopped when the poly hits it
	                				ball->idle_timer = 0; //reset idle timer
                					}
                				
                				}
	            	            	ball->s_med = ball_offset.pos;
	            	            	ball->mirage_visible = 1;
                	         	ball->body = ball_offset;
	            	            	ball->v = ball->body.vel;
                    	            	ball->f_angle = atan2(ball->v.y, ball->v.x);
                        	        ball->v.x = ball->speed*cos(ball->f_angle);
                            	    	ball->v.y = ball->speed*sin(ball->f_angle);
                            	    	
                            	    	//do trajectory calculations if traj is currently active
                            	    	if(pl->traj_active){
	                            	    	pl->traj_v[pl->traj_count] = ball->s_med;
	                            	    	if(++pl->traj_count >= MAX_TRAJECTORY_POINTS){
		                            	    	ball->speed = 0;
		                            	    	pl->traj_calc_done = 1;
	                            	    		}
                            	    		}
	                                return;
                            	    	}	
				}
			}    	      
                }
}

//load ball images
int load_ball_images(BALL * ball){
        //splash animation..
        ball->anim_def[1].speed = 4;
        ball->anim_def[1].num_frames = 4;
        ball->anim_def[1].cur_frame = 0;

        ball->anim_def[1].pic[0] = load_pcx(base_filename("splash00_pcx"), NULL);
        ball->anim_def[1].pic[1] = load_pcx(base_filename("splash01_pcx"), NULL);
        ball->anim_def[1].pic[2] = load_pcx(base_filename("splash02_pcx"), NULL);
        ball->anim_def[1].pic[3] = load_pcx(base_filename("splash03_pcx"), NULL);
        ball->anim_def[1].pic[4] = load_pcx(base_filename("splash04_pcx"), NULL);

        //ball shadow
        if(ball->shad_pic == NULL){
                ball->shad_pic = create_bitmap(ball->radius*2, ball->radius*2);
                clear_to_color(ball->shad_pic, 0);
                circlefill(ball->shad_pic, ball->radius, ball->radius, ball->radius, 5);
                }

        return 0;
}

//initialize the player before the next hole
void hole_initialize_player(PLAYER * pl, int plno, int cur_hole){
        pl->ball.s.x = pl->ball.s.y = -100;
        pl->ball.v.x = pl->ball.v.y = 0;
        pl->ball.f.x = pl->ball.f.y = 0;
        pl->ball.s_old = pl->ball.s;
        pl->ball.speed = 0;
        pl->ball.visible = 0;
        pl->ball.cur_vert = 0;
        pl->ball.idle_timer = 0;
        pl->ball.idle_timeout = 75;
        pl->ball.radius = BALL_STD_RADIUS;

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

        if(plno == 0){
                pl->hud.x = 5;
                pl->hud.y = 10;
                }
        else if(plno == 1){
                pl->hud.x = 5;
                pl->hud.y = 440;
                }
        else if(plno == 2){
                pl->hud.x = 435;
                pl->hud.y = 10;
                }
        else if(plno == 3){
                pl->hud.x = 435;
                pl->hud.y = 440;
                }
	//reset trajectory point list
        reset_trajectory_points(pl->traj_v, MAX_TRAJECTORY_POINTS);        
}

//initialize all players before hole
void hole_initialize_players(PLAYER pl[], int num_players, int cur_hole){
        int i;

        main_game_vars.player_huds_hiding = main_game_vars.player_huds_hidden = 0;
        for(i = 0; i < num_players; i++){
                hole_initialize_player(&pl[i], i, cur_hole);
                }
}

//hit the ball
void do_player_hit_ball(PLAYER * pl){
        pl->ball.s_old = pl->ball.s;
        pl->ball.angle = pl->ball.f_angle = pl->f_club_angle;
        pl->ball.mode = BALL_MODE_HIT;
        pl->ball.speed = pl->club_power/8.0f;
        pl->ball.v.x = pl->ball.speed*cos(pl->f_club_angle);
        pl->ball.v.y = pl->ball.speed*sin(pl->f_club_angle);
        pl->ball.body.vel = pl->ball.v;
        pl->cur_strokes++;
        if(!main_game_vars.player_huds_hiding && main_game_vars.player_huds_hidden && game_config.reset_huds) main_game_vars.player_huds_hiding = 1;
}

//swing the club.. (two keypresses required for ball hit)
void do_player_club_swing(PLAYER * pl){
        if(pl->club_swing == 1){
                if(pl->club_power < MAX_CLUB_POWER) pl->club_power++;
                else pl->club_swing = 2;
                }
        if(pl->club_swing == 2){
                if(pl->club_power > 0) pl->club_power--;
                else pl->club_swing = 0;
                }

        pl->club_v[0].x = -pl->club_power/2*cos(pl->f_club_angle+pl->f_club_width);
        pl->club_v[0].y = -pl->club_power/2*sin(pl->f_club_angle+pl->f_club_width);
        pl->club_v[1].x = -pl->club_power/2*cos(pl->f_club_angle-pl->f_club_width);
        pl->club_v[1].y = -pl->club_power/2*sin(pl->f_club_angle-pl->f_club_width);

        if(controller_key_swing_released(pl->controller)){
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
                //clear the controller/key buffer
                controller_clear_keybuf(pl->controller, 0);
                }
}

//hide the player huds to get a clear view at the playing area, or bring them back
void hide_player_huds(PLAYER pl[], int num_players, char hidden){
        int p;

        //move player huds out of the way..
        if(!hidden){
                for(p = 0; p < num_players; p++){
                        if(!p || p == 2){
                                if(pl[p].hud.y > -55) pl[p].hud.y -= 4;
                                else{
                                        main_game_vars.player_huds_hidden = 1;
                                        main_game_vars.player_huds_hiding = 0;
                                        }
                                }
                        else{
                                if(pl[p].hud.y < SCREEN_H+15) pl[p].hud.y += 4;
                                else{
                                        main_game_vars.player_huds_hidden = 1;
                                        main_game_vars.player_huds_hiding = 0;
                                        }
                                }
                        }
                }
        //move player huds back to visibility
        else{
                for(p = 0; p < num_players; p++){
                        if(!p || p == 2){
                                if(pl[p].hud.y < 10) pl[p].hud.y += 4;
                                else{
                                        pl[p].hud.y = 10;
                                        main_game_vars.player_huds_hidden = 0;
                                        main_game_vars.player_huds_hiding = 0;
                                        }
                                }
                        else{
                                if(pl[p].hud.y > 440) pl[p].hud.y -= 4;
                                else{
                                        pl[p].hud.y = 440;
                                        main_game_vars.player_huds_hidden = 0;
                                        main_game_vars.player_huds_hiding = 0;
                                        }
                                }
                        }
                }
}

//get key input when the quit prompt is active
void get_quit_prompt_keys(void){
	if(key[KEY_ESC]){       //exit from the prompt and return to the game
		play_esc_sample();
                main_game_vars.game_state = main_game_vars.game_state_old = GAMESTATE_READY_TOPLAY;
                deactivate_vfont_message(&game_vmessage[0]);
		
		key[KEY_ESC] = 0;
		}
	else if(key[KEY_Y]){
             	main_game_vars.quit_game = 1;
             	main_game_vars.quit_title = 0;
             	deactivate_vfont_message(&game_vmessage[0]);
             	
             	key[KEY_Y] = 0;
             	}
	else if(key[KEY_N]){
             	main_game_vars.game_state = main_game_vars.game_state_old = GAMESTATE_READY_TOPLAY;
             	deactivate_vfont_message(&game_vmessage[0]);
             	
             	key[KEY_N] = 0;
                }
}

//get controller input from exiting, pausing, etc.)
void get_main_keys(void){
        if(key[KEY_ESC]){
	        play_esc_sample();
                if(!main_game_vars.cur_vmessage_slot){
	                add_quit_prompt_message();
                        main_game_vars.game_state_old = main_game_vars.game_state;  //save the previous game state
                        main_game_vars.game_state = GAMESTATE_QUIT_PROMPT;          //current game state is the quit prompt
                        }
                key[KEY_ESC] = 0;
                }
	else if(key[KEY_TILDE]){  //toggle debug mode
		main_game_vars.debug_on = !main_game_vars.debug_on;
		key[KEY_TILDE] = 0;
		}
}

//handle the logic of the animal verts
void handle_animal_vert(VERT * vert, HOLE * hole, int t){
        int i;

        vert->s.x += vert->ref_no[1]*ANIMAL_MOVE_FACTOR;
        vert->s.y += vert->ref_no[2]*ANIMAL_MOVE_FACTOR;

        //keep animal within original radius..
        if(distance(v_offset(hole->s, vert->s).x, v_offset(hole->s, vert->s).y, vert->ref_no[4], vert->ref_no[5]) > vert->radius){
                vert->s.x -= vert->ref_no[1]*ANIMAL_MOVE_FACTOR;
                vert->s.y -= vert->ref_no[2]*ANIMAL_MOVE_FACTOR;
                }

        //animate animal..
        vert->ref_no[3]++;   //+= timer_change
        if(!(vert->ref_no[3] % ANIMAL_ANIM_TIME)){
                if(vert->ref_no[1] > 0) vert->ref_no[0] = 0;
                else vert->ref_no[0] = 2;
                }
        if(!(vert->ref_no[3] % (ANIMAL_ANIM_TIME*2))){
                if(vert->ref_no[1] > 0) vert->ref_no[0] = 1;
                else vert->ref_no[0] = 3;
                }

        //give animal random movement vector..
        if(vert->ref_no[3] >= (ANIMAL_MOVE_TIME-50)+random()%(ANIMAL_MOVE_TIME)){
                vert->ref_no[1] = random() % 3;
                i = random()%2;
                if(i) vert->ref_no[1] *= -1;

                vert->ref_no[2] = random() % 3;
                i = random()%2;
                if(i) vert->ref_no[2] *= -1;

                vert->ref_no[3] = 0;
                }
}

//handle the various vert actions, depending on the vert's flag
void handle_vert_action(VERT * vert, HOLE * hole, int t){
        int i;

        switch(vert->flag){
                case VERT_FLAG_ANIMAL_0:
                        handle_animal_vert(vert, hole, t);
                        break;
                case VERT_FLAG_ANIMAL_1:
                        handle_animal_vert(vert, hole, t);
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
                        calc_body_normals(&hole->body[vert->ref_no[1]].poly);
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
		case VERT_FLAG_LASER_SEND:  //this vert fires a laser to an end-vert
	                if(!vert->ref_no[0]){  //laser inactive, run timer to delay ceiling
        	                if(vert->ref_no[1] < vert->ref_no[2]*2){  //keep counting while the laser is being fired (multiply by two to slow it down a little)
                	                vert->ref_no[1] += t;
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
                        	        vert->ref_no[1] += t;   //keep counting while laser is being fired (multiply by two to slow it down a bit)
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
                        	if(vert->ref_no[4] < vert->ref_no[2]*4-1) vert->ref_no[4] += t;
                        	else vert->ref_no[4] = 0;
                        	}
                	break;
                default:
                        break;
                }
/*
                }
        else if(vert->flag == VERT_FLAG_COIN_0){  //this vert represents a coin (for coin collector mode)
                if(vert->ref_no[0] > 0){
                        if(vert->ref_no[1] < vert->ref_no[2]) vert->ref_no[1] += timer_change;
                        else{
                                if(vert->ref_no[3] < MAX_COIN_PICS*2-1){
                                        vert->ref_no[3] += timer_change;
                                        }
                                else{
                                        vert->ref_no[1] = vert->ref_no[3] = 0;
                                        vert->ref_no[2] = (random() % 200)+100;
                                        }
                                }
                        }
                else{
                        if(vert->ref_no[4] >= 0 && vert->ref_no[4] < 15) vert->ref_no[4] += timer_change;
                        else{
                                vert->ref_no[4] = -1;
                                }
                        }
                }
*/
}

//vert logic..
void vert_logic(VERT * vert, HOLE * hole){
        handle_vert_action(vert, hole, main_game_vars.ingame_timer);
}

//hole logic..
void hole_logic(HOLE * hole){
        int v;

        for(v = 0; v < hole->num_verts+hole->num_coins*(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT); v++){
                vert_logic(&hole->vert[v], hole);
                }
}

//get main in-game input (for exlayer whose ball is furthest from the hole
int furthest_player_from_hole(PLAYER pl[], int num_players, HOLE * hole){
        int p;
        int hole_vert = 0;
        int nearest = -1;

        for(p = 0; p < num_players; p++){
                if(pl[p].ball.mode != BALL_MODE_SUNK){
                        nearest = 0;
                        break;
                        }
                }
        if(nearest == -1) return nearest;

        for(p = 0; p < hole->num_verts; p++){
                if(hole->vert[p].flag == VERT_FLAG_HOLE){
                        hole_vert = p;
                        break;
                        }
                }
        for(p = 1; p < num_players; p++){
                if(v_distance(pl[p].ball.s, v_offset(hole->s, hole->vert[hole_vert].s)) > v_distance(pl[nearest].ball.s, v_offset(hole->s, hole->vert[hole_vert].s)) && pl[p].ball.mode != BALL_MODE_SUNK){
                        nearest = p;
                        }
                }
        return nearest;
}

//finish the current player's turn, go to the next player (if there are others)
void do_player_turn_finish(PLAYER * pl, HOLE * hole){
/*	
        if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
                if(game_player[main_game_vars.cur_player].coins_total == hole->num_coins){
                        game_player[main_game_vars.cur_player].ball.mode = BALL_MODE_COIN_FINISHED;
                        main_game_vars.game_state = GAMESTATE_COIN_COLLECT_FINISHED;
                        if(game_player[main_game_vars.cur_player].coin_best_combo > game_player[main_game_vars.cur_player].scorecard.coin_best_combo){
                                game_player[main_game_vars.cur_player].scorecard.coin_best_combo = game_player[main_game_vars.cur_player].coin_best_combo;
                                }
                        //add coins collected in this hole to the round total
                        game_player[main_game_vars.cur_player].coins_overall += game_player[main_game_vars.cur_player].coins_total;
                        game_player[main_game_vars.cur_player].scorecard.coin_score += COIN_COMPLETE_BONUS;
                        game_player[main_game_vars.cur_player].scorecard.coin_score += COIN_COMPLETE_BONUS/game_player[main_game_vars.cur_player].cur_strokes;
                        game_player[main_game_vars.cur_player].scorecard.score[main_game_vars.cur_hole] = game_player[main_game_vars.cur_player].coins_total;
                        game_player[main_game_vars.cur_player].scorecard.par_score[main_game_vars.cur_hole] = hole->num_coins;
                        game_player[main_game_vars.cur_player].scorecard.match_won[main_game_vars.cur_hole] = 1;
                        }
                else if(game_player[main_game_vars.cur_player].cur_strokes > 10){
                        game_player[main_game_vars.cur_player].ball.mode = BALL_MODE_COIN_FINISHED;
                        main_game_vars.game_state = GAMESTATE_COIN_COLLECT_FAILED_STROKES;
                        if(game_player[main_game_vars.cur_player].coin_best_combo > game_player[main_game_vars.cur_player].scorecard.coin_best_combo){
                                game_player[main_game_vars.cur_player].scorecard.coin_best_combo = game_player[main_game_vars.cur_player].coin_best_combo;
                                }
                        //add coins collected in this hole to the round total
                        game_player[main_game_vars.cur_player].coins_overall += game_player[main_game_vars.cur_player].coins_total;
                        game_player[main_game_vars.cur_player].scorecard.score[main_game_vars.cur_hole] = game_player[main_game_vars.cur_player].coins_total;
                        game_player[main_game_vars.cur_player].scorecard.par_score[main_game_vars.cur_hole] = hole->num_coins;
                        game_player[main_game_vars.cur_player].scorecard.match_won[main_game_vars.cur_hole] = 0;
                        }
                }
*/                
        if(main_game_vars.num_players > 1){
                if(main_game_vars.cur_vmessage_slot == 0){
                        int p_no = furthest_player_from_hole(game_player, main_game_vars.num_players, hole);

                        if(p_no != -1) main_game_vars.cur_player = p_no;
                        }
                }
}

//either seg to next hole, or let the next player in line take their turn
void do_player_hole_finish(PLAYER * pl, HOLE * hole){
        if(main_game_vars.num_players == 1){
                if(!main_game_vars.cur_vmessage_slot){
                        //begin seg to scoreboard
                        main_game_vars.game_state = GAMESTATE_HOLE_FINISHED;
                        main_game_vars.hole_finished = 1;
                        pl->pic_mode = 0;
                        }
                }
        else{
                if(main_game_vars.cur_vmessage_slot == 0){
                        int p = furthest_player_from_hole(game_player, main_game_vars.num_players, hole);
                
                        if(p != -1){
                                int v, vert_ref;

                                //find the vert for the hole
                                for(v = 0; v < hole->num_verts; v++){
                                        if(hole->vert[v].flag == VERT_FLAG_HOLE){
                                                vert_ref = v;
                                                break;
                                                }
                                        }
                                hole->vert[vert_ref].ref_no[0] = 0;  //empty the hole out for the next player..
                                main_game_vars.cur_player = p;
                                }
                        else{
                                //begin seg to scoreboard
                                //set current player to winner of previous hole..
                                main_game_vars.game_state = GAMESTATE_HOLE_FINISHED;
                                main_game_vars.hole_finished = 1;
                                }
                        pl->pic_mode = 0;
                        }
                }
}

//move the ball around
void move_player_ball(BALL * ball, HOLE * hole){
        ball->s.x += ball->v.x;
        ball->s.y += ball->v.y;
        ball->v.x = ball->speed*cos(ball->f_angle) + ball->f.x;
        ball->v.y = ball->speed*sin(ball->f_angle) + ball->f.y;
        ball->body.pos = ball->s;
        ball->body.vel = ball->v;
}

//slow the ball down
//TODO: might need some work..
void decell_ball(BALL * ball, PLAYER * pl, HOLE * hole){
	//keep slowing the ball down until its speed is 0
        if(ball->speed > 0){
                ball->speed -= hole->deccel_factor;
                }
        else{
	        //set speed to 0
                ball->speed = 0;

                //reset ball's force vector
                ball->f.x = ball->f.y = 0;
                                
                //get ball ready for another hit, finish this player's turn
                if(ball->mode != BALL_MODE_SUNK && ball->mode != BALL_MODE_WATER_SINKING && ball->mode != BALL_MODE_IN_MOTION_AREA && ball->mode != BALL_MODE_LASER_ZAPPED && ball->mode != BALL_MODE_ON_CONVEYOR){
                        ball->mode = BALL_MODE_IDLE;
                        ball->idle_timer = 0;
                        do_player_turn_finish(pl, hole);
                        }
                //ball is currently within a motion area, do the idle timer
                else if(ball->mode == BALL_MODE_IN_MOTION_AREA){
                        //if(ball->idle_timer < ball->idle_timeout) ball->idle_timer += main_game_vars.ingame_timer_change;
                        //make sure ball is within the motion area before handling the idle timer
                        if(check_circle_collision(ball->s, v_offset(hole->s, hole->vert[ball->cur_vert].s), ball->radius, hole->vert[ball->cur_vert].radius)){
	                        //handle the idle timer
                        	if(++ball->idle_timer >= ball->idle_timeout){
                        		ball->idle_timer = 0;
                                	ball->mode = BALL_MODE_IDLE;
                                	do_player_turn_finish(pl, hole);
                                	}
                        	}
                        else{
				ball->idle_timer = 0;
                                ball->mode = BALL_MODE_IDLE;
                                do_player_turn_finish(pl, hole);
                        	}
                        }
                //ball is on a conveyor belt..
                else if(ball->mode == BALL_MODE_ON_CONVEYOR){
                        if(!check_circle_collision(ball->s, v_offset(hole->s, hole->vert[ball->cur_vert].s), ball->radius, hole->vert[ball->cur_vert].radius)){
                                ball->mode = BALL_MODE_IDLE;
                                do_player_turn_finish(pl, hole);
                                }
                        else ball->mode = BALL_MODE_HIT;
                        }
                else{
                        ball->idle_timer = 0;
                        }
                //reset swing coin counter
                pl->coins_current = 0;
                }
}

//hit the ball from the previous hit position
void do_ball_rehit(BALL * ball, HOLE * hole){
        ball->s = ball->s_old;        //place the ball at the last position it was hit from
        ball->visible = 1;            //make ball visible
        ball->mode = BALL_MODE_IDLE;  //reset ball mode
        ball->idle_timer = 0;         //reset idle timer if needed
}

//drop the ball to the edge of the offending vert (water, ob, etc)
//TODO: make ball drop near the closest edge..
void do_ball_drop(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole){
	int ang = abs(64-fixtoi(fatan2(ftofix((hole->s.x+vert->s.x)-ball->s.x), ftofix((hole->s.y+vert->s.y)-ball->s.y)))+256*(ball->s.x < vert->s.x+hole->s.x && ball->s.y > vert->s.y+hole->s.y));
	//float ang = mod_angle(atan2(ball->v.y, ball->v.x));
	//float ang = atan2((hole->s.x+vert->s.x)-ball->s.x, (hole->s.y+vert->s.y)-ball->s.y);
        //int ang = abs((ball->s.x > vert->s.x+hole->s.x || ball->s.y < vert->s.y+hole->s.y)*64-fixtoi(fatan2(ftofix((hole->s.x+vert->s.x)-ball->s.x), ftofix((hole->s.y+vert->s.y)-ball->s.y)))-64*(ball->s.x < vert->s.x+hole->s.x && ball->s.y > vert->s.y+hole->s.y));
        //int ang = abs(64-fixtoi(fatan2(ftofix((hole->s.x+vert->s.x)-ball->s.x), ftofix((hole->s.y+vert->s.y)-ball->s.y))));

        //ball->s.x = hole->s.x+vert->s.x-(vert->radius+ball->radius)*cos(ang);
        //ball->s.y = hole->s.y+vert->s.y-(vert->radius+ball->radius)*sin(ang);
        ball->s.x = hole->s.x+vert->s.x-(vert->radius+ball->radius)*fixtof(fcos(itofix(ang)));
        ball->s.y = hole->s.y+vert->s.y-(vert->radius+ball->radius)*fixtof(fsin(itofix(ang)));
        ball->visible = 1;                //make ball visible again
        ball->mode = BALL_MODE_IDLE;      //reset ball mode for next hit
        pl->coins_current = 0;            //reset coin counter
        do_player_turn_finish(pl, hole);  //finish the player's turn
}

//drop the ball on the nearest laser respawn pad (after zap)
void do_ball_laser_drop(BALL * ball, PLAYER * pl, HOLE * hole){
        int v;
        int dist = 0;
        int cur_slot = 0;

        for(v = 0; v < hole->num_verts; v++){
	        //check for a laser respawn position vert
                if(hole->vert[v].flag == VERT_FLAG_LASER_RESPAWN){
	                //find the distance between the ball and this vert..
                        dist = v_distance(ball->s, v_offset(hole->s, hole->vert[v].s));
                        //if this is the least distance of the currently found respawn positions, set the current slot to this vert
                        if(dist < v_distance(ball->s, v_offset(hole->s, hole->vert[cur_slot].s))){
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
void do_ball_laser_zap(BALL * ball, PLAYER * pl, HOLE * hole){
        ball->speed = 0;                      //set ball speed to 0
        ball->v.x = ball->v.y = 0;
        ball->mode = BALL_MODE_LASER_ZAPPED;  //set ball to laser zap mode
        ball->visible = 0;                    //set ball invisible
        pl->cur_strokes++;                    //add another stroke to the player stroke total

        //play the zap sound effect
        play_ball_zap_sample();
        
        //add some random message to the queue
        add_ball_zap_message();
        
        pl->pic_mode = 2;                     //set player pic to "ouch" mode
}

//ball sinks into water hazard
void do_ball_water_sink(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole){
        ball->speed = 0;                       //set speed to zero
        ball->mode = BALL_MODE_WATER_SINKING;  //ball is sinking (now we can animate it)
        ball->visible = 0;                     //ball is invisible temporarily
        pl->cur_strokes++;                     //add another stroke to the player's current total

        //play the "splash" sample
        play_ball_splash_sample();

        //add the commentary message, and the prompt message
        add_ball_water_sunk_messages();        
        
        pl->pic_mode = 2;    //player is unhappy
}

//ball enters the teleport pad, send ball to linked teleport pad
void do_ball_teleport(BALL * ball, VERT * vert, HOLE * hole){
	//play the teleport sound
	play_ball_teleport_sample();
	
        vert->ref_no[4] = hole->vert[vert->ref_no[0]].ref_no[4] = 0;
        ball->angle = vert->ref_no[1];           //set the exit angle to the one specified by the vert
        ball->f_angle = M_PI*ball->angle/128.0f-M_PI;      //floating angle needs to be tweaked a bit
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
void do_ball_pipe_enter(BALL * ball, VERT * vert, HOLE * hole){
        ball->visible = 0;

	//if the ball isn't currently in pipe enter mode, set it
        if(ball->mode != BALL_MODE_PIPE_ENTER){
                vert->ref_no[5] = (int)(ball->speed*100);   //record the current speed
                ball->speed = 0;                            //stop the ball temporarily
                ball->mode = BALL_MODE_PIPE_ENTER;
                }

        if(++vert->ref_no[2] >= vert->ref_no[3]){
                vert->ref_no[2] = 0;
                //ball->mode = BALL_MODE_PIPE_EXIT;
                ball->angle = vert->ref_no[4];
                ball->f_angle = PI*ball->angle/128.0;   //convert the 0-256 angle to 0-2PI angle
                ball->s = v_offset(hole->s, hole->vert[vert->ref_no[1]].s);
                ball->speed = vert->ref_no[5]/100.0;    //get the old speed back
                ball->mode = BALL_MODE_HIT;             //go back to normal hit mode
                ball->visible = 1;
                }
}

//ball sinks in the cup..
void do_ball_hole_sink(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole){
        ball->speed = 0;
        ball->mode = BALL_MODE_SUNK;
        ball->visible = 0;
        ball->s = v_offset(hole->s, vert->s);
        vert->ref_no[0] = 1;
        
        //play the cup sink sample
        play_ball_holesink_sample();

        //player got a hole-in-one
        if(pl->cur_strokes == 1){
	        //play a hole-in-one sample
	        play_hole_in_one_sample();
	        
	        //add the hole-in-one message to the queue
	        add_hole_in_one_message();
                }
        //player got par
        if(!(hole->par_score-pl->cur_strokes)){
	        //plays sample for a good finish
	        play_good_finish_sample();
	        
	        //add the par finish messages to the message queue
	        add_par_finish_messages();
	        
                pl->pic_mode = 1;              //player is happy
                }
        //player got birdie
        else if(hole->par_score-pl->cur_strokes == 1){
	        //plays sample for a good finish
	        play_good_finish_sample();
	        
	        //add the birdie finish messages to the message queue
	        add_birdie_finish_messages();
	        
                pl->scorecard.num_birdies++;  //increment player's birdie count
                pl->pic_mode = 1;             //player is happy
                }
        //player got eagle
        else if(hole->par_score-pl->cur_strokes == 2){
	        //plays sample for a good finish
	        play_good_finish_sample();
	        
	        //add the eagle finish messages to the message queue
	        add_eagle_finish_messages();
	        
                pl->scorecard.num_eagles++;  //increment player's eagle count
                pl->pic_mode = 1;            //player is happy
                }
        //player got albatross
        else if(hole->par_score-pl->cur_strokes >= 3){
	        //play an "albatross" sample
	        play_hole_in_one_sample();
	        
	        //add the albatross finish messages to the message queue
	        add_albatross_finish_messages();
	        
                pl->scorecard.num_albo++;  //increment player's albatross count
                pl->pic_mode = 1;          //player is happy
                }
        //player got bogey
        else if(hole->par_score-pl->cur_strokes == -1){
	        //plays a "bad finish" sample
	        play_poor_finish_sample();
	        
	        //add the bogey finish messages to the message queue
	        add_bogey_finish_messages();
	        
                pl->pic_mode = 3;          //player is upset..
                }
        //player got worse than bogey
        else if(hole->par_score-pl->cur_strokes < -1){
	        //plays a "bad finish" sample
	        play_poor_finish_sample();
	        
	        //add the worse-than-bogey finish messages to the message queue
	        add_under_bogey_finish_messages(hole->par_score-pl->cur_strokes);
	        
                pl->pic_mode = 3;         //player makes an upset face
                }
                
        //add the latest player tallies to his scorecard
        pl->scorecard.score[main_game_vars.cur_hole] = pl->cur_strokes;
        pl->scorecard.par_score[main_game_vars.cur_hole] = hole->par_score;
}

//does the vert's reaction to the ball being nearby
void do_ball_vert_reaction(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole, int v_no){
        switch(vert->flag){
                case VERT_FLAG_HOLE:      //the cup..
                	//make sure the ball isn't going too fast..
                        if(ball->speed < BALL_MAX_HOLEHIT_SPEED && ball->speed >= hole->deccel_factor){
	                        //make sure the ball is within a certain distance of the cup's center
                                if(v_distance(ball->s, v_offset(hole->s, vert->s)) < ball->radius){
	                                //drop the ball in the cup
                                        do_ball_hole_sink(ball, pl, vert, hole);
                                        }
                                }
			//ball is really close to the hole, but not in the cup..
                        else if(ball->speed <= 0){
                                //add a "so close" message to the queue to aggravate the player further
                                add_so_close_message();
                                
                                //show the crowd's discomfort with the "ahh" sound
                                play_ahhh_sample();
                                }

                        break;
                case VERT_FLAG_HAZARD_WATER:   //this vert is a circular water hole
                        if(v_distance(ball->s, v_offset(hole->s, vert->s)) < vert->radius){
                                do_ball_water_sink(ball, pl, vert, hole);
                                }
                        ball->cur_vert = v_no;

                        break;
                case VERT_FLAG_PIPE_ENTER:     //this vert just teleports the ball to another vert position after a delay
                        if(check_circle_collision(ball->s, v_offset(hole->s, vert->s), ball->radius, vert->radius-2)){
                        //if(v_distance(ball->s, v_offset(hole->s, vert->s)) < ball->radius){
                                do_ball_pipe_enter(ball, vert, hole);
                                }

                        break;
                case VERT_FLAG_PIPE_EXIT:
                        //do_ball_pipe_exit(ball, vert, hole);

                        break;
                case VERT_FLAG_INDENT_HILL:   //this vert is a circular hill that moves the ball away from its center
                /*
                        if(ball->s.x < hole->s.x+vert->s.x){
                                ball->s.x += (v_distance(ball->s, v_offset(hole->s, vert->s))/vert->radius)*fixtof(fcos(itofix(abs(fixtoi(fatan2(ftofix(ball->s.x-vert->s.x), ftofix(ball->s.y-vert->s.y))) + 128))));
                                //turn the ball angle ever so slightly
                                ball->f_angle = (++ball->angle)/128.0;
                                }
                        else{
                                ball->s.x -= (v_distance(ball->s, v_offset(hole->s, vert->s))/vert->radius)*fixtof(fcos(itofix(abs(fixtoi(fatan2(ftofix(ball->s.x-vert->s.x), ftofix(ball->s.y-vert->s.y))) + 128))));
                                //turn the ball angle ever so slightly
                                ball->f_angle = (--ball->angle)/128.0;
                                }
                        if(ball->s.y < hole->s.y+vert->s.y) ball->s.y += (v_distance(ball->s, v_offset(hole->s, vert->s))/vert->radius)*fixtof(fsin(itofix(abs(fixtoi(fatan2(ftofix(ball->s.x-vert->s.x), ftofix(ball->s.y-vert->s.y))) + 128))));
                        else ball->s.y -= (v_distance(ball->s, v_offset(hole->s, vert->s))/vert->radius)*fixtof(fsin(itofix(abs(fixtoi(fatan2(ftofix(ball->s.x-vert->s.x), ftofix(ball->s.y-vert->s.y))) + 128))));
		*/
                  	{
	                  	//find angle between ball and the hill's center point
	                  	float ang = atan2(ball->s.y-(hole->s.y+vert->s.y), ball->s.x-(hole->s.x+vert->s.x));
	                  	float hill_f = 0.025;  //hill force
	                  	
	                  	//add the force to the ball's force vector
	                  	ball->f.x += hill_f*cos(ang);
	                  	ball->f.y += hill_f*sin(ang);
	                  	}      
                        break;
                case VERT_FLAG_INDENT_RUT:    //this vert is a circular rut that moves the ball toward its center
                        if(ball->s.x < hole->s.x+vert->s.x){
                                ball->s.x -= (v_distance(ball->s, v_offset(hole->s, vert->s))/vert->radius)*fixtof(fcos(itofix(abs(fixtoi(fatan2(ftofix(ball->s.x-vert->s.x), ftofix(ball->s.y-vert->s.y))) + 128))));
                                //turn the ball angle ever so slightly
                                ball->f_angle = (--ball->angle)/128.0;
                                }
                        else{
                                ball->s.x += (v_distance(ball->s, v_offset(hole->s, vert->s))/vert->radius)*fixtof(fcos(itofix(abs(fixtoi(fatan2(ftofix(ball->s.x-vert->s.x), ftofix(ball->s.y-vert->s.y))) + 128))));
                                //turn the ball angle ever so slightly
                                ball->f_angle = (++ball->angle)/128.0;
                                }
                        if(ball->s.y < hole->s.y+vert->s.y) ball->s.y -= (v_distance(ball->s, v_offset(hole->s, vert->s))/vert->radius)*fixtof(fsin(itofix(abs(fixtoi(fatan2(ftofix(ball->s.x-vert->s.x), ftofix(ball->s.y-vert->s.y))) + 128))));
                        else ball->s.y += (v_distance(ball->s, v_offset(hole->s, vert->s))/vert->radius)*fixtof(fsin(itofix(abs(fixtoi(fatan2(ftofix(ball->s.x-vert->s.x), ftofix(ball->s.y-vert->s.y))) + 128))));

                        break;
                case VERT_FLAG_HAZARD_SAND:   //this vert is a circle of sand that slows the ball down
                        if(ball->speed > 0) ball->speed -= hole->deccel_factor;

                        break;
                case VERT_FLAG_HAZARD_SLIME:  //this vert is a circle of slime that slows the ball down even more
                        if(ball->speed > 0) ball->speed -= hole->deccel_factor*2;

                        break;
                case VERT_FLAG_POLY_MOTION_AREA:   //this vert defines a motion area that surrounds a moving polygon
                        ball->cur_vert = v_no;

                        //rotating poly
                        if(vert->ref_no[0] == 0){
                                }

                        //horizontally moving poly
                        else if(vert->ref_no[0] == 1){
                                RECT temp_rect;    //create a temp rect and check to see if ball is touching it
                                temp_rect.x1 = vert->s.x-vert->radius;
                                temp_rect.y1 = vert->s.y-vert->ref_no[1];
                                temp_rect.x2 = vert->s.x+vert->radius;
                                temp_rect.y2 = vert->s.y+vert->ref_no[2];

                                //if the rects touch, keep the ball "hit" so it stays here until the poly contacts it again (moving it out of the space)
                                if(rect_collision(ball->hit_rect, temp_rect, ball->s, hole->s)){
                                        ball->mode = BALL_MODE_IN_MOTION_AREA;
                                        }
                                else ball->mode = BALL_MODE_HIT;
                                }

                        //vertically moving poly
                        else if(vert->ref_no[0] == 2){
                                RECT temp_rect;    //create a temp rect and check to see if ball is touching it
                                temp_rect.x1 = vert->s.x-vert->ref_no[1];
                                temp_rect.y1 = vert->s.y-vert->radius;
                                temp_rect.x2 = vert->s.x+vert->ref_no[2];
                                temp_rect.y2 = vert->s.y+vert->radius;

                                //if the rects touch, keep the ball "hit" so it stays here until the poly contacts it again (moving it out of the space)
                                if(rect_collision(ball->hit_rect, temp_rect, ball->s, hole->s)){
                                        ball->mode = BALL_MODE_IN_MOTION_AREA;
                                        }
                                else{
                                        ball->mode = BALL_MODE_HIT;
                                        }
                                }

                        break;
                case VERT_FLAG_POLY_MORPH:
                        ball->cur_vert = v_no;
                        if(vert->ref_no[0] == 0) ball->mode = BALL_MODE_IN_MOTION_AREA;

                        break;
                case VERT_FLAG_TELEPORT:
                        if(vert->ref_no[4] == 0){  //if active flag isn't set, do teleport
                                if(v_distance(ball->s, v_offset(hole->s, vert->s)) < ball->radius){  //make sure the ball is close enough to the center of the teleport pad
                                        ball->visible = 0;       //set ball to visible for a second
                                        do_ball_teleport(ball, vert, hole);  //teleport the ball to the specified teleport pad
                                        }
                                }
                        break;
                case VERT_FLAG_CONVEYOR:
                        ball->cur_vert = v_no;

                        if(point_in_poly(ball->s, hole->body[vert->ref_no[5]].poly, hole->s)){
                        //if(rect_collision(ball->hit_rect, temp_rect, ball->s, hole->s)){
                                if(!vert->ref_no[0]){
                                        ball->s.y -= vert->ref_no[2]/2.0;
                                        //if(ball->angle < 64) ball->angle++;
                                        //else if(ball->angle > 64) ball->angle--;
                                        }
                                else if(vert->ref_no[0] == 1){
                                        ball->s.y += vert->ref_no[2]/2.0;
                                        //if(ball->angle < 192) ball->angle++;
                                        //else if(ball->angle > 192) ball->angle--;
                                        }
                                else if(vert->ref_no[0] == 2){
                                        ball->s.x -= vert->ref_no[2]/2.0;
                                        //if(ball->angle > 0) ball->angle--;
                                        }
                                else if(vert->ref_no[0] == 3){
                                        ball->s.x += vert->ref_no[2]/2.0;
                                        //if(ball->angle < 128) ball->angle++;
                                        //else if(ball->angle > 128) ball->angle--;
                                        }
                                ball->mode = BALL_MODE_ON_CONVEYOR;
                                }
                        //else{
                        //        ball->mode = BALL_MODE_HIT;
                        //        }

                        break;
                default:
                        break;
                }
}

//check to see if the ball is within the radius of the vert..
void check_ball_vert_collision(BALL * ball, PLAYER * pl, VERT * vert, HOLE * hole, int v_no){
	//check for a collision between rectangles surrounding the ball and this vert first (might decrease # of calculations in the long run)
        if(rect_collision_f(-ball->radius, -ball->radius, ball->radius, ball->radius, -vert->radius, -vert->radius, vert->radius, vert->radius, ball->s, v_offset(hole->s, vert->s))){
		//check for a collision between the ball circle and the vert circle
                if(check_circle_collision(ball->s, v_offset(hole->s, vert->s), ball->radius, vert->radius)){
	                //react according to the vert's flag
                        do_ball_vert_reaction(ball, pl, vert, hole, v_no);
                        }
                }
}

//check the ball against all verts for possible collisions
void check_ball_vert_collision_all(BALL * ball, PLAYER * pl, HOLE * hole){
        int v;

        for(v = 0; v < hole->num_verts; v++){
                check_ball_vert_collision(ball, pl, &hole->vert[v], hole, v);
                }
}

//find the p player
void get_player_keys(PLAYER * pl, HOLE * hole){
	//only get club swinging keys when ball is idle
        if(pl->ball.mode == BALL_MODE_IDLE){
                if(pl->controller.type != CONTROLLER_MOUSE){   //keyboard/joystick control
	                //left key was pressed, rotate the club left
                        if(controller_key_left_pressed(pl->controller)){
	                        //play the rotate sample
	                        play_club_rotate_sample();
                                pl->f_club_angle = mod_angle(pl->f_club_angle-pl->f_club_rot_speed);
                                
                                //rotate the club vertices the appropriate amount
                                rotate_vertex(&pl->club_v[0], -pl->f_club_rot_speed);
                                rotate_vertex(&pl->club_v[1], -pl->f_club_rot_speed);
                                
                                //if trajectory display active, set it up so we can recalculate new trajectory based on the new club angle
                                if(pl->traj_active){
	                                pl->traj_count = 0;
	                                pl->traj_calc_done = 0;
                                	}
                                	
                                controller_clear_keybuf(pl->controller, 1);
                                }
	                //right key was pressed, rotate the club right
                        else if(controller_key_right_pressed(pl->controller)){
	                        //play the rotate sample
	                        play_club_rotate_sample();
                                pl->f_club_angle = mod_angle(pl->f_club_angle+pl->f_club_rot_speed);
                                
                                //rotate the club vertices the appropriate amount
                                rotate_vertex(&pl->club_v[0], pl->f_club_rot_speed);
                                rotate_vertex(&pl->club_v[1], pl->f_club_rot_speed);
                                
                                //if trajectory display active, set it up so we can recalculate new trajectory based on the new club angle
                                if(pl->traj_active){
	                                pl->traj_count = 0;
	                                pl->traj_calc_done = 0;
                                	}
                                	
                                controller_clear_keybuf(pl->controller, 2);
                                }
	                //"show trajectory" key pressed, start trajectory stuff
                        else if(controller_key_traj_pressed(pl->controller)){
	                        pl->traj_active = !pl->traj_active;
	                        pl->traj_line_timer = 0;
	                        if(pl->traj_calc_done){
		                        pl->traj_calc_done = 0;
		                        pl->traj_count = 0;
	                        	}
	                        else{
		                        pl->traj_count = 0;
	                        	}
	                        
                                controller_clear_keybuf(pl->controller, 3);
                                }
                        }
                else{   //mouse control..
                	//set the angle based on where the mouse cursor is currently located
                        pl->club_angle = 128-abs(fixtoi(fatan2(ftofix(mouse_x-pl->ball.s.x), ftofix(mouse_y-pl->ball.s.y))) + 128)-64;
                        pl->f_club_angle = pl->club_angle/128.0;
                        pl->club_angle = mod_angle(pl->f_club_angle);
                        
                        //set the club vertices according to the new angle
                        pl->club_v[0].x = pl->club_power/2*fixtof(fcos(itofix(pl->club_angle+pl->club_width)));
                        pl->club_v[0].y = pl->club_power/2*fixtof(fsin(itofix(pl->club_angle+pl->club_width)));
                        pl->club_v[1].x = pl->club_power/2*fixtof(fcos(itofix(pl->club_angle-pl->club_width)));
                        pl->club_v[1].y = pl->club_power/2*fixtof(fsin(itofix(pl->club_angle-pl->club_width)));
                        }

                if(controller_key_swing_pressed(pl->controller)){
                        pl->club_swing = 1;     //swing mode active..
                        
                        //if trajectory display active, deactivate it before swing begins
                        if(pl->traj_active) pl->traj_active = 0;
                        
                        controller_clear_keybuf(pl->controller, 0);
                        }
                }
        //if ball is sunk in the water, wait for "d" or "r" to be pressed to drop or rehit
        else if(pl->ball.mode == BALL_MODE_WATER_SUNK){
                if(main_game_vars.cur_vmessage_slot == 1){    //make sure the current message is the "[d]rop or [r]ehit?" one
                	//drop the ball nearby
                        if(key[KEY_D]){
                                do_ball_drop(&pl->ball, pl, &hole->vert[pl->ball.cur_vert], hole);
                                deactivate_vfont_message(&game_vmessage[0]);
                                pl->pic_mode = 0;             //make player's face content again
                                key[KEY_D] = 0;
                                }
                        //drop the ball from the last place it was hit
                        else if(key[KEY_R]){
                                do_ball_rehit(&pl->ball, hole);
                                deactivate_vfont_message(&game_vmessage[0]);
                                pl->pic_mode = 0;             //make player's face content again
                                key[KEY_R] = 0;
                                }
                        }
        	}
	//clear huds from obstructing view..
        if(controller_key_clearhud_pressed(pl->controller)){
                if(!main_game_vars.player_huds_hiding){         //if the huds aren't already hiding..
                        main_game_vars.player_huds_hiding = 1;  //..hide the huds
                        }
                }
        if(key[KEY_1]){   //just a test cheat to put the ball in the hole when this key is hit
        	int i;
        	
                game_player[main_game_vars.cur_player].ball.s = v_offset(game_course.current_hole.s, game_course.current_hole.vert[1].s);
                game_player[main_game_vars.cur_player].cur_strokes = game_course.current_hole.par_score;
                game_player[main_game_vars.cur_player].ball.mode = BALL_MODE_HIT;
                game_player[main_game_vars.cur_player].ball.speed = game_course.current_hole.deccel_factor+0.05;
                key[KEY_1] = 0;
                }
}

//finalize the match
void do_match_finish(HOLE * hole, int cur_hole, PLAYER pl[], int num_players){
        int i;
        V_INT p_no, p_total; //p_no contains the player numbers from 0 to num_players
                             //p_total contains the stroke total for each player in p_no

        for(i = 0; i < num_players; i++){
                p_no.c[i] = i;
                p_total.c[i] = pl[i].cur_strokes;
                }
        //sort from smallest to largest score
        v_int_heap_sort_ref(&p_total, &p_no, num_players);
        //p_no should now be sorted from the player with the best score to that with the worst

        //if there are only 2 players, the sort is simple
        if(num_players == 2){
                if(p_total.c[0] < p_total.c[1]){
                        //the player with least score is tagged the winner
                        pl[p_no.c[0]].scorecard.match_won[cur_hole] = 1;
                        //other players are tagged losers
                        pl[p_no.c[1]].scorecard.match_won[cur_hole] = 0;
                        //winning player's win total increments
                        pl[p_no.c[0]].scorecard.matches_won++;
                        }
                else{
                        //players are tied
                        pl[p_no.c[0]].scorecard.match_won[cur_hole] = pl[p_no.c[1]].scorecard.match_won[cur_hole] = 2;
                        }
                }
        else if(num_players == 3){
                //players are tied
                if(p_total.c[0] == p_total.c[1]){
                        pl[p_no.c[0]].scorecard.match_won[cur_hole] = pl[p_no.c[1]].scorecard.match_won[cur_hole] = pl[p_no.c[2]].scorecard.match_won[cur_hole] = 2;
                        }
                else{
                        //the player with least score is tagged the winner
                        pl[p_no.c[0]].scorecard.match_won[cur_hole] = 1;
                        //other players are tagged losers
                        pl[p_no.c[1]].scorecard.match_won[cur_hole] = pl[p_no.c[2]].scorecard.match_won[cur_hole] = 0;
                        //winning player's win total increments
                        pl[p_no.c[0]].scorecard.matches_won++;
                        }
                }
        else{
                //players are tied
                if(p_total.c[0] == p_total.c[1]){
                        //all players are tagged as tied
	                pl[0].scorecard.match_won[cur_hole] = pl[1].scorecard.match_won[cur_hole] = pl[2].scorecard.match_won[cur_hole] = pl[3].scorecard.match_won[cur_hole] = 2;
                        }
                else{
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
void ball_mirage_logic(BALL * ball){
	if(ball->mirage_visible){                                    //mirage is visible, continue
		if(++ball->mirage_timer >= ball->mirage_timer_end){  //increment the timer until the end
			ball->mirage_visible = 0;                    //make mirage invisible
			ball->mirage_timer = 0;                      //reset timer for next time
			}
		}
}

//ball logic..
void ball_logic(BALL * ball, PLAYER * pl, HOLE * hole){
        if(ball->mode == BALL_MODE_HIT || ball->mode == BALL_MODE_IN_MOTION_AREA || ball->mode == BALL_MODE_ON_CONVEYOR){
                check_ball_hole_collision(ball, pl, hole);  //check for collisions between ball and walls
                move_player_ball(ball, hole);               //move the ball around
                decell_ball(ball, pl, hole);                //slow the ball down until it stops
                }
        if(ball->mode == BALL_MODE_HIT || ball->mode == BALL_MODE_PIPE_ENTER || ball->mode == BALL_MODE_IN_MOTION_AREA || ball->mode == BALL_MODE_ON_CONVEYOR){
	        //check for collisions with the various verts, and react according to the vert flag
                check_ball_vert_collision_all(ball, pl, hole);  
//                if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
//                        check_ball_coinvert_collision_all(ball, pl, hole);
//                        }
                }
        if(ball->mode == BALL_MODE_WATER_SUNK){
                }
        else if(ball->mode == BALL_MODE_LASER_ZAPPED){      //ball has been zapped by a laser
                if(!main_game_vars.cur_vmessage_slot){      //wait for the "zap" message to clear before dropping ball into play again
                        //bring ball back to the nearest respawn pad
                        do_ball_laser_drop(ball, pl, hole);
                        }
                }
        if(ball->mode == BALL_MODE_SUNK){                   //ball is in the cup
                do_player_hole_finish(pl, hole);            //finish the hole
                }
        ball_mirage_logic(ball);                            //countdown until the mirage disappears
//        if(main_game_vars.debug_on) draw_poly2(bp, poly_v_offset(ball->poly, ball->v), ball->s.x, ball->s.y);
}

//handle the logic involving the trajectory calculations
void player_trajectory_logic(PLAYER * pl, HOLE * hole){
	if(pl->traj_active && !pl->traj_calc_done){
		BALL temp_ball = pl->ball;
		
	        temp_ball.angle = temp_ball.f_angle = pl->f_club_angle;
        	temp_ball.mode = BALL_MODE_HIT;
        	temp_ball.speed = 10.0f;
        	temp_ball.v.x = temp_ball.speed*cos(pl->f_club_angle);
        	temp_ball.v.y = temp_ball.speed*sin(pl->f_club_angle);
        	temp_ball.body.vel = temp_ball.v;
        	
        	pl->traj_v[0] = temp_ball.s;
        	pl->traj_count++;
        	while(temp_ball.speed > 0){
			move_player_ball(&temp_ball, hole);
			check_ball_hole_collision(&temp_ball, pl, hole);
			temp_ball.speed -= hole->deccel_factor;
			}
		pl->traj_calc_done = 1;
		}
	if(pl->traj_active){
		pl->traj_line_timer = !pl->traj_line_timer;    //alternate the dots on the dotted line display
		}
}

//does the player logic for one player
void player_logic(PLAYER * pl, HOLE * hole){
	//if the player's club isn't being swung, get key input
        if(!pl->club_swing){               
                get_player_keys(pl, hole);
                }
        //swing the player's club
        else{
                do_player_club_swing(pl);
                }
        //handle ball logic (movement, collision, etc)
        ball_logic(&pl->ball, pl, hole);
        
        //handle the trajectory logic if necessary
        player_trajectory_logic(pl, hole);
}

//does the player logic for all players in the game
void player_logic_all(PLAYER pl[], int num_players, HOLE * hole){
//        int p;
//        for(p = 0; p < num_players; p++){
//                player_logic(&pl[p], hole);
//                }
	//handle logic for the current player
        player_logic(&pl[main_game_vars.cur_player], hole);
}

//drop the player's ball on the tee vert spot
void player_start_hole(PLAYER * pl, HOLE * hole){
        int i;

        //look through all the verts for the tee vert
        for(i = 0; i < hole->num_verts; i++){
                if(hole->vert[i].flag == VERT_FLAG_TEE){                 //tee flag found..
                        pl->ball.s = v_offset(hole->s, hole->vert[i].s); //place ball on tee vert position
                        break;
                        }
                }
        pl->ball.mode = BALL_MODE_IDLE;          //ball needs to be idle to start off
        pl->ball.idle_timer = 0;                 //reset the idle timer
        pl->ball.visible = 1;                    //ball should be visible
        main_game_vars.hole_finished = 0;        //hole isn't finished
}

//handle the in-game message queue..
void game_vmessage_logic(V_MESSAGE msg[], int * cur_slot, V_FONT * vf){
        int i;

        switch(msg[0].active){
                case -1:  //message needs activation
                        msg[0].active = 1;
                        break;

                case 1:   //message is activated, do the logic
                        vfont_message_logic(vf, &msg[0], msg[0].method, 1);
                        break;

                case 2:   //message is done, deactivate it
                        msg[0].active = 0;
                        for(i = 0; i < *cur_slot; i++){  //move all the messages down in the queue
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
void start_scoreboard_segout(void){
	main_game_vars.game_state = main_game_vars.game_state_old = GAMESTATE_SCOREBOARD_SEGOUT;
}

//start the victory screen seg-in sequence
void start_victory_screen_segin(MAIN_GAME_VARS * gvars, COURSE * cr, PLAYER pl[], int num_players){
	set_victory_screen_vars(gvars);                //set up the victory screen
        if(num_players == 1) gvars->game_state = gvars->game_state_old = GAMESTATE_COURSE_COMPLETED_1P;
	else gvars->game_state = gvars->game_state_old = GAMESTATE_COURSE_COMPLETED_XP;

        //check for high scores
        if(!round_hiscore_check(cr, pl, num_players)){ //check to see if there was a top score tallied during this game
		char hi_fname[32];                     //temp var to store the filename
                        
                //save high scores to file
                gvars->got_hiscore = 1;                //indicates we have obtained a high score
                strcpy(hi_fname, cr->fname);           //copy the course's base name to the temp string
                strcat(hi_fname, ".crh");              //add the extension .crh
                save_course_top_scores(cr, hi_fname);  //save the top scores of this course to file
                }
        else gvars->got_hiscore = 0;                   //no top score this time..
                        
        gvars->cur_hole = -1;	                       //restart the cur_hole counter
}

//get key input on the scoreboard screen
void get_scoreboard_keys(COURSE * cr, PLAYER pl[], int num_players, int cur_hole){
	if(key[KEY_ENTER] || key[KEY_1] || key[KEY_2]){
		if(key[KEY_1]){   //cheat key to goto the last hole
			int i;
			for(i = 0; i < 17; i++){
				pl[main_game_vars.cur_player].scorecard.score[i] = 4;
				pl[main_game_vars.cur_player].scorecard.par_score[i] = 4;
				}
			main_game_vars.cur_hole = 16;
			key[KEY_1] = 0;
			}
		if(key[KEY_2]){   //cheat key to goto hole 10
			int i;
			for(i = 0; i < 9; i++){
				pl[main_game_vars.cur_player].scorecard.score[i] = 4;
				pl[main_game_vars.cur_player].scorecard.par_score[i] = 4;
				}
			main_game_vars.cur_hole = 8;
			key[KEY_2] = 0;
			}
	        if(cur_hole < cr->num_holes-1){
			//increment to next hole
                        main_game_vars.cur_hole++;
                        //set the filename for the next hole to load (should be COURSENAME##.HOL)
                        hole_fname(cr->fname, cr->fname_prefix, main_game_vars.cur_hole);

                        //load hole data
                        load_hole(&cr->current_hole, cr->fname_prefix);

                        //load coin data if coin collector mode
	                if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
        	                char filename[64];

                	        strcpy(filename, cr->fname_prefix);
                                filename[strlen(filename)-1] = 'c';
                              	load_hole_coindata(&cr->current_hole, filename);
                                }
                        initialize_hole(&cr->current_hole);       //initialize the next hole before we start it
                        hole_initialize_players(pl, num_players, main_game_vars.cur_hole);   //initialize the players before starting the next hole
			start_scoreboard_segout();                //start the seg-out sequence
                        }
		else{
                        //last hole has been passed, go to victory screen
                        start_victory_screen_segin(&main_game_vars, cr, pl, num_players);
                        }

		key[KEY_ENTER] = 0;
		}
}

//seg-out is done, goto next game state
void finish_scoreboard_segout(void){
	//since we're doing the seg-out sequence on the victory screen also, do a check first
	//before going to the "start hole" state
	if(main_game_vars.game_state != GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGIN && main_game_vars.game_state != GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGIN){
		main_game_vars.game_state = GAMESTATE_START_HOLE;
		}
}

//handles logic related to the scoreboard screen seg'ing out
void course_scoreboard_segout_logic(COURSE * cr){
	//increment segment counter until hits the end, then decrease the # of segments to show
	if(++cr->scoreboard.scoreboard_seg_counter >= cr->scoreboard.scoreboard_seg_counter_end){
		cr->scoreboard.scoreboard_seg_counter = 0;
		cr->scoreboard.scoreboard_seg_count--;
		if(cr->scoreboard.scoreboard_seg_count <= 0){
			//all segments are removed now, finish seg-out sequence
			finish_scoreboard_segout();
			}
		}
}

//seg-in is done, goto next game state
void finish_scoreboard_segin(void){
	main_game_vars.game_state = GAMESTATE_SCOREBOARD_SEGMID;
}

//handles logic related to the scoreboard screen seg'ing in
void course_scoreboard_segin_logic(COURSE * cr){
	//increment segment counter until hits the end, then increase the # of segments to show
	if(++cr->scoreboard.scoreboard_seg_counter >= cr->scoreboard.scoreboard_seg_counter_end){
		cr->scoreboard.scoreboard_seg_counter = 0;
		cr->scoreboard.scoreboard_seg_count++;
		if(cr->scoreboard.scoreboard_seg_count >= cr->scoreboard.num_scoreboard_segs){
			//all segments are visible now, finish the seg-in sequence
			finish_scoreboard_segin();
			}
		}
}


//handles main logic related to the scoreboard screen
void course_scoreboard_main_logic(COURSE * cr, PLAYER pl[], int num_players, int cur_hole){
	get_scoreboard_keys(cr, pl, num_players, cur_hole);
}

//do the single player logic upon course completion
void single_player_course_completed_logic(MAIN_GAME_VARS * gvars, PLAYER * pl, COURSE * cr){
	//start the victory screen seg-in sequence
	gvars->game_state = GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGIN;
}

//do the multi-player logic upon course completion
void multi_player_course_completed_logic(MAIN_GAME_VARS * gvars, PLAYER pl[], int num_players, COURSE * cr){
	//if there are only two players, sorting will be a little easier..
	if(num_players == 2){
	        switch(gvars->game_mode){
	        	case GAMEMODE_STROKE_PLAY:   
	        		//check to see if player 1's score is lower than player 2's
                                if(player_course_stroke_total(pl[0], cr->num_holes, cr->num_holes) > player_course_stroke_total(pl[1], cr->num_holes, cr->num_holes)){
                                        gvars->vict_winner[0] = 1;
                                        gvars->vict_winner[1] = 0;
                                        }
                                //check to see if player 2's score is lower than player 1's
                                else if(player_course_stroke_total(pl[0], game_course.num_holes, game_course.num_holes) < player_course_stroke_total(pl[1], game_course.num_holes, game_course.num_holes)){
                                        gvars->vict_winner[1] = 1;
                                        gvars->vict_winner[0] = 0;
                                        }
                                //if neither of the above is true, the game is a tie
                                else gvars->vict_winner[1] = gvars->vict_winner[0] = -1;
                                break;
                        case GAMEMODE_MATCH_PLAY:
	        		//check to see if player 1 won more matches than player 2
                                if(pl[0].scorecard.matches_won > pl[1].scorecard.matches_won){
                                        gvars->vict_winner[0] = 0;
                                        gvars->vict_winner[1] = 1;
                                        }
	        		//check to see if player 2 won more matches than player 1
                                else if(pl[0].scorecard.matches_won < pl[1].scorecard.matches_won){
                                        gvars->vict_winner[0] = 1;
                                        gvars->vict_winner[1] = 0;
                                        }
                                //if neither of the above is true, the game is a tie
                                else gvars->vict_winner[1] = gvars->vict_winner[0] = -1;
                                break;
                        default:
                        	break;
                        }
		}
	else{
        	V_INT p_total, p_no;
                int i;

                switch(gvars->game_mode){
	                case GAMEMODE_STROKE_PLAY:
                                for(i = 0; i < num_players; i++){
                                        p_no.c[i] = i;
                                        p_total.c[i] = player_course_stroke_total(pl[i], cr->num_holes, cr->num_holes);
                                        }

                                //sort from smallest to largest score
                                v_int_heap_sort_ref(&p_total, &p_no, num_players);

                                //set winner array
                                for(i = 0; i < num_players; i++){
                                        gvars->vict_winner[i] = p_no.c[i];
                                        }

                                //check for tie
                                if(p_total.c[gvars->vict_winner[0]] == p_total.c[gvars->vict_winner[1]]){
                                        gvars->vict_winner[0] = gvars->vict_winner[1] = gvars->vict_winner[2] = -1;
                                        }
                                break;
                        case GAMEMODE_MATCH_PLAY:
                                for(i = 0; i < num_players; i++){
                                        p_no.c[i] = i;
                                        p_total.c[i] = pl[i].scorecard.matches_won;
                                        }

                                //sort from largest to smallest score
                                v_int_heap_sort_ref_rev(&p_total, &p_no, num_players);

                                //set winner array
                                for(i = 0; i < num_players; i++){
                                        gvars->vict_winner[i] = p_no.c[i];
                                        }

                                //check for tie
                                if(p_total.c[gvars->vict_winner[0]] == p_total.c[gvars->vict_winner[1]]){
                                        gvars->vict_winner[0] = gvars->vict_winner[1] = gvars->vict_winner[2] = -1;
                                        }
				break;
			default:
				break;                        
			}
        	}
	
	//start the victory screen seg-in sequence
	gvars->game_state = GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGIN;
}

//move the clouds on the victory screen
void victory_screen_cloud_logic(MAIN_GAME_VARS * gvars){
	int i;
	
        for(i = 0; i < 4; i++){
                gvars->vict_cloud_x[i] -= 0.5*(1+i);  //make the further clouds move slower
                if(gvars->vict_cloud_x[i] <= -250){
                        gvars->vict_cloud_x[i] = SCREEN_W+random()%150;  //respawn clouds at the edge of the screen when they go off the other edge
                        gvars->vict_cloud_y[i] = random()%200;
                        }
                }
}

//do all the victory screen stuff
void victory_screen_segin_logic(MAIN_GAME_VARS * gvars, COURSE * cr){
        //move podium
        if(gvars->vict_pody > 480-cr->image_bank.victory_podiumpic->h+1){
                gvars->vict_pody -= 2;
                }
        else{
	        //seg-in is done, go to next state
	        if(gvars->num_players == 1) gvars->game_state = gvars->game_state_old = GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGMID;
	        else gvars->game_state = gvars->game_state_old = GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGMID;
        	}
        //move clouds
        victory_screen_cloud_logic(gvars);
        
        //do the scoreboard disappearance
        course_scoreboard_segout_logic(cr);
}

//get keys for the victory screen
void get_victory_screen_keys(MAIN_GAME_VARS * gvars, COURSE * cr){
	//wait for enter or space to be pressed to continue
	if(key[KEY_ENTER] || key[KEY_SPACE]){
		//go back to menu
		gvars->quit_game = 1;
		gvars->quit_title = 0;
		
                //if(!round_hiscore_check(&game_course)){
                //        char hi_fname[32];
                //
                //        strcpy(hi_fname, game_course.fname);
                //        strcat(hi_fname, ".crh");
                //        save_course_top_scores(&game_course, hi_fname);
                //        }
                
                //prompt for save
                if(main_game_vars.save_stats == 0){
                        }
                //automatically save stats
                else if(main_game_vars.save_stats == 1){
                        update_statsheets(gvars, cr);
                        save_statsheets(gvars, "vgolf.cst");
                        }
                key[KEY_ENTER] = key[KEY_SPACE] = joy[0].button[0].b = 0;
                }
}

//victory screen logic for single player game
void victory_screen_single_player_logic_main(MAIN_GAME_VARS * gvars, PLAYER * pl, COURSE * cr){
        //move clouds
        victory_screen_cloud_logic(gvars);
        //get keys
        get_victory_screen_keys(gvars, cr);
}

//victory screen logic for multiplayer game
void victory_screen_multi_player_logic_main(MAIN_GAME_VARS * gvars, PLAYER pl[], int num_players, COURSE * cr){
        //move clouds
        victory_screen_cloud_logic(gvars);
        //get keys
        get_victory_screen_keys(gvars, cr);
}

//the main logic stuff
void main_game_logic(PLAYER pl[], int num_players, COURSE * cr){
        switch(main_game_vars.game_state){
                int i;

		case GAMESTATE_SCOREBOARD_SEGIN:   //scoreboard is coming onscreen
		        course_scoreboard_segin_logic(cr);
			break;
		case GAMESTATE_SCOREBOARD_SEGMID:  //scoreboard is onscreen
		        course_scoreboard_main_logic(cr, pl, num_players, main_game_vars.cur_hole);
			break;
		case GAMESTATE_SCOREBOARD_SEGOUT:  //scoreboard is disappearing
		        course_scoreboard_segout_logic(cr);
			break;
                case GAMESTATE_START_HOLE:         //hole needs to be set up before playing
                        for(i = 0; i < main_game_vars.num_players; i++){
                                player_start_hole(&pl[i], &cr->current_hole);
                                }
                        main_game_vars.game_state = main_game_vars.game_state_old = GAMESTATE_READY_TOPLAY;

                        add_vfont_queue_message(game_vmessage, GAME_VMESSAGE_QUEUE_SIZE, &game_vfont, &main_game_vars.cur_vmessage_slot, "Tee off!", SCREEN_W, SCREEN_H/2, 0, VMSG_TEXT_ENTER_FROMRIGHT, -6, 0, 0, 0, 99999);

                        break;
                case GAMESTATE_READY_TOPLAY:       //we're ready to play golf
                        hole_logic(&cr->current_hole);
                        player_logic_all(pl, num_players, &cr->current_hole);
                        if(main_game_vars.player_huds_hiding) hide_player_huds(pl, num_players, main_game_vars.player_huds_hidden);
                        get_main_keys();

                        game_vmessage_logic(game_vmessage, &main_game_vars.cur_vmessage_slot, &game_vfont);

                        break;
                case GAMESTATE_HOLE_FINISHED:      //hole is finished..
                        //wait for messages to finish
	                if(main_game_vars.cur_vmessage_slot == 0){
        	                if(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY){
	        	                //finalize the match
                	                do_match_finish(&cr->current_hole, main_game_vars.cur_hole, pl, num_players);
                        	        }
	                        else if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
        	                        }
        	                //start the scoreboard seg-in
                	        main_game_vars.game_state = GAMESTATE_SCOREBOARD_SEGIN;
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
                case GAMESTATE_QUIT_PROMPT:       //the quit prompt is active..
                	//get keyboard input from player(s) (y, n, esc)
                        get_quit_prompt_keys();

                        //handle the message logic for the quit prompt message
                        game_vmessage_logic(game_vmessage, &main_game_vars.cur_vmessage_slot, &game_vfont);
                        break;
                default:
                	//get main keyboard input (for pausing, exiting, etc.)
                        get_main_keys();

                        break;
                }
}

//draw background layer..
void draw_hole_bkg(BITMAP * bp, HOLE * hole){
        clear_to_color(bp, 10);
        if(hole->layer_pic[0] != NULL) masked_blit(hole->layer_pic[0], bp, 0, 0, 0, 0, hole->layer_pic[0]->w, hole->layer_pic[0]->h);
}               

//draw foreground layer..
void draw_hole_fg(BITMAP * bp, HOLE * hole){
        if(hole->layer_pic[1] != NULL) masked_blit(hole->layer_pic[1], bp, 0, 0, 0, 0, hole->layer_pic[1]->w, hole->layer_pic[1]->h);
}

//draw the "morphing poly" layer..
void draw_hole_polymorph_layer(BITMAP * bp, HOLE * hole){
        if(hole->layer_pic[2] != NULL) masked_blit(hole->layer_pic[2], bp, 0, 0, 0, 0, hole->layer_pic[2]->w, hole->layer_pic[2]->h);
}

//hole foreground display stuff
void hole_fg_display(BITMAP * bp, HOLE * hole){
        //draw the polygons if there are any..
        draw_hole_polymorph_layer(bp, hole);

        //draw the foreground if it's there
        draw_hole_fg(bp, hole);
}

//draws a laser between the endpoints
void laser_display(BITMAP * bp, int x1, int y1, int x2, int y2, int color){
	//use the two-step line algorithm so we can add some noise outside the line
	draw_line_twostep(bp, x1, y1, x2, y2, color, 3);
}

//display one vert
void hole_vert_display(BITMAP * bp, VERT * vert, HOLE * hole, COURSE_IMAGE_BANK i_bank){
        int i;

        switch(vert->flag){
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
                        if(vert->ref_no[0]){
	                        //laser_display(hole->layer_pic[2], hole->s.x+vert->s.x, hole->s.y+vert->s.y, hole->s.x+hole->vert[vert->ref_no[5]].s.x, hole->s.y+hole->vert[vert->ref_no[5]].s.y, hole->vert[vert->ref_no[5]].ref_no[1]);
	                        line(hole->layer_pic[2], hole->s.x+vert->s.x, hole->s.y+vert->s.y, hole->s.x+hole->vert[vert->ref_no[5]].s.x, hole->s.y+hole->vert[vert->ref_no[5]].s.y, hole->vert[vert->ref_no[5]].ref_no[1]);
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
                        if(vert->ref_no[0] < 2){
                                for(i = 0; i < (vert->radius*2)/i_bank.object_pic[4][0]->h; i++){
                                        if(vert->ref_no[0] == 0) masked_blit(i_bank.object_pic[4][vert->ref_no[4]/vert->ref_no[2]], bp, 0, 0, hole->s.x+vert->s.x-i_bank.object_pic[4][0]->w/2, hole->s.y+vert->s.y-vert->radius+i*i_bank.object_pic[4][0]->h, i_bank.object_pic[4][0]->w, i_bank.object_pic[4][0]->h);
                                        else draw_sprite_v_flip(bp, i_bank.object_pic[4][vert->ref_no[4]/vert->ref_no[2]], hole->s.x+vert->s.x-i_bank.object_pic[4][0]->w/2, hole->s.y+vert->s.y-vert->radius+i*i_bank.object_pic[4][0]->h);
                                        }
                                }
                        else{
                                for(i = 0; i < (vert->radius*2)/i_bank.object_pic[4][0]->h; i++){
                                        if(vert->ref_no[0] == 3) rotate_sprite(bp, i_bank.object_pic[4][vert->ref_no[4]/vert->ref_no[2]], hole->s.x+vert->s.x-vert->radius+i*i_bank.object_pic[4][0]->h, hole->s.y+vert->s.y-i_bank.object_pic[4][0]->w/2, itofix(64));
                                        else rotate_sprite_v_flip(bp, i_bank.object_pic[4][vert->ref_no[4]/vert->ref_no[2]], hole->s.x+vert->s.x-vert->radius+i*i_bank.object_pic[4][0]->h, hole->s.y+vert->s.y-i_bank.object_pic[4][0]->w/2, itofix(64));
                                        }
                                }
                        break;

                case VERT_FLAG_COIN_0:
                        if(vert->ref_no[0] > 0){
                                masked_blit(i_bank.coinpic[vert->ref_no[3]/2], bp, 0, 0, hole->s.x+vert->s.x-i_bank.coinpic[vert->ref_no[3]/2]->w/2, hole->s.y+vert->s.y-i_bank.coinpic[vert->ref_no[3]/2]->h/2, i_bank.coinpic[vert->ref_no[3]/2]->w, i_bank.coinpic[vert->ref_no[3]/2]->h);
                                }
                        else{
                                if(vert->ref_no[4] > 0){
                                        vfont_printf(bp, &game_vfont, hole->s.x+vert->s.x, hole->s.y+vert->s.y, 0, "%i", vert->ref_no[5]);
                                        }
                                }
                        break;

                default:
                        break;
                }
}

//go through the display code for all verts in this hole
void hole_vert_display_all(BITMAP * bp, HOLE * hole, COURSE_IMAGE_BANK i_bank){
        int i;

        for(i = 0; i < hole->num_verts+hole->num_coins*(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT); i++){
                hole_vert_display(bp, &hole->vert[i], hole, i_bank);
                }
}

//debugging stuff
void draw_hole_debug(BITMAP * bp, HOLE * hole){
	int p;
	
	for(p = 0; p < hole->num_polys; p++){
		draw_poly2(bp, hole->body[p].poly, hole->s, 11);
		}
}

//hole background display stuff
void hole_bkg_display(BITMAP * bp, HOLE * hole, COURSE_IMAGE_BANK i_bank){

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
void ball_mirage_display(BITMAP * bp, BALL * ball){
	//set to transparent drawing mode
	drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
	//draw the circle primitive
	circlefill(bp, ball->s_med.x, ball->s_med.y, ball->radius, 15);
	//return to normal drawing mode
	solid_mode();		
}

void ball_shadow_display(BITMAP * bp, BALL * ball){
	//set to transparent drawing mode
	drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
	//draw the circle primitive
	circlefill(bp, ball->s.x+1, ball->s.y+3, ball->radius, 6);
	//return to normal drawing mode
	solid_mode();	
}

//draw this player's ball
void player_ball_display(BITMAP * bp, BALL * ball, char active, char shadows_on){
        if(active){
                if(ball->visible){
                        if(shadows_on){
	                        ball_shadow_display(bp, ball);
                                }
			//draw the ball circle
                        circlefill(bp, ball->s.x, ball->s.y, ball->radius, 15);
                        //draw the idle timer if it's active
                        if(ball->idle_timer > 0){
                                //change this to transparency later..
                                vfont_printf(bp, &game_vfont, ball->s.x, ball->s.y-ball->radius*2-ball->idle_timer/25, 0, "%i", (ball->idle_timeout+1-ball->idle_timer)/19+1);
                                }
                        }
                else{
	                //draw the splash animation if the ball is sinking
                        if(ball->mode == BALL_MODE_WATER_SINKING){
                                if(do_anim_def(bp, &ball->anim_def[1], ball->s.x-12, ball->s.y-12, 0) >= (ball->anim_def[1].num_frames-1)*ball->anim_def[1].speed){
                                        ball->anim_def[1].cur_frame = 0;   //reset the anim counter
                                        ball->mode = BALL_MODE_WATER_SUNK; //ball is now sunk
                                        }
                                }
                        }
                }
                //draw the translucent ball if necessary
                if(ball->mirage_visible){
	                ball_mirage_display(bp, ball);
                	}
        else{
	        //draw the ball as a smaller circle if it's inactive
                if(ball->visible) circlefill(bp, ball->s.x, ball->s.y, ball->radius/3, 15);
                }
}

//do the ball display code for all players' balls
void player_ball_display_all(BITMAP * bp, PLAYER pl[], int num_players, char shadows_on){
        int p;

        for(p = 0; p < num_players; p++){
                player_ball_display(bp, &pl[p].ball, (p == main_game_vars.cur_player), shadows_on);
                }
}

//draw one player's club
void player_club_display(BITMAP * bp, PLAYER * pl){
        if(pl->ball.mode == BALL_MODE_IDLE){
		//set to transparent drawing mode
	        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
		//draw the triangle primitives that define the club area
                triangle(bp, pl->ball.s.x, pl->ball.s.y, pl->ball.s.x-(MAX_CLUB_POWER/2)*cos(pl->f_club_angle+pl->f_club_width), pl->ball.s.y-(MAX_CLUB_POWER/2)*sin(pl->f_club_angle+pl->f_club_width), pl->ball.s.x-(MAX_CLUB_POWER/2)*cos(pl->f_club_angle-pl->f_club_width), pl->ball.s.y-(MAX_CLUB_POWER/2)*sin(pl->f_club_angle-pl->f_club_width), 15);
                triangle(bp, pl->ball.s.x, pl->ball.s.y, pl->ball.s.x+pl->club_v[0].x, pl->ball.s.y+pl->club_v[0].y, pl->ball.s.x+pl->club_v[1].x, pl->ball.s.y+pl->club_v[1].y, pl->club_color);
        	//return to normal drawing mode
	        solid_mode();
                }
}

//draw all players' clubs
void player_club_display_all(BITMAP * bp, PLAYER pl[], int num_players, int cur_player){
        int i;

        player_club_display(bp, &pl[cur_player]);
}

//draws the player's trajectory line
void player_trajectory_display(BITMAP * bp, PLAYER * pl, HOLE * hole){
	//only display when the trajectory is activated
	if(pl->traj_active){		
		int i;

		//set to transparent drawing mode
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
void player_hud_display(BITMAP * bp, PLAYER * pl, V_FONT * vft, HOLE * hole, int plno){
        int p;

        if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
                vfont_printf(bp, vft, pl->hud.x+vft->w*3, pl->hud.y, 0, "%s", pl->name);
                vfont_printf(bp, vft, pl->hud.x+vft->w*3+vft->w*strlen("Abe Lincoln"), pl->hud.y, 0, "coins:%i/%i", pl->coins_total, hole->num_coins);
                vfont_printf(bp, vft, pl->hud.x+vft->w*3+vft->w*strlen("Abe Lincoln coins:00/00   "), pl->hud.y, 0, "strokes:%i", pl->cur_strokes);
                vfont_printf(bp, vft, pl->hud.x+vft->w*3, pl->hud.y+vft->h, 0, "score:%i", pl->scorecard.coin_score);
                masked_blit(main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode], bp, 0, 0, pl->hud.x-5, pl->hud.y-6, main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode]->w, main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode]->h);
                }
        else{
	        //print the current stroke tally
                vfont_printf(bp, vft, pl->hud.x+vft->w*3, pl->hud.y, 0, "strokes:%i", pl->cur_strokes);

                //check to see if the char pic overlays any important stuff (hole, ball, etc)
                if(plno == 0 || plno == 2){
                        masked_blit(main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode], bp, 0, 0, pl->hud.x-5, pl->hud.y-6, main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode]->w, main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode]->h);
                        }
                else{
                        masked_blit(main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode], bp, 0, 0, pl->hud.x-5, pl->hud.y-vft->h, main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode]->w, main_game_vars.image_bank.charpic[pl->pic_ref][pl->pic_mode]->h);
                        }
                //print the player's name
                vfont_printf(bp, vft, pl->hud.x+vft->w*3, pl->hud.y+vft->h*((main_game_vars.game_mode == GAMEMODE_COIN_COLLECT)+1), 0, "%s", pl->name);

                //highlight the player's hud if he is the current player
                if(plno == main_game_vars.cur_player){
                        rect(bp, pl->hud.x+48, pl->hud.y, pl->hud.x+pl->hud.w, pl->hud.y+pl->hud.h, 12);
                        rect(bp, pl->hud.x+49, pl->hud.y-1, pl->hud.x+pl->hud.w-1, pl->hud.y+pl->hud.h-1, 13);
                        }
                }
}

//go through and display all player huds..
void player_hud_display_all(BITMAP * bp, PLAYER pl[], int num_players, V_FONT * vf, HOLE * hole){
        int p;

        for(p = 0; p < num_players; p++){
                player_hud_display(bp, &pl[p], vf, hole, p);
                }
}

//display misc. hole info
void hole_hud_display(BITMAP * bp, V_FONT * vf, HOLE * hole, int mode){
        //draw par score
        if(mode != GAMEMODE_COIN_COLLECT){
                vfont_printf_center(bp, vf, SCREEN_W/2, SCREEN_H-vf->h-1, 0, "par %i", hole->par_score);
                }

}

//handle the in-game message queue..
void game_vmessage_display(BITMAP * bp, V_MESSAGE msg, V_FONT * vf){
        int i;

        switch(msg.active){
                case 1:  //only draw the message when it's active
                        vfont_message_textout(bp, vf, &msg, msg.method, main_game_vars.ingame_timer_change);
                        break;

                default:
                        break;
                }
}

//draw the victory screen background image
void course_victory_screen_bg_display(BITMAP * bp, COURSE * cr){
	masked_blit(cr->image_bank.victory_bgpic, bp, 0, 0, 0, 0, cr->image_bank.victory_bgpic->w, cr->image_bank.victory_bgpic->h);
}

//draw the victory screen clouds
void course_victory_screen_cloud_display(BITMAP * bp, COURSE * cr, MAIN_GAME_VARS * gvars){
	int i;
	
        for(i = 3; i >= 0; i--){
                masked_blit(cr->image_bank.victory_cloudpic[i/2], bp, 0, 0, gvars->vict_cloud_x[i], gvars->vict_cloud_y[i], cr->image_bank.victory_cloudpic[i/2]->w, cr->image_bank.victory_cloudpic[i/2]->h);
                }
}

//display the victory message
void victory_screen_text_display(BITMAP * bp, V_FONT * vf, int x, int y, PLAYER * pl, COURSE * cr, MAIN_GAME_VARS * gvars, char tied){
	int i;
	char completed = 1;
                
	switch(gvars->game_mode){
        	case GAMEMODE_STROKE_PLAY:
        		if(!tied){
		                vfont_printf_center(bp, vf, x, y, 0, "Congratulations, %s!", pl->name);
        		        vfont_printf_center(bp, vf, x, y+30, 0, "Here is a trophy courtesy of");
                		vfont_printf_center(bp, vf, x, y+60, 0, "the %s club membership!", cr->name);
                	
	                	if(gvars->got_hiscore) vfont_printf_center(bp, vf, x, y+90, 0, "You got a high score!");
        	        	vfont_printf_center(bp, vf, x, SCREEN_H-100, 0, "Do come again!");
	        		}
                	break;
        	case GAMEMODE_COIN_COLLECT:

	                for(i = 0; i < cr->num_holes; i++){
        	                if(!pl->scorecard.match_won[i]) completed = 0;
                	        }
	                if(completed){
        	                vfont_printf_center(bp, vf, x, y, 0, "Coin collector mode completed.");
                	        vfont_printf_center(bp, vf, x, y+30, 0, "Nicely done, %s!", pl->name);
                        	}
	                if(main_game_vars.got_hiscore){
        	                vfont_printf_center(bp, vf, x, y+60, 0, "You got a high score!");
                	        }
                	break;
		default:
			break;
		}
}

//draw the player podium and trophy stuff
void victory_screen_player_podium_display(BITMAP * bp, int x, int y, V_FONT * vf, COURSE * cr, MAIN_GAME_VARS * gvars, PLAYER pl, int plno, int place){
	int dy, dx, p_total;
	
	//the lower the place, the higher the podium sits
	dy = (gvars->vict_pody/(gvars->num_players+1))*place;
	if(gvars->num_players == 1){
	        //podium sits in the center of the screen in single player mode
	        dx = 0;
		}
	else if(gvars->num_players == 2){
	        //podiums sit side by side centered in the middle of the screen
		if(!place) dx = -cr->image_bank.victory_podiumpic->w/2;
		else dx = cr->image_bank.victory_podiumpic->w/2;
		}
	else if(gvars->num_players >= 3){
	        //podiums sit in cross-pattern in the middle of the screen
		if(!place) dx = 0;
		else if(place == 1) dx = -3*cr->image_bank.victory_podiumpic->w/2;
		else dx = cr->image_bank.victory_podiumpic->w/2;
		}
	
	//draw podium, trophy, and player character image
	masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, x+dx-cr->image_bank.victory_podiumpic->w/2, gvars->vict_pody+dy, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
        masked_blit(cr->image_bank.victory_trophypic[place], bp, 0, 0, x+dx-cr->image_bank.victory_trophypic[0]->w/2, gvars->vict_pody+dy-cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
	masked_blit(gvars->image_bank.charpic[pl.pic_ref][1], bp, 0, 0, x+dx-main_game_vars.image_bank.charpic[pl.pic_ref][1]->w/2, gvars->vict_pody+dy+55, gvars->image_bank.charpic[pl.pic_ref][1]->w, gvars->image_bank.charpic[pl.pic_ref][1]->h);
	
	//draw place string (1st, 2nd, or 3rd)
	if(!place) vfont_printf_center(bp, vf, x+dx, gvars->vict_pody+dy+20, 0, "1st");
	else if(place == 1) vfont_printf_center(bp, vf, x+dx, gvars->vict_pody+dy+20, 0, "2nd");
	else if(place == 2) vfont_printf_center(bp, vf, x+dx, gvars->vict_pody+dy+20, 0, "3rd");
	
	switch(gvars->game_mode){
		case GAMEMODE_STROKE_PLAY:			
			//get the player's stroke total for this round
			p_total = player_course_stroke_total(pl, cr->num_holes, cr->num_holes);
			
			//display score differently depending on its relationship to the course par score
                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, x+dx, gvars->vict_pody+dy+40, 0, " e >> %i", p_total);
                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, x+dx, gvars->vict_pody+dy+40, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                        else vfont_printf_center(bp, vf, x+dx, gvars->vict_pody+dy+40, 0, "%i >> %i", p_total-cr->course_par, p_total);

			break;
		case GAMEMODE_MATCH_PLAY:
			vfont_printf_center(bp, vf, x+dx, gvars->vict_pody+dy+70, 0, "%i", pl.scorecard.matches_won);
                        
			break;
		default:
			break;
		}
}

//draws the victory screen
void victory_screen_display(BITMAP * bp, V_FONT * vf, COURSE * cr, MAIN_GAME_VARS * gvars, PLAYER pl[], int num_players){
        int i;

        course_victory_screen_bg_display(bp, cr);            //draw background layer
	course_victory_screen_cloud_display(bp, cr, gvars);  //draw clouds

	switch(gvars->num_players){
		case 1:    //single player mode..
			victory_screen_player_podium_display(bp, SCREEN_W/2, SCREEN_H/2, vf, cr, gvars, pl[0], 0, 0);
			//draw the text last..
			victory_screen_text_display(bp, vf, SCREEN_W/2, 50, &pl[0], cr, gvars, 0);
			break;
		case 2:    //2-player mode..
                        if(gvars->vict_winner[0] != -1){  //not tied, do standard screen
                        	//draw victory screen for all players
                        	for(i = 0; i < num_players; i++){
	                        	victory_screen_player_podium_display(bp, SCREEN_W/2, SCREEN_H/2, vf, cr, gvars, pl[gvars->vict_winner[i]], gvars->vict_winner[i], i);
                        		}
				//draw the text last..
				victory_screen_text_display(bp, vf, SCREEN_W/2, 100, &pl[gvars->vict_winner[0]], cr, gvars, (gvars->vict_winner[0] != -1));
				}
			else{
				}
			break;
		case 3:    //3 and 4-player mode.. (only display 3 scores)
		case 4:
                        if(gvars->vict_winner[0] != -1){  //not tied, do standard screen
                        	//draw victory screen for all players
                        	for(i = 0; i < 3; i++){
	                        	victory_screen_player_podium_display(bp, SCREEN_W/2, SCREEN_H/2, vf, cr, gvars, pl[gvars->vict_winner[i]], gvars->vict_winner[i], i);
                        		}
				//draw the text last..
				victory_screen_text_display(bp, vf, SCREEN_W/2, 100, &pl[gvars->vict_winner[0]], cr, gvars, (gvars->vict_winner[0] != -1));
				}
			else{
				}
			break;
		default:
			break;
		}        
/*	
        if(main_game_vars.num_players == 1){
                int p_total;

                //show totals
                if(main_game_vars.game_mode == GAMEMODE_STROKE_PLAY){
                        p_total = player_course_stroke_total(&game_player[0], cr->num_holes, cr->num_holes);
                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+40, 0, " e >> %i", p_total);
                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+40, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                        else vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+40, 0, "%i >> %i", p_total-cr->course_par, p_total);
                        }
                else if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
                        masked_blit(main_game_vars.image_bank.charpic[game_player[0].pic_ref][1], bp, 0, 0, SCREEN_W/2-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+85, main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w, main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->h);
                        p_total = player_course_stroke_total(&game_player[0], cr->num_holes, cr->num_holes);
                        vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+40, 0, "score:%i", game_player[0].scorecard.coin_score);
                        vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+60, 0, "best combo:%i", game_player[0].scorecard.coin_best_combo);
                        }
                }
        else{
                if(main_game_vars.num_players == 2){
                        //it's not a tie, proceed as usual..
                        if(main_game_vars.vict_winner[0] != -1){
                                masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                masked_blit(cr->image_bank.victory_trophypic[1], bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2-cr->image_bank.victory_trophypic[1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3-cr->image_bank.victory_trophypic[1]->h, cr->image_bank.victory_trophypic[1]->w, cr->image_bank.victory_trophypic[1]->h);
                                vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+20, 0, "2nd");
                                vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+35, 0, "%s", game_player[main_game_vars.vict_winner[1]].name);
                                masked_blit(main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[1]].pic_ref][0], bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2-main_game_vars.image_bank.charpic[game_player[0].pic_ref][0]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+85, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[1]].pic_ref][0]->w, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[1]].pic_ref][0]->h);

                                masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                masked_blit(cr->image_bank.victory_trophypic[0], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w/2-cr->image_bank.victory_trophypic[0]->w/2, main_game_vars.vict_pody-cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
                                vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+20, 0, "1st");
                                vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+35, 0, "%s", game_player[main_game_vars.vict_winner[0]].name);
                                masked_blit(main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[0]].pic_ref][1], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+85, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[0]].pic_ref][1]->w, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[0]].pic_ref][1]->h);
                                if(main_game_vars.game_mode == GAMEMODE_STROKE_PLAY){
                                        //print total scores
                                        int p_total;
        
                                        for(i = 0; i < main_game_vars.num_players; i++){
                                                p_total = player_course_stroke_total(&game_player[main_game_vars.vict_winner[i]], cr->num_holes, cr->num_holes);
                                                if(i == 0){
                                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, " e >> %i", p_total);
                                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, " +%i >> %i", p_total-cr->course_par, p_total);
                                                        else vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, " %i >> %i", p_total-cr->course_par, p_total);
                                                        }
                                                else{
                                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+70, 0, " e >> %i", p_total);
                                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+70, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                                        else vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+70, 0, "%i >> %i", p_total-cr->course_par, p_total);
                                                        }
                                                }
                                        }
                                else if(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY){
                                        //print total scores
                                        vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, "%i", game_player[main_game_vars.vict_winner[0]].scorecard.matches_won);
                                        vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+70, 0, "%i", game_player[main_game_vars.vict_winner[1]].scorecard.matches_won);
                                        }
                                vfont_printf_center(bp, vf, SCREEN_W/2, 100, 0, "Splendid job, %s!", game_player[main_game_vars.vict_winner[0]].name);
                                }
                        else{
                                masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2, main_game_vars.vict_pody, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                masked_blit(cr->image_bank.victory_trophypic[0], bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2-cr->image_bank.victory_trophypic[1]->w/2, main_game_vars.vict_pody-cr->image_bank.victory_trophypic[1]->h, cr->image_bank.victory_trophypic[1]->w, cr->image_bank.victory_trophypic[1]->h);
                                vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+20, 0, "T-1st");
                                vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+35, 0, "%s", game_player[0].name);
                                masked_blit(main_game_vars.image_bank.charpic[game_player[0].pic_ref][0], bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+85, main_game_vars.image_bank.charpic[game_player[0].pic_ref][0]->w, main_game_vars.image_bank.charpic[game_player[0].pic_ref][0]->h);

                                masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                masked_blit(cr->image_bank.victory_trophypic[0], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w/2-cr->image_bank.victory_trophypic[0]->w/2, main_game_vars.vict_pody-cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
                                vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+20, 0, "T-1st");
                                vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+35, 0, "%s", game_player[1].name);
                                masked_blit(main_game_vars.image_bank.charpic[game_player[1].pic_ref][0], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+85, main_game_vars.image_bank.charpic[game_player[1].pic_ref][0]->w, main_game_vars.image_bank.charpic[game_player[1].pic_ref][0]->h);
                                if(main_game_vars.game_mode == GAMEMODE_STROKE_PLAY){
                                        //print total scores
                                        int p_total = player_course_stroke_total(&game_player[0], cr->num_holes, cr->num_holes);
        
                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, " e >> %i", p_total);
                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, " +%i >> %i", p_total-cr->course_par, p_total);
                                        else vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, " %i >> %i", p_total-cr->course_par, p_total);

                                        p_total = player_course_stroke_total(&game_player[1], cr->num_holes, cr->num_holes);

                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, " e >> %i", p_total);
                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                        else vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, "%i >> %i", p_total-cr->course_par, p_total);
                                        }
                                else if(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY){
                                        //print total scores
                                        int p_total = player_course_stroke_total(&game_player[0], cr->num_holes, cr->num_holes);
        
                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+95, 0, " e >> %i", p_total);
                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+95, 0, " +%i >> %i", p_total-cr->course_par, p_total);
                                        else vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+95, 0, " %i >> %i", p_total-cr->course_par, p_total);

                                        p_total = player_course_stroke_total(&game_player[1], cr->num_holes, cr->num_holes);

                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+95, 0, " e >> %i", p_total);
                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+95, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                        else vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+95, 0, "%i >> %i", p_total-cr->course_par, p_total);

                                        //print match scores
                                        vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, "%i", game_player[main_game_vars.vict_winner[0]].scorecard.matches_won);
                                        vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, "%i", game_player[main_game_vars.vict_winner[1]].scorecard.matches_won);
                                        }
                                vfont_printf_center(bp, vf, SCREEN_W/2, 100, 0, "It's a tie!");
                                }
                        }
                else{
                        //it's not a tie, proceed as usual..
                        if(main_game_vars.vict_winner[0] != -1){
                                masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2-3*cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                masked_blit(cr->image_bank.victory_trophypic[1], bp, 0, 0, SCREEN_W/2-3*cr->image_bank.victory_podiumpic->w/2+cr->image_bank.victory_podiumpic->w/2-cr->image_bank.victory_trophypic[1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4-cr->image_bank.victory_trophypic[1]->h, cr->image_bank.victory_trophypic[1]->w, cr->image_bank.victory_trophypic[1]->h);
                                vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+20, 0, "2nd");
                                vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+35, 0, "%s", game_player[main_game_vars.vict_winner[1]].name);
                                masked_blit(main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[1]].pic_ref][0], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+85, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[1]].pic_ref][1]->w, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[1]].pic_ref][0]->h);

                                masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                masked_blit(cr->image_bank.victory_trophypic[2], bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w-cr->image_bank.victory_trophypic[1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3-cr->image_bank.victory_trophypic[2]->h, cr->image_bank.victory_trophypic[2]->w, cr->image_bank.victory_trophypic[2]->h);
                                vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+20, 0, "3rd");
                                vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+35, 0, "%s", game_player[main_game_vars.vict_winner[2]].name);
                                masked_blit(main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[2]].pic_ref][0], bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+85, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[2]].pic_ref][0]->w, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[2]].pic_ref][0]->h);

                                masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                masked_blit(cr->image_bank.victory_trophypic[0], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_trophypic[0]->w/2, main_game_vars.vict_pody-cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
                                vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+20, 0, "1st");
                                vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+35, 0, "%s", game_player[main_game_vars.vict_winner[0]].name);
                                masked_blit(main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[0]].pic_ref][1], bp, 0, 0, SCREEN_W/2-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+85, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[0]].pic_ref][1]->w, main_game_vars.image_bank.charpic[game_player[main_game_vars.vict_winner[0]].pic_ref][1]->h);
                                if(main_game_vars.game_mode == GAMEMODE_STROKE_PLAY){
                                        //print total scores
                                        int p_total;

                                        for(i = 0; i < 3; i++){
                                                p_total = player_course_stroke_total(&game_player[main_game_vars.vict_winner[i]], cr->num_holes, cr->num_holes);
                                                if(i == 0){
                                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+70, 0, " e >> %i", p_total);
                                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+70, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                                        else vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+70, 0, "%i >> %i", p_total-cr->course_par, p_total);
                                                        }
                                                else if(i == 1){
                                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+70, 0, " e >> %i", p_total);
                                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+70, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                                        else vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+70, 0, "%i >> %i", p_total-cr->course_par, p_total);
                                                        }
                                                else{
                                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+70, 0, " e >> %i", p_total);
                                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+70, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                                        else vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+70, 0, "%i >> %i", p_total-cr->course_par, p_total);
                                                        }
                                                }
                                        }
                                else if(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY){
                                        //print total scores
                                        int p_total;

                                        for(i = 0; i < 3; i++){
                                                p_total = player_course_stroke_total(&game_player[main_game_vars.vict_winner[i]], cr->num_holes, cr->num_holes);
                                                if(i == 0){
                                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+95, 0, " e >> %i", p_total);
                                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+95, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                                        else vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+95, 0, "%i >> %i", p_total-cr->course_par, p_total);
                                                        }
                                                else if(i == 1){
                                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+95, 0, " e >> %i", p_total);
                                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+95, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                                        else vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+95, 0, "%i >> %i", p_total-cr->course_par, p_total);
                                                        }
                                                else{
                                                        if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+95, 0, " e >> %i", p_total);
                                                        else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+95, 0, "+%i >> %i", p_total-cr->course_par, p_total);
                                                        else vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+95, 0, "%i >> %i", p_total-cr->course_par, p_total);
                                                        }
                                                }

                                        //print match statistics
                                        vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+70, 0, "%i", game_player[main_game_vars.vict_winner[0]].scorecard.matches_won);
                                        vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+70, 0, "%i", game_player[main_game_vars.vict_winner[1]].scorecard.matches_won);
                                        vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/3+70, 0, "%i", game_player[main_game_vars.vict_winner[2]].scorecard.matches_won);
                                        }
                                vfont_printf_center(bp, vf, SCREEN_W/2, 100, 0, "Splendid job, %s!", game_player[main_game_vars.vict_winner[0]].name);
                                }
                        else{
                                if(main_game_vars.num_players == 3){
                                        masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2-3*cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                        masked_blit(cr->image_bank.victory_trophypic[0], bp, 0, 0, SCREEN_W/2-3*cr->image_bank.victory_podiumpic->w/2+cr->image_bank.victory_trophypic[0]->w/2, main_game_vars.vict_pody-cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
                                        vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+20, 0, "T-1st");
                                        vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+35, 0, "%s", game_player[0].name);
                                        masked_blit(main_game_vars.image_bank.charpic[game_player[0].pic_ref][0], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+85, main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w, main_game_vars.image_bank.charpic[game_player[0].pic_ref][0]->h);

                                        masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                        masked_blit(cr->image_bank.victory_trophypic[0], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_trophypic[0]->w/2, main_game_vars.vict_pody-cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
                                        vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+20, 0, "T-1st");
                                        vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+35, 0, "%s", game_player[1].name);
                                        masked_blit(main_game_vars.image_bank.charpic[game_player[1].pic_ref][0], bp, 0, 0, SCREEN_W/2-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+85, main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w, main_game_vars.image_bank.charpic[game_player[0].pic_ref][0]->h);

                                        masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                        masked_blit(cr->image_bank.victory_trophypic[0], bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w-cr->image_bank.victory_trophypic[0]->w/2, main_game_vars.vict_pody-cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
                                        vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+20, 0, "T-1st");
                                        vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+35, 0, "%s", game_player[2].name);
                                        masked_blit(main_game_vars.image_bank.charpic[game_player[2].pic_ref][0], bp, 0, 0, SCREEN_W/2+cr->image_bank.victory_podiumpic->w-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+85, main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w, main_game_vars.image_bank.charpic[game_player[0].pic_ref][0]->h);
                                        if(main_game_vars.game_mode == GAMEMODE_STROKE_PLAY){
                                                //print total scores
                                                int p_total;
                                
                                                for(i = 0; i < 3; i++){
                                                        p_total = player_course_stroke_total(&game_player[i], cr->num_holes, cr->num_holes);
                                                        if(i == 0){
                                                                if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+70, 0, "e");
                                                                else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+70, 0, "+%i", p_total-cr->course_par);
                                                                else vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+70, 0, "%i", p_total-cr->course_par);
                                                                }
                                                        else if(i == 1){
                                                                if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+70, 0, "e");
                                                                else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+70, 0, "+%i", p_total-cr->course_par);
                                                                else vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+70, 0, "%i", p_total-cr->course_par);
                                                                }
                                                        else{
                                                                if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+70, 0, "e");
                                                                else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+70, 0, "+%i", p_total-cr->course_par);
                                                                else vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+70, 0, "%i", p_total-cr->course_par);
                                                                }
                                                        }
                                                }
                                        else if(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY){
                                                //print total scores
                                                int p_total;
                                
                                                for(i = 0; i < 3; i++){
                                                        p_total = player_course_stroke_total(&game_player[i], cr->num_holes, cr->num_holes);
                                                        if(i == 0){
                                                                if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+95, 0, "e");
                                                                else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+95, 0, "+%i", p_total-cr->course_par);
                                                                else vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+95, 0, "%i", p_total-cr->course_par);
                                                                }
                                                        else if(i == 1){
                                                                if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+95, 0, "e");
                                                                else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+95, 0, "+%i", p_total-cr->course_par);
                                                                else vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+95, 0, "%i", p_total-cr->course_par);
                                                                }
                                                        else{
                                                                if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+95, 0, "e");
                                                                else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+95, 0, "+%i", p_total-cr->course_par);
                                                                else vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+95, 0, "%i", p_total-cr->course_par);
                                                                }
                                                        }
                                                vfont_printf_center(bp, vf, SCREEN_W/2-cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+70, 0, "%i", game_player[0].scorecard.matches_won);
                                                vfont_printf_center(bp, vf, SCREEN_W/2, main_game_vars.vict_pody+70, 0, "%i", game_player[1].scorecard.matches_won);
                                                vfont_printf_center(bp, vf, SCREEN_W/2+cr->image_bank.victory_podiumpic->w, main_game_vars.vict_pody+70, 0, "%i", game_player[2].scorecard.matches_won);
                                                }
                                        }
                                else{
                                        int p, p_total;

                                        for(p = 0; p < main_game_vars.num_players; p++){
                                                masked_blit(cr->image_bank.victory_podiumpic, bp, 0, 0, (cr->image_bank.victory_podiumpic->w-20)*p, main_game_vars.vict_pody, cr->image_bank.victory_podiumpic->w, cr->image_bank.victory_podiumpic->h);
                                                masked_blit(cr->image_bank.victory_trophypic[0], bp, 0, 0, (cr->image_bank.victory_podiumpic->w-20)*p-cr->image_bank.victory_trophypic[0]->w/2, main_game_vars.vict_pody-cr->image_bank.victory_trophypic[0]->h, cr->image_bank.victory_trophypic[0]->w, cr->image_bank.victory_trophypic[0]->h);
                                                vfont_printf_center(bp, vf, (cr->image_bank.victory_podiumpic->w-20)*p+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+20, 0, "T-1st");
                                                vfont_printf_center(bp, vf, (cr->image_bank.victory_podiumpic->w-20)*p+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+35, 0, "%s", game_player[0].name);
                                                masked_blit(main_game_vars.image_bank.charpic[game_player[p].pic_ref][0], bp, 0, 0, SCREEN_W/2-cr->image_bank.victory_podiumpic->w-main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w/2, main_game_vars.vict_pody+cr->image_bank.victory_podiumpic->h/4+85, main_game_vars.image_bank.charpic[game_player[0].pic_ref][1]->w, main_game_vars.image_bank.charpic[game_player[0].pic_ref][0]->h);

                                                //print total scores
                                                p_total = player_course_stroke_total(&game_player[i], cr->num_holes, cr->num_holes);
                                                if(p_total-cr->course_par == 0) vfont_printf_center(bp, vf, (cr->image_bank.victory_podiumpic->w-20)*p+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70+25*(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY), 0, "e");
                                                else if(p_total-cr->course_par > 0) vfont_printf_center(bp, vf, (cr->image_bank.victory_podiumpic->w-20)*p+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70+25*(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY), 0, "+%i", p_total-cr->course_par);
                                                else vfont_printf_center(bp, vf, (cr->image_bank.victory_podiumpic->w-20)*p+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70+25*(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY), 0, "%i", p_total-cr->course_par);

                                                //print match totals
                                                if(main_game_vars.game_mode == GAMEMODE_MATCH_PLAY){
                                                        vfont_printf_center(bp, vf, (cr->image_bank.victory_podiumpic->w-20)*p+cr->image_bank.victory_podiumpic->w/2, main_game_vars.vict_pody+70, 0, "%i", game_player[p].scorecard.matches_won);
                                                        }
                                                }
                                        }
                                vfont_printf_center(bp, vf, SCREEN_W/2, 100, 0, "It's a tie..");
                                }
                        }
                }
*/
}


//draws the course scoreboard
void course_scoreboard_display(BITMAP * bp, V_FONT * vf, COURSE * cr, PLAYER pl[], int num_players, int cur_hole){
	int s, i, seg_h = bp->h/cr->scoreboard.num_scoreboard_segs;
	
	for(s = 0; s < cr->scoreboard.scoreboard_seg_count; s++){
		masked_blit(cr->scoreboard.bgpic, bp, 0, 0, 0, s*seg_h, cr->scoreboard.bgpic->w, cr->scoreboard.bgpic->h);
		}

        for(i = 0; i < num_players; i++){
	        int n, p_total, c_total;

	        //only draw this stuff when it appears within the visible scoreboard area
	        if(cr->scoreboard.y+cr->scoreboard.grid_y[i] <= cr->scoreboard.scoreboard_seg_count*seg_h){
	                masked_blit(cr->scoreboard.gridpic, bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i], cr->scoreboard.y+cr->scoreboard.grid_y[i], cr->scoreboard.gridpic->w, cr->scoreboard.gridpic->h);
        	        vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i], cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.gridpic->h+10, 0, "P%i: %s", i+1, pl[i].name);
	        	}
                
                switch(main_game_vars.game_mode){
	                case GAMEMODE_STROKE_PLAY:
			        //only draw this stuff when it appears within the visible scoreboard area
			        if(cr->scoreboard.y+cr->scoreboard.grid_y[i] > cr->scoreboard.scoreboard_seg_count*seg_h) break;
				        
	                	//current totals
                                p_total = player_course_stroke_total(pl[i], cur_hole+1, cr->num_holes);
                                c_total = course_stroke_total(pl[i], cur_hole+1, cr->num_holes);
                                                                
                                //draw player's current stroke tally
                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "%i", c_total);
                                //draw course par tally
                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2, 0, "%i", p_total);
                                
                                //draw golf-styled display of player's score
                                if(p_total-c_total == 0) vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3, 0, " E");
                                else if(p_total-c_total > 0) vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3, 0, "+%i", p_total-c_total);
                                else vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3, 0, "%i", p_total-c_total);

                                //draw the numbers for the holes
                                for(n = 0; n < 9; n++){
                                        masked_blit(cr->scoreboard.numpic[n+1], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                        if(n <= cur_hole){
                                                masked_blit(cr->scoreboard.numpic[pl[i].scorecard.par_score[n]], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                if(pl[i].scorecard.score[n] < 10){
                                                        masked_blit(cr->scoreboard.numpic[pl[i].scorecard.score[n]], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                        }
                                                else vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+1, 0, "X");
                                                }

                                        //if there are more than 9 holes, draw the numbers for the back 9
                                        if(cr->num_holes > 9){
                                                masked_blit(cr->scoreboard.numpic[n+10], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                if(n+9 <= cur_hole){
                                                        if(pl[i].scorecard.score[n+9] < 10){
                                                                masked_blit(cr->scoreboard.numpic[pl[i].scorecard.score[n+9]], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                                }
                                                        else vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+1, 3*vf->w/4, "X");
                                                        masked_blit(cr->scoreboard.numpic[pl[i].scorecard.par_score[n+9]], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                        }
                                                }
                                        }
                                //draw individual player scores
	                	break;
	                case GAMEMODE_MATCH_PLAY:
	        	        //only draw this stuff when it appears within the visible scoreboard area
			        if(cr->scoreboard.y+cr->scoreboard.grid_y[i] > cr->scoreboard.scoreboard_seg_count*seg_h) break;
			        
		                //current totals
                                p_total = player_course_stroke_total(pl[i], cur_hole, cr->num_holes);
                                c_total = course_stroke_total(pl[i], cur_hole, cr->num_holes);
                                if(p_total-c_total == 0) vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "e");
                                else if(p_total-c_total > 0) vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "+%i", p_total-c_total);
                                else vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "%i", p_total-c_total);

                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2, 0, "%i", pl[i].scorecard.matches_won);

                                for(n = 0; n < 9; n++){
                                        masked_blit(cr->scoreboard.numpic[n+1], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+1, cr->scoreboard.num_w, cr->scoreboard.num_h);

                                        if(n <= main_game_vars.cur_hole){
                                                //print stroke total
                                                if(pl[i].scorecard.score[n] < 10){
                                                        masked_blit(cr->scoreboard.numpic[pl[i].scorecard.score[n]], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                        }
                                                else vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+1, 0, "X");

                                                //print win-loss statistic
                                                if(pl[i].scorecard.match_won[n] == 0){
                                                        vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+1, 0, "L");
                                                        }
                                                else if(pl[i].scorecard.match_won[n] == 1){
                                                        vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+1, 0, "W");
                                                        }
                                                else{
                                                        vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+1, 0, "T");
                                                        }
                                                }

                                        if(cr->num_holes > 9){
                                                masked_blit(cr->scoreboard.numpic[n+10], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                if(n+9 <= main_game_vars.cur_hole){
                                                        //print stroke total
                                                        if(pl[i].scorecard.score[n+9] < 10){
                                                                masked_blit(cr->scoreboard.numpic[pl[i].scorecard.score[n+9]], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                                }
                                                        else vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+1, 3*vf->w/4, "X");

                                                        //print win-loss statistic
                                                        if(pl[i].scorecard.match_won[n+9] == 0){
                                                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+1, 0, "L");
                                                                }
                                                        else if(pl[i].scorecard.match_won[n+9] == 1){
                                                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+1, 0, "W");
                                                                }
                                                        else{
                                                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+1, 0, "T");
                                                                }
                                                        }
                                                }
                                        }
                                //draw individual player scores
                                if(main_game_vars.hole_finished){
                                        if(pl[i].scorecard.match_won[main_game_vars.cur_hole] == 1){
                                                vfont_printf_center(bp, vf, SCREEN_W/2, SCREEN_H/2-40, 0, "Match goes to %s!", pl[i].name);
                                                }
                                        else if(pl[i].scorecard.match_won[main_game_vars.cur_hole] == 2){
                                                vfont_printf_center(bp, vf, SCREEN_W/2, SCREEN_H/2-40, 0, "It's a tie..");
                                                }
                                        }
                
	                	break;
	                case GAMEMODE_COIN_COLLECT:
	        		//only draw this stuff when it appears within the visible scoreboard area
	        		if(cr->scoreboard.y+cr->scoreboard.grid_y[i] > cr->scoreboard.scoreboard_seg_count*seg_h) break;
		                
	        		//current totals
                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+cr->scoreboard.num_w*9+5, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h, 0, "%i", pl[i].scorecard.coin_score);

                                for(n = 0; n < 9; n++){
                                        masked_blit(cr->scoreboard.numpic[n+1], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                        if(n <= main_game_vars.cur_hole){
                                                rectfill(bp, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+1, cr->scoreboard.x+cr->scoreboard.grid_x[i]+(n+1)*cr->scoreboard.num_w-1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2-1, 14+(29+50*(pl[i].scorecard.score[n] != pl[i].scorecard.par_score[n]))*(pl[i].scorecard.par_score[n] != 0));
                                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+3, 2*vf->w/3, "%i", pl[i].scorecard.score[n]/10);
                                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1+vf->w/2, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h+3, 2*vf->w/3, "%i", pl[i].scorecard.score[n]%10);
                                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+3, 2*vf->w/3, "%i", pl[i].scorecard.par_score[n]/10);
                                                vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1+vf->w/2, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*2+3, 2*vf->w/3, "%i", pl[i].scorecard.par_score[n]%10);
                                                }

                                        if(cr->num_holes > 9){
                                                masked_blit(cr->scoreboard.numpic[n+10], bp, 0, 0, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*3+1, cr->scoreboard.num_w, cr->scoreboard.num_h);
                                                if(n+9 <= main_game_vars.cur_hole){
                                                        rectfill(bp, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+1, cr->scoreboard.x+cr->scoreboard.grid_x[i]+(n+1)*cr->scoreboard.num_w-1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5-1, 14+(29+50*(pl[i].scorecard.score[n+9] != pl[i].scorecard.par_score[n+9]))*(pl[i].scorecard.par_score[n+9] != 0));
                                                        vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+3, 2*vf->w/3, "%i", pl[i].scorecard.score[n+9]/10);
                                                        vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1+vf->w/2, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*4+3, 2*vf->w/3, "%i", pl[i].scorecard.score[n+9]%10);
                                                        vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+3, 2*vf->w/3, "%i", pl[i].scorecard.par_score[n+9]/10);
                                                        vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.grid_x[i]+n*cr->scoreboard.num_w+1+vf->w/2, cr->scoreboard.y+cr->scoreboard.grid_y[i]+cr->scoreboard.num_h*5+3, 2*vf->w/3, "%i", pl[i].scorecard.par_score[n+9]%10);
                                                        }
                                                }
                                        }
	                	break;
	               	default:
	               		break;
                                }
                        }
        //identify next hole.. (only display if it lies within the visible scoreboard area)
        if(cr->scoreboard.y+cr->scoreboard.menu_y[0] <= cr->scoreboard.scoreboard_seg_count*seg_h){
        	vfont_printf_center(bp, vf, bp->w/2, cr->scoreboard.y+cr->scoreboard.menu_y[0]-30, 0, "Press enter to continue");
		}
	if(cr->scoreboard.x+cr->scoreboard.menu_x[1] <= cr->scoreboard.scoreboard_seg_count*seg_h){
        	if(cur_hole <= cr->num_holes){
                	vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.menu_x[1], cr->scoreboard.y+cr->scoreboard.menu_y[1], 0, "Next hole: %i", cur_hole+2);
                	}
        	else{
                	vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.menu_x[1]-110, cr->scoreboard.y+cr->scoreboard.menu_y[1], 0, "Next: awards ceremony");
                	}
        	}
                
        //identify course name (only display if it lies within the visible scoreboard area)
        if(cr->scoreboard.y+cr->scoreboard.cname_y <= cr->scoreboard.scoreboard_seg_count*seg_h){
		vfont_printf(bp, vf, cr->scoreboard.x+cr->scoreboard.cname_x, cr->scoreboard.y+cr->scoreboard.cname_y, 0, "Course: %s", cr->name);
		}
}

//the main display stuff
void main_game_display(BITMAP * bp, PLAYER pl[], int num_players, COURSE * cr){
        int i;

        //clear the screen buffer before doing anything
        clear_to_color(bp, 0);

        if(main_game_vars.cur_hole >= 0){  //make sure the current hole isn't negative before doing anything
	        //draw the hole image
                hole_bkg_display(bp, &cr->current_hole, cr->image_bank);

                //draw all player balls
        	player_ball_display_all(bp, pl, num_players, game_config.fx_on);

        	//draw the foreground layer
        	hole_fg_display(bp, &cr->current_hole);

        	//draw the player clubs
        	player_club_display_all(bp, pl, num_players, main_game_vars.cur_player);
        	
        	//draw the current player's trajectory if active
        	player_trajectory_display(bp, &pl[main_game_vars.cur_player], &cr->current_hole);
		
        	//draw the player heads up displays
        	player_hud_display_all(bp, pl, num_players, &game_vfont, &cr->current_hole);

        	//draw the hole heads up display
        	hole_hud_display(bp, &game_vfont, &cr->current_hole, main_game_vars.game_mode);
		}
        	
	switch(main_game_vars.game_state){
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

		
/*
        draw_trans_layer(bp, trans_bp);
        if(main_game_vars.game_state == GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGMID || main_game_vars.game_state == GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGMID || main_game_vars.game_state == GAMESTATE_COURSE_COMPLETED_1P_VICTORY_SEGIN || main_game_vars.game_state == GAMESTATE_COURSE_COMPLETED_XP_VICTORY_SEGIN){
                draw_course_victory_screen(bp, trans_bp, &game_vfont, &game_course);
                }
        if(main_game_vars.game_state == GAMESTATE_COIN_COLLECT_FINISHED){
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80, 0, "Coin Collection Complete!");
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+game_vfont.h+2, 0, "Completion bonus: %ipts", COIN_COMPLETE_BONUS);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*2, 0, "%s", game_player[main_game_vars.cur_player].name);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*3, 0, "----------");
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*4, 0, "Coins:%i/%i", game_player[main_game_vars.cur_player].coins_total, hole->num_coins);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*5, 0, "Strokes:%i (%ipts)", game_player[main_game_vars.cur_player].cur_strokes, COIN_COMPLETE_BONUS/game_player[main_game_vars.cur_player].cur_strokes);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*6, 0, "Score:%i", game_player[main_game_vars.cur_player].scorecard.coin_score);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*7, 0, "Best combo:%i", game_player[main_game_vars.cur_player].coin_best_combo);
                }
        else if(main_game_vars.game_state == GAMESTATE_COIN_COLLECT_FAILED_STROKES){
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80, 0, "Coin Collection Failed..");
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+game_vfont.h+2, 0, "Too many strokes");
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*2, 0, "%s", game_player[main_game_vars.cur_player].name);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*3, 0, "----------");
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*4, 0, "Coins:%i/%i", game_player[main_game_vars.cur_player].coins_total, hole->num_coins);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*5, 0, "Strokes:%i (0pts)", game_player[main_game_vars.cur_player].cur_strokes);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*6, 0, "Score:%i", game_player[main_game_vars.cur_player].scorecard.coin_score);
                vfont_printf_center(bp, &game_vfont, SCREEN_W/2, SCREEN_H/2-80+(game_vfont.h+2)*7, 0, "Best combo:%i", game_player[main_game_vars.cur_player].coin_best_combo);
                }
*/
	//draw the message found in slot 0 (this is the current message)
        game_vmessage_display(bp, game_vmessage[0], &game_vfont);
}

//logic related to menu cursor
void menu_cursor_logic(MENU_CURSOR * mcur, CMENU * mn){
        //make the cursor go up and down a little bit for effect
        if(mcur->active == 1){
                if(++mcur->offset_counter >= 30) mcur->active = 2;
                }
        else if(mcur->active == 2){
                if(--mcur->offset_counter <= -30) mcur->active = 1;
                }

        //keep the cursor near the item currently selected
	mcur->x = mn->x+mn->menu_item[mn->cur_item]->x-3*mcur->radius/2;
        mcur->y = mn->y+mn->menu_item[mn->cur_item]->y+mcur->radius;
}

//set the sound quality based on the current menu list selection
void menu_set_sound_quality(int ref){
	game_config.sound_quality = ref;
}

//change music volume based on current menu selection
void menu_set_sound_music_volume(int ref){
	game_config.sound_music_volume = ref;
}

//change sound effect volume based on current menu selection
void menu_set_sound_sfx_volume(int ref){
	game_config.sound_sfx_volume = ref;
}

//change the shadows setting via menu
void menu_toggle_shadows(int ref){
	game_config.fx_on = !ref;
}

//change the huds setting via menu
void menu_toggle_hide_huds(int ref){
	game_config.reset_huds = !ref;
}

//set the player's pic based on the menu setting
void menu_set_player_pic(PLAYER * pl, int ref){
	pl->pic_ref = pl->scorecard.pic_ref = ref;
}
	
//set the player's keys based on the menu setting
void menu_set_player_predefined_keys(CMENU * mn){
	if(mn->menu_item[mn->cur_item]->item_ref_set < 5){
		//if the current selection is < 5, it means the chosen preset is keyboard or mouse
        	game_player[mn->cur_item].controller = main_game_vars.controller[mn->menu_item[mn->cur_item]->item_ref_set];
                }
	else if(num_joysticks > 0){
		//if the current selection is >= 5, it means the chosen preset is one of the joysticks found
               game_player[mn->cur_item].controller.type = CONTROLLER_JOYSTICK_1+mn->menu_item[mn->cur_item]->item_ref_set-5;
               }
}

//change to a custom key input for this player based on the menu selection
void menu_set_player_custom_keys(PLAYER * pl, CMENU * mn){
	switch(mn->cur_item){
		case 0:   //left key define
			pl->controller.key_left = mn->menu_item[mn->cur_item]->item_ref_set;
			break;
		case 1:   //right key define
			pl->controller.key_right = mn->menu_item[mn->cur_item]->item_ref_set;
			break;
		case 2:   //swing key define
			pl->controller.key_swing_pri = mn->menu_item[mn->cur_item]->item_ref_set;
			break;
		case 3:   //hud key define
			pl->controller.key_swing_alt = mn->menu_item[mn->cur_item]->item_ref_set;
			break;
		default:
			break;
		}
	//controller type is set as "custom"
	pl->controller.type = CONTROLLER_KEYBOARD_X;
}

//load the course preview based on the menu filename parameter
int menu_load_course_preview(COURSE * cr, char * filename){
	return load_course_preview(cr, filename);
}

//this is the sequence that occurs in the transition between the title and the game
void do_title_game_transition(void){
        main_game_vars.quit_game = 0;                       //quit game = false
        main_game_vars.quit_title = 1;                      //quit title = true
        main_game_vars.game_state = GAMESTATE_SCOREBOARD_SEGIN;    //we're ready for the scoreboard seg-in sequence
        main_game_vars.game_state_old = main_game_vars.game_state;
        main_game_vars.cur_vmessage_slot = 0;                      //reset the message slot to 0
        main_game_vars.cur_player = 0;                             //give player 1 the first shot
        set_victory_screen_vars(&main_game_vars);                  //set the victory screen up
        main_game_vars.cur_hole = -1;                              //current hole is -1 (means we're just starting up)
}
	
//load the course based on the menu filename parameter, then get ready to exit the menu loop and start the game loop
int menu_load_course(COURSE * cr, char * filename){
	int exit_code;
	char temp_string[32];

	//unload memory taken up by previous course loaded (if exists)
	deinitialize_course(cr);	
	//load course from filename
	exit_code = load_course(cr, filename);
	
	//start the course's music (if exists)
	start_new_music(replace_extension(temp_string, filename, "xm", 32));
	
	main_game_vars.quit_title = 1;      //exit title/frontend
	main_game_vars.quit_game = 0;       //allow game to start
	main_game_vars.quit_all = 0;        //don't exit the program yet
	
	do_title_game_transition();         //start the title->game transition
		
	free(temp_string);
	return exit_code;
}
	
//read a string of text for getting the player's character name
void menu_get_string(V_FONT * vf, CMENU * mn){
        char * temp_string;

        //clear the keyboard buffer before getting the text input
        clear_keybuf();   
        //get a string from keyboard input
        temp_string = get_string(vf, 16, mn->x+mn->menu_item[mn->cur_item+1]->x, mn->y+mn->menu_item[mn->cur_item+1]->y, 42);
        //copy the temp string to the menu item's string to save it for later use
        strcpy(mn->menu_item[mn->cur_item+1]->text, temp_string);

        //unload temp string from memory
        free(temp_string);
}

//game mode was changed in the menu
void menu_change_game_mode(CMENU * mn){
	if(main_game_vars.num_players < 2){   //if only one player, skip over match play
        	if(mn->menu_item[mn->cur_item]->item_ref_set == 1) mn->menu_item[mn->cur_item]->item_ref_set = (key[KEY_RIGHT] != 0)*2;
                }
	else{
		if(mn->menu_item[mn->cur_item]->item_ref_set == 2) mn->menu_item[mn->cur_item]->item_ref_set--;
                }
	main_game_vars.game_mode = mn->menu_item[mn->cur_item]->item_ref_set;
}

//# of players changed via menu
void menu_change_num_players(CMENU * mn){
	main_game_vars.num_players = mn->menu_item[mn->cur_item]->item_ref_set+1;
        if(main_game_vars.num_players < 2) mn->menu_item[mn->cur_item+1]->item_ref_set = main_game_vars.game_mode = 0;
        else mn->menu_item[mn->cur_item+1]->item_ref_set = main_game_vars.game_mode = 0;
}

//get key input for main menu..
int get_main_menu_keys(CMENU * mn, CMENU ** mnp){
	switch(get_menu_keys(&main_menu_cursor, mnp)){
                case MENU_QUIT_FRONTEND:             //exit the program
                        main_game_vars.quit_title = main_game_vars.quit_game = main_game_vars.quit_all = 1;
                        break;
                case 0:  //standard
                        break;
                case MENU_CHANGE_ITEM_SELECTION:   //new item selected
                	//cue the sound effect
                	play_item_select_sample(); 
                	break;
                case MENU_CHANGE_TO_SUBMENU:  	   //goto submenu
			//cue the sound effect
                	play_select_sample(); 
                	break;
                case MENU_CHANGE_TO_LASTITEM:      //selects last item on the main menu when esc hit
                case MENU_CHANGE_TO_ROOTMENU:      //goto root menu
			//cue the sound effect
                	play_esc_sample(); 
                	break;
                case MENU_CHANGE_SOUND_QUALITY:      //the sound quality setting was changed, set it in the config 
                	//cue the sound effect
                	play_slider_sample(); 
                	menu_set_sound_quality(mn->menu_item[mn->cur_item]->item_ref_set);
                	break;
                case MENU_CHANGE_SOUND_MUSIC_VOLUME: //the music volume was changed, set it in the config 
                	//cue the sound effect
                	play_slider_sample(); 
                	menu_set_sound_music_volume(mn->menu_item[mn->cur_item]->item_ref_set);
                	break;
                case MENU_CHANGE_SOUND_SFX_VOLUME:   //the sfx volume was changed, set it in the config
                	//cue the sound effect
                	play_slider_sample(); 
                	menu_set_sound_sfx_volume(mn->menu_item[mn->cur_item]->item_ref_set);
                	break;
                case MENU_CHANGE_FX_SHADOWS:         //the shadows option was toggled, set it in the config
                	//cue the sound effect
                	play_slider_sample(); 
                	menu_toggle_shadows(mn->menu_item[mn->cur_item]->item_ref_set);
                	break;
                case MENU_CHANGE_RESET_HUDS:         //the reset HUDs option was toggled, set it in the config
                	//cue the sound effect
                	play_slider_sample(); 
                	menu_toggle_hide_huds(mn->menu_item[mn->cur_item]->item_ref_set);
                	break;
               	case MENU_CHANGE_PLAYER_1_PIC:       //the player has chosen a new pic, change the reference to it in the player's vars
                	//cue the sound effect
                	play_slider_sample(); 
			menu_set_player_pic(&game_player[0], mn->menu_item[mn->cur_item]->item_ref_set);
               		break;
               	case MENU_CHANGE_PLAYER_2_PIC:
                	//cue the sound effect
                	play_slider_sample(); 
			menu_set_player_pic(&game_player[1], mn->menu_item[mn->cur_item]->item_ref_set);
               		break;
               	case MENU_CHANGE_PLAYER_3_PIC:
                	//cue the sound effect
                	play_slider_sample(); 
			menu_set_player_pic(&game_player[2], mn->menu_item[mn->cur_item]->item_ref_set);
               		break;
               	case MENU_CHANGE_PLAYER_4_PIC:
                	//cue the sound effect
                	play_slider_sample(); 
			menu_set_player_pic(&game_player[3], mn->menu_item[mn->cur_item]->item_ref_set);
               		break;
               	case MENU_CHANGE_PLAYER_1_NAME:      //unused right now..
               		break;
               	case MENU_CHANGE_PLAYER_2_NAME:
               		break;
               	case MENU_CHANGE_PLAYER_3_NAME:
               		break;
               	case MENU_CHANGE_PLAYER_4_NAME:
               		break;
               	case MENU_DEFINE_KEYS_PLAYER_1:
               	case MENU_DEFINE_KEYS_PLAYER_2:
               	case MENU_DEFINE_KEYS_PLAYER_3:
               	case MENU_DEFINE_KEYS_PLAYER_4:      //set the player's keys to one of the predefine key configs
                	//cue the sound effect
                	play_slider_sample(); 
               		main_game_vars.cur_player = abs(mn->menu_item[mn->cur_item]->item_ref)-1;
			menu_set_player_predefined_keys(mn);
               		break;
               	case MENU_CUSTOM_KEY_OFFSET+MENU_DEFINE_KEYS_PLAYER_1:
               	case MENU_CUSTOM_KEY_OFFSET+MENU_DEFINE_KEYS_PLAYER_2:
               	case MENU_CUSTOM_KEY_OFFSET+MENU_DEFINE_KEYS_PLAYER_3:
               	case MENU_CUSTOM_KEY_OFFSET+MENU_DEFINE_KEYS_PLAYER_4:   //go to the custom key menu for this player
                	//cue the sound effect
                	play_select_sample(); 
               		main_game_vars.cur_player = abs(mn->menu_item[mn->cur_item]->item_ref-MENU_CUSTOM_KEY_OFFSET)-1;
               		*mnp = mn->submenu[mn->menu_item[mn->cur_item]->item_ref_end];
               		break;
               	case MENUITEM_KEYDEFINE:             //set player custom keys
			menu_set_player_custom_keys(&game_player[mn->cur_item], mn);
               		break;
               	case MENU_LOAD_COURSE_PREVIEW_NEW:   //load a course preview if none is yet loaded
               		if(!strcmp(game_course.name, "")){   //if no course is loaded, load the course preview for the first item
               			menu_load_course_preview(&game_course, mn->menu_item[mn->cur_item]->text);
       				}
               		break;
               	case MENU_LOAD_COURSE_PREVIEW:       //load a course preview for the selected course
                	//cue the sound effect
                	play_item_select_sample(); 
               		menu_load_course_preview(&game_course, mn->menu_item[mn->cur_item]->text);
               		break;
               	case MENU_CHANGE_PLAYER_NAME:        //let the player type in a new name and store it
               		key[KEY_ENTER] = 0;
	               	menu_get_string(&game_vfont, mn);
	               	key[KEY_ENTER] = 0;               		
               	case MENU_CHANGE_PLAYER_SELECTION:
			//cue the sound effect
                	play_item_select_sample(); 
               		{
               			int i = mn->cur_item/3;   //set the player # to mess with
				//copy the input string to the player's name
                		strcpy(game_player[i].name, mn->menu_item[mn->cur_item+1]->text);
                		strcpy(game_player[i].scorecard.name, game_player[i].name);
                		
                		//check to see if there is a saved statsheet using this name
                		main_game_vars.cur_statsheet = player_statsheet_check(game_player[i].name);
	               		if(main_game_vars.cur_statsheet != -1){
                			//if a saved statsheet was found, give the player the character pic of the saved stats
                        		game_player[i].pic_ref = game_player[i].scorecard.pic_ref = mn->menu_item[mn->cur_item+2]->item_ref_set = main_game_vars.player_statsheet[main_game_vars.cur_statsheet].pic_ref;
                        		}
                		}
               		break;
               	case MENU_CHANGE_GAME_MODE:          //game mode was changed via menu..
			//cue the sound effect
               		play_slider_sample(); 
			menu_change_game_mode(mn);
                	break;
               	case MENU_CHANGE_NUMBER_OF_PLAYERS:  //# of players changed via menu..
			//cue the sound effect
               		play_slider_sample(); 
			menu_change_num_players(mn);
                	break;
               	case MENU_LOAD_COURSE:               //load the selected course into memory and exit to the game loop
			//cue the sound effect
               		play_select_sample(); 
               		menu_load_course(&game_course, mn->menu_item[mn->cur_item]->text);
               		break;
                default:
                        break;
                }
	
        return 0;
}

//menu logic stuff..
void main_menu_logic(CMENU ** mn, MENU_CURSOR * mcur){
	//handle keyboard input for this menu
        if(get_main_menu_keys(*mn, mn)){
        	}
        
        //handle logic for the cursor
        menu_cursor_logic(mcur, *mn);
}

//draw one top score
void draw_course_top_score(BITMAP * bp, V_FONT * vf, SCORECARD sc, int num_holes, int x, int y, int strokes, int score, int num){
        int holes_in_one = 0;

        //draw the character pic for this top score
        masked_stretch_blit(main_game_vars.image_bank.charpic[sc.pic_ref][0], bp, 0, 0, main_game_vars.image_bank.charpic[sc.pic_ref][0]->w, main_game_vars.image_bank.charpic[sc.pic_ref][0]->h, x+vf->w*2, y, main_game_vars.image_bank.charpic[sc.pic_ref][0]->w/2, main_game_vars.image_bank.charpic[sc.pic_ref][0]->h/2);
        vfont_printf(bp, vf, x, y, 0, "%i)", num);

        //print the player's name for this top score
        vfont_printf(bp, vf, x+vf->w*4, y, 0, "%s", sc.name);

	//print this scorecard's score in golf format
        if(strokes == 0) vfont_printf(bp, vf, x+vf->w*4, y+vf->h, 0, "Score:  E / %i", score);
        else if(strokes > 0) vfont_printf(bp, vf, x+vf->w*4, y+vf->h, 0, "Score: +%i / %i", strokes, score);
        else vfont_printf(bp, vf, x+vf->w*4, y+vf->h, 0, "Score: %i / %i", strokes, score);
        
        //display the statbar (shows # of birdies, eagles, albatrosses, and holes in one attained during this round)
        masked_blit(main_game_vars.image_bank.statbarpic, bp, 0, 0, x+vf->w*4, y+vf->h*2, main_game_vars.image_bank.statbarpic->w, main_game_vars.image_bank.statbarpic->h);
        vfont_printf(bp, vf, x+vf->w*4+40, y+vf->h*2, 4*vf->w/5, "%i", sc.num_birdies);
        vfont_printf(bp, vf, x+vf->w*4+76, y+vf->h*2, 4*vf->w/5, "%i", sc.num_eagles);
        vfont_printf(bp, vf, x+vf->w*4+112, y+vf->h*2, 4*vf->w/5, "%i", sc.num_albo);
        {
                int i;
                for(i = 0; i < num_holes; i++){
                        holes_in_one += (sc.score[i] == 1);
                        }
                }
        vfont_printf(bp, vf, x+vf->w*4+152, y+vf->h*2, 4*vf->w/5, "%i", holes_in_one);
}

//draw the course preview
void course_preview_display(BITMAP * bp, V_FONT * vf, COURSE * cr){
        int s, score = 0, strokes = 0;

        if(cr->image_bank.iconpic != NULL){
	        //draw the smallish preview icon that gives a general idea of what the course is
                masked_blit(cr->image_bank.iconpic, bp, 0, 0, SCREEN_W-cr->image_bank.iconpic->w-40, 35, cr->image_bank.iconpic->w, cr->image_bank.iconpic->h);
                
                //print the course name, par score, and number of holes
                vfont_printf_center(bp, vf, SCREEN_W-(cr->image_bank.iconpic->w/2)-40, cr->image_bank.iconpic->h+40, 0, "%s", cr->name);
                vfont_printf_center(bp, vf, SCREEN_W-(cr->image_bank.iconpic->w/2)-40, cr->image_bank.iconpic->h+60, 0, "par: %i", cr->course_par);
                vfont_printf_center(bp, vf, SCREEN_W-(cr->image_bank.iconpic->w/2)-40, cr->image_bank.iconpic->h+80, 0, "# holes: %i", cr->num_holes);

                //print the listing of top round scores for this course                
                vfont_printf_center(bp, vf, SCREEN_W-cr->image_bank.iconpic->w/2-40, cr->image_bank.iconpic->h+105, 0, "[Best rounds]");
                for(s = 0; s < MAX_COURSE_TOPSCORES; s++){
                        if(main_game_vars.game_mode == GAMEMODE_COIN_COLLECT){
                                int x = SCREEN_W-cr->image_bank.iconpic->w-100, y = cr->image_bank.iconpic->h+121+vf->h*3*s;
                                masked_stretch_blit(main_game_vars.image_bank.charpic[cr->coin_top_scorecard[s].pic_ref][0], bp, 0, 0, main_game_vars.image_bank.charpic[cr->coin_top_scorecard[s].pic_ref][0]->w, main_game_vars.image_bank.charpic[cr->coin_top_scorecard[s].pic_ref][0]->h, x+vf->w*2, y, main_game_vars.image_bank.charpic[cr->coin_top_scorecard[s].pic_ref][0]->w/2, main_game_vars.image_bank.charpic[cr->coin_top_scorecard[s].pic_ref][0]->h/2);
                                vfont_printf(bp, vf, x, y, 0, "%i)", s);
                                vfont_printf(bp, vf, x+vf->w*4, y, 0, "%s", cr->coin_top_scorecard[s].name);
                                {       //find out if all stages have been completed..
                                        int c;
                                        char c_done = 1;
                                        for(c = 0; c < cr->num_holes; c++){
                                                if(!cr->coin_top_scorecard[s].match_won[c]){
                                                        c_done = 0;
                                                        break;
                                                        }
                                                }
                                        if(c_done) vfont_printf(bp, vf, x+vf->w*5+strlen(cr->coin_top_scorecard[s].name)*vf->w, y, 0, "*");
                                        }
                                vfont_printf(bp, vf, x+vf->w*6, y+vf->h, 0, "score:%i", cr->coin_top_scorecard[s].coin_score);
                                vfont_printf(bp, vf, x+vf->w*6, y+vf->h*2, 0, "best combo:%i", cr->coin_top_scorecard[s].coin_best_combo);
                                }
                        else{
                                int j;

                                score = strokes = 0;
                                for(j = 0; j < cr->num_holes; j++){
                                        score += cr->top_scorecard[s].score[j];
                                        //textprintf(screen, font, 50*s, 15*j, 15, "score: %i", score);
                                        }
                                strokes = score-cr->course_par;
                                draw_course_top_score(bp, vf, cr->top_scorecard[s], cr->num_holes, SCREEN_W-cr->image_bank.iconpic->w-100, cr->image_bank.iconpic->h+121+vf->h*3*s, strokes, score, s);
                                //draw_course_top_score(bp, vf, cr->top_scorecard[s], cr->num_holes, SCREEN_W-cr->image_bank.iconpic->w-100, cr->image_bank.iconpic->h+121+vf->h*3*s, strokes, score, s+1);
                                }
                	}                	
                }
}

//draw the player's saved stats (if found)
void current_player_statsheet_display(BITMAP * bp, V_FONT * vf, int x, int y, int plno){
	int stat_no = main_game_vars.cur_statsheet;
	
        if(stat_no == -1){   //no statsheet loaded, so no saved data exists for this player
                vfont_printf_center(bp, vf, x+400, y+40, 0, "no saved data");
                }
        else{                //display the statsheet data for this play
                int i,strokes,score,par;

                vfont_printf_center(bp, vf, x+400, y+40, 0, "%s", main_game_vars.player_statsheet[stat_no].name);
                if(main_game_vars.player_statsheet[stat_no].total_strokes-main_game_vars.player_statsheet[stat_no].total_par == 0){
                        vfont_printf(bp, vf, x+260, y+40+vf->h, 0, "Overall:     e", main_game_vars.player_statsheet[stat_no].total_strokes-main_game_vars.player_statsheet[stat_no].total_par);
                        }
                else if(main_game_vars.player_statsheet[stat_no].total_strokes-main_game_vars.player_statsheet[stat_no].total_par > 0){
                        vfont_printf(bp, vf, x+260, y+40+vf->h, 0, "Overall:    +%i", main_game_vars.player_statsheet[stat_no].total_strokes-main_game_vars.player_statsheet[stat_no].total_par);
                        }
                else{
                        vfont_printf(bp, vf, x+260, y+40+vf->h, 0, "Overall:     %i", main_game_vars.player_statsheet[stat_no].total_strokes-main_game_vars.player_statsheet[stat_no].total_par);
                        }
                vfont_printf(bp, vf, x+260, y+40+(vf->h+1)*2, 0,   "Stroke play: %i-%i", main_game_vars.player_statsheet[stat_no].total_rounds_won, main_game_vars.player_statsheet[stat_no].total_rounds_played-main_game_vars.player_statsheet[stat_no].total_rounds_won);
                vfont_printf(bp, vf, x+260, y+40+(vf->h+1)*3, 0,   "Match play:  %i-%i", main_game_vars.player_statsheet[stat_no].total_matches_won, main_game_vars.player_statsheet[stat_no].total_matches_played-main_game_vars.player_statsheet[stat_no].total_matches_won);
                vfont_printf(bp, vf, x+260, y+40+(vf->h+1)*4, 0,   "Courses completed:");
                for(i = 0; i < main_game_vars.player_statsheet[stat_no].total_courses_completed; i++){
                        vfont_printf(bp, vf, x+300, y+40+(vf->h)*(5+i), 0, "%s", main_game_vars.player_statsheet[stat_no].course_name[i]);
                        }
                vfont_printf(bp, vf, x+260, y+40+(vf->h+1)*10, 0,   "Best rounds:");
                for(i = 0; i < main_game_vars.player_statsheet[stat_no].total_top_scores; i++){
                        {
                                int j;

                                score = strokes = par = 0;
                                for(j = 0; j < main_game_vars.player_statsheet[stat_no].course_top_num_holes[i]; j++){
                                        score += main_game_vars.player_statsheet[stat_no].top_scorecard[i].score[j];
                                        par += main_game_vars.player_statsheet[stat_no].top_scorecard[i].par_score[j];
                                        }
                                if(score == 0) score = par;
                                strokes = score-par;
                                }
                        draw_course_top_score(bp, vf, main_game_vars.player_statsheet[stat_no].top_scorecard[i], main_game_vars.player_statsheet[stat_no].course_top_num_holes[i], x+260, y+40+(vf->h+1)*11+vf->h*3*i, strokes, score, i+1);
                        }
                }
}

//draw the menu cursor
void menu_cursor_display(BITMAP * bp, MENU_CURSOR * mcur){
        //draw the ball selector
        circlefill(bp, mcur->x, mcur->y+mcur->offset_counter/15, mcur->radius, mcur->color);
}

//menu item display stuff
void menu_item_display(BITMAP * bp, V_FONT * vf, MENU_ITEM * mi, int x, int y, int scale, int item_no){
	vfont_printf(bp, vf, x+mi->x, y+mi->y, 5*scale*vf->w/4, mi->text);
	switch(mi->item_type){
		case MENUITEM_FROMLIST:          //draw the text list
			if(mi->c_ref_list[mi->item_ref_set] != NULL){
				vfont_printf(bp, vf, x+mi->x+vf->w*3, y+mi->y+vf->h+6, 0, mi->c_ref_list[mi->item_ref_set]);
				}
			break;
        	case MENUITEM_CHARPIC_SELECT:    //draw the character pic if this is the charpic selector
        		{
               			int i = item_no/3;   //set the player # to mess with
		                masked_blit(main_game_vars.image_bank.charpic[mi->item_ref_set][0], bp, 0, 0, x+mi->x+(strlen(mi->text)+1)*16, y+mi->y, main_game_vars.image_bank.charpic[mi->item_ref_set][0]->w, main_game_vars.image_bank.charpic[mi->item_ref_set][0]->h);
        		        current_player_statsheet_display(bp, vf, x, y, i);
		        	}
        	        break;
        	case MENUITEM_SLIDER:
        	        rectfill(bp, x+mi->x+vf->w*1-1, y+mi->y+vf->h+6, x+mi->x+vf->w*1+mi->item_ref_end+1, y+mi->y+vf->h+13, 39);
                	rectfill(bp, x+mi->x+vf->w*1, y+mi->y+vf->h+6, x+mi->x+vf->w*1+mi->item_ref_set, y+mi->y+vf->h+13, 41);
                	rectfill(bp, x+mi->x+vf->w*1+(mi->item_ref_set-2), y+mi->y+vf->h+6, x+mi->x+vf->w*1+mi->item_ref_set, y+mi->y+vf->h+13, 42);
			break;
		case MENUITEM_KEYDEFINE:         //draw the key definition item
	                vfont_printf(bp, vf, x+mi->x+vf->w*strlen("rotate right: "), y+mi->y, 0, key_names[mi->item_ref_set]);
        		break;
		case MENUITEM_LOAD_COURSE:         //draw the key definition item
			course_preview_display(bp, vf, &game_course);
        		break;
		default:
			break;
		}
}

//menu display stuff..
void main_menu_display(BITMAP * bp, V_FONT * vf, CMENU * mn, MENU_CURSOR * mcur){
        int i;

        //clear the background
        clear_to_color(bp, 2);
        
        //draw an outline around this menu
        rect(bp, mn->x, mn->y, mn->x+mn->w, mn->y+mn->h, 8);
        //print this menu's label text
        vfont_printf_center(bp, vf, mn->x+mn->w/2, mn->y-vf->h/2, 0, mn->label);
        
        //draw all this menu's items
        for(i = 0; i < mn->num_items; i++){
	        menu_item_display(bp, vf, mn->menu_item[i], mn->x, mn->y, (i == mn->cur_item), i);
                }
                
        //draw the menu cursor
        menu_cursor_display(bp, mcur);
}

//the main menu stuff..
void main_menu_loop(BITMAP * bp){
        while(!main_game_vars.quit_title){
	        poll_music();
	        poll_joystick();
                
	        main_menu_logic(cur_menu, &main_menu_cursor);
                main_menu_display(bp, &game_vfont, *cur_menu, &main_menu_cursor);

                vsync();
                blit(bp, screen, 0, 0, 0, 0, bp->w, bp->h);
                }
}

//the main loop..
void main_loop(void){
        BITMAP * scbuff = create_bitmap(SCREEN_W, SCREEN_H);   //create the screen buffer surface
        float dt = 1.0/60.0f;                                  //set the time delta (1 / 60)
        
        //create transparency tables
        create_trans_table(&trans_table, game_palette, 140, 140, 140, NULL);
        color_map = &trans_table;

        clear_to_color(scbuff, 0);                             //clear the screen buffer before drawing anything
        printf("Entering main loop..\n");
        while(1){
                //put intro stuff here..

                //start the title song
                start_new_music(TITLE_SONG_FILENAME);
			
                //put menu/frontend stuff here..
                main_menu_loop(scbuff);
                
                if(main_game_vars.quit_all) break;    //exit the program if this condition is met

                printf("Entering in-game loop..\n");

                while(!main_game_vars.quit_game){
	                //do the game logic in this loop, then exit and handle game display out when the desired framerate is reached
                        while(main_game_vars.ingame_timer > 0){
                                poll_music();             //keep polling the music and joystick handlers
                                poll_joystick();
                
  				//handle all the game logic stuff
                                main_game_logic(game_player, main_game_vars.num_players, &game_course);
                                main_game_vars.ingame_timer--;
                                }
                        main_game_vars.ingame_timer = 0;  //reset the frame rate timer

                        //draw everything we need to display on the screen buffer
                        main_game_display(scbuff, game_player, main_game_vars.num_players, &game_course);

                        //handle vsync
                        vsync();
                        //display the screen buffer to the screen
                        blit(scbuff, screen, 0, 0, 0, 0, scbuff->w, scbuff->h);
                        }
                //stop_music();
                }

        printf("Exited main loop..\n");
}

//main initialization sequence
void initialize(){
        //start allegro
        allegro_init();

        //start allegro's timer stuff
        install_timer();

        //seed the random number generator
        srandom(time(0));

        //install the joystick, keyboard, and mouse
        install_joystick(JOY_TYPE_AUTODETECT);
        install_keyboard();
        install_mouse();
        
        //try loading the configuration file..
        if(!load_config(&game_config, CONFIG_FILE_FILENAME)){
                //if none found, use the default configuration settings
                set_default_config(&game_config);
                }

        //if sound is on, set DUMB up for use
        if(game_config.sound_on){
	        //register dumb's exit function so it runs when the program exits
                atexit(&dumb_exit);
                //register dumb's file setup
                dumb_register_stdfiles();
                
                //install allegro's sound driver
                install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
                }

        //set the main game variables up
        set_main_game_vars();

        //set the framerate timers up
//        initialize_gamespeed_timer();
        initialize_ingame_timer();

        //set the graphics mode..
	set_gfx_mode(game_config.gfx_mode, game_config.screen_w, game_config.screen_h, game_config.screen_w, game_config.screen_h);

        //load the vfont from the specified bitmap
        load_vfont_bmp(&game_vfont, 16, 16, base_filename("font_pcx"));

        //load the main image bank up
        load_main_image_bank(&main_game_vars.image_bank);

        //load the main sample bank up
        load_main_game_samples(&main_game_vars.sound_bank);

/*
        //initialize all the title and frontend (menus) stuff
        initialize_title();
*/
        //load saved statsheets if present
        if(!load_statsheets(&main_game_vars, "vgolf.cst")){
	        //give the stat sheets default values if not present
                set_default_statsheets(&main_game_vars);
        	}

        //set the palette
        set_palette(game_palette);

        //give the course some preset values before using it
        preinitialize_course(&game_course);
        
        //load the game menus into menu and set them up for use
        initialize_game_menus();

        //set the hole up...
        preinitialize_hole(&game_course.current_hole);

        //set the players up..
        preinitialize_players(game_player, MAX_PLAYERS);
}

//main deinitialization routine
void deinitialize(void){
	//unload everything we allocated from memory
	deinitialize_course(&game_course);
	deinitialize_main_game_vars();
        deinitialize_game_menus();
        deinitialize_players(game_player, main_game_vars.num_players);
        
        //unload allegro keyboard and joystick handlers
        remove_keyboard();
        remove_joystick();
        
        //save the current configuration settings to file
        save_config(&game_config, "vgolf.cfg");
        
        //stop any music currently playing
	stop_music();

        //exit allegro
        allegro_exit();
}

int main(int argc, char ** argv){
        initialize();

        main_loop();

        deinitialize();
        
        return 0;
}END_OF_MAIN()
