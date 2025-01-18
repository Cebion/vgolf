#ifndef VGOLF_PROFILE_H
#define VGOLF_PROFILE_H

#include "main.h"

#define VGOLF_MAX_PROFILES              32
#define VGOLF_MAX_PROFILE_DATA          64

#define VGOLF_PROFILE_DATA_WINS          0
#define VGOLF_PROFILE_DATA_LOSSES        1
#define VGOLF_PROFILE_DATA_PLAYED        2
#define VGOLF_PROFILE_DATA_HOLES         3
#define VGOLF_PROFILE_DATA_STROKES       4
#define VGOLF_PROFILE_DATA_HOLES_IN_ONE  5
#define VGOLF_PROFILE_DATA_ALBATROSSES   6
#define VGOLF_PROFILE_DATA_EAGLES        7
#define VGOLF_PROFILE_DATA_BIRDIES       8
#define VGOLF_PROFILE_DATA_PARS          9

typedef struct
{
	
	int data[VGOLF_MAX_PROFILE_DATA];
	unsigned long time;
	
} MISC_INFO;

typedef struct
{
	
	/* course filename associated with this info */
//	char name[256];
	unsigned long id;
	
	/* best scores per hole */
	int score[18];
	
	/* best total score */
	int total_score;
	
	/* wins for this course (multiplayer) */
	int won;
	
	/* losses for this course (multiplayer) */
	int lost;
	
	/* times completed */
	int completed;
	
} COURSE_INFO;

typedef struct
{
	
	/* name of player */
	char name[128];
	unsigned long avatar;
	
	/* statistics for each course */
	COURSE_INFO course_info[VGOLF_MAX_COURSES];
	
	/* totals */
	MISC_INFO total;
	
} PROFILE;

typedef struct
{
	
	PROFILE item[VGOLF_MAX_PROFILES];
	int items;
	
} PROFILE_LIST;

int load_profiles(PROFILE_LIST * lp, char * fn);
int save_profiles(PROFILE_LIST * lp, char * fn);
void clear_profiles(PROFILE_LIST * lp);

#endif
