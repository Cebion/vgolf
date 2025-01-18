#include <netstick.h>
#include <winalleg.h>
#include <curl/curl.h>
#include "ncdnet.h"
#include "mygui.h"

char * list_get_text(int index, int * size);
static char etext[16] = {0};
static char stext[256] = {0};
static char server_list_text[1024][256] = {0};

FONT * ncdnet_gui_font = NULL;

/* typedef for the listbox callback functions */
typedef char *(*getfuncptr)(int, int *);

/* split_around_tab:
 *  Helper function for splitting a string into two tokens
 *  delimited by the first TAB character.
 */
static char* split_around_tab(const char *s, char **tok1, char **tok2)
{
   char *buf, *last;
   char tmp[16];

   buf = ustrdup(s);
   *tok1 = ustrtok_r(buf, uconvert_ascii("\t", tmp), &last);
   *tok2 = ustrtok_r(NULL, empty_string, &last);

   return buf;
}

static void inner_bevel_box(BITMAP * bp, int x, int y, int x2, int y2, int c1, int c2, int c3, int c4, int bev, int outline)
{
	int i;
	int offs = 1;
	
	rectfill(screen, x - 2, y - 2, x2 + 2, y2 + 2, c1);
	if(outline)
	{
		rect(screen, x - 3, y - 3, x2 + 3, y2 + 3, c2);
		offs = 0;
	}
	for(i = 0; i < bev; i++)
	{
		line(screen, x - 4 - i, y - 4 - i, x2 + 3 + i, y - 4 - i, c4);
		line(screen, x - 4 - i, y - 4 - i, x - 4 - i, y2 + 3 + i, c4);
		line(screen, x - 3 - i, y2 + 4 + i, x2 + 4 + i, y2 + 4 + i, c3);
		line(screen, x2 + 4 + i, y - 3 - i, x2 + 4 + i, y2 + 4 + i, c3);
	}
}

static void outer_bevel_box(BITMAP * bp, int x, int y, int x2, int y2, int c1, int c2, int c3, int c4, int bev, int outline)
{
	int i;
	int offs = 1;
	
	rectfill(bp, x, y, x2, y2, c1);
	if(outline)
	{
		rect(bp, x, y, x2, y2, c2);
		offs = 0;
	}
	for(i = 0; i < bev; i++)
	{
		line(screen, x + 1 + i - offs, y + 1 + i - offs, x + 1 + i - offs, y2 - 2 - i + offs, c3);
		line(screen, x + 1 + i - offs, y + 1 + i - offs, x2 - 2 - i + offs, y + 1 + i - offs, c3);
		line(screen, x2 - 1 - i + offs, y + 2 + i - offs, x2 - 1 - i + offs, y2 - 1 - i + offs, c4);
		line(screen, x + 2 + i - offs, y2 - 1 - i + offs, x2 - 1 - i + offs, y2 - 1 - i + offs, c4);
	}
}

void ncdnet_gui_draw_scrollable_frame(DIALOG *d, int listsize, int offset, int height, int fg_color, int bg)
{
   BITMAP *gui_bmp = gui_get_screen();
   int i, len;
   BITMAP *pattern;
   int xx, yy;

   /* draw frame */
//   rect(gui_bmp, d->x, d->y, d->x+d->w-1, d->y+d->h-1, fg_color);

   /* possibly draw scrollbar */
   if (listsize > height) {
//      vline(gui_bmp, d->x+d->w-13, d->y+1, d->y+d->h-2, fg_color);

      /* create and draw the scrollbar */
      pattern = create_bitmap(2, 2);
      i = ((d->h-5) * height + listsize/2) / listsize;
      xx = d->x+d->w-11;
      yy = d->y+2;

      putpixel(pattern, 0, 1, bg);
      putpixel(pattern, 1, 0, bg);
      putpixel(pattern, 0, 0, fg_color);
      putpixel(pattern, 1, 1, fg_color);

      if (offset > 0) {
	 len = (((d->h-5) * offset) + listsize/2) / listsize;
	 drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
	 rectfill(gui_bmp, xx, yy - 4, xx+8, yy+len + 4, bg);
	 solid_mode();
	 yy += len;
      }
      if (yy+i < d->y+d->h-3) {
//	 rectfill(gui_bmp, xx, yy, xx+8, yy+i, 0);
	 outer_bevel_box(screen, xx, yy - 4, xx+8, yy+i + 4, bg, fg_color, bg + 7, bg - 7, 1, 1);
	 yy += i+1;
	 drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
	 rectfill(gui_bmp, xx, yy + 4, xx+8, d->y+d->h-3 + 4, bg);
	 solid_mode();
      }
      else {
//	 drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
//	 rectfill(gui_bmp, xx, yy, xx+8, d->y+d->h-3, 0);
//	 solid_mode();
	 outer_bevel_box(screen, xx, yy - 4, xx+8, d->y+d->h-3 + 4, bg, fg_color, bg + 7, bg - 7, 1, 1);
      }
      destroy_bitmap(pattern);
   }
   else {
      /* no scrollbar necessary */
/*      if (d->flags & D_GOTFOCUS)
	 dotted_rect(d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-2, fg_color, bg);
      else
	 rect(gui_bmp, d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-2, bg); */
   }
}

void ncdnet_gui_draw_listbox(DIALOG *d)
{
	BITMAP *gui_bmp = gui_get_screen();
	int height, listsize, i, len, bar, x, y, w;
	int fg_color, fg, bg;
	char *sel = d->dp2;
	char s[1024];

	(*(getfuncptr)d->dp)(-1, &listsize);
	height = (d->h-4) / text_height(font);
	bar = (listsize > height);
	w = (bar ? d->w-15 : d->w-3);
	fg_color = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;

//   inner_bevel_box(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, makecol(255, 255, 255), d->fg, d->bg + 7, d->bg - 7, 1, 1);
	inner_bevel_box(screen, d->x, d->y, d->x + d->w - 5, d->y + d->h - 1, makecol(255, 255, 255), d->fg, d->bg + 7, d->bg - 7, 1, 1);
   
	/* draw box contents */
	for (i=0; i<height; i++)
	{
		if (d->d2+i < listsize)
		{
			if (sel)
			{ 
				if ((sel[d->d2+i]) && (d->d2+i == d->d1))
				{
					fg = makecol(255, 255, 255);
					bg = makecol(0, 0, 255);
				}
				else if (sel[d->d2+i])
				{
					fg = makecol(255, 255, 255);
					bg = makecol(0, 0, 255);
				}
				else
				{
					fg = fg_color;
					bg = d->bg;
				}
			}
			else if (d->d2+i == d->d1)
			{
				fg = makecol(255, 255, 255);
				bg = makecol(0, 0, 255);
			}
			else
			{
				fg = fg_color;
				bg = makecol(255, 255, 255);
			}
			ustrzcpy(s, sizeof(s), (*(getfuncptr)d->dp)(i+d->d2, NULL));
			x = d->x + 2;
			y = d->y + 2 + i*text_height(font);
			x += 8;
			len = ustrlen(s);
			while (text_length(ncdnet_gui_font, s) >= MAX(d->w - 1 - (bar ? 22 : 10), 1))
			{
				len--;
				usetat(s, len, 0);
			}
			if(listsize > height)
			{
				rectfill(screen, x - 8, y, d->x + d->w - 16, y + text_height(font), bg);
			}
			else
			{
				rectfill(screen, x - 8, y, d->x + d->w - 7, y + text_height(font), bg);
			}
			textout_ex(gui_bmp, ncdnet_gui_font, s, x, y, fg, -1);
			x += text_length(ncdnet_gui_font, s);
		}
	}

	/* draw frame, maybe with scrollbar */
	ncdnet_gui_draw_scrollable_frame(d, listsize, d->d2, height, fg_color, d->bg);
}

