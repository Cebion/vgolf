#include <allegro.h>
#include <string.h>
#include "newmenu.h"
#include "message.h"
#include "main.h"

NEWMENU * current_newmenu = NULL;
int newmenu_frame = 0;
NEWMENU_OBJECT newmenu_object[MAX_NEWMENU_OBJECTS];
BITMAP * nm_scratch = NULL;
int newmenu_controls_active = 1;
int newmenu_hidden = 0;

void newmenu_disable_controls(void)
{
	newmenu_controls_active = 0;
}

void newmenu_enable_controls(void)
{
	newmenu_controls_active = 1;
}

void newmenu_hide(void)
{
	newmenu_hidden = 1;
}

void newmenu_show(void)
{
	newmenu_hidden = 0;
}

/*void outline_text(BITMAP * bp, FONT * fp, int x, int y, int c1, int c2, char * buffer)
{
	textprintf_ex(bp, fp, x - 1, y, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + 1, y, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y - 1, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y + 1, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y, c1, -1, "%s", buffer);
}

void outline_text_shadow(BITMAP * bp, FONT * fp, int x, int y, int sx, int sy, int c1, int c2, int c3, char * buffer)
{
	textprintf_ex(bp, fp, x - 1 + sx, y + sy, c3, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + 1 + sx, y + sy, c3, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + sx, y - 1 + sy, c3, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + sx, y + 1 + sy, c3, -1, "%s", buffer);
//	textprintf_ex(bp, fp, x, y, c1, -1, "%s", buffer);
	textprintf_ex(bp, fp, x - 1, y, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x + 1, y, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y - 1, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y + 1, c2, -1, "%s", buffer);
	textprintf_ex(bp, fp, x, y, c1, -1, "%s", buffer);
} */

NEWMENU * create_newmenu(char * title, void(*update_proc)(), int ox, int oy)
{
	NEWMENU * mp;
	
	mp = (NEWMENU *)malloc(sizeof(NEWMENU));
	if(!mp)
	{
		return NULL;
	}
	mp->items = 0;
	mp->ox = ox;
	mp->oy = oy;
	strcpy(mp->title, title);
	mp->update_proc = update_proc;
	
	return mp;
}

void add_newmenu_item(NEWMENU * mp, char * text, int type, int x, int y, FONT * fp, int color, FONT * afp, int acolor, NEWMENU * smp, void(*enter_proc)(), void(*left_proc)(), void(*right_proc)())
{
	int i;
	
	if(mp)
	{
		if(mp->items < MAX_NEWMENU_ITEMS)
		{
			mp->item[mp->items].type = type;
			mp->item[mp->items].font = fp;
			mp->item[mp->items].color = color;
			mp->item[mp->items].afont = afp;
			mp->item[mp->items].acolor = acolor;
			(NEWMENU *)mp->item[mp->items].sub_menu = smp;
			strcpy(mp->item[mp->items].text, text);
			mp->item[mp->items].vtext = NULL;
			mp->item[mp->items].x = x;
			mp->item[mp->items].y = y;
			mp->item[mp->items].enter_proc = enter_proc;
			mp->item[mp->items].left_proc = left_proc;
			mp->item[mp->items].right_proc = right_proc;
			for(i = 0; i < 4; i++)
			{
				mp->item[mp->items].child_item[i] = NULL;
			}
			mp->item[mp->items].children = 0;
			mp->parent = NULL;
			mp->items++;
		}
	}
}

void add_newmenu_child_item(NEWMENU * mp, char * text, int type, int x, int y, FONT * fp, int color, FONT * afp, int acolor)
{
	NEWMENU_ITEM * ip;
	
	if(mp)
	{
		if(mp->items > 0)
		{
			ip = malloc(sizeof(NEWMENU_ITEM));
			if(!ip)
			{
				return;
			}
			ip->type = type;
			ip->font = fp;
			ip->color = color;
			ip->afont = afp;
			ip->acolor = acolor;
			ip->sub_menu = NULL;
			strcpy(ip->text, text);
			ip->vtext = NULL;
			ip->x = x;
			ip->y = y;
			ip->enter_proc = NULL;
			ip->left_proc = NULL;
			ip->right_proc = NULL;
			(NEWMENU_ITEM *)mp->item[mp->items - 1].child_item[mp->item[mp->items - 1].children] = (NEWMENU_ITEM *)ip;
			(NEWMENU_ITEM *)mp->item[mp->items - 1].children++;
		}
	}
}

