#include <stdio.h>
#include "main.h"
#include "gui.h"
#include "guiproc.h"
#include "../game/avatar.h"
#include "../modules/agup/agup.h"
#include "../modules/ncds.h"
#include "../modules/fx.h"
#include "../modules/ncdgui.h"

static char ce_voice_text[128] = {0};

/* link to lists for simultaneous update */
int d_ce_list_proc(int msg, DIALOG * d, int c)
{
	if(d->dp3)
	{
		if(msg == MSG_DRAW)
		{
			object_message(d->dp3, MSG_DRAW, 0);
		}
		else if(msg == MSG_CLICK || msg == MSG_CHAR)
		{
			((DIALOG *)(d->dp3))->d1 = 0;
		}
	}
	return d_agup_list_proc(msg, d, c);
}

int ce_menu_file_new(void)
{
	int ret;
	int i;
	
	if(ce_commentary)
	{
		if(ce_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				ce_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
		destroy_vgolf_commentary(ce_commentary);
	}
	ce_commentary = malloc(sizeof(VGOLF_COMMENTARY));
	if(ce_commentary)
	{
		for(i = 0; i < MAX_MESSAGE_STRING_TYPES; i++)
		{
			ce_commentary->comments[i].count = 0;
		}
	}
	ce_changes = 0;
	ustrcpy(ce_filename, "");
	ce_fix_window_title();
	return 1;
}

int ce_menu_file_load(void)
{
	int ret;
	char * returnedfn = NULL;
	
	if(ce_commentary)
	{
		if(ce_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				ce_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
	}
	returnedfn = ncd_file_select(0, ce_filename, "Select Commentary File", ce_filter_commentary_files);
	if(returnedfn)
	{
		if(ce_commentary)
		{
			destroy_vgolf_commentary(ce_commentary);
		}
		ce_commentary = load_vgolf_commentary(returnedfn);
		ustrcpy(ce_filename, returnedfn);
		ce_changes = 0;
		ce_fix_window_title();
	}
	ce_prepare_menus();
	return 1;
}

int ce_menu_file_save(void)
{
	if(ce_commentary)
	{
		if(!ustricmp(ce_filename, ""))
		{
			ce_menu_file_save_as();
		}
		else
		{
			replace_extension(ce_filename, ce_filename, "vc", 1024);
			save_vgolf_commentary(ce_commentary, ce_filename);
		}
		ce_changes = 0;
		ce_fix_window_title();
	}
	return 1;
}

int ce_menu_file_save_as(void)
{
	char * returnedfn = NULL;
	
	if(ce_commentary)
	{
		returnedfn = ncd_file_select(1, ce_filename, "Save Commentary As", ce_filter_commentary_files);
		if(returnedfn)
		{
			ustrcpy(ce_filename, returnedfn);
			replace_extension(ce_filename, ce_filename, "vc", 1024);
			ce_menu_file_save();
		}
	}
	return 1;
}

int ce_menu_file_exit(void)
{
	int ret;
	
	if(alert(NULL, "Want to quit?", NULL, "Yes", "No", KEY_ENTER, KEY_ESC) == 1)
	{
		if(ce_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				ce_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
		ce_quit = 1;
	}
	return 1;
}

int ce_menu_help_manual(void)
{
	PACKFILE * fp;
	
	fp = pack_fopen("commentary_editor.txt", "r");
	pack_fread(ce_help_text, file_size_ex("commentary_editor.txt"), fp);
	pack_fclose(fp);
	
	ncdgui_color_dialog(ce_help_dialog, gui_fg_color, gui_bg_color);
	centre_dialog(ce_help_dialog);
	if(ncdgui_popup_dialog(ce_help_dialog, 0) == 1)
	{
	}
	return 1;
}

int ce_menu_help_about(void)
{
	alert("vGolf Commentary Editor v1.1", NULL, "Copyright 2009 T^3 Software", "OK", NULL, KEY_ENTER, 0);
	return 1;
}

char * ce_type_list_proc(int index, int * list_size)
{
	switch(index)
	{
		case 0:
		{
			return "Finish Good";
		}
		case 1:
		{
			return "Finish Better";
		}
		case 2:
		{
			return "Finish Great";
		}
		case 3:
		{
			return "Finish Excellent";
		}
		case 4:
		{
			return "Finish Poor";
		}
		case 5:
		{
			return "Finish Very Poor";
		}
		case 6:
		{
			return "Ball Zapped";
		}
		case 7:
		{
			return "Close Shot";
		}
		case 8:
		{
			return "Water Hazard";
		}
		case 9:
		{
			return "Ball Crushed";
		}
		default:
		{
			*list_size = MAX_MESSAGE_STRING_TYPES;
			return NULL;
		}
	}
	return NULL;
}

char * ce_voice_list_proc(int index, int *list_size)
{
	if(index < 0)
	{
		*list_size = ce_commentary->comments[ce_main_dialog[2].d1].count;
		return NULL;
	}
	else
	{
		sprintf(ce_voice_text, "Voice %d", index + 1);
		return ce_voice_text;
	}
	return NULL;
}

int ce_voice_listen(DIALOG *d)
{
	play_sample(ce_commentary->comments[ce_main_dialog[2].d1].comment[ce_main_dialog[3].d1].voice, 255, 128, 1000, 0);
	return D_O_K;
}

int ce_voice_add(DIALOG *d)
{
	SAMPLE * sp;
	char * returnedfn;
	
	returnedfn = ncd_file_select(0, ce_wav_filename, "Select Sound File", ce_filter_sound_files);
	if(returnedfn)
	{
		sp = load_wav(returnedfn);
		if(sp)
		{
			ce_commentary->comments[ce_main_dialog[2].d1].comment[ce_commentary->comments[ce_main_dialog[2].d1].count].voice = sp;
			ce_commentary->comments[ce_main_dialog[2].d1].count++;
			ce_changes++;
			ce_fix_window_title();
		}
		ustrcpy(ce_wav_filename, returnedfn);
	}
	return D_O_K;
}

int ce_voice_delete(DIALOG *d)
{
	int i;
	
	if(ce_commentary->comments[ce_main_dialog[2].d1].count > 0)
	{
		if(ce_commentary->comments[ce_main_dialog[2].d1].comment[ce_main_dialog[3].d1].voice)
		{
			destroy_sample(ce_commentary->comments[ce_main_dialog[2].d1].comment[ce_main_dialog[3].d1].voice);
		}
		for(i = ce_main_dialog[3].d1; i < ce_commentary->comments[ce_main_dialog[2].d1].count - 1; i++)
		{
			ce_commentary->comments[ce_main_dialog[2].d1].comment[i].voice = ce_commentary->comments[ce_main_dialog[2].d1].comment[i + 1].voice;
		}
		ce_commentary->comments[ce_main_dialog[2].d1].count--;
		ce_changes++;
		ce_fix_window_title();
	}
	return D_O_K;
}

int ce_voice_export(DIALOG *d)
{
	char * returnedfn;
	
	if(ce_commentary->comments[ce_main_dialog[2].d1].count > 0)
	{
		returnedfn = ncd_file_select(1, ce_wav_filename, "Export Sound", ce_filter_sound_files);
		if(returnedfn)
		{
			ustrcpy(ce_wav_filename, returnedfn);
			replace_extension(ce_wav_filename, ce_wav_filename, "wav", 1024);
			ncds_save_wav(ce_commentary->comments[ce_main_dialog[2].d1].comment[ce_main_dialog[3].d1].voice, ce_wav_filename);
		}
	}
	return D_O_K;
}

int ce_menu_file_tags(void)
{
	sprintf(ce_edit_text[0], "%s", ce_commentary->name);
	sprintf(ce_edit_text[1], "%s", ce_commentary->author);
	sprintf(ce_edit_text[2], "%s", ce_commentary->comment);
	ncdgui_color_dialog(ce_tags_dialog, gui_fg_color, gui_bg_color);
	centre_dialog(ce_tags_dialog);
	if(ncdgui_popup_dialog(ce_tags_dialog, 0) == 7)
	{
		if(stricmp(ce_commentary->name, ce_edit_text[0]) || stricmp(ce_commentary->author, ce_edit_text[1]) || stricmp(ce_commentary->comment, ce_edit_text[2]))
		{
			ustrcpy(ce_commentary->name, ce_edit_text[0]);
			ustrcpy(ce_commentary->author, ce_edit_text[1]);
			ustrcpy(ce_commentary->comment, ce_edit_text[2]);
			ce_changes++;
			ce_fix_window_title();
		}
	}
	return 1;
}