int ncdnet_gui_shadow_box_proc(int msg, struct DIALOG *d, int c)
{
	switch(msg)
	{
		case MSG_DRAW:
		{
			outer_bevel_box(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg, d->fg, d->bg + 7, d->bg - 7, 2, 1);
			break;
		}
	}
	return 0;
}

int ncdnet_gui_button_proc(int msg, struct DIALOG *d, int c)
{
	int shift = 0;
	int state1, state2;
	int g;
	int swap;
	
	switch(msg)
	{
		case MSG_DRAW:
		{
			if(d->flags & D_GOTFOCUS)
			{
				shift = 1;
			}
			if (d->flags & D_SELECTED)
			{
				g = 1;
				state1 = d->fg;
				state2 = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
				outer_bevel_box(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg + shift, d->fg, d->bg + 7 + shift, d->bg - 7 + shift, 0, 1);
			}
			else
			{
				g = 0;
				state1 = d->fg;
				state2 = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
				outer_bevel_box(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, d->bg + shift, d->fg, d->bg + 7 + shift, d->bg - 7 + shift, 2, 1);
			}
			gui_textout_ex(screen, d->dp, d->x + d->w / 2 + g, d->y + d->h / 2 - text_height(font) / 2 + g, state1, -1, TRUE);
			break;
		}
		case MSG_WANTFOCUS:
		{
			return D_WANTFOCUS;
		}
		case MSG_KEY:
		{
			/* close dialog? */
			if (d->flags & D_EXIT)
			{
				return D_CLOSE;
			}

			/* or just toggle */
			d->flags ^= D_SELECTED;
			object_message(d, MSG_DRAW, 0);
			
			break;
		}
		case MSG_CLICK:
		{
			/* what state was the button originally in? */
			state1 = d->flags & D_SELECTED;
			if (d->flags & D_EXIT)
			{
				swap = FALSE;
			}
			else
			{
				swap = state1;
			}

			/* track the mouse until it is released */
			while (gui_mouse_b())
			{
				state2 = ((gui_mouse_x() >= d->x) && (gui_mouse_y() >= d->y) && (gui_mouse_x() < d->x + d->w) && (gui_mouse_y() < d->y + d->h));
				if (swap)
				{
					state2 = !state2;
				}

				/* redraw? */
				if (((state1) && (!state2)) || ((state2) && (!state1)))
				{
					d->flags ^= D_SELECTED;
					state1 = d->flags & D_SELECTED;
					object_message(d, MSG_DRAW, 0);
				}

				/* let other objects continue to animate */
				broadcast_dialog_message(MSG_IDLE, 0);
			}

			/* should we close the dialog? */
			if ((d->flags & D_SELECTED) && (d->flags & D_EXIT))
			{
				d->flags ^= D_SELECTED;
				return D_CLOSE;
			}
			break; 
		}
	}
	return D_O_K;
}

int ncdnet_gui_edit_proc(int msg, struct DIALOG *d, int c)
{
	static int ignore_next_uchar = FALSE;
	BITMAP *gui_bmp;
	int last_was_space, new_pos, i, k;
	int f, l, p, w, x, fg, b, scroll;
	char buf[16];
	char *s, *t;
	ASSERT(d);
	char out_text[1024] = {0};
   
	gui_bmp = gui_get_screen();

	s = d->dp;
	l = ustrlen(s);
	if (d->d2 > l) 
	{
		d->d2 = l;
	}

	/* calculate maximal number of displayable characters */
	if (d->d2 == l)
	{
		usetc(buf+usetc(buf, ' '), 0);
		x = text_length(ncdnet_gui_font, buf);
	}
	else
	{
		x = 0;
	}

	b = 0;

	for (p=d->d2; p>=0; p--)
	{
		usetc(buf+usetc(buf, ugetat(s, p)), 0);
		x += text_length(ncdnet_gui_font, buf);
		b++;
		if (x > d->w)
		{
			break;
		}
	}

	if (x <= d->w)
	{
		b = l; 
		scroll = FALSE;
	}
	else
	{
		b--; 
		scroll = TRUE;
	}

	switch (msg)
	{

		case MSG_START:
		{
			d->d2 = l;
			break;
		}

		case MSG_DRAW:
      	{
			fg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
			x = 0;

			if (scroll)
			{
				p = d->d2-b+1; 
				b = d->d2; 
			}
			else
			{
				p = 0;
			}
			inner_bevel_box(screen, d->x, d->y, d->x + d->w - 1, d->y + d->h - 1, makecol(255, 255, 255), d->fg, d->bg + 7, d->bg - 7, 1, 1);
			for (; p<=b; p++)
			{
				f = ugetat(s, p);
//				usetc(buf+usetc(buf, (f) ? f : ' '), 0);
				usetc(buf+usetc(buf, (f) ? f : '_'), 0);
				w = text_length(ncdnet_gui_font, buf);
				if (x+w > d->w)
				{
					break;
				}
				f = ((p == d->d2) && (d->flags & D_GOTFOCUS));
//				textout_ex(gui_bmp, font, buf, d->x+x, d->y, (f) ? d->bg : fg, -1);
				if(f)
				{
					textout_ex(gui_bmp, ncdnet_gui_font, buf, d->x+x, d->y, fg, -1);
				}
				else if(p != d->d2)
				{
					textout_ex(gui_bmp, ncdnet_gui_font, buf, d->x+x, d->y, fg, -1);
				}
				x += w;
			}
//			if (x < d->w)
//			{
//				rectfill(gui_bmp, d->x+x, d->y, d->x+d->w-1, d->y+text_height(font)-1, d->bg);
//			}
			break;
		}

		case MSG_CLICK:
		{
			x = d->x;

			if (scroll)
			{
				p = d->d2-b+1; 
				b = d->d2; 
			}
			else
			{
				p = 0; 
			}

			for (; p<b; p++)
			{
				usetc(buf+usetc(buf, ugetat(s, p)), 0);
				x += text_length(ncdnet_gui_font, buf);
				if (x > gui_mouse_x()) 
				{
					break;
				}
			}
			d->d2 = MID(0, p, l);
			object_message(d, MSG_DRAW, 0);
			break;
		}

		case MSG_WANTFOCUS:
		case MSG_LOSTFOCUS:
		case MSG_KEY:
		{
			return D_WANTFOCUS;
		}

		case MSG_CHAR:
		{
			ignore_next_uchar = FALSE;

			if ((c >> 8) == KEY_LEFT)
			{
				if (d->d2 > 0)
				{
					if (key_shifts & KB_CTRL_FLAG)
					{
						last_was_space = TRUE;
						new_pos = 0;
						t = s;
						for (i = 0; i < d->d2; i++)
						{
							k = ugetx(&t);
							if (uisspace(k))
							{
								last_was_space = TRUE;
							}
							else if (last_was_space)
							{
								last_was_space = FALSE;
								new_pos = i;
							}
						}
						d->d2 = new_pos;
					}
					else
					{
						d->d2--;
					}
				}
			}
			else if ((c >> 8) == KEY_RIGHT)
			{
				if (d->d2 < l)
				{
					if (key_shifts & KB_CTRL_FLAG)
					{
						t = s + uoffset(s, d->d2);
						for (k = ugetx(&t); uisspace(k); k = ugetx(&t))
						{
							d->d2++;
						}
						for (; k && !uisspace(k); k = ugetx(&t))
						{
							d->d2++;
						}
					}
					else
					{
						d->d2++;
					}
				}
			}
			else if ((c >> 8) == KEY_HOME)
			{
				d->d2 = 0;
			}
			else if ((c >> 8) == KEY_END)
			{
				d->d2 = l;
			}
			else if ((c >> 8) == KEY_DEL)
			{
				if (d->d2 < l)
				{
					uremove(s, d->d2);
				}
			}
			else if ((c >> 8) == KEY_BACKSPACE)
			{
				if (d->d2 > 0)
				{
					d->d2--;
					uremove(s, d->d2);
				}
			}
			else if ((c >> 8) == KEY_ENTER)
			{
				if (d->flags & D_EXIT)
				{
					object_message(d, MSG_DRAW, 0);
					return D_CLOSE;
				}
				else
				{
					return D_O_K;
				}
			}
			else if ((c >> 8) == KEY_TAB)
			{
				ignore_next_uchar = TRUE;
				return D_O_K;
			}
			else
			{
				/* don't process regular keys here: MSG_UCHAR will do that */
				break;
			}
			object_message(d, MSG_DRAW, 0);
			return D_USED_CHAR;
		}

		case MSG_UCHAR:
		{
			if ((c >= ' ') && (uisok(c)) && (!ignore_next_uchar))
			{
				if (l < d->d1)
				{
					uinsert(s, d->d2, c);
					d->d2++;
	
					object_message(d, MSG_DRAW, 0);
				}
				return D_USED_CHAR;
			}
			break;
		}
	}
	return D_O_K;
}

