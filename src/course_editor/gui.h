#ifndef CRSEDGUI_H
#define CRSEDGUI_H

#define CRSED_GUI_OBJECT_PROPERTIES_COLOR     0
#define CRSED_GUI_OBJECT_PROPERTIES_CONNECT   1
#define CRSED_GUI_OBJECT_PROPERTIES_ANGLE     2
#define CRSED_GUI_OBJECT_PROPERTIES_TIMER     3
#define CRSED_GUI_OBJECT_PROPERTIES_TIMER2    4
#define CRSED_GUI_OBJECT_PROPERTIES_SPEED     5
#define CRSED_GUI_OBJECT_PROPERTIES_SIZE      6
#define CRSED_GUI_OBJECT_PROPERTIES_DIRECTION 7

extern char crsed_help_text[8192];
extern char crsed_edit_text[4][1024];

extern MENU crsed_file_menu[];
extern MENU crsed_help_menu[];

extern DIALOG crsed_main_dialog[];
extern DIALOG crsed_palette_dialog[];
extern DIALOG crsed_timer_dialog[];
extern DIALOG crsed_speed_dialog[];
extern DIALOG crsed_speed2_dialog[];
extern DIALOG crsed_direction_dialog[];
extern DIALOG crsed_size_dialog[];
extern DIALOG crsed_par_dialog[];
extern DIALOG crsed_tags_dialog[];
extern DIALOG crsed_help_dialog[];

void crsed_prepare_menus(void);

#endif
