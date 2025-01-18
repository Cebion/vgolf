#ifndef CRSED_MAIN_H
#define CRSED_MAIN_H

#include <allegro.h>
#include "../modules/wfsel.h"
#include "../game/course.h"

extern int crsed_quit;
extern BITMAP * crsed_menu_image;
extern BITMAP * crsed_screen;
extern PALETTE crsed_palette;
extern BITMAP ** crsed_current_image;
extern char crsed_filename[1024];
extern char crsed_image_filename[1024];
extern VGOLF_COURSE * crsed_course;
extern int crsed_change_count;
extern int crsed_changes;
extern int crsed_selected_hole;
extern int crsed_selected_poly;
extern int crsed_selected_point;
extern int crsed_selected_vert;
extern int crsed_selected_vert_type;
extern int crsed_selected_image;
extern int crsed_mode;
extern int crsed_vert_mode;
extern int crsed_connect_count;

extern NCDFS_FILTER_LIST * crsed_filter_course_files;
extern NCDFS_FILTER_LIST * crsed_filter_image_files;
extern NCDFS_FILTER_LIST * crsed_filter_save_image_files;
extern NCDFS_FILTER_LIST * crsed_filter_all_files;
extern NCDFS_FILTER_LIST * crsed_filter_hole_files;

extern int crsed_object_type[];

void crsed_fix_window_title(void);
void crsed_mark_change(void);
void crsed_delete_poly(int poly);
void crsed_render(void);

#endif