int ncdnet_gui_list_proc(int msg, struct DIALOG *d, int c)
{
   int listsize, i, bottom, height, bar, orig;
   char *sel = d->dp2;
   int redraw = FALSE;
   ASSERT(d);

   switch (msg) {

      case MSG_START:
	 (*(getfuncptr)d->dp)(-1, &listsize);
	 _handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);
	 break;

      case MSG_DRAW:
	 ncdnet_gui_draw_listbox(d);
	 break;

      case MSG_CLICK:
	 (*(getfuncptr)d->dp)(-1, &listsize);
	 height = (d->h-4) / text_height(font);
	 bar = (listsize > height);
	 if ((!bar) || (gui_mouse_x() < d->x+d->w-13)) {
	    if ((sel) && (!(key_shifts & KB_CTRL_FLAG))) {
	       for (i=0; i<listsize; i++) {
		  if (sel[i]) {
		     redraw = TRUE;
		     sel[i] = FALSE;
		  }
	       }
	       if (redraw)
		  object_message(d, MSG_DRAW, 0);
	    }
	    _handle_listbox_click(d);
	    while (gui_mouse_b()) {
	       broadcast_dialog_message(MSG_IDLE, 0);
	       d->flags |= D_INTERNAL;
	       _handle_listbox_click(d);
	       d->flags &= ~D_INTERNAL;
	    }
	 }
	 else {
	    _handle_scrollable_scroll_click(d, listsize, &d->d2, height);
	 }
	 break;

      case MSG_DCLICK:
	 (*(getfuncptr)d->dp)(-1, &listsize);
	 height = (d->h-4) / text_height(font);
	 bar = (listsize > height);
	 if ((!bar) || (gui_mouse_x() < d->x+d->w-13)) {
	    if (d->flags & D_EXIT) {
	       if (listsize) {
		  i = d->d1;
		  object_message(d, MSG_CLICK, 0);
		  if (i == d->d1) 
		     return D_CLOSE;
	       }
	    }
	 }
	 break;

      case MSG_WHEEL:
	 (*(getfuncptr)d->dp)(-1, &listsize);
	 height = (d->h-4) / text_height(font);
	 if (height < listsize) {
	    int delta = (height > 3) ? 3 : 1;
	    if (c > 0) 
	       i = MAX(0, d->d2-delta);
	    else
	       i = MIN(listsize-height, d->d2+delta);
	    if (i != d->d2) {
	       d->d2 = i;
	       object_message(d, MSG_DRAW, 0);
	    }
	 }
	 break;

      case MSG_KEY:
	 (*(getfuncptr)d->dp)(-1, &listsize);
	 if ((listsize) && (d->flags & D_EXIT))
	    return D_CLOSE;
	 break;

      case MSG_WANTFOCUS:
	 return D_WANTFOCUS;

      case MSG_CHAR:
	 (*(getfuncptr)d->dp)(-1, &listsize);

	 if (listsize) {
	    c >>= 8;

	    bottom = d->d2 + (d->h-4)/text_height(font) - 1;
	    if (bottom >= listsize-1)
	       bottom = listsize-1;

	    orig = d->d1;

	    if (c == KEY_UP)
	       d->d1--;
	    else if (c == KEY_DOWN)
	       d->d1++;
	    else if (c == KEY_HOME)
	       d->d1 = 0;
	    else if (c == KEY_END)
	       d->d1 = listsize-1;
	    else if (c == KEY_PGUP) {
	       if (d->d1 > d->d2)
		  d->d1 = d->d2;
	       else
		  d->d1 -= (bottom - d->d2) ? bottom - d->d2 : 1;
	    }
	    else if (c == KEY_PGDN) {
	       if (d->d1 < bottom)
		  d->d1 = bottom;
	       else
		  d->d1 += (bottom - d->d2) ? bottom - d->d2 : 1;
	    } 
	    else 
	       return D_O_K;

	    if (sel) {
	       if (!(key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG))) {
		  for (i=0; i<listsize; i++)
		     sel[i] = FALSE;
	       }
	       else if (key_shifts & KB_SHIFT_FLAG) {
		  for (i=MIN(orig, d->d1); i<=MAX(orig, d->d1); i++) {
		     if (key_shifts & KB_CTRL_FLAG)
			sel[i] = (i != d->d1);
		     else
			sel[i] = TRUE;
		  }
	       }
	    }

	    /* if we changed something, better redraw... */ 
	    _handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);
	    d->flags |= D_DIRTY;
	    return D_USED_CHAR;
	 }
	 break;
   }

   return D_O_K;
}

