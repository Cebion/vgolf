#ifndef AE_MAIN_H
#define AE_MAIN_H

#include <allegro.h>
#include "../modules/wfsel.h"
#include "../game/avatar.h"

#define AE_TOOL_PAINT            0
#define AE_TOOL_LINE             1
#define AE_TOOL_FILL             2
#define AE_TOOL_RECTANGLE        3
#define AE_TOOL_FILLED_RECTANGLE 4
#define AE_TOOL_CLEAR            5
#define AE_TOOL_CIRCLE           6
#define AE_TOOL_FILLED_CIRCLE    7
#define AE_TOOL_SELECT           8

extern int ae_quit;
extern BITMAP * ae_menu_image;
extern BITMAP * ae_screen;
extern BITMAP * ae_scratch;
extern BITMAP * ae_clipboard;
extern BITMAP * ae_float;
extern BITMAP * ae_float_bg;
extern PALETTE ae_palette;
extern char ae_filename[1024];
extern char ae_image_filename[1024];
extern VGOLF_AVATAR * ae_avatar;
extern int ae_change_count;
extern int ae_changes;
extern int ae_selected_image;

extern int ae_selected_tool;
extern int ae_float_x, ae_float_y;

/* drawing state */
extern int ae_color;
extern int ae_drawing;
extern int ae_selecting;
extern int ae_select_left;
extern int ae_select_top;
extern int ae_select_right;
extern int ae_select_bottom;
extern int ae_float_left;
extern int ae_float_top;
extern int ae_float_right;
extern int ae_float_bottom;
extern int ae_clipboard_left;
extern int ae_clipboard_top;
extern int ae_clipboard_right;
extern int ae_clipboard_bottom;

extern NCDFS_FILTER_LIST * ae_filter_avatar_files;
extern NCDFS_FILTER_LIST * ae_filter_image_files;

void ae_fix_window_title(void);
void ae_reset_state(void);
void ae_unfloat_selection(void);
void ae_prepare_undo(int type);
void ae_render(void);

#endif
