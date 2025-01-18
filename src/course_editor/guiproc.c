#include <stdio.h>
#include "main.h"
#include "gui.h"
#include "guiproc.h"
#include "../game/course.h"
#include "../game/poly.h"
#include "../modules/fx.h"
#include "../modules/ncdgui.h"

static BITMAP * crsed_palette_proc_bitmap = NULL;

int d_crsed_palette_proc(int msg, DIALOG * d, int c)
{
	int i;
	float scale_x, scale_y;
	
	switch(msg)
	{
		/* generate palette image */
		case MSG_START:
		{
			crsed_palette_proc_bitmap = create_bitmap(16, 16);
			if(!crsed_palette_proc_bitmap)
			{
				return D_CLOSE;
			}
			for(i = 0; i < 256; i++)
			{
				putpixel(crsed_palette_proc_bitmap, i % 16, i / 16, i);
			}
			break;
		}
		case MSG_END:
		{
			if(crsed_palette_proc_bitmap)
			{
				destroy_bitmap(crsed_palette_proc_bitmap);
				crsed_palette_proc_bitmap = NULL;
			}
			break;
		}
		case MSG_DRAW:
		{
			stretch_blit(crsed_palette_proc_bitmap, screen, 0, 0, 16, 16, d->x, d->y, d->w, d->h);
			scale_x = (float)d->w / 16.0;
			scale_y = (float)d->h / 16.0;
			rect(screen, d->x + (float)(d->d1 % 16) * scale_x, d->y + (float)(d->d1 / 16) * scale_y, d->x + (float)(d->d1 % 16) * scale_x + scale_x - 1, d->y + (float)(d->d1 / 16) * scale_y + scale_y - 1, makecol(255, 255, 255));
			rect(screen, d->x + (float)(d->d1 % 16) * scale_x + 1, d->y + (float)(d->d1 / 16) * scale_y + 1, d->x + (float)(d->d1 % 16) * scale_x + scale_x - 2, d->y + (float)(d->d1 / 16) * scale_y + scale_y - 2, makecol(0, 0, 0));
			break;
		}
		case MSG_CLICK:
		{
			scale_x = (float)d->w / 16.0;
			scale_y = (float)d->h / 16.0;
			d->d1 = (int)((float)(gui_mouse_y() - d->y) / scale_y) * 16 + (int)((float)(gui_mouse_x() - d->x) / scale_x);
			return D_REDRAWME;
			break;
		}
		case MSG_WANTFOCUS:
		{
			return D_WANTFOCUS;
			break;
		}
	}
	return D_O_K;
}