int ncdnet_gui_text_list_proc(int msg, struct DIALOG *d, int c)
{
	int listsize, i, a, failure;
	char *selected, *thisitem;
	char *sel = d->dp2;
	ASSERT(d);

	switch (msg)
	{

		case MSG_START:
		case MSG_CLICK:
		case MSG_DCLICK:
		case MSG_WANTFOCUS:
		case MSG_LOSTFOCUS:
		{
			d->dp3 = 0;
			break;
		}

		case MSG_CHAR:
		{
			if ((c & 0xFF) < ' ')
			{
				d->dp3 = 0;
			}
			break;
		}

		case MSG_UCHAR:
		{
			(*(getfuncptr)d->dp)(-1, &listsize);
	
			if (listsize)
			{
				if (c >= ' ')
				{
					selected = (*(getfuncptr)d->dp)(d->d1, NULL);
	
					i = d->d1;
	
					do
					{
						thisitem = (*(getfuncptr)d->dp)(i, NULL);
						failure = FALSE;
	
						if ((int)((unsigned long)d->dp3) < ustrlen(thisitem))
						{
							for (a=0; a < (int)((unsigned long)d->dp3); a++)
							{
								if (utolower(ugetat(thisitem, a)) != utolower(ugetat(selected, a)))
								{	
									failure = TRUE;
									break;
								}	
							}	
	
							if ((!failure) && (utolower(ugetat(thisitem, (int)(unsigned long)d->dp3)) == utolower(c)))
							{
								d->d1 = i;
								d->dp3 = (void *)((unsigned long)d->dp3 + 1);
	
								if (sel)
								{
									for (i=0; i<listsize; i++)
									{
										sel[i] = FALSE;
									}
								}
	
								_handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);
								object_message(d, MSG_DRAW, 0);
								return D_USED_CHAR;
							}
						}
	
						i++;
						if (i >= listsize)
						{
							i = 0;
						}
	
					} while (i != d->d1);
	
					if (d->dp3)
					{
						d->dp3 = 0;
						return d_text_list_proc(msg, d, c);
					}
				}
			}
			break;
		}
   }

   return ncdnet_gui_list_proc(msg, d, c);
}

int ncdnet_gui_check_proc(int msg, DIALOG *d, int c)
{
   BITMAP *gui_bmp = gui_get_screen();
   int x;
   int fg, bg;
   ASSERT(d);

   if (msg==MSG_DRAW) {
      fg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
      bg = (d->bg < 0) ? gui_bg_color : d->bg;

      x = d->x + ((d->d1) ? 0 : gui_textout_ex(gui_bmp, d->dp, d->x, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, fg, d->bg, FALSE) + text_height(font)/2);
	inner_bevel_box(screen, x + 3, d->y + 3, x + d->h - 4, d->y + d->h - 4, makecol(255, 255, 255), d->fg, d->bg + 7, d->bg - 7, 1, 1);
//      rectfill(gui_bmp, x+1, d->y+1, x+d->h-2, d->y+d->h-2, bg);
//      rect(gui_bmp, x, d->y, x+d->h-1, d->y+d->h-1, fg);
      if (d->d1)
	 gui_textout_ex(gui_bmp, d->dp, x+d->h-1+text_height(font)/2, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, fg, d->bg, FALSE);
      if (d->flags & D_SELECTED) {
	 line(gui_bmp, x, d->y, x+d->h-1, d->y+d->h-1, fg);
	 line(gui_bmp, x, d->y+d->h-1, x+d->h-1, d->y, fg);
      }
//      if (d->flags & D_GOTFOCUS)
//	 dotted_rect(x+1, d->y+1, x+d->h-2, d->y+d->h-2, fg, bg);

      return D_O_K;
   } 

   return d_button_proc(msg, d, 0);
}

void ncdnet_gui_draw_menu(int x, int y, int w, int h)
{
   int c;

   outer_bevel_box(screen, x, y, x + w - 2, y + h - 1, gui_bg_color, gui_fg_color, gui_bg_color + 7, gui_bg_color - 7, 1, 0);
//      rect(screen, m->x, m->y, m->x+m->w-2, m->y+m->h-2, gui_fg_color);
  //    vline(screen, m->x+m->w-1, m->y+1, m->y+m->h-1, gui_fg_color);
    //  hline(screen, m->x+1, m->y+m->h-1, m->x+m->w-1, gui_fg_color);
//   }

//   for (c=0; m->menu[c].text; c++)
//      draw_menu_item(m, c);
}

void ncdnet_gui_draw_menu_item(MENU *m, int x, int y, int w, int h, int bar, int sel)
{
   int fg, bg;
//   int x, y, w;
   char *buf, *tok1, *tok2;
   int my;

   if (m->flags & D_DISABLED) {
      if (sel) {
	 fg = gui_mg_color;
//	 bg = gui_fg_color;
	bg = makecol(0, 0, 255);
      }
      else {
	 fg = gui_mg_color;
	 bg = gui_bg_color;
      }
   }
   else {
      if (sel) {
//	 fg = gui_bg_color;
//	 bg = gui_fg_color;
	 fg = makecol(255, 255, 255);
	 bg = makecol(0, 0, 255);
      }
      else {
	 fg = gui_fg_color;
	 bg = gui_bg_color;
      }
   }

   rectfill(screen, x, y, x+w-1, y+text_height(font)+3, bg);

   if (ugetc(m->text)) {
      buf = split_around_tab(m->text, &tok1, &tok2);
      gui_textout_ex(screen, tok1, x+8, y+1, fg, bg, FALSE);
      if (tok2)
 	 gui_textout_ex(screen, tok2, x+w-gui_strlen(tok2)-10, y+1, fg, bg, FALSE);

      if ((m->child) && (!bar)) {
         my = y + text_height(font)/2;
         hline(screen, x+w-8, my+1, x+w-4, fg);
         hline(screen, x+w-8, my+0, x+w-5, fg);
         hline(screen, x+w-8, my-1, x+w-6, fg);
         hline(screen, x+w-8, my-2, x+w-7, fg);
         putpixel(screen, x+w-8, my-3, fg);
         hline(screen, x+w-8, my+2, x+w-5, fg);
         hline(screen, x+w-8, my+3, x+w-6, fg);
         hline(screen, x+w-8, my+4, x+w-7, fg);
         putpixel(screen, x+w-8, my+5, fg);
      }

      free(buf);
   }
   else
      hline(screen, x, y+text_height(font)/2+2, x+w, fg);

   if (m->flags & D_SELECTED) {
      line(screen, x+1, y+text_height(font)/2+1, x+3, y+text_height(font)+1, fg);
      line(screen, x+3, y+text_height(font)+1, x+6, y+2, fg);
   }
}

