#include "main.h"
#include "gui.h"
#include "guiproc.h"
#include "undo.h"
#include "../game/avatar.h"
#include "../modules/fx.h"
#include "../modules/ncdgui.h"

int ae_menu_file_new(void)
{
	int ret;
	int i;
	
	if(ae_avatar)
	{
		if(ae_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				ae_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
		vgolf_destroy_avatar(ae_avatar);
	}
	ae_avatar = vgolf_create_avatar(create_bitmap(64, 58), create_bitmap(64, 58), create_bitmap(64, 58), create_bitmap(64, 58));
	if(ae_avatar)
	{
		for(i = 0; i < 4; i++)
		{
			clear_bitmap(ae_avatar->image[i]);
		}
	}
	ae_reset_state();
	ae_unfloat_selection();
	ustrcpy(ae_filename, "");
	ae_fix_window_title();
	ae_prepare_menus();
	return 1;
}

int ae_menu_file_load(void)
{
	int ret;
	char * returnedfn = NULL;
	
	if(ae_avatar)
	{
		if(ae_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				ae_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
	}
	returnedfn = ncd_file_select(0, ae_filename, "Select Avatar File", ae_filter_avatar_files);
	if(returnedfn)
	{
		if(ae_avatar)
		{
			vgolf_destroy_avatar(ae_avatar);
		}
		ae_avatar = vgolf_load_avatar(returnedfn, ae_palette);
		ustrcpy(ae_filename, returnedfn);
	}
	ae_prepare_menus();
	ae_reset_state();
	ae_unfloat_selection();
	ae_fix_window_title();
	return 1;
}

int ae_menu_file_save(void)
{
	if(ae_avatar)
	{
		if(!ustricmp(ae_filename, ""))
		{
			ae_menu_file_save_as();
		}
		else
		{
			replace_extension(ae_filename, ae_filename, "pcx", 1024);
			vgolf_save_avatar(ae_avatar, ae_palette, ae_filename);
			ae_changes = 0;
		}
	}
	return 1;
}

int ae_menu_file_save_as(void)
{
	char * returnedfn = NULL;
	
	if(ae_avatar)
	{
		returnedfn = ncd_file_select(1, ae_filename, "Save Avatar As", ae_filter_avatar_files);
		if(returnedfn)
		{
			ustrcpy(ae_filename, returnedfn);
			replace_extension(ae_filename, ae_filename, "pcx", 1024);
			ae_menu_file_save();
		}
	}
	return 1;
}

int ae_menu_file_load_image(void)
{
	BITMAP * temp_image;
	PALETTE temp_palette;
	char * returnedfn;
	
	if(ae_avatar)
	{
		returnedfn = ncd_file_select(0, ae_image_filename, "Select Image File", ae_filter_image_files);
		if(returnedfn)
		{
			temp_image = load_bitmap(returnedfn, temp_palette);
			if(temp_image)
			{
				ae_prepare_undo(0);
				convert_palette(temp_image, temp_palette, ae_palette);
				clear_bitmap(ae_avatar->image[ae_selected_image]);
				stretch_sprite(ae_avatar->image[ae_selected_image], temp_image, 0, 0, ae_avatar->image[ae_selected_image]->w, ae_avatar->image[ae_selected_image]->h);
				destroy_bitmap(temp_image);
			}
			ustrcpy(ae_image_filename, returnedfn);
		}
		ae_unfloat_selection();
	}
	return 1;
}

int ae_menu_file_exit(void)
{
	int ret;
	
	if(alert(NULL, "Want to quit?", NULL, "Yes", "No", KEY_ENTER, KEY_ESC) == 1)
	{
		if(ae_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				ae_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
		ae_quit = 1;
	}
	return 1;
}

int ae_menu_edit_undo(void)
{
	if(ae_avatar)
	{
		ae_undo_apply();
		ae_redo_toggle = 1;
		ae_unfloat_selection();
	}
	return 1;
}

int ae_menu_edit_redo(void)
{
	if(ae_avatar)
	{
		ae_redo_apply();
		ae_redo_toggle = 0;
		ae_unfloat_selection();
	}
	return 1;
}

int ae_menu_edit_copy(void)
{
	if(ae_avatar)
	{
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting >= 2)
		{
			if(!ae_clipboard)
			{
				ae_clipboard = create_bitmap(64, 58);
				if(!ae_clipboard)
				{
					return 1;
				}
				clear_bitmap(ae_clipboard);
			}
			blit(ae_float_bg, ae_clipboard, 0, 0, -ae_float_left, -ae_float_top, ae_float_right + 1, ae_float_bottom + 1);
//			blit(ae_float, ae_clipboard, 0, 0, 0, 0, 64, 58);
			ae_clipboard_left = ae_select_left;
			ae_clipboard_right = ae_select_right;
			ae_clipboard_top = ae_select_top;
			ae_clipboard_bottom = ae_select_bottom;
//			ae_float_x = (ae_clipboard_right - ae_clipboard_left) * 5 + 8;
//			ae_float_y = (ae_clipboard_bottom - ae_clipboard_top) * 5 + 57;
		}
	}
	return 1;
}

int ae_menu_edit_paste(void)
{
	int l = ae_clipboard_left, r = ae_clipboard_right, t = ae_clipboard_top, b = ae_clipboard_bottom;
	
	if(ae_avatar)
	{
		if(ae_clipboard)
		{
			ae_prepare_undo(0);
			blit(ae_clipboard, ae_float, 0, 0, 0, 0, 64, 58);
			ae_select_left = 0;
			ae_select_right = r - l;
			ae_select_top = 0;
			ae_select_bottom = b - t;
			ae_selecting = 5;
			clear_bitmap(ae_float_bg);
			blit(ae_avatar->image[ae_selected_image], ae_float_bg, 0, 0, 0, 0, 64, 58);
			draw_sprite(ae_avatar->image[ae_selected_image], ae_float, ae_select_left, ae_select_top);
//			draw_sprite(ae_avatar->image[ae_selected_image], ae_clipboard, ae_select_left, ae_select_top);
		}
	}
	return 1;
}

int ae_menu_image_mirror(void)
{
	if(ae_avatar)
	{
		ae_prepare_undo(0);
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting)
		{
			ae_unfloat_selection();
		}
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
		clear_bitmap(ae_avatar->image[ae_selected_image]);
		draw_sprite_h_flip(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0);
	}
	return 1;
}

int ae_menu_image_flip(void)
{
	if(ae_avatar)
	{
		ae_prepare_undo(0);
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting)
		{
			ae_unfloat_selection();
		}
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
		clear_bitmap(ae_avatar->image[ae_selected_image]);
		draw_sprite_v_flip(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0);
	}
	return 1;
}