int crsed_menu_file_new(void)
{
	int ret;
	
	if(crsed_course)
	{
		if(crsed_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				crsed_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
		vgolf_destroy_course(crsed_course);
	}
	crsed_course = vgolf_load_course("course_editor.dat");
	if(crsed_course)
	{
		set_palette(crsed_course->palette);
	}
	crsed_selected_hole = 0;
	crsed_selected_poly = 0;
	crsed_selected_vert = 0;
	crsed_selected_point = 0;
	crsed_selected_image = 0;
	crsed_mode = 0;
	crsed_changes = 0;
	strcpy(crsed_filename, "");
	crsed_fix_window_title();
	if(show_os_cursor(2) < 0)
	{
		ncdgui_initialize(NCDGUI_CURSOR_SOFTWARE);
	}
	else
	{
		ncdgui_initialize(NCDGUI_CURSOR_OS);
	}
	crsed_prepare_menus();
	return 1;
}

int crsed_menu_file_load(void)
{
	int ret;
	char * returnedfn = NULL;
	
	if(crsed_course)
	{
		if(crsed_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				crsed_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
	}
	returnedfn = ncd_file_select(0, crsed_filename, "Select Course File", crsed_filter_course_files);
	if(returnedfn)
	{
		if(crsed_course)
		{
			vgolf_destroy_course(crsed_course);
		}
		crsed_course = vgolf_load_course(returnedfn);
		ustrcpy(crsed_filename, returnedfn);
	}
	if(crsed_course)
	{
		set_palette(crsed_course->palette);
		if(show_os_cursor(2) < 0)
		{
			ncdgui_initialize(NCDGUI_CURSOR_SOFTWARE);
		}
		else
		{
			ncdgui_initialize(NCDGUI_CURSOR_OS);
		}
		crsed_selected_hole = 0;
		crsed_selected_image = 0;
		crsed_selected_poly = 0;
		crsed_selected_point = 0;
		crsed_selected_vert = 0;
		crsed_mode = 0;
		crsed_changes = 0;
		crsed_fix_window_title();
	}
	crsed_prepare_menus();
	return 1;
}

int crsed_menu_file_import(void)
{
	int ret;
	char * returnedfn = NULL;
	
	if(crsed_course)
	{
		if(crsed_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				crsed_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
	}
	returnedfn = ncd_file_select(0, crsed_filename, "Select Course File", crsed_filter_all_files);
	if(returnedfn)
	{
		if(crsed_course)
		{
			vgolf_destroy_course(crsed_course);
		}
		crsed_course = vgolf_load_course_old(returnedfn);
		ustrcpy(crsed_filename, returnedfn);
	}
	if(crsed_course)
	{
		set_palette(crsed_course->palette);
		if(show_os_cursor(2) < 0)
		{
			ncdgui_initialize(NCDGUI_CURSOR_SOFTWARE);
		}
		else
		{
			ncdgui_initialize(NCDGUI_CURSOR_OS);
		}
	}
	crsed_prepare_menus();
	return 1;
}

int crsed_menu_file_load_palette(void)
{
	int i, j;
	char * returnedfn = NULL;
	PALETTE temp_palette;
	BITMAP * temp_bitmap = NULL;
	
	if(!crsed_course)
	{
		return 1;
	}
	returnedfn = ncd_file_select(0, crsed_filename, "Select Image File", crsed_filter_image_files);
	if(returnedfn)
	{
		temp_bitmap = load_bitmap(returnedfn, temp_palette);
		if(temp_bitmap)
		{
			destroy_bitmap(temp_bitmap);
			for(i = 0; i < crsed_course->num_holes; i++)
			{
				if(crsed_course->hole[i].layer_pic[0])
				{
					convert_palette(crsed_course->hole[i].layer_pic[0], crsed_course->palette, temp_palette);
				}
				if(crsed_course->hole[i].layer_pic[1])
				{
					convert_palette(crsed_course->hole[i].layer_pic[1], crsed_course->palette, temp_palette);
				}
			}
			for(i = 0; i < MAX_COURSE_VERT_PIC_TYPES; i++)
			{
				for(j = 0; j < MAX_VERT_ANIMS; j++)
				{
					if(crsed_course->image_bank.vert_pic[i][j])
					{
						convert_palette(crsed_course->image_bank.vert_pic[i][j], crsed_course->palette, temp_palette);
					}
				}
			}
			for(i = 0; i < MAX_COURSE_ANIMAL_TYPES; i++)
			{
				for(j = 0; j < MAX_ANIMAL_ANIMS; j++)
				{
					if(crsed_course->image_bank.animal_pic[i][j])
					{
						convert_palette(crsed_course->image_bank.animal_pic[i][j], crsed_course->palette, temp_palette);
					}
				}
			}
			for(i = 0; i < MAX_COURSE_OBJECT_TYPES; i++)
			{
				for(j = 0; j < MAX_OBJECT_ANIMS; j++)
				{
					if(crsed_course->image_bank.object_pic[i][j])
					{
						convert_palette(crsed_course->image_bank.object_pic[i][j], crsed_course->palette, temp_palette);
					}
				}
			}
			if(crsed_course->image_bank.victory_bgpic)
			{
				convert_palette(crsed_course->image_bank.victory_bgpic, crsed_course->palette, temp_palette);
			}
			if(crsed_course->image_bank.victory_podiumpic)
			{
				convert_palette(crsed_course->image_bank.victory_podiumpic, crsed_course->palette, temp_palette);
			}
			if(crsed_course->image_bank.iconpic)
			{
				convert_palette(crsed_course->image_bank.iconpic, crsed_course->palette, temp_palette);
			}
			for(i = 0; i < 4; i++)
			{
				if(crsed_course->image_bank.victory_cloudpic[i])
				{
					convert_palette(crsed_course->image_bank.victory_cloudpic[i], crsed_course->palette, temp_palette);
				}
			}
			for(i = 0; i < 3; i++)
			{
				if(crsed_course->image_bank.victory_trophypic[i])
				{
					convert_palette(crsed_course->image_bank.victory_trophypic[i], crsed_course->palette, temp_palette);
				}
			}
			for(i = 0; i < MAX_COIN_PICS; i++)
			{
				if(crsed_course->image_bank.coinpic[i])
				{
					convert_palette(crsed_course->image_bank.coinpic[i], crsed_course->palette, temp_palette);
				}
			}
			convert_palette(crsed_course->scoreboard.bgpic, crsed_course->palette, temp_palette);
			for(i = 0; i < crsed_course->num_holes; i++)
			{
				for(j = 0; j < crsed_course->hole[i].num_verts; j++)
				{
					switch(crsed_course->hole[i].vert[j].flag)
					{
						case VERT_FLAG_POLY_MORPH:
						case VERT_FLAG_POLY_MOVE_HORIZ:
						case VERT_FLAG_POLY_MOVE_VERT:
						{
							crsed_course->hole[i].vert[j].ref_no[3] = closest_match(crsed_course->hole[i].vert[j].ref_no[3], crsed_course->palette, temp_palette);
							break;
						}
						case VERT_FLAG_BLINKER:
						case VERT_FLAG_POLY_DOOR_HORIZ:
						case VERT_FLAG_POLY_DOOR_VERT:
						case VERT_FLAG_CLOUD_HORIZ:
						case VERT_FLAG_CLOUD_VERT:
						{
							crsed_course->hole[i].vert[j].ref_no[5] = closest_match(crsed_course->hole[i].vert[j].ref_no[5], crsed_course->palette, temp_palette);
							break;
						}
						case VERT_FLAG_LASER_RECV:
						{
							crsed_course->hole[i].vert[j].ref_no[1] = closest_match(crsed_course->hole[i].vert[j].ref_no[1], crsed_course->palette, temp_palette);
							break;
						}
					}
				}
			}
			memcpy(crsed_course->palette, temp_palette, sizeof(PALETTE));
			set_palette(crsed_course->palette);
			crsed_mark_change();
			if(show_os_cursor(2) < 0)
			{
				ncdgui_initialize(NCDGUI_CURSOR_SOFTWARE);
			}
			else
			{
				ncdgui_initialize(NCDGUI_CURSOR_OS);
			}
			crsed_prepare_menus();
		}
	}
	return 1;
}

int crsed_menu_file_course_tags(void)
{
	sprintf(crsed_edit_text[0], "%s", crsed_course->name);
	sprintf(crsed_edit_text[1], "%s", crsed_course->author);
	sprintf(crsed_edit_text[2], "%s", crsed_course->comment);
	ncdgui_color_dialog(crsed_tags_dialog, gui_fg_color, gui_bg_color);
	centre_dialog(crsed_tags_dialog);
	if(ncdgui_popup_dialog(crsed_tags_dialog, 0) == 7)
	{
		if(stricmp(crsed_course->name, crsed_edit_text[0]) || stricmp(crsed_course->author, crsed_edit_text[1]) || stricmp(crsed_course->comment, crsed_edit_text[2]))
		{
			ustrcpy(crsed_course->name, crsed_edit_text[0]);
			ustrcpy(crsed_course->author, crsed_edit_text[1]);
			ustrcpy(crsed_course->comment, crsed_edit_text[2]);
			crsed_mark_change();
		}
	}
	return 1;
}

int crsed_menu_file_save(void)
{
	if(crsed_course)
	{
		if(!ustricmp(crsed_filename, ""))
		{
			crsed_menu_file_save_as();
		}
		else
		{
			replace_extension(crsed_filename, crsed_filename, "crs", 1024);
			vgolf_save_course(crsed_course, crsed_filename);
			crsed_changes = 0;
			crsed_fix_window_title();
		}
	}
	return 1;
}

int crsed_menu_file_save_as(void)
{
	char * returnedfn = NULL;
	
	if(crsed_course)
	{
		returnedfn = ncd_file_select(1, crsed_filename, "Save Course As", crsed_filter_course_files);
		if(returnedfn)
		{
			ustrcpy(crsed_filename, returnedfn);
			replace_extension(crsed_filename, crsed_filename, "crs", 1024);
			crsed_menu_file_save();
		}
	}
	return 1;
}

int crsed_menu_file_exit(void)
{
	int ret;
	
	if(alert(NULL, "Want to quit?", NULL, "Yes", "No", KEY_ENTER, KEY_ESC) == 1)
	{
		if(crsed_changes)
		{
			ret = alert3(NULL, "You have unsaved changes.", NULL, "Save", "Discard", "Cancel", KEY_ENTER, 0, KEY_ESC);
			if(ret == 1)
			{
				crsed_menu_file_save();
			}
			else if(ret == 3)
			{
				return 1;
			}
		}
		crsed_quit = 1;
	}
	return 1;
}

int crsed_menu_mode_polygons(void)
{
	crsed_mode = 0;
	crsed_prepare_menus();
	return 1;
}

int crsed_menu_mode_objects(void)
{
	crsed_mode = 1;
	crsed_prepare_menus();
	return 1;
}

int crsed_menu_mode_images(void)
{
	crsed_mode = 2;
	crsed_prepare_menus();
	return 1;
}

static void crsed_menu_hole_select_helper(int hole)
{
	crsed_selected_hole = hole;
	crsed_selected_poly = 0;
	crsed_selected_point = 0;
	crsed_selected_vert = 0;
}

int crsed_menu_hole_select_0(void)
{
	crsed_menu_hole_select_helper(0);
	return 1;
}

int crsed_menu_hole_select_1(void)
{
	crsed_menu_hole_select_helper(1);
	return 1;
}

int crsed_menu_hole_select_2(void)
{
	crsed_menu_hole_select_helper(2);
	return 1;
}

int crsed_menu_hole_select_3(void)
{
	crsed_menu_hole_select_helper(3);
	return 1;
}

int crsed_menu_hole_select_4(void)
{
	crsed_menu_hole_select_helper(4);
	return 1;
}

int crsed_menu_hole_select_5(void)
{
	crsed_menu_hole_select_helper(5);
	return 1;
}

int crsed_menu_hole_select_6(void)
{
	crsed_menu_hole_select_helper(6);
	return 1;
}

int crsed_menu_hole_select_7(void)
{
	crsed_menu_hole_select_helper(7);
	return 1;
}

int crsed_menu_hole_select_8(void)
{
	crsed_menu_hole_select_helper(8);
	return 1;
}

int crsed_menu_hole_select_9(void)
{
	crsed_menu_hole_select_helper(9);
	return 1;
}

int crsed_menu_hole_select_10(void)
{
	crsed_menu_hole_select_helper(10);
	return 1;
}

int crsed_menu_hole_select_11(void)
{
	crsed_menu_hole_select_helper(11);
	return 1;
}

int crsed_menu_hole_select_12(void)
{
	crsed_menu_hole_select_helper(12);
	return 1;
}

int crsed_menu_hole_select_13(void)
{
	crsed_menu_hole_select_helper(13);
	return 1;
}

int crsed_menu_hole_select_14(void)
{
	crsed_menu_hole_select_helper(14);
	return 1;
}

int crsed_menu_hole_select_15(void)
{
	crsed_menu_hole_select_helper(15);
	return 1;
}

int crsed_menu_hole_select_16(void)
{
	crsed_menu_hole_select_helper(16);
	return 1;
}

int crsed_menu_hole_select_17(void)
{
	crsed_menu_hole_select_helper(17);
	return 1;
}

int crsed_menu_hole_add(void)
{
	if(crsed_course->num_holes < 18)
	{
		memset(&crsed_course->hole[crsed_course->num_holes], 0, sizeof(HOLE));
		crsed_course->hole[crsed_course->num_holes].s.x = 320;
		crsed_course->hole[crsed_course->num_holes].s.y = 240;
		crsed_course->num_holes++;
		crsed_mark_change();
	}
	return 1;
}

int crsed_menu_hole_background_load(void)
{
	BITMAP * temp_image;
	PALETTE temp_palette;
	char * returnedfn;
	
	if(crsed_course)
	{
		returnedfn = ncd_file_select(0, crsed_filename, "Select Image File", crsed_filter_image_files);
		if(returnedfn)
		{
			temp_image = load_bitmap(returnedfn, temp_palette);
			if(temp_image)
			{
				convert_palette(temp_image, temp_palette, crsed_course->palette);
				if(!crsed_course->hole[crsed_selected_hole].layer_pic[0])
				{
					crsed_course->hole[crsed_selected_hole].layer_pic[0] = create_bitmap(640, 480);
				}
				if(crsed_course->hole[crsed_selected_hole].layer_pic[0])
				{
					stretch_blit(temp_image, crsed_course->hole[crsed_selected_hole].layer_pic[0], 0, 0, temp_image->w, temp_image->h, 0, 0, 640, 480);
				}
				stretch_blit(temp_image, crsed_course->hole[crsed_selected_hole].layer_pic[0], 0, 0, temp_image->w, temp_image->h, 0, 0, 640, 480);
				destroy_bitmap(temp_image);
				crsed_mark_change();
			}
		}
	}
	return 1;
}

int crsed_menu_hole_foreground_load(void)
{
	BITMAP * temp_image;
	PALETTE temp_palette;
	char * returnedfn;
	
	if(crsed_course)
	{
		returnedfn = ncd_file_select(0, crsed_filename, "Select Image File", crsed_filter_image_files);
		if(returnedfn)
		{
			temp_image = load_bitmap(returnedfn, temp_palette);
			if(temp_image)
			{
				convert_palette(temp_image, temp_palette, crsed_course->palette);
				if(!crsed_course->hole[crsed_selected_hole].layer_pic[1])
				{
					crsed_course->hole[crsed_selected_hole].layer_pic[1] = create_bitmap(640, 480);
				}
				if(crsed_course->hole[crsed_selected_hole].layer_pic[1])
				{
					stretch_blit(temp_image, crsed_course->hole[crsed_selected_hole].layer_pic[1], 0, 0, temp_image->w, temp_image->h, 0, 0, 640, 480);
				}
				destroy_bitmap(temp_image);
				crsed_mark_change();
			}
		}
	}
	return 1;
}

int crsed_menu_hole_background_save(void)
{
	char * ext[2] = {"bmp", "tga"};
	int i;
	char * returnedfn;
	
	if(crsed_course)
	{
		returnedfn = ncd_file_select(1, crsed_filename, "Save Image File", crsed_filter_save_image_files);
		if(returnedfn)
		{
			for(i = 0; i < 2; i++)
			{
				if(!ustricmp(ext[i], get_extension(returnedfn)))
				{
					switch(i)
					{
						case 0:
						{
							save_bmp(returnedfn, crsed_course->hole[crsed_selected_hole].layer_pic[0], crsed_course->palette);
							break;
						}
						case 1:
						{
							save_tga(returnedfn, crsed_course->hole[crsed_selected_hole].layer_pic[0], crsed_course->palette);
							break;
						}
					}
					break;
				}
			}
			if(i >= 2)
			{
				replace_extension(returnedfn, returnedfn, "pcx", 1024);
				save_pcx(returnedfn, crsed_course->hole[crsed_selected_hole].layer_pic[0], crsed_course->palette);
			}
		}
	}
	return 1;
}

int crsed_menu_hole_foreground_save(void)
{
	char * ext[2] = {"bmp", "tga"};
	int i;
	char * returnedfn;
	
	if(crsed_course)
	{
		returnedfn = ncd_file_select(1, crsed_filename, "Save Image File", crsed_filter_save_image_files);
		if(returnedfn)
		{
			for(i = 0; i < 2; i++)
			{
				if(!ustricmp(ext[i], get_extension(returnedfn)))
				{
					switch(i)
					{
						case 0:
						{
							save_bmp(returnedfn, crsed_course->hole[crsed_selected_hole].layer_pic[1], crsed_course->palette);
							break;
						}
						case 1:
						{
							save_tga(returnedfn, crsed_course->hole[crsed_selected_hole].layer_pic[1], crsed_course->palette);
							break;
						}
					}
					break;
				}
			}
			if(i >= 2)
			{
				replace_extension(returnedfn, returnedfn, "pcx", 1024);
				save_pcx(returnedfn, crsed_course->hole[crsed_selected_hole].layer_pic[1], crsed_course->palette);
			}
		}
	}
	return 1;
}

int crsed_menu_hole_background_remove(void)
{
	if(crsed_course->hole[crsed_selected_hole].layer_pic[0])
	{
		destroy_bitmap(crsed_course->hole[crsed_selected_hole].layer_pic[0]);
		crsed_course->hole[crsed_selected_hole].layer_pic[0] = NULL;
		crsed_mark_change();
	}
	return 1;
}

int crsed_menu_hole_foreground_remove(void)
{
	if(crsed_course->hole[crsed_selected_hole].layer_pic[1])
	{
		destroy_bitmap(crsed_course->hole[crsed_selected_hole].layer_pic[1]);
		crsed_course->hole[crsed_selected_hole].layer_pic[1] = NULL;
		crsed_mark_change();
	}
	return 1;
}


int crsed_menu_hole_set_par(void)
{
	int i;
	
	sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].par_score);
	ncdgui_color_dialog(crsed_par_dialog, gui_fg_color, gui_bg_color);
	centre_dialog(crsed_par_dialog);
	if(ncdgui_popup_dialog(crsed_par_dialog, 0) == 3)
	{
		if(crsed_course->hole[crsed_selected_hole].par_score != atoi(crsed_edit_text[0]))
		{
			crsed_course->hole[crsed_selected_hole].par_score = atoi(crsed_edit_text[0]);
			crsed_course->course_par = 0;
			for(i = 0; i < crsed_course->num_holes; i++)
			{
				crsed_course->course_par += crsed_course->hole[i].par_score;
			}
			crsed_mark_change();
		}
	}
	return 1;
}

int crsed_menu_hole_import(void)
{
	char * returnedfn;
	PALETTE temp_palette;
	PACKFILE * fp;
	int i;
	
	if(crsed_course)
	{
		returnedfn = ncd_file_select(0, crsed_filename, "Import Hole File", crsed_filter_hole_files);
		if(returnedfn)
		{
			fp = pack_fopen(returnedfn, "rp");
			if(fp)
			{
				load_hole_fp(&crsed_course->hole[crsed_selected_hole], fp);
				for(i = 0; i < 256; i++)
				{
					temp_palette[i].r = pack_getc(fp);
					temp_palette[i].g = pack_getc(fp);
					temp_palette[i].b = pack_getc(fp);
				}
				pack_fclose(fp);
				if(crsed_course->hole[crsed_selected_hole].layer_pic[0])
				{
					convert_palette(crsed_course->hole[crsed_selected_hole].layer_pic[0], temp_palette, crsed_course->palette);
				}
				if(crsed_course->hole[crsed_selected_hole].layer_pic[1])
				{
					convert_palette(crsed_course->hole[crsed_selected_hole].layer_pic[1], temp_palette, crsed_course->palette);
				}
				crsed_mark_change();
			}
		}
	}
	return 1;
}

int crsed_menu_hole_export(void)
{
	char * returnedfn;
	PACKFILE * fp;
	int i;
	
	if(crsed_course)
	{
		returnedfn = ncd_file_select(1, crsed_filename, "Export Hole File", crsed_filter_hole_files);
		if(returnedfn)
		{
			replace_extension(returnedfn, returnedfn, "vgh", 1024);
			fp = pack_fopen(returnedfn, "wp");
			if(fp)
			{
				save_hole_fp(&crsed_course->hole[crsed_selected_hole], fp);
				for(i = 0; i < 256; i++)
				{
					pack_putc(crsed_course->palette[i].r, fp);
					pack_putc(crsed_course->palette[i].g, fp);
					pack_putc(crsed_course->palette[i].b, fp);
				}
				pack_fclose(fp);
			}
		}
	}
	return 1;
}

int crsed_menu_hole_delete(void)
{
	int i;
	
	free_hole(&crsed_course->hole[crsed_selected_hole]);
	for(i = crsed_selected_hole; i < crsed_course->num_holes - 1; i++)
	{
		memcpy(&crsed_course->hole[i], &crsed_course->hole[i + 1], sizeof(HOLE));
	}
	crsed_course->num_holes--;
	if(crsed_selected_hole >= crsed_course->num_holes)
	{
		if(crsed_course->num_holes > 0)
		{
			crsed_selected_hole = crsed_course->num_holes - 1;
		}
		else
		{
			crsed_selected_hole = 0;
		}
	}
	crsed_mark_change();
	return 1;
}

int crsed_menu_poly_invert(void)
{
	POLY temp_poly;
	int i;
	
	memcpy(&temp_poly, &crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly, sizeof(POLY));
	for(i = 0; i < temp_poly.num_vertices; i++)
	{
		crsed_course->hole[crsed_selected_hole].body[crsed_selected_poly].poly.p[i] = temp_poly.p[temp_poly.num_vertices - 1 - i];
	}
	crsed_mark_change();
	return 1;
}

int crsed_menu_poly_delete(void)
{
	crsed_delete_poly(crsed_selected_poly);
	crsed_mark_change();
	return 1;
}

int crsed_menu_object_type_TEE(void)
{
	crsed_selected_vert_type = VERT_FLAG_TEE;
	return 1;
}

int crsed_menu_object_type_CUP(void)
{
	crsed_selected_vert_type = VERT_FLAG_HOLE;
	return 1;
}

int crsed_menu_object_type_TREE_0(void)
{
	crsed_selected_vert_type = VERT_FLAG_TREE_0;
	return 1;
}

int crsed_menu_object_type_TREE_1(void)
{
	crsed_selected_vert_type = VERT_FLAG_TREE_1;
	return 1;
}

int crsed_menu_object_type_TREE_2(void)
{
	crsed_selected_vert_type = VERT_FLAG_TREE_2;
	return 1;
}

int crsed_menu_object_type_TREE_3(void)
{
	crsed_selected_vert_type = VERT_FLAG_TREE_3;
	return 1;
}

int crsed_menu_object_type_ANIMAL_0(void)
{
	crsed_selected_vert_type = VERT_FLAG_ANIMAL_0;
	return 1;
}

int crsed_menu_object_type_ANIMAL_1(void)
{
	crsed_selected_vert_type = VERT_FLAG_ANIMAL_1;
	return 1;
}

int crsed_menu_object_type_ANIMAL_2(void)
{
	crsed_selected_vert_type = VERT_FLAG_ANIMAL_2;
	return 1;
}

int crsed_menu_object_type_HAZARD_WATER(void)
{
	crsed_selected_vert_type = VERT_FLAG_HAZARD_WATER;
	return 1;
}

int crsed_menu_object_type_HAZARD_SAND(void)
{
	crsed_selected_vert_type = VERT_FLAG_HAZARD_SAND;
	return 1;
}

int crsed_menu_object_type_HAZARD_ROUGH(void)
{
	crsed_selected_vert_type = VERT_FLAG_HAZARD_ROUGH;
	return 1;
}

int crsed_menu_object_type_HAZARD_SLIME(void)
{
	crsed_selected_vert_type = VERT_FLAG_HAZARD_SLIME;
	return 1;
}

int crsed_menu_object_type_PIPE_ENTER(void)
{
	crsed_selected_vert_type = VERT_FLAG_PIPE_ENTER;
	return 1;
}

int crsed_menu_object_type_PIPE_EXIT(void)
{
	crsed_selected_vert_type = VERT_FLAG_PIPE_EXIT;
	return 1;
}

int crsed_menu_object_type_HILL(void)
{
	crsed_selected_vert_type = VERT_FLAG_INDENT_HILL;
	return 1;
}

int crsed_menu_object_type_RUT(void)
{
	crsed_selected_vert_type = VERT_FLAG_INDENT_RUT;
	return 1;
}

int crsed_menu_object_type_POLY_MORPH(void)
{
	crsed_selected_vert_type = VERT_FLAG_POLY_MORPH;
	return 1;
}

int crsed_menu_object_type_POLY_MOTION_AREA(void)
{
	crsed_selected_vert_type = VERT_FLAG_POLY_MOTION_AREA;
	return 1;
}

int crsed_menu_object_type_POLY_MOVE_HORIZ(void)
{
	crsed_selected_vert_type = VERT_FLAG_POLY_MOVE_HORIZ;
	return 1;
}

int crsed_menu_object_type_POLY_MOVE_VERT(void)
{
	crsed_selected_vert_type = VERT_FLAG_POLY_MOVE_VERT;
	return 1;
}

int crsed_menu_object_type_CLOUD_HORIZ(void)
{
	crsed_selected_vert_type = VERT_FLAG_CLOUD_HORIZ;
	return 1;
}

int crsed_menu_object_type_CLOUD_VERT(void)
{
	crsed_selected_vert_type = VERT_FLAG_CLOUD_VERT;
	return 1;
}

int crsed_menu_object_type_LASER_SEND(void)
{
	crsed_selected_vert_type = VERT_FLAG_LASER_SEND;
	return 1;
}

int crsed_menu_object_type_LASER_RECEIVE(void)
{
	crsed_selected_vert_type = VERT_FLAG_LASER_RECV;
	return 1;
}

int crsed_menu_object_type_LASER_RESPAWN(void)
{
	crsed_selected_vert_type = VERT_FLAG_LASER_RESPAWN;
	return 1;
}

int crsed_menu_object_type_TELEPORT(void)
{
	crsed_selected_vert_type = VERT_FLAG_TELEPORT;
	return 1;
}

int crsed_menu_object_type_CONVEYOR(void)
{
	crsed_selected_vert_type = VERT_FLAG_CONVEYOR;
	return 1;
}

int crsed_menu_object_type_BLINKER(void)
{
	crsed_selected_vert_type = VERT_FLAG_BLINKER;
	return 1;
}

int crsed_menu_object_type_DOOR_HORIZ(void)
{
	crsed_selected_vert_type = VERT_FLAG_POLY_DOOR_HORIZ;
	return 1;
}

int crsed_menu_object_type_DOOR_VERTICAL(void)
{
	crsed_selected_vert_type = VERT_FLAG_POLY_DOOR_VERT;
	return 1;
}

int crsed_menu_object_type_DOOR_RESPAWN(void)
{
	crsed_selected_vert_type = VERT_FLAG_SMASH_RESPAWN;
	return 1;
}

int crsed_menu_object_type_BUBBLE_HORIZ(void)
{
	crsed_selected_vert_type = VERT_FLAG_BUBBLE_HORIZ;
	return 1;
}

int crsed_menu_object_type_BUBBLE_VERT(void)
{
	crsed_selected_vert_type = VERT_FLAG_BUBBLE_VERT;
	return 1;
}

int crsed_menu_object_properties_color(void)
{
	switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
	{
		case VERT_FLAG_POLY_MORPH:
		case VERT_FLAG_POLY_MOVE_HORIZ:
		case VERT_FLAG_POLY_MOVE_VERT:
		{
			crsed_palette_dialog[1].d1 = crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3];
			ncdgui_color_dialog(crsed_palette_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_palette_dialog);
			if(ncdgui_popup_dialog(crsed_palette_dialog, 0) == 2)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3] = crsed_palette_dialog[1].d1;
				crsed_mark_change();
			}
			break;
		}
		case VERT_FLAG_LASER_SEND:
		{
			crsed_palette_dialog[1].d1 = crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5]].ref_no[1];
			ncdgui_color_dialog(crsed_palette_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_palette_dialog);
			if(ncdgui_popup_dialog(crsed_palette_dialog, 0) == 2)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5]].ref_no[1] = crsed_palette_dialog[1].d1;
				crsed_mark_change();
			}
			break;
		}
		case VERT_FLAG_BLINKER:
		case VERT_FLAG_POLY_DOOR_HORIZ:
		case VERT_FLAG_POLY_DOOR_VERT:
		case VERT_FLAG_CLOUD_HORIZ:
		case VERT_FLAG_CLOUD_VERT:
		{
			crsed_palette_dialog[1].d1 = crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5];
			ncdgui_color_dialog(crsed_palette_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_palette_dialog);
			if(ncdgui_popup_dialog(crsed_palette_dialog, 0) == 2)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5] = crsed_palette_dialog[1].d1;
				crsed_mark_change();
			}
			break;
		}
	}
	return 1;
}