void ncdnet_gui_init(void)
{
	gui_shadow_box_proc = ncdnet_gui_shadow_box_proc;
	gui_button_proc = ncdnet_gui_button_proc;
	gui_edit_proc = ncdnet_gui_edit_proc;
	gui_list_proc = ncdnet_gui_list_proc;
	gui_text_list_proc = ncdnet_gui_text_list_proc;
	gui_menu_draw_menu = ncdnet_gui_draw_menu;
	gui_menu_draw_menu_item = ncdnet_gui_draw_menu_item;
	ncdnet_gui_font = font;
}

DIALOG main_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { ncdnet_gui_shadow_box_proc, 160, 152, 320, 176, 2, 23, 0, 0, 0, 0, NULL, NULL, NULL },
//   { ncdnet_gui_button_proc, 168, 128, 304, 32, 2, 23, 0, D_EXIT, 0, 0, "Normal Game", NULL, NULL },
   { ncdnet_gui_button_proc, 168, 160, 304, 32, 2, 23, 0, D_EXIT, 0, 0, "Host Game", NULL, NULL },
   { ncdnet_gui_button_proc, 168, 192, 304, 32, 2, 23, 0, D_EXIT, 0, 0, "Host Private Game", NULL, NULL },
   { ncdnet_gui_button_proc, 168, 224, 304, 32, 2, 23, 0, D_EXIT, 0, 0, "Join Game", NULL, NULL },
   { ncdnet_gui_button_proc, 168, 256, 304, 32, 2, 23, 0, D_EXIT, 0, 0, "Join Private Game", NULL, NULL },
   { ncdnet_gui_button_proc, 168, 288, 304, 32, 2, 23, 0, D_EXIT, 0, 0, "Quit", NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG list_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { ncdnet_gui_shadow_box_proc, 160, 120, 320, 240, 2, 23, 0,      0, 0, 0,          NULL, NULL, NULL },
   { ncdnet_gui_list_proc,       168, 128, 304, 176, 2, 23, 0, D_EXIT, 0, 0, list_get_text, NULL, NULL },
   { ncdnet_gui_button_proc,     168, 320, 152,  32, 2, 23, 0, D_EXIT, 0, 0,     "Connect", NULL, NULL },
   { ncdnet_gui_button_proc,     320, 320, 152,  32, 2, 23, 0, D_EXIT, 0, 0,      "Cancel", NULL, NULL },
   { NULL,                    0,   0,   0,  0,  0,  0, 0,      0, 0, 0,          NULL, NULL, NULL }
};

DIALOG ip_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { ncdnet_gui_shadow_box_proc, 80, 60, 184, 64, 2, 23, 0, 0, 0, 0, NULL, NULL, NULL },
   { d_ctext_proc, 120, 68, 104, 8, 2, 23, 0, 0, 0, 0, "Enter IP", NULL, NULL },