void add_newmenu_child_variable(NEWMENU * mp, char * text, int x, int y, FONT * fp, int color, FONT * afp, int acolor)
{
	NEWMENU_ITEM * ip;
	
	if(mp)
	{
		if(mp->items > 0)
		{
			ip = malloc(sizeof(NEWMENU_ITEM));
			if(!ip)
			{
				return;
			}
			ip->type = NEWMENU_ITEM_TYPE_HEADER;
			ip->font = fp;
			ip->color = color;
			ip->afont = afp;
			ip->acolor = acolor;
			ip->sub_menu = NULL;
			strcpy(ip->text, "");
			ip->vtext = text;
			ip->x = x;
			ip->y = y;
			ip->enter_proc = NULL;
			ip->left_proc = NULL;
			ip->right_proc = NULL;
			(NEWMENU_ITEM *)mp->item[mp->items - 1].child_item[mp->item[mp->items - 1].children] = (NEWMENU_ITEM *)ip;
			(NEWMENU_ITEM *)mp->item[mp->items - 1].children++;
		}
	}
}

void newmenu_get_first_item(NEWMENU * mp)
{
	int i;
	
	for(i = 0; i < mp->items; i++)
	{
		if(mp->item[i].type != NEWMENU_ITEM_TYPE_HEADER)
		{
			mp->current_item = i;
			break;
		}
	}
}

void newmenu_get_next_item(NEWMENU * mp)
{
	int i, start;
	
	start = mp->current_item;
	for(i = mp->current_item + 1; i < mp->items; i++)
	{
		if(mp->item[i].type != NEWMENU_ITEM_TYPE_HEADER)
		{
			mp->current_item = i;
			return;
		}
	}
	for(i = 0; i < start; i++)
	{
		if(mp->item[i].type != NEWMENU_ITEM_TYPE_HEADER)
		{
			mp->current_item = i;
			return;
		}
	}
}

void newmenu_get_previous_item(NEWMENU * mp)
{
	int i, start;
	
	start = mp->current_item;
	for(i = mp->current_item - 1; i >= 0; i--)
	{
		if(mp->item[i].type != NEWMENU_ITEM_TYPE_HEADER)
		{
			mp->current_item = i;
			return;
		}
	}
	for(i = mp->items - 1; i > start; i--)
	{
		if(mp->item[i].type != NEWMENU_ITEM_TYPE_HEADER)
		{
			mp->current_item = i;
			return;
		}
	}
}

