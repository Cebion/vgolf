#ifndef VGOLF_COMMENTARY_H
#define VGOLF_COMMENTARY_H

#include <allegro.h>

#define VGOLF_COMMENTARY_VERSION 1

#define MAX_MESSAGE_LENGTH 64
#define MAX_MESSAGE_STRING_TYPES 10
#define MAX_MESSAGE_STRINGS 64

#define MESSAGE_TYPE_FINISH_GOOD 0
#define MESSAGE_TYPE_FINISH_BETTER 1
#define MESSAGE_TYPE_FINISH_GREAT 2
#define MESSAGE_TYPE_FINISH_EXCELLENT 3
#define MESSAGE_TYPE_FINISH_POOR 4
#define MESSAGE_TYPE_FINISH_VERYPOOR 5
#define MESSAGE_TYPE_ZAPPED 6
#define MESSAGE_TYPE_CLOSE 7
#define MESSAGE_TYPE_WATER 8
#define MESSAGE_TYPE_CRUSHED 9

typedef struct
{
	
	char string[MAX_MESSAGE_LENGTH];
	SAMPLE * voice;
	char voice_file[1024]; // need to cache the filename so we can load the voices
	
} VGOLF_COMMENT;

typedef struct
{
	
	VGOLF_COMMENT comment[MAX_MESSAGE_STRINGS];
	int count;
	
} VGOLF_COMMENT_SET;

typedef struct
{
	
	char name[128];
	char author[256];
	char comment[1024];
	
	VGOLF_COMMENT_SET comments[MAX_MESSAGE_STRING_TYPES];
	
} VGOLF_COMMENTARY;

VGOLF_COMMENTARY * load_vgolf_commentary_script(const char * fn);
VGOLF_COMMENTARY * load_vgolf_commentary(const char * fn);
VGOLF_COMMENTARY * load_vgolf_commentary_info(const char * fn);
int save_vgolf_commentary(VGOLF_COMMENTARY * cp, const char * fn);
void destroy_vgolf_commentary(VGOLF_COMMENTARY * cp);

#endif