int crsed_menu_object_properties_connect(void)
{
	crsed_vert_mode = 1;
	crsed_connect_count = 0;
	crsed_mark_change();
	return 1;
}

int crsed_menu_object_properties_set_angle(void)
{
	crsed_vert_mode = 2;
	crsed_mark_change();
	return 1;
}

int crsed_menu_object_properties_set_timer(void)
{
	switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
	{
		case VERT_FLAG_PIPE_ENTER:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3]);
			ncdgui_color_dialog(crsed_timer_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_timer_dialog);
			if(ncdgui_popup_dialog(crsed_timer_dialog, 0) == 3)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3] = atoi(crsed_edit_text[0]);
				crsed_mark_change();
			}
			break;
		}
		case VERT_FLAG_BLINKER:
		case VERT_FLAG_LASER_SEND:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2]);
			ncdgui_color_dialog(crsed_timer_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_timer_dialog);
			if(ncdgui_popup_dialog(crsed_timer_dialog, 0) == 3)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2] = atoi(crsed_edit_text[0]);
				crsed_mark_change();
			}
			break;
		}
		case VERT_FLAG_POLY_DOOR_HORIZ:
		case VERT_FLAG_POLY_DOOR_VERT:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2]);
			ncdgui_color_dialog(crsed_timer_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_timer_dialog);
			if(ncdgui_popup_dialog(crsed_timer_dialog, 0) == 3)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2] = atoi(crsed_edit_text[0]);
				crsed_mark_change();
			}
			break;
		}
	}
	return 1;
}