void newmenu_logic(void)
{
	NEWMENU * last;
	int i;
	int tl;
	
	if(current_newmenu)
	{
		if(newmenu_controls_active)
		{
			if(key[KEY_ENTER])
			{
				if(current_newmenu->item[current_newmenu->current_item].enter_proc)
				{
					current_newmenu->item[current_newmenu->current_item].enter_proc();
					play_select_sample();
				}
				if(current_newmenu->item[current_newmenu->current_item].sub_menu)
				{
					last = current_newmenu;
					set_current_newmenu((NEWMENU *)current_newmenu->item[current_newmenu->current_item].sub_menu);
					(NEWMENU *)current_newmenu->parent = last;
					play_select_sample();
				}
				key[KEY_ENTER] = 0;
			}
			if(key[KEY_ESC])
			{
				if(current_newmenu->parent)
				{
					current_newmenu = (NEWMENU *)current_newmenu->parent;
					for(i = 0; i < current_newmenu->items; i++)
					{
						if(current_newmenu->item[i].type == NEWMENU_ITEM_TYPE_OPTION)
						{
							current_newmenu->item[i].tz = itofix(0);
							current_newmenu->item[i].tvz = itofix(0);
						}
						else
						{
							current_newmenu->item[i].tz = -itofix(100);
							current_newmenu->item[i].tvz = itofix(0);
						}
					}
					current_newmenu->item[current_newmenu->current_item].tz = -itofix(100);
					current_newmenu->item[current_newmenu->current_item].tvz = itofix(0);
					play_esc_sample();
				}
				key[KEY_ESC] = 0;
			}
			if(key[KEY_LEFT])
			{	
				if(current_newmenu->item[current_newmenu->current_item].left_proc)
				{	
					play_slider_sample();
					current_newmenu->item[current_newmenu->current_item].left_proc();
				}	
				key[KEY_LEFT] = 0;
			}
			if(key[KEY_RIGHT])
			{
				if(current_newmenu->item[current_newmenu->current_item].right_proc)
				{
					play_slider_sample();
					current_newmenu->item[current_newmenu->current_item].right_proc();
				}
				key[KEY_RIGHT] = 0;
			}
			if(key[KEY_UP])
			{
				current_newmenu->item[current_newmenu->current_item].tvz = itofix(5);
				newmenu_get_previous_item(current_newmenu);
				current_newmenu->item[current_newmenu->current_item].tz = -itofix(130);
				current_newmenu->item[current_newmenu->current_item].tvz = itofix(0);
				play_item_select_sample();
				key[KEY_UP] = 0;
			}
			if(key[KEY_DOWN])
			{
				current_newmenu->item[current_newmenu->current_item].tvz = itofix(5);
				newmenu_get_next_item(current_newmenu);
				current_newmenu->item[current_newmenu->current_item].tz = -itofix(130);
				current_newmenu->item[current_newmenu->current_item].tvz = itofix(0);
				play_item_select_sample();
				key[KEY_DOWN] = 0;
			}
		}
		if(current_newmenu->update_proc)
		{
			current_newmenu->update_proc();
		}
		for(i = 0; i < current_newmenu->items; i++)
		{
			if(current_newmenu->item[i].type != NEWMENU_ITEM_TYPE_HEADER)
			{
				tl = strlen(current_newmenu->item[i].text);
				if(i == current_newmenu->current_item)
				{
					if(current_newmenu->item[i].tz < itofix(-100))
					{
						current_newmenu->item[i].tz += current_newmenu->item[i].tvz;
						current_newmenu->item[i].tvz += ftofix(0.5);
						if(current_newmenu->item[i].tz > itofix(-100))
						{
							current_newmenu->item[i].tz = itofix(-100);
						}
					}
				}
				else
				{
					if(current_newmenu->item[i].tz < itofix(0))
					{
						current_newmenu->item[i].tz += current_newmenu->item[i].tvz;
						current_newmenu->item[i].tvz += ftofix(0.5);
						if(current_newmenu->item[i].tz > itofix(0))
						{
							current_newmenu->item[i].tz = itofix(0);
						}
					}
				}
			}
		}
	}
}