//   { d_text_proc, 88, 84, 64, 8, 2, 23, 0, 0, 0, 0, "IP:", NULL, NULL },
   { ncdnet_gui_edit_proc, 96, 84, 152, 8, 2, 23, 0, 0, 15, 0, etext, NULL, NULL },
   { ncdnet_gui_button_proc, 92, 100, 76, 16, 2, 23, 0, D_EXIT, 0, 0, "OK", NULL, NULL },
   { ncdnet_gui_button_proc, 176, 100, 76, 16, 2, 23, 0, D_EXIT, 0, 0, "Cancel", NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG name_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { ncdnet_gui_shadow_box_proc, 80, 60, 184, 64, 2, 23, 0, 0, 0, 0, NULL, NULL, NULL },
   { d_ctext_proc, 120, 68, 104, 8, 2, 23, 0, 0, 0, 0, "Enter Server Name", NULL, NULL },
//   { d_text_proc, 88, 84, 64, 8, 2, 23, 0, 0, 0, 0, "Name:", NULL, NULL },
   { ncdnet_gui_edit_proc, 96, 84, 152, 8, 2, 23, 0, 0, 63, 0, stext, NULL, NULL },
   { ncdnet_gui_button_proc, 92, 100, 76, 16, 2, 23, 0, D_EXIT, 0, 0, "OK", NULL, NULL },
   { ncdnet_gui_button_proc, 176, 100, 76, 16, 2, 23, 0, D_EXIT, 0, 0, "Cancel", NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG server_wait_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { ncdnet_gui_shadow_box_proc, 80, 60, 184, 48, 2, 23, 0, 0, 0, 0, NULL, NULL, NULL },
   { d_ctext_proc, 120, 68, 104, 8, 2, 23, 0, 0, 0, 0, "Waiting for Players", NULL, NULL },
//   { d_text_proc, 88, 84, 64, 8, 2, 23, 0, 0, 0, 0, "Name:", NULL, NULL },
//   { ncdnet_gui_edit_proc, 96, 84, 152, 8, 2, 23, 0, 0, 15, 0, stext, NULL, NULL },
//   { ncdnet_gui_button_proc, 92, 100, 76, 16, 2, 23, 0, D_EXIT, 0, 0, "OK", NULL, NULL },
   { ncdnet_gui_button_proc, 88, 84, 164, 16, 2, 23, 0, D_EXIT, 0, 0, "Cancel", NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

JOYNET_SERVER_INFO ncdnet_client_info[256];
int ncdnet_client_info_count = 0;
int ncdnet_client_ip_pos = 0;
char ncdnet_client_server_ip[256] = {0};

int ncdnet_client_list_flag = 0;
int ncdnet_client_state = 0;
int ncdnet_client_selected_ip = 0;
int ncdnet_client_listed = 0;
int ncdnet_private_server = 0;

PALETTE ncdnet_palette;

JOYNET_CLIENT * ncdnet_client = NULL;
JOYNET_SERVER * ncdnet_server = NULL;
int ncdnet_client_game_type = 0;
int ncdnet_client_max_clients = 0;
BITMAP * ncdnet_splash = NULL;
FONT * ncdnet_font = NULL;

void poll_ncdnet(void)
{
	if(ncdnet_server)
	{
		joynet_poll_server(ncdnet_server);
	}
	if(ncdnet_client)
	{
		joynet_poll_client(ncdnet_client);
	}
}

void ncdnet_show_splash(void)
{
	char out_text[256];
	if(ncdnet_splash)
	{
		stretch_blit(ncdnet_splash, screen, 0, 0, ncdnet_splash->w, ncdnet_splash->h, 0, 0, SCREEN_W, SCREEN_H);
	}
	else
	{
		clear_to_color(screen, makecol(0, 0, 0));
	}
	sprintf(out_text, "NCDNet v0.1 (c) 2005 NewCreature Design.");
	textprintf_ex(screen, font, SCREEN_W - text_length(font, out_text), SCREEN_H - text_height(font), makecol(255, 255, 255), -1, "%s", out_text);
}

char * list_get_text(int index, int * size)
{
	if(index >= 0)
	{
//		return layer_list_text[index];
		if(index >= ncdnet_client_ip_pos || ncdnet_client_info[index].max_players <= 0)
		{
			sprintf(server_list_text[index], "%s - %s (n/a)", ncdnet_ip_name[index], ncdnet_ip_string[index]);
		}
		else
		{
			sprintf(server_list_text[index], "%s - %s (%d/%d)", ncdnet_ip_name[index], ncdnet_ip_string[index], ncdnet_client_info[index].players, ncdnet_client_info[index].max_players);
		}
		return server_list_text[index];
	}
	if(index < 0)
	{
		*size = ncdnet_ip_count;
	}
	return NULL;
}

/* returns 0 for success, otherwise error code */
int ncdnet_client_start_public_server(char * name)
{
	int done = 0;
	
	ncdnet_client_listed = 0;
	
	/* create the server */
	if(!ncdnet_server)
	{
		ncdnet_server = joynet_create_server(ncdnet_client_max_clients);
		if(!ncdnet_server)
		{
			return 1;
		}
	}
				
	/* create the client */
	if(!ncdnet_client)
	{
		ncdnet_client = joynet_create_client();
		if(!ncdnet_client)
		{
			return 2;
		}
	}
	if(ncdnet_private_server)
	{
		if(!joynet_start_server(ncdnet_server, 5556))
		{
			joynet_destroy_server(ncdnet_server);
			ncdnet_server = NULL;
			return 6;
		}
		if(!joynet_connect(ncdnet_client, "127.0.0.1", 5556))
		{
			joynet_destroy_client(ncdnet_client);
			ncdnet_client = NULL;
			joynet_close_server(ncdnet_server);
			while(ncdnet_server->state != SERVER_STATE_CLOSED)
			{
				poll_ncdnet();
			}
			joynet_destroy_server(ncdnet_server);
			ncdnet_server = NULL;
			return 7;
		}
		return 0;
	}
				
	done = 1;
				
	/* open server to public */
	if(!joynet_connect(ncdnet_client, ncdnet_client_server_ip, 5555))
	{
		return 3;
	}
	while(1)
	{
		if(ncdnet_client->state == CLIENT_STATE_CONNECTED && !ncdnet_client_listed)
		{
			if(!stricmp(name, ""))
			{
				sprintf(joynet_internal_buffer, " Unnamed Server");
			}
			else
			{
				sprintf(joynet_internal_buffer, " %s", name);
			}
			joynet_internal_buffer[0] = ncdnet_client_game_type;
			joynet_add_message(ncdnet_client->outbox, NCDNET_MESSAGE_START_SERVER, NULL, joynet_internal_buffer, 0);
			ncdnet_client_listed = 1;
		}
		poll_ncdnet();
		if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED)
		{
			if(!ncdnet_client_listed)
			{
				done = 0;
				return 4;
			}
			else
			{
				/* connect to yourself (makes client-server) */
				if(!joynet_start_server(ncdnet_server, 5556))
				{
					joynet_destroy_server(ncdnet_server);
					ncdnet_server = NULL;
					return 6;
				}
				if(!joynet_connect(ncdnet_client, "127.0.0.1", 5556))
				{
					joynet_destroy_client(ncdnet_client);
					ncdnet_client = NULL;
					joynet_close_server(ncdnet_server);
					while(ncdnet_server->state != SERVER_STATE_CLOSED)
					{
						poll_ncdnet();
					}
					joynet_destroy_server(ncdnet_server);
					ncdnet_server = NULL;
					return 7;
				}
				return 0;
			}
		}
		rest(50);
	}
	return 5;
}

int ncdnet_client_close_public_server(void)
{
	/* close the server */
	joynet_close_server(ncdnet_server);
	while(ncdnet_server->state != SERVER_STATE_CLOSED || ncdnet_client->state != CLIENT_STATE_NOT_CONNECTED)
	{
		poll_ncdnet();
	}
	if(ncdnet_private_server)
	{
		return 0;
	}
	
	if(!joynet_connect(ncdnet_client, ncdnet_client_server_ip, 5555))
	{
		return 1;
	}
	int ldone = 0;
	while(1)
	{
		if(ncdnet_client->state == CLIENT_STATE_CONNECTED && !ldone)
		{
			joynet_add_message(ncdnet_client->outbox, NCDNET_MESSAGE_CLOSE_SERVER, NULL, NULL, 0);
			ldone = 1;
		}
		poll_ncdnet();
		if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED)
		{
			if(!ldone)
			{
				return 2;
			}
			break;
		}
		rest(50);
	}
	joynet_destroy_server(ncdnet_server);
	ncdnet_server = NULL;
	joynet_destroy_client(ncdnet_client);
	ncdnet_client = NULL;	
	return 0;
}

int ncdnet_client_retrieve_list(void)
{
	ncdnet_ip_count = 0;
	ncdnet_client_ip_pos = 0;
	ncdnet_client_info_count = 0;
	if(!ncdnet_client)
	{
		ncdnet_client = joynet_create_client();
		if(!ncdnet_client)
		{
			return 1;
		}
	}
	if(!joynet_connect(ncdnet_client, ncdnet_client_server_ip, 5555))
	{
		return 2;
	}
	int ldone = 0;
	ncdnet_show_splash();
	textprintf_centre_ex(screen, font, SCREEN_W / 2, SCREEN_H / 2 - text_height(font) / 2, makecol(255, 255, 255), -1, "Downloading Server List");
	while(1)
	{
		if(ncdnet_client->state == CLIENT_STATE_CONNECTED && !ldone)
		{
			joynet_internal_buffer[0] = ncdnet_client_game_type;
			joynet_add_message(ncdnet_client->outbox, NCDNET_MESSAGE_GET_LIST, NULL, joynet_internal_buffer, 0);
			ldone = 1;
		}
		poll_ncdnet();
		if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED)
		{
			if(!ldone)
			{
				return 3;
			}
			else if(ncdnet_ip_count <= 0)
			{
				ncdnet_show_splash();
				alert(NULL, "No servers currently running!", NULL, "OK", NULL, 0, 0);
				ldone = 0;
				return 0;
			}
			break;
		}
		rest(50);
	}
	
	return 0;
}

