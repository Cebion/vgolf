#ifndef NEWMENU_H
#define NEWMENU_H

#include <allegro.h>

#define MAX_NEWMENU_ITEMS    32
#define MAX_NEWMENU_OBJECTS 256

#define NEWMENU_ITEM_TYPE_HEADER  0
#define NEWMENU_ITEM_TYPE_OPTION  1
#define NEWMENU_ITEM_TYPE_VOPTION 2

/* use these for various objects on title screen,
   for "eye-candy" */
typedef struct
{
	
	int type;
	fixed x, y, z;
	fixed vx, vy, vz;
	int active;
	
	BITMAP * bp;
	
} NEWMENU_OBJECT;

typedef struct
{
	
	int type;
	FONT * font, * afont;
	int color, acolor; /* -1 to use bitmap colors */
	struct NEWMENU * sub_menu;
	struct NEWMENU_ITEM * child_item[4];
	int children;
	char text[256];
	char * vtext;
	fixed tz;
	fixed tvz;
//	int tf[256];
//	int tpos;
	int x, y;
	
	/* this function activated when user presses enter and menu transition
	   is completed */
	void(*enter_proc)();
	
	/* these procedures allow you to modify options */
	void(*left_proc)();
	void(*right_proc)();
	
} NEWMENU_ITEM;

typedef struct
{
	
	/* items for this menu */
	NEWMENU_ITEM item[MAX_NEWMENU_ITEMS];
	int items;
	int ox, oy;
	char title[256];
	char vtext[16][256];

	/* this function used to update the menu item,
	   used for special cases like when you need to sprintf a string into the menu */
	void(*update_proc)();	
	
	int current_item;
	struct NEWMENU * parent;
	
} NEWMENU;

extern NEWMENU * current_newmenu;
extern int newmenu_frame;
extern NEWMENU_OBJECT newmenu_object[MAX_NEWMENU_OBJECTS];
extern BITMAP * letter_images[256];
extern int newmenu_controls_active;

void newmenu_disable_controls(void);
void newmenu_enable_controls(void);

NEWMENU * create_newmenu(char * title, void(*update_proc)(), int ox, int oy);
void add_newmenu_item(NEWMENU * mp, char * text, int type, int x, int y, FONT * fp, int color, FONT * afp, int acolor, NEWMENU * smp, void(*enter_proc)(), void(*left_proc)(), void(*right_proc)());
void add_newmenu_child_item(NEWMENU * mp, char * text, int type, int x, int y, FONT * fp, int color, FONT * afp, int acolor);
void add_newmenu_child_variable(NEWMENU * mp, char * text, int x, int y, FONT * fp, int color, FONT * afp, int acolor);
void newmenu_logic(void);
void newmenu_render(BITMAP * bp);

void set_current_newmenu(NEWMENU * mp);

#endif