void newmenu_render(BITMAP * bp)
{
	int i, k;
	FONT * pick;
	int cpick;
	int tx, tl;
	int py;
	NEWMENU_ITEM * ip;
	char * cp;
	int ox, oy, sx, sy;
	
	if(current_newmenu && !newmenu_hidden)
	{
		py = current_newmenu->oy + current_newmenu->items * 8;
		for(i = 0; i < current_newmenu->items; i++)
		{
			if(current_newmenu->current_item == i)
			{
				cpick = current_newmenu->item[i].acolor;
				if(current_newmenu->item[i].afont)
				{
					pick = current_newmenu->item[i].afont;
				}
				else
				{
					pick = current_newmenu->item[i].font;
				}
				ox = 2;
				oy = -2;
				sx = -4;
				sy = 4;
			}
			else
			{
				pick = current_newmenu->item[i].font;
				cpick = current_newmenu->item[i].color;
				ox = 0;
				oy = 0;
				sx = -2;
				sy = 2;
			}
			switch(current_newmenu->item[i].type)
			{
				case NEWMENU_ITEM_TYPE_HEADER:
				case NEWMENU_ITEM_TYPE_OPTION:
				{
					if(current_newmenu->item[i].vtext)
					{
						cp = current_newmenu->item[i].vtext;
					}
					else
					{
						cp = current_newmenu->item[i].text;
					}
//					tl = strlen(cp);
					tl = text_length(pick, cp);
					tx = bp->w / 2 - tl / 2;
//					for(j = 0; j < tl; j++)
//					{
//						zbuffer_add_text(&newmenu_zbuffer, pick, cpick, cp[j], tx, current_newmenu->oy + current_newmenu->item[i].y, fixtoi(current_newmenu->item[i].tz));
//						fx_draw_3d_sprite(bp, letter_images[cp[j]], tx, current_newmenu->oy + current_newmenu->item[i].y, fixtoi(current_newmenu->item[i].tz), bp->w / 2, py);
//						tx += 16;
//					}
					outline_text_shadow(bp, pick, tx + ox, current_newmenu->oy + current_newmenu->item[i].y + oy, sx, sy, cpick, makecol(0, 0, 0), makecol(0, 0, 0), cp);
					if(current_newmenu->item[current_newmenu->current_item].children > 0)
					{
						for(k = 0; k < current_newmenu->item[i].children; k++)
						{
							ip = (NEWMENU_ITEM *)current_newmenu->item[i].child_item[k];
							if(current_newmenu->current_item == i)
							{
								cpick = ip->acolor;
								if(ip->afont)
								{
									pick = ip->afont;
								}
								else
								{
									pick = ip->font;
								}
								ox = 2;
								oy = -2;
								sx = -4;
								sy = 4;
							}
							else
							{
								pick = ip->font;
								cpick = ip->color;
								ox = 0;
								oy = 0;
								sx = -2;
								sy = 2;
							}
							if(ip->vtext)
							{
								cp = ip->vtext;
							}
							else
							{
								cp = ip->text;
							}
							tl = text_length(pick, cp);
							tx = bp->w / 2 - tl / 2;
//							outline_text(bp, font, tx, current_newmenu->oy + ip->y, cpick, makecol(0, 0, 0), "test");
							outline_text_shadow(bp, pick, tx + ox, current_newmenu->oy + ip->y + oy, sx, sy, cpick, makecol(0, 0, 0), makecol(0, 0, 0), cp);
//							for(j = 0; j < tl; j++)
//							{
//								zbuffer_add_text(&newmenu_zbuffer, pick, cpick, cp[j], tx, current_newmenu->oy + ip->y, fixtoi(current_newmenu->item[i].tz));
	//							fx_draw_3d_sprite(bp, letter_images[cp[j]], tx, current_newmenu->oy + ip->y, fixtoi(current_newmenu->item[i].tz), bp->w / 2, py);
//								tx += 16;
//							}
						}
					}
					break;
				}
			}
		}
	}
//	zbuffer_sort_text(&newmenu_zbuffer);
//	zbuffer_render_text(&newmenu_zbuffer, bp, 0, 0, bp->w / 2, py);
}

void set_current_newmenu(NEWMENU * mp)
{
	int i;
	
	if(current_newmenu)
	{
		for(i = 0; i < current_newmenu->items; i++)
		{
			if(current_newmenu->item[i].type == NEWMENU_ITEM_TYPE_OPTION)
			{
				current_newmenu->item[i].tz = itofix(0);
				current_newmenu->item[i].tvz = itofix(0);
			}
			else
			{
				current_newmenu->item[i].tz = itofix(-100);
				current_newmenu->item[i].tvz = itofix(0);
			}
		}
	}
	current_newmenu = mp;
	for(i = 0; i < current_newmenu->items; i++)
	{
		if(current_newmenu->item[i].type == NEWMENU_ITEM_TYPE_OPTION)
		{
			current_newmenu->item[i].tz = itofix(0);
			current_newmenu->item[i].tvz = itofix(0);
		}
		else
		{
			current_newmenu->item[i].tz = itofix(-100);
			current_newmenu->item[i].tvz = itofix(0);
		}
	}
	newmenu_get_first_item(current_newmenu);
	current_newmenu->item[current_newmenu->current_item].tz = -itofix(100);
	current_newmenu->item[current_newmenu->current_item].tvz = itofix(0);
}