int crsed_menu_object_properties_set_timer2(void)
{
	switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
	{
		case VERT_FLAG_BLINKER:
		case VERT_FLAG_LASER_SEND:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3]);
			ncdgui_color_dialog(crsed_timer_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_timer_dialog);
			if(ncdgui_popup_dialog(crsed_timer_dialog, 0) == 3)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3] = atoi(crsed_edit_text[0]);
				crsed_course->hole[crsed_selected_hole].vert[crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5]].ref_no[1] = makecol(255, 255, 255);
				crsed_mark_change();
			}
			break;
		}
	}
	return 1;
}

int crsed_menu_object_properties_set_speed(void)
{
	switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].flag)
	{
		case VERT_FLAG_POLY_MORPH:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2]);
			ncdgui_color_dialog(crsed_speed_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_speed_dialog);
			if(ncdgui_popup_dialog(crsed_speed_dialog, 0) == 3)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2] = atoi(crsed_edit_text[0]);
				crsed_mark_change();
			}
			break;
		}
		case VERT_FLAG_POLY_MOVE_HORIZ:
		case VERT_FLAG_POLY_MOVE_VERT:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5]);
			ncdgui_color_dialog(crsed_speed_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_speed_dialog);
			if(ncdgui_popup_dialog(crsed_speed_dialog, 0) == 3)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[5] = atoi(crsed_edit_text[0]);
				crsed_mark_change();
			}
			break;
		}
		case VERT_FLAG_POLY_DOOR_HORIZ:
		case VERT_FLAG_POLY_DOOR_VERT:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3]);
			ncdgui_color_dialog(crsed_speed_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_speed_dialog);
			if(ncdgui_popup_dialog(crsed_speed_dialog, 0) == 3)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[3] = atoi(crsed_edit_text[0]);
				crsed_mark_change();
			}
			break;
		}
		case VERT_FLAG_CONVEYOR:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2]);
			ncdgui_color_dialog(crsed_speed_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_speed_dialog);
			if(ncdgui_popup_dialog(crsed_speed_dialog, 0) == 3)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2] = atoi(crsed_edit_text[0]);
				crsed_mark_change();
			}
			break;
		}
		case VERT_FLAG_ANIMAL_0:
		case VERT_FLAG_ANIMAL_1:
		{
			sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[1]);
			sprintf(crsed_edit_text[1], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2]);
			ncdgui_color_dialog(crsed_speed2_dialog, gui_fg_color, gui_bg_color);
			centre_dialog(crsed_speed2_dialog);
			if(ncdgui_popup_dialog(crsed_speed2_dialog, 0) == 5)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[1] = atoi(crsed_edit_text[0]);
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2] = atoi(crsed_edit_text[1]);
				crsed_mark_change();
			}
			break;
		}
	}
	return 1;
}

