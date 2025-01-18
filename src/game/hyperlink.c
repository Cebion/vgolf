#include <allegro.h>
#include "main.h"
#include "hyperlink.h"
#include "message.h"

static int * hyperlink_mouse_x;
static int * hyperlink_mouse_y;
static int * hyperlink_mouse_z;
static int * hyperlink_mouse_b;

void hyperlink_link_mouse(int * x, int * y, int * z, int * b)
{
	hyperlink_mouse_x = x;
	hyperlink_mouse_y = y;
	hyperlink_mouse_z = z;
	hyperlink_mouse_b = b;
}

HYPERLINK_PAGE * hyperlink_page_create(int ox, int oy)
{
	HYPERLINK_PAGE * lp;
	lp = malloc(sizeof(HYPERLINK_PAGE));
	if(!lp)
	{
		return NULL;
	}
	lp->elements = 0;
	lp->ox = ox;
	lp->oy = oy;
	lp->hover_element = -1;
	return lp;
}

void hyperlink_page_destroy(HYPERLINK_PAGE * pp)
{
	if(pp)
	{
		free(pp);
	}
}

int hyperlink_page_add_element_image(HYPERLINK_PAGE * pp, int (*proc)(int, void *), BITMAP * bp, int ox, int oy, int flags)
{
	pp->element[pp->elements].type = HYPERLINK_ELEMENT_TYPE_IMAGE;
	pp->element[pp->elements].proc = proc;
	pp->element[pp->elements].data = bp;
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].flags = flags;
	pp->elements++;
	return 1;
}

int hyperlink_page_add_element_text(HYPERLINK_PAGE * pp, int (*proc)(int, void *), char * text, FONT * fp, int ox, int oy, int color, int hcolor, int outline, int flags)
{
	pp->element[pp->elements].type = HYPERLINK_ELEMENT_TYPE_TEXT;
	pp->element[pp->elements].proc = proc;
	pp->element[pp->elements].data = text;
	pp->element[pp->elements].aux_data = fp;
	pp->element[pp->elements].ox = ox;
	pp->element[pp->elements].oy = oy;
	pp->element[pp->elements].d1 = color;
	pp->element[pp->elements].d2 = hcolor;
	pp->element[pp->elements].d3 = outline;
	pp->element[pp->elements].flags = flags;
	pp->elements++;
	return 1;
}

void hyperlink_page_logic(HYPERLINK_PAGE * pp)
{
	int i;
	
	if(pp)
	{
		pp->hover_element = -1;
		for(i = 0; i < pp->elements; i++)
		{
			if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_CLICKABLE)
			{
				switch(pp->element[i].type)
				{
					case HYPERLINK_ELEMENT_TYPE_TEXT:
					{
						if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_CENTER)
						{
							if(*hyperlink_mouse_x >= pp->ox + pp->element[i].ox - text_length((FONT *)pp->element[i].aux_data, (char *)pp->element[i].data) / 2 && *hyperlink_mouse_x < pp->ox + pp->element[i].ox + text_length((FONT *)pp->element[i].aux_data, (char *)pp->element[i].data) / 2 && *hyperlink_mouse_y >= pp->oy + pp->element[i].oy && *hyperlink_mouse_y < pp->oy + pp->element[i].oy + text_height((FONT *)pp->element[i].aux_data))
							{
								pp->hover_element = i;
							}
						}
						else
						{
							if(*hyperlink_mouse_x >= pp->ox + pp->element[i].ox && *hyperlink_mouse_x < pp->ox + pp->element[i].ox + text_length((FONT *)pp->element[i].aux_data, (char *)pp->element[i].data) && *hyperlink_mouse_y >= pp->oy + pp->element[i].oy && *hyperlink_mouse_y < pp->oy + pp->element[i].oy + text_height((FONT *)pp->element[i].aux_data))
							{
								pp->hover_element = i;
							}
						}
						break;
					}
					case HYPERLINK_ELEMENT_TYPE_IMAGE:
					{
						if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_CENTER)
						{
							if(*hyperlink_mouse_x >= pp->ox + pp->element[i].ox - ((BITMAP *)(pp->element[i].data))->w / 2 && *hyperlink_mouse_x < pp->ox + pp->element[i].ox + ((BITMAP *)(pp->element[i].data))->w / 2 && *hyperlink_mouse_y >= pp->oy + pp->element[i].oy - ((BITMAP *)(pp->element[i].data))->h / 2 && *hyperlink_mouse_y < pp->oy + pp->element[i].oy + ((BITMAP *)(pp->element[i].data))->h / 2)
							{
								pp->hover_element = i;
							}
						}
						else
						{
							if(*hyperlink_mouse_x >= pp->ox + pp->element[i].ox && *hyperlink_mouse_x < pp->ox + pp->element[i].ox + ((BITMAP *)(pp->element[i].data))->w && *hyperlink_mouse_y >= pp->oy + pp->element[i].oy && *hyperlink_mouse_y < pp->oy + pp->element[i].oy + ((BITMAP *)(pp->element[i].data))->h)
							{
								pp->hover_element = i;
							}
						}
						break;
					}
				}
			}
		}
		if(left_click == 1 && pp->hover_element >= 0)
		{
			if(pp->element[pp->hover_element].proc)
			{
				pp->element[pp->hover_element].proc(0, pp);
			}
		}
	}
}

