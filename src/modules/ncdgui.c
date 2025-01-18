#include <allegro.h>
#include "agup/agup.h"
#include "gametime.h"
#include "g-idle.h"
#include "ncdgui.h"

static int ncdgui_cursor_type = 0;
static int ncdgui_keyboard_shortcut = 0;

static int work_around_fsel_bug = 0;
/* The AGUP d_agup_edit_proc isn't exactly compatible to Allegro's stock
 * d_edit_proc, which has no border at all. Therefore we resort to this little
 * hack.
 */
int d_hackish_edit_proc (int msg, DIALOG *d, int c)
{
    if (msg == MSG_START && !work_around_fsel_bug)
    {
        /* Adjust position/dimension so it is the same as AGUP's. */
        d->y -= 3;
        d->h += 6;
        /* The Allegro GUI has a bug where it repeatedely sends MSG_START to a
         * custom GUI procedure. We need to work around that.
         */
        work_around_fsel_bug = 1;
    }
    if (msg == MSG_END && work_around_fsel_bug)
    {
        d->y += 3;
        d->h -= 6;
        work_around_fsel_bug = 0;
    }
    return d_agup_edit_proc (msg, d, c);
}

void ncdgui_color_dialog(DIALOG * dp, int fg, int bg)
{
	int i;
	
	for(i = 0; dp[i].proc != NULL; i++)
	{
		dp[i].fg = fg;
		dp[i].bg = bg;
	}
}

static void ncdgui_show_mouse(BITMAP * bp)
{
	if(ncdgui_cursor_type == NCDGUI_CURSOR_SOFTWARE)
	{
		show_mouse(bp);
	}
}

static void ncdgui_clear_input(void)
{
	int i;
	
	clear_keybuf();
	for(i = 0; i < KEY_MAX; i++)
	{
		if(i != KEY_ALTGR && i != KEY_ALT && i != KEY_LCONTROL && i != KEY_RCONTROL && i != KEY_LSHIFT && i != KEY_RSHIFT)
		{
			key[i] = 0;
		}
	}
}

int ncdgui_popup_dialog(DIALOG * dp, int n)
{
	int ret;
	
	DIALOG_PLAYER * player = init_dialog(dp, n);
	if(!player)
	{
		allegro_message("no player!");
		return 0;
	}
	ncdgui_show_mouse(screen);
	clear_keybuf();
	while(update_dialog(player))
	{
		
		/* special handling of the main menu */
		if(dp[0].proc == d_agup_menu_proc && !dp[1].proc)
		{
			
			/* if user wants the menu, force dialog to stay open */
			if(key[KEY_ALTGR] && !ncdgui_keyboard_shortcut)
			{
				if(key[KEY_F])
				{
					simulate_keypress(KEY_F << 8);
					ncdgui_keyboard_shortcut = 1;
					player->mouse_obj = 0;
				}
				else if(key[KEY_E])
				{
					simulate_keypress(KEY_E << 8);
					ncdgui_keyboard_shortcut = 1;
					player->mouse_obj = 0;
				}
				else if(key[KEY_S])
				{
					simulate_keypress(KEY_S << 8);
					ncdgui_keyboard_shortcut = 1;
					player->mouse_obj = 0;
				}
				else if(key[KEY_N])
				{
					simulate_keypress(KEY_N << 8);
					ncdgui_keyboard_shortcut = 1;
					player->mouse_obj = 0;
				}
				else if(key[KEY_B])
				{
					simulate_keypress(KEY_B << 8);
					ncdgui_keyboard_shortcut = 1;
					player->mouse_obj = 0;
				}
				else if(key[KEY_H])
				{
					simulate_keypress(KEY_H << 8);
					ncdgui_keyboard_shortcut = 1;
					player->mouse_obj = 0;
				}
			}
			else if(key[KEY_F] || key[KEY_E] || key[KEY_S] || key[KEY_N] || key[KEY_B] || key[KEY_H])
			{
				ncdgui_keyboard_shortcut = 1;
				player->mouse_obj = 0;
			}
			
			if(mouse_b & 1)
			{
				ncdgui_keyboard_shortcut = 2;
			}
			if(ncdgui_keyboard_shortcut == 2 && !(mouse_b & 1))
			{
				ncdgui_keyboard_shortcut = 0;
			}
			
			/* if mouse isn't hovering over the menu, try and deactivate it */
			if(player->mouse_obj < 0)
			{
				/* if the user isn't about to press a menu shortcut */
				if(!KEY_EITHER_ALT && !ncdgui_keyboard_shortcut)
				{
					break;
				}
			}
		}
		
		Idle(10);
	}
	ret = shutdown_dialog(player);
	
	ncdgui_clear_input();
   	gametime_reset();
   	ncdgui_show_mouse(NULL);
	ncdgui_keyboard_shortcut = 0;
   	
   	return ret;
}