void ncdnet_client_logic(void)
{
	poll_ncdnet();
	if(ncdnet_client)
	{
		if(ncdnet_client->state == CLIENT_STATE_CONNECTED && !ncdnet_client_list_flag)
		{
			joynet_internal_buffer[0] = 1;
			joynet_add_message(ncdnet_client->outbox, NCDNET_MESSAGE_GET_LIST, NULL, joynet_internal_buffer, 0);
			ncdnet_client_list_flag = 1;
		}
		if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED && !ncdnet_client_list_flag)
		{
		}
	}
}

int load_ip_config(char * fn)
{
	PACKFILE * fp;
	char buffer[1024] = {0};
	int size;
	int i;
	
	size = file_size(fn);
	fp = pack_fopen(fn, "r");
	if(!fp)
	{
		return 0;
	}
	pack_fread(buffer, size, fp);
	pack_fclose(fp);
	for(i = 0; i < size; i++)
	{
		if(buffer[i] == '\n' || buffer[i] == '\r')
		{
			buffer[i] = 0;
		}
	}
	strcpy(ncdnet_client_server_ip, buffer);
	return 1;
	
}

int my_dialog(DIALOG * dialog)
{
	DIALOG_PLAYER *player;
	
	centre_dialog(dialog);
	player = init_dialog(dialog, 0);
	show_mouse(screen);
	while (update_dialog(player))
	{
		ncdnet_client_logic();
		rest(10);
	}
	show_mouse(NULL);
	return shutdown_dialog(player);
}

int my_wait_dialog(DIALOG * dialog)
{
	DIALOG_PLAYER *player;
	int i;
	int done = 0;
	int tcount;
	int rerror = 0;
	
	centre_dialog(dialog);
	player = init_dialog(dialog, 0);
	show_mouse(screen);
	int pdone = 0;
	int pcon = 0;
	while (update_dialog(player) && !pdone)
	{
		joynet_poll_client(ncdnet_client);
		
		/* make sure client is set to play so we can count correctly */
		if(ncdnet_client->state == CLIENT_STATE_CONNECTED && !pcon)
		{
			joynet_internal_buffer[0] = 1;
			joynet_add_message(ncdnet_client->outbox, JOYNET_MESSAGE_SET_CLIENT_TYPE, NULL, joynet_internal_buffer, 0);
			pcon = 1;
		}
		if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED)
		{
			rerror = 1;
			break;
		}
		poll_ncdnet();
//		ncdnet_client_logic();
		rest(30);
		tcount = 0;
		for(i = 0; i < ncdnet_server->max_clients; i++)
		{
//			textprintf_ex(screen, font, 0, i * 8, makecol(255, 255, 255), 0, "%d %d", ncdnet_server->client[i]->state, ncdnet_server->client[i]->type);
			if(ncdnet_server->client[i]->state == CLIENT_STATE_CONNECTED && ncdnet_server->client[i]->type == CLIENT_TYPE_PLAYING)
			{
				tcount++;
				if(tcount > 1)
				{
					done = 1;
					pdone = 1;
					joynet_server_find_master(ncdnet_server);
				}
			}
		}
	}
	show_mouse(NULL);
	if(rerror)
	{
		alert(NULL, "Failed to set up client/server!", NULL, "OK", NULL, 0, 0);
		return -2;
	}
	if(!done)
	{
		return shutdown_dialog(player);
	}
	set_control_mode(ncdnet_server, ncdnet_client);
	return -1;
}

int my_wait_dialog2(DIALOG * dialog)
{
	DIALOG_PLAYER *player;
	int i;
	int done = 0;
	
	centre_dialog(dialog);
	player = init_dialog(dialog, 0);
	show_mouse(screen);
	int pdone = 0;
	while (update_dialog(player) && !pdone)
	{
		/* get info on servers */
		if(ncdnet_client_ip_pos < ncdnet_ip_count)
		{
			if(info_client)
			{
				poll_server_info();
				if(info_flag || !info_client || (info_client && info_client->state == CLIENT_STATE_NOT_CONNECTED))
				{
					ncdnet_client_ip_pos++;
					ncdnet_client_info_count++;
					dialog_message(dialog, MSG_DRAW, 0, NULL);
				}
			}
			else
			{
				get_server_info(ncdnet_ip_string[ncdnet_client_ip_pos], 5556, &ncdnet_client_info[ncdnet_client_info_count]);
			}
		}
		
		ncdnet_client_logic();
		rest(30);
	}
	show_mouse(NULL);
	return shutdown_dialog(player);
}

/* run this to show the client program, when it returns your game should initialize and run */
int ncdnet_main(int argc, char * argv[])
{
	int ret, ret2;
	int done = 0;
	FONT * old_font = font;
	
	ncdnet_gui_init();
	set_palette(black_palette);
	if(set_gfx_mode(GFX_AUTODETECT_WINDOWED, 600, 450, 0, 0))
	{
		if(set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0))
		{
			return 1;
		}
	}
	set_display_switch_mode(SWITCH_BACKGROUND);
	destroy_bitmap(load_pcx("ncdnet.dat#palette.pcx", ncdnet_palette));
	if(exists("ncdnet.dat#splash.pcx"))
	{
		ncdnet_splash = load_pcx("ncdnet.dat#splash.pcx", NULL);
	}
	if(exists("ncdnet.dat#font.pcx"))
	{
		ncdnet_font = load_font("ncdnet.dat#font.pcx", NULL, NULL);
		ncdnet_gui_font = ncdnet_font;
		font = ncdnet_font;
	}
	ncdnet_show_splash();
	set_palette(ncdnet_palette);
	gui_fg_color = 2;
	gui_bg_color = 23;
