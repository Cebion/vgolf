#include <allegro.h>
#include "../modules/wfsel.h"
#include "../modules/ncds.h"
#include "../modules/g-idle.h"
#include "../modules/ncdgui.h"
#include "../modules/gametime.h"
#include "../modules/fx.h"
#include "../game/commentary.h"
#include "main.h"
#include "gui.h"
#include "guiproc.h"

int ce_quit = 0;
char ce_filename[1024] = {0};
char ce_wav_filename[1024] = {0};
int ce_change_count = 0;
int ce_changes = 0;
VGOLF_COMMENTARY * ce_commentary = NULL;

NCDFS_FILTER_LIST * ce_filter_commentary_files = NULL;
NCDFS_FILTER_LIST * ce_filter_sound_files = NULL;

void ce_fix_window_title(void)
{
	char window_text[1024] = {0};
	
	if(ustricmp(ce_filename, ""))
	{
		usprintf(window_text, "%svGolf Commentary Editor - %s", ce_changes ? "*" : "", ce_filename);
	}
	else
	{
		usprintf(window_text, "%svGolf Commentary Editor - Untitled", ce_changes ? "*" : "");
	}
	set_window_title(window_text);
}

int ce_initialize(int argc, char * argv[])
{
	allegro_init();
	set_window_title("vGolf Commentary Editor");
	if(install_keyboard())
	{
		allegro_message("Can't set up keyboard!");
		return 0;
	}
	if(install_mouse() < 0)
	{
		allegro_message("Can't set up mouse!");
		return 0;
	}
	ncds_install();

	ce_filter_commentary_files = ncdfs_filter_list_create();
	if(!ce_filter_commentary_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(ce_filter_commentary_files, "vc", "Commentary Files (*.vc)", 1);

	ce_filter_sound_files = ncdfs_filter_list_create();
	if(!ce_filter_sound_files)
	{
		allegro_message("Could not create file list filter!");
		return 0;
	}
	ncdfs_filter_list_add(ce_filter_sound_files, "wav", "Sound Files (*.wav)", 1);

	InitIdleSystem();
	
	set_color_depth(desktop_color_depth());
	
	if(set_gfx_mode(GFX_AUTODETECT_WINDOWED, 464, 200, 0, 0))
	{
		return 0;
	}
	set_display_switch_mode(SWITCH_PAUSE);
	PrepVSyncIdle();
	
	show_mouse(NULL);
	if(show_os_cursor(2) < 0)
	{
		ncdgui_initialize(NCDGUI_CURSOR_SOFTWARE);
	}
	else
	{
		ncdgui_initialize(NCDGUI_CURSOR_OS);
	}
	ce_menu_file_new();
	ce_prepare_menus();
	
	gametime_init(60); // 100hz timer
	return 1;
}

void ce_exit(void)
{
}

int main(int argc, char * argv[])
{
	if(!ce_initialize(argc, argv))
	{
		return -1;
	}
	while(!ce_quit)
	{
		clear_keybuf();
		ce_prepare_menus();
		ncdgui_popup_dialog(ce_main_dialog, 0);
		if(key[KEY_ESC])
		{
			ce_menu_file_exit();
		}
	}
	ce_exit();
	return 0;
}
END_OF_MAIN()