int crsed_menu_object_properties_set_size(void)
{
	sprintf(crsed_edit_text[0], "%d", crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[4] - crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2]);
	ncdgui_color_dialog(crsed_size_dialog, gui_fg_color, gui_bg_color);
	centre_dialog(crsed_size_dialog);
	if(ncdgui_popup_dialog(crsed_size_dialog, 0) == 3)
	{
		crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[2] = 0;
		crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[4] = atoi(crsed_edit_text[0]);
		crsed_mark_change();
	}
	return 1;
}

int crsed_menu_object_properties_set_direction(void)
{
	int i;
	
	crsed_direction_dialog[1].flags = 0;
	crsed_direction_dialog[2].flags = 0;
	crsed_direction_dialog[3].flags = 0;
	crsed_direction_dialog[4].flags = 0;
	switch(crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0])
	{
		case 0:
		{
			crsed_direction_dialog[1].flags = D_SELECTED;
			break;
		}
		case 1:
		{
			crsed_direction_dialog[2].flags = D_SELECTED;
			break;
		}
		case 2:
		{
			crsed_direction_dialog[3].flags = D_SELECTED;
			break;
		}
		default:
		{
			crsed_direction_dialog[4].flags = D_SELECTED;
			break;
		}
	}
	ncdgui_color_dialog(crsed_direction_dialog, gui_fg_color, gui_bg_color);
	centre_dialog(crsed_direction_dialog);
	if(ncdgui_popup_dialog(crsed_direction_dialog, 0) == 5)
	{
		for(i = 1; i < 5; i++)
		{
			if(crsed_direction_dialog[i].flags & D_SELECTED)
			{
				crsed_course->hole[crsed_selected_hole].vert[crsed_selected_vert].ref_no[0] = i - 1;
				break;
			}
		}
		crsed_mark_change();
	}
	return 1;
}