//	font = ncdnet_font;
	set_mouse_sprite(NULL);
	strcpy(stext, "Unnamed");
	
	while(1)
	{
		ncdnet_show_splash();
		ret = my_dialog(main_dialog);
		switch(ret)
		{
			/* start a public server, you must open the server yourself and connect
			   the client to it yourself */
			case 1:
			{
//				centre_dialog(name_dialog);
				ncdnet_show_splash();
				ret2 = my_dialog(name_dialog);
				if(ret2 == 4)
				{
					break;
				}
				load_ip_config("ip.cfg");
				int tries = 5;
				int ret_code;
				ncdnet_show_splash();
				textprintf_centre_ex(screen, font, SCREEN_W / 2, SCREEN_H / 2 - text_height(font) / 2, makecol(255, 255, 255), -1, "Connecting to list server!");
				while((ret_code = ncdnet_client_start_public_server(stext)) && tries > 0)
				{
					if(tries == 5)
					{
						if(!download_file("ip.cfg", "http://ncdgames.t3-i.com/ip.cfg"))
						{
							ncdnet_show_splash();
							alert(NULL, "Could not find list server!", NULL, "OK", NULL, 0, 0);
							tries = 0;
							break;
						}
						load_ip_config("ip.cfg");
					}
//					textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), 0, "%d", ret_code);
					rest(1000);
					tries--;
				}
				if(!ncdnet_client_listed)
				{
					ncdnet_show_splash();
					alert(NULL, "Server could not be listed!", NULL, "OK", NULL, 0, 0);
				}
				else
				{
					ncdnet_show_splash();
					ret2 = my_wait_dialog(server_wait_dialog);
					if(ret2 == -1)
					{
						return 0;
					}
					else if(ret == -2)
					{
						return 1;
					}
					else if(ret2 == 2)
					{
						tries = 5;
						ncdnet_show_splash();
						textprintf_centre_ex(screen, font, SCREEN_W / 2, SCREEN_H / 2 - text_height(font) / 2, makecol(255, 255, 255), -1, "Connecting to list server!");
						while((ret_code = ncdnet_client_close_public_server()) && tries > 0)
						{
//							textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), 0, "%d", ret_code);
							rest(1000);
							tries--;
						}
						if(tries <= 0)
						{
							ncdnet_show_splash();
							alert(NULL, "Server could not be removed from list!", NULL, "OK", NULL, 0, 0);
						}
					}
				}
				break;
			}
			case 2:
			{
				ncdnet_private_server = 1;
				ncdnet_client_start_public_server(stext);
				ncdnet_show_splash();
				textprintf_centre_ex(screen, font, SCREEN_W / 2 - 1, 1, makecol(0, 0, 0), -1, "%s", joynet_local_address_string);
				textprintf_centre_ex(screen, font, SCREEN_W / 2 + 1, 1, makecol(0, 0, 0), -1, "%s", joynet_local_address_string);
				textprintf_centre_ex(screen, font, SCREEN_W / 2, 0, makecol(0, 0, 0), -1, "%s", joynet_local_address_string);
				textprintf_centre_ex(screen, font, SCREEN_W / 2, 2, makecol(0, 0, 0), -1, "%s", joynet_local_address_string);
				textprintf_centre_ex(screen, font, SCREEN_W / 2, 1, makecol(255, 255, 255), -1, "%s", joynet_local_address_string);
				ret2 = my_wait_dialog(server_wait_dialog);
				if(ret2 == -1)
				{
					return 0;
				}
				else if(ret2 == 2)
				{
					ncdnet_client_close_public_server();
					ncdnet_private_server = 0;
				}
				break;
			}			
			case 3:
			{
				int loopflag = 1;
				
				/* load config first */
				load_ip_config("ip.cfg");
				int tries = 5;
				
				/* attempt to connect */
				while(ncdnet_client_retrieve_list() && tries > 0)
				{
					
					/* if first try fails, download the config from the web */
					if(tries == 5)
					{
						if(!download_file("ip.cfg", "http://ncdgames.t3-i.com/ip.cfg"))
						{
							alert(NULL, "Could not find list server!", NULL, "OK", NULL, 0, 0);
							tries = 0;
							break;
						}
						load_ip_config("ip.cfg");
					}
					rest(1000);
					tries--;
				}
				if(tries <= 0)
				{
					ncdnet_show_splash();
					alert(NULL, "Could not download server list!", NULL, "OK", NULL, 0, 0);
				}
				while(ncdnet_ip_count && loopflag)
				{
					ret2 = my_wait_dialog2(list_dialog);
					if(ret2 == 1 || ret2 == 2)
					{
						done = 1;
						if(!joynet_connect(ncdnet_client, ncdnet_ip_string[list_dialog[1].d1], 5556))
						{
							return 1;
						}
						ncdnet_show_splash();
						textprintf_centre_ex(screen, font, SCREEN_W / 2, SCREEN_H / 2 - text_height(font) / 2, makecol(255, 255, 255), -1, "Attempting Connection");
						while(ncdnet_client->state != CLIENT_STATE_CONNECTED && done)
						{
							rest(50);
							joynet_poll_client(ncdnet_client);
							if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED)
							{
								ncdnet_show_splash();
								alert(NULL, "Failed to connect!", NULL, "OK", NULL, 0, 0);
								done = 0;
							}
						}
						if(done)
						{
							set_control_mode(ncdnet_server, ncdnet_client);
							joynet_internal_buffer[0] = 1;
							joynet_add_message(ncdnet_client->outbox, JOYNET_MESSAGE_SET_CLIENT_TYPE, NULL, joynet_internal_buffer, 0);
							while(ncdnet_client->type != CLIENT_TYPE_PLAYING && ncdnet_client->master < 0)
							{
								poll_ncdnet();
								if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED)
								{
									alert(NULL, "Could not enter game!", NULL, "OK", NULL, 0, 0);
									return 1;
								}
							}
							return 0;
						}
					}
					else if(ret2 == 3)
					{
						loopflag = 0;
					}
				}
				break;
			}
			case 4:
			{
//				centre_dialog(ip_dialog);
				ncdnet_show_splash();
				ret2 = my_dialog(ip_dialog);
				if(ret2 == 3)
				{
					ncdnet_client = joynet_create_client();
					if(!ncdnet_client)
					{
						return 1;
					}
					done = 1;
					if(!joynet_connect(ncdnet_client, etext, 5556))
					{
						done = 0;
					}
					ncdnet_show_splash();
					textprintf_centre_ex(screen, font, SCREEN_W / 2, SCREEN_H / 2 - text_height(font) / 2, makecol(255, 255, 255), -1, "Attempting Connection");
					while(ncdnet_client->state != CLIENT_STATE_CONNECTED && done)
					{
						rest(50);
						joynet_poll_client(ncdnet_client);
						if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED)
						{
							ncdnet_show_splash();
							alert(NULL, "Failed to connect!", NULL, "OK", NULL, 0, 0);
							done = 0;
						}
					}
					if(done)
					{
						set_control_mode(ncdnet_server, ncdnet_client);
						joynet_internal_buffer[0] = 1;
						joynet_add_message(ncdnet_client->outbox, JOYNET_MESSAGE_SET_CLIENT_TYPE, NULL, joynet_internal_buffer, 0);
						while(ncdnet_client->type != CLIENT_TYPE_PLAYING && ncdnet_client->master < 0)
						{
							poll_ncdnet();
							if(ncdnet_client->state == CLIENT_STATE_NOT_CONNECTED)
							{
								alert(NULL, "Could not enter game!", NULL, "OK", NULL, 0, 0);
								return 1;
							}
						}
						return 0;
					}
				}
				break;
			}
			case 5:
			{
				return 1;
			}
			default:
			{
				return 1;
			}
		}
	}
	return 1;
}

void ncdnet_client_exit(void)
{
	if(ncdnet_splash)
	{
		destroy_bitmap(ncdnet_splash);
		ncdnet_splash = NULL;
	}
	if(ncdnet_font)
	{
		destroy_font(ncdnet_font);
		ncdnet_font = NULL;
	}
}
