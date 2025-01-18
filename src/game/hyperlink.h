#ifndef HYPERLINK_H
#define HYPERLINK_H

#include <allegro.h>

#define HYPERLINK_PAGE_MAX_ELEMENTS 128

#define HYPERLINK_ELEMENT_TYPE_TEXT      0
#define HYPERLINK_ELEMENT_TYPE_IMAGE     1

#define HYPERLINK_ELEMENT_FLAG_CLICKABLE 1
#define HYPERLINK_ELEMENT_FLAG_CENTER    2
#define HYPERLINK_ELEMENT_FLAG_SHADOW    4
#define HYPERLINK_ELEMENT_FLAG_BLIT      8

#define HYPERLINK_PAGE_FLAG_DISABLED     1

typedef struct
{
	
	int type;
	void * data;
	void * aux_data;
	int flags;
	int (*proc)(int, void *);
	
	int ox, oy;
	int d1, d2, d3;
	
} HYPERLINK_ELEMENT;

typedef struct
{
	
	HYPERLINK_ELEMENT element[HYPERLINK_PAGE_MAX_ELEMENTS];
	int elements;
	int flags;
	
	int ox, oy;
	
	int hover_element;
	
} HYPERLINK_PAGE;

void hyperlink_link_mouse(int * x, int * y, int * z, int * b);

HYPERLINK_PAGE * hyperlink_page_create(int ox, int oy);
void hyperlink_page_destroy(HYPERLINK_PAGE * pp);

int hyperlink_page_add_element_image(HYPERLINK_PAGE * pp, int (*proc)(int, void *), BITMAP * bp, int ox, int oy, int flags);
int hyperlink_page_add_element_text(HYPERLINK_PAGE * pp, int (*proc)(int, void *), char * text, FONT * fp, int ox, int oy, int color, int hcolor, int outline, int flags);

void hyperlink_page_logic(HYPERLINK_PAGE * pp);
void hyperlink_page_render(HYPERLINK_PAGE * pp, BITMAP * bp);

#endif