static int ncdgui_get_x(DIALOG * dp)
{
	int first_x = 32768;
	int i;
	
	for(i = 0; dp[i].proc != NULL; i++)
	{
		if(dp[i].x < first_x)
		{
			first_x = dp[i].x;
		}
	}
	return first_x;
}

static int ncdgui_get_y(DIALOG * dp)
{
	int first_y = 32768;
	int i;
	
	for(i = 0; dp[i].proc != NULL; i++)
	{
		if(dp[i].y < first_y)
		{
			first_y = dp[i].y;
		}
	}
	return first_y;
}

static int ncdgui_get_width(DIALOG * dp)
{
	int first_x = 32768;
	int last_x = -1;
	int i;
	
	for(i = 0; dp[i].proc != NULL; i++)
	{
		if(dp[i].x < first_x)
		{
			first_x = dp[i].x;
		}
		if(dp[i].x + dp[i].w - 1 > last_x)
		{
			last_x = dp[i].x + dp[i].w;
		}
	}
	return last_x - first_x;
}

static int ncdgui_get_height(DIALOG * dp)
{
	int first_y = 32768;
	int last_y = -1;
	int i;
	
	for(i = 0; dp[i].proc != NULL; i++)
	{
		if(dp[i].y < first_y)
		{
			first_y = dp[i].y;
		}
		if(dp[i].y + dp[i].h - 1 > last_y)
		{
			last_y = dp[i].y + dp[i].h;
		}
	}
	return last_y - first_y;
}

BITMAP * ncdgui_generate_image(DIALOG * dp)
{
	BITMAP * bp;
	BITMAP * oldscreen = screen;
	BITMAP * fakescreen;
	DIALOG_PLAYER * player;
	
	/* create a fake screen and draw the GUI */
	fakescreen = create_bitmap(SCREEN_W, SCREEN_H);
	if(!fakescreen)
	{
		return NULL;
	}
	bp = create_bitmap(ncdgui_get_width(dp), ncdgui_get_height(dp));
	if(!bp)
	{
		destroy_bitmap(fakescreen);
		return NULL;
	}
	clear_to_color(bp, 0);
	screen = fakescreen;
	player = init_dialog(dp, 0);
	dialog_message(dp, MSG_DRAW, 0, NULL);
	update_dialog(player);
	shutdown_dialog(player);
	screen = oldscreen;
	blit(fakescreen, bp, ncdgui_get_y(dp), ncdgui_get_x(dp), 0, 0, ncdgui_get_width(dp), ncdgui_get_height(dp));
	destroy_bitmap(fakescreen);
	return bp;
}

void ncdgui_initialize(int cursor)
{
    gui_shadow_box_proc = d_agup_shadow_box_proc;
    gui_button_proc = d_agup_button_proc;
    gui_ctext_proc = d_agup_ctext_proc;
    gui_text_list_proc = d_agup_text_list_proc;
	gui_edit_proc = d_hackish_edit_proc;
	
    gui_fg_color = agup_fg_color;
    gui_bg_color = agup_bg_color;
    gui_mg_color = agup_mg_color;
	agup_init(awin95_theme);
}