int crsed_menu_image_type_title(void)
{
	crsed_selected_image = 0;
	return 1;
}

int crsed_menu_image_type_scoreboard(void)
{
	crsed_selected_image = 1;
	return 1;
}

int crsed_menu_image_type_end_bg(void)
{
	crsed_selected_image = 2;
	return 1;
}

int crsed_menu_image_type_end_podium(void)
{
	crsed_selected_image = 3;
	return 1;
}

int crsed_menu_image_type_end_cloud_1(void)
{
	crsed_selected_image = 4;
	return 1;
}

int crsed_menu_image_type_end_cloud_2(void)
{
	crsed_selected_image = 5;
	return 1;
}

int crsed_menu_image_type_1st_trophy(void)
{
	crsed_selected_image = 6;
	return 1;
}

int crsed_menu_image_type_2nd_trophy(void)
{
	crsed_selected_image = 7;
	return 1;
}

int crsed_menu_image_type_3rd_trophy(void)
{
	crsed_selected_image = 8;
	return 1;
}

int crsed_menu_image_type_object_hole(void)
{
	crsed_selected_image = 9;
	return 1;
}

int crsed_menu_image_type_object_hole_sunk(void)
{
	crsed_selected_image = 10;
	return 1;
}

int crsed_menu_image_type_object_tree_1(void)
{
	crsed_selected_image = 11;
	return 1;
}