int ae_menu_image_turn_clockwise(void)
{
	if(ae_avatar)
	{
		ae_prepare_undo(0);
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting)
		{
			ae_unfloat_selection();
		}
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
		clear_bitmap(ae_avatar->image[ae_selected_image]);
		rotate_sprite(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, itofix(64));
	}
	return 1;
}

int ae_menu_image_turn_counter_clockwise(void)
{
	if(ae_avatar)
	{
		ae_prepare_undo(0);
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting)
		{
			ae_unfloat_selection();
		}
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
		clear_bitmap(ae_avatar->image[ae_selected_image]);
		rotate_sprite(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, itofix(-64));
	}
	return 1;
}

int ae_menu_image_shift_left(void)
{
	if(ae_avatar)
	{
		ae_prepare_undo(0);
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting)
		{
			ae_unfloat_selection();
		}
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
		clear_bitmap(ae_avatar->image[ae_selected_image]);
		draw_sprite(ae_avatar->image[ae_selected_image], ae_scratch, -1, 0);
		draw_sprite(ae_avatar->image[ae_selected_image], ae_scratch, ae_avatar->image[ae_selected_image]->w - 1, 0);
	}
	return 1;
}

int ae_menu_image_shift_right(void)
{
	if(ae_avatar)
	{
		ae_prepare_undo(0);
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting)
		{
			ae_unfloat_selection();
		}
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
		clear_bitmap(ae_avatar->image[ae_selected_image]);
		draw_sprite(ae_avatar->image[ae_selected_image], ae_scratch, 1, 0);
		draw_sprite(ae_avatar->image[ae_selected_image], ae_scratch, -ae_avatar->image[ae_selected_image]->w + 1, 0);
	}
	return 1;
}

int ae_menu_image_shift_up(void)
{
	if(ae_avatar)
	{
		ae_prepare_undo(0);
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting)
		{
			ae_unfloat_selection();
		}
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
		clear_bitmap(ae_avatar->image[ae_selected_image]);
		draw_sprite(ae_avatar->image[ae_selected_image], ae_scratch, 0, -1);
		draw_sprite(ae_avatar->image[ae_selected_image], ae_scratch, 0, ae_avatar->image[ae_selected_image]->h - 1);
	}
	return 1;
}

int ae_menu_image_shift_down(void)
{
	if(ae_avatar)
	{
		ae_prepare_undo(0);
		if(ae_selected_tool == AE_TOOL_SELECT && ae_selecting)
		{
			ae_unfloat_selection();
		}
		blit(ae_avatar->image[ae_selected_image], ae_scratch, 0, 0, 0, 0, 64, 58);
		clear_bitmap(ae_avatar->image[ae_selected_image]);
		draw_sprite(ae_avatar->image[ae_selected_image], ae_scratch, 0, 1);
		draw_sprite(ae_avatar->image[ae_selected_image], ae_scratch, 0, -ae_avatar->image[ae_selected_image]->h + 1);
	}
	return 1;
}

int ae_menu_help_manual(void)
{
	PACKFILE * fp;
	
	fp = pack_fopen("avatar_editor.txt", "r");
	pack_fread(ae_help_text, file_size_ex("avatar_editor.txt"), fp);
	pack_fclose(fp);
	
	ae_render();
	ncdgui_color_dialog(ae_help_dialog, gui_fg_color, gui_bg_color);
	centre_dialog(ae_help_dialog);
	if(ncdgui_popup_dialog(ae_help_dialog, 0) == 1)
	{
	}
	return 1;
}

int ae_menu_help_about(void)
{
	alert("vGolf Avatar Editor v1.2", NULL, "Copyright 2009 T^3 Software", "OK", NULL, KEY_ENTER, 0);
	return 1;
}