void hyperlink_page_render(HYPERLINK_PAGE * pp, BITMAP * bp)
{
	int i;
	
	if(pp)
	{
		for(i = 0; i < pp->elements; i++)
		{
			if(i != pp->hover_element)
			{
				switch(pp->element[i].type)
				{
					case HYPERLINK_ELEMENT_TYPE_TEXT:
					{
						int sx, sy;
						if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_SHADOW)
						{
							sx = -2;
							sy = 2;
							if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_CENTER)
							{
								outline_text_shadow_center(bp, (FONT *)pp->element[i].aux_data, pp->ox + pp->element[i].ox - sx, pp->oy + pp->element[i].oy - sy, sx, sy, pp->element[i].d1, pp->element[i].d3, makecol(0, 0, 0), (char *)pp->element[i].data);
							}
							else
							{
								outline_text_shadow(bp, (FONT *)pp->element[i].aux_data, pp->ox + pp->element[i].ox - sx, pp->oy + pp->element[i].oy - sy, sx, sy, pp->element[i].d1, pp->element[i].d3, makecol(0, 0, 0), (char *)pp->element[i].data);
							}
						}
						else
						{
						}
						break;
					}
					case HYPERLINK_ELEMENT_TYPE_IMAGE:
					{
						int sx, sy;
						if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_SHADOW)
						{
							sx = -2;
							sy = 2;
							if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_CENTER)
							{
								if(pp->element[i].data)
								{
									draw_character_ex(bp, (BITMAP *)(pp->element[i].data), pp->ox + pp->element[i].ox - ((BITMAP *)(pp->element[i].data))->w / 2, pp->oy + pp->element[i].oy - ((BITMAP *)(pp->element[i].data))->h / 2, makecol(0, 0, 0), -1);
									if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_BLIT)
									{
										blit((BITMAP *)(pp->element[i].data), bp, 0, 0, pp->ox + pp->element[i].ox - ((BITMAP *)(pp->element[i].data))->w / 2 - sx, pp->oy + pp->element[i].oy - ((BITMAP *)(pp->element[i].data))->h / 2 - sy, bp->w, bp->h);
									}
									else
									{
										draw_sprite(bp, (BITMAP *)(pp->element[i].data), pp->ox + pp->element[i].ox - ((BITMAP *)(pp->element[i].data))->w / 2 - sx, pp->oy + pp->element[i].oy - ((BITMAP *)(pp->element[i].data))->h / 2 - sy);
									}
								}
							}
							else
							{
								if(pp->element[i].data)
								{
									draw_character_ex(bp, (BITMAP *)(pp->element[i].data), pp->ox + pp->element[i].ox, pp->oy + pp->element[i].oy, makecol(0, 0, 0), -1);
									if(pp->element[i].flags & HYPERLINK_ELEMENT_FLAG_BLIT)
									{
										blit((BITMAP *)(pp->element[i].data), bp, 0, 0, pp->ox + pp->element[i].ox - sx, pp->oy + pp->element[i].oy - sy, bp->w, bp->h);
									}
									else
									{
										draw_sprite(bp, (BITMAP *)(pp->element[i].data), pp->ox + pp->element[i].ox - sx, pp->oy + pp->element[i].oy - sy);
									}
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
		}
		
		/* render the hover element last so it appears on top */
		if(pp->hover_element >= 0)
		{
			switch(pp->element[pp->hover_element].type)
			{
				case HYPERLINK_ELEMENT_TYPE_TEXT:
				{
					int sx, sy;
					if(pp->element[pp->hover_element].flags & HYPERLINK_ELEMENT_FLAG_SHADOW)
					{
						sx = -4;
						sy = 4;
						if(pp->element[pp->hover_element].flags & HYPERLINK_ELEMENT_FLAG_CENTER)
						{
							outline_text_shadow_center(bp, (FONT *)pp->element[pp->hover_element].aux_data, pp->ox + pp->element[pp->hover_element].ox - sx, pp->oy + pp->element[pp->hover_element].oy - sy, sx, sy, pp->element[pp->hover_element].d2, pp->element[pp->hover_element].d3, makecol(0, 0, 0), (char *)pp->element[pp->hover_element].data);
						}
						else
						{
							outline_text_shadow(bp, (FONT *)pp->element[pp->hover_element].aux_data, pp->ox + pp->element[pp->hover_element].ox - sx, pp->oy + pp->element[pp->hover_element].oy - sy, sx, sy, pp->element[pp->hover_element].d2, pp->element[pp->hover_element].d3, makecol(0, 0, 0), (char *)pp->element[pp->hover_element].data);
						}
					}
					else
					{
					}
					break;
				}
				case HYPERLINK_ELEMENT_TYPE_IMAGE:
				{
					int sx, sy;
					if(pp->element[pp->hover_element].flags & HYPERLINK_ELEMENT_FLAG_SHADOW)
					{
						sx = -4;
						sy = 4;
						if(pp->element[pp->hover_element].flags & HYPERLINK_ELEMENT_FLAG_CENTER)
						{
							if(pp->element[pp->hover_element].data)
							{
								draw_character_ex(bp, (BITMAP *)(pp->element[pp->hover_element].data), pp->ox + pp->element[pp->hover_element].ox - ((BITMAP *)(pp->element[pp->hover_element].data))->w / 2, pp->oy + pp->element[pp->hover_element].oy - ((BITMAP *)(pp->element[pp->hover_element].data))->h / 2, makecol(0, 0, 0), -1);
								if(pp->element[pp->hover_element].flags & HYPERLINK_ELEMENT_FLAG_BLIT)
								{
									blit((BITMAP *)(pp->element[pp->hover_element].data), bp, 0, 0, pp->ox + pp->element[pp->hover_element].ox - ((BITMAP *)(pp->element[pp->hover_element].data))->w / 2 - sx, pp->oy + pp->element[pp->hover_element].oy - ((BITMAP *)(pp->element[pp->hover_element].data))->h / 2 - sy, bp->w, bp->h);
								}
								else
								{
									draw_sprite(bp, (BITMAP *)(pp->element[pp->hover_element].data), pp->ox + pp->element[pp->hover_element].ox - ((BITMAP *)(pp->element[pp->hover_element].data))->w / 2 - sx, pp->oy + pp->element[pp->hover_element].oy - ((BITMAP *)(pp->element[pp->hover_element].data))->h / 2 - sy);
								}
							}
						}
						else
						{
							if(pp->element[pp->hover_element].data)
							{
								draw_character_ex(bp, (BITMAP *)(pp->element[pp->hover_element].data), pp->ox + pp->element[pp->hover_element].ox, pp->oy + pp->element[pp->hover_element].oy, makecol(0, 0, 0), -1);
								if(pp->element[pp->hover_element].flags & HYPERLINK_ELEMENT_FLAG_BLIT)
								{
									blit((BITMAP *)(pp->element[pp->hover_element].data), bp, 0, 0, pp->ox + pp->element[pp->hover_element].ox - sx, pp->oy + pp->element[pp->hover_element].oy - sy, bp->w, bp->h);
								}
								else
								{
									draw_sprite(bp, (BITMAP *)(pp->element[pp->hover_element].data), pp->ox + pp->element[pp->hover_element].ox - sx, pp->oy + pp->element[pp->hover_element].oy - sy);
								}
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
	}
}