int crsed_menu_image_type_object_tree_2(void)
{
	crsed_selected_image = 12;
	return 1;
}

int crsed_menu_image_type_object_laser_1(void)
{
	crsed_selected_image = 13;
	return 1;
}

int crsed_menu_image_type_object_laser_2(void)
{
	crsed_selected_image = 14;
	return 1;
}

int crsed_menu_image_type_object_teleport_1(void)
{
	crsed_selected_image = 15;
	return 1;
}

int crsed_menu_image_type_object_teleport_2(void)
{
	crsed_selected_image = 16;
	return 1;
}

int crsed_menu_image_type_object_conveyor_1(void)
{
	crsed_selected_image = 17;
	return 1;
}

int crsed_menu_image_type_object_conveyor_2(void)
{
	crsed_selected_image = 18;
	return 1;
}

int crsed_menu_image_type_object_conveyor_3(void)
{
	crsed_selected_image = 19;
	return 1;
}

int crsed_menu_image_type_object_conveyor_4(void)
{
	crsed_selected_image = 20;
	return 1;
}

int crsed_menu_image_type_object_animal_1_1(void)
{
	crsed_selected_image = 21;
	return 1;
}

int crsed_menu_image_type_object_animal_1_2(void)
{
	crsed_selected_image = 22;
	return 1;
}

int crsed_menu_image_type_object_animal_1_3(void)
{
	crsed_selected_image = 23;
	return 1;
}

int crsed_menu_image_type_object_animal_1_4(void)
{
	crsed_selected_image = 24;
	return 1;
}

int crsed_menu_image_type_object_animal_2_1(void)
{
	crsed_selected_image = 25;
	return 1;
}

int crsed_menu_image_type_object_animal_2_2(void)
{
	crsed_selected_image = 26;
	return 1;
}

int crsed_menu_image_type_object_animal_2_3(void)
{
	crsed_selected_image = 27;
	return 1;
}

int crsed_menu_image_type_object_animal_2_4(void)
{
	crsed_selected_image = 28;
	return 1;
}

int crsed_menu_image_load(void)
{
	BITMAP * temp_image;
	PALETTE temp_palette;
	char * returnedfn;
	
	if(crsed_course)
	{
		returnedfn = ncd_file_select(0, crsed_image_filename, "Select Image File", crsed_filter_image_files);
		if(returnedfn)
		{
			temp_image = load_bitmap(returnedfn, temp_palette);
			if(temp_image)
			{
				convert_palette(temp_image, temp_palette, crsed_course->palette);
				if(*crsed_current_image)
				{
					destroy_bitmap(*crsed_current_image);
				}
				*crsed_current_image = temp_image;
				crsed_mark_change();
			}
			ustrcpy(crsed_image_filename, returnedfn);
		}
	}
	return 1;
}

int crsed_menu_image_save(void)
{
	char * returnedfn;
	char * ext[3] = {"bmp", "tga"};
	int i;
	
	if(crsed_course && *crsed_current_image)
	{
		returnedfn = ncd_file_select(1, crsed_image_filename, "Save Image File", crsed_filter_save_image_files);
		if(returnedfn)
		{
			for(i = 0; i < 2; i++)
			{
				if(!ustricmp(ext[i], get_extension(returnedfn)))
				{
					switch(i)
					{
						case 0:
						{
							save_bmp(returnedfn, *crsed_current_image, crsed_course->palette);
							break;
						}
						case 1:
						{
							save_tga(returnedfn, *crsed_current_image, crsed_course->palette);
							break;
						}
					}
					break;
				}
			}
			if(i >= 2)
			{
				replace_extension(returnedfn, returnedfn, "pcx", 1024);
				save_pcx(returnedfn, *crsed_current_image, crsed_course->palette);
			}
			ustrcpy(crsed_image_filename, returnedfn);
		}
	}
	return 1;
}

int crsed_menu_help_manual(void)
{
	PACKFILE * fp;
	
	fp = pack_fopen("course_editor.txt", "r");
	pack_fread(crsed_help_text, file_size_ex("course_editor.txt"), fp);
	pack_fclose(fp);
	
	crsed_render();
	ncdgui_color_dialog(crsed_help_dialog, gui_fg_color, gui_bg_color);
	centre_dialog(crsed_help_dialog);
	if(ncdgui_popup_dialog(crsed_help_dialog, 0) == 1)
	{
	}
	return 1;
}

int crsed_menu_help_about(void)
{
	alert("vGolf Course Editor v1.3", NULL, "Copyright 2011 T^3 Software", "OK", NULL, KEY_ENTER, 0);
	return 1;
}
