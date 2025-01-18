#ifndef GUIPROC_H
#define GUIPROC_H

int ae_menu_file_new(void);
int ae_menu_file_load(void);
int ae_menu_file_save(void);
int ae_menu_file_save_as(void);
int ae_menu_file_load_image(void);
int ae_menu_file_exit(void);

int ae_menu_edit_undo(void);
int ae_menu_edit_redo(void);
int ae_menu_edit_copy(void);
int ae_menu_edit_paste(void);

int ae_menu_image_mirror(void);
int ae_menu_image_flip(void);
int ae_menu_image_turn_clockwise(void);
int ae_menu_image_turn_counter_clockwise(void);
int ae_menu_image_shift_left(void);
int ae_menu_image_shift_right(void);
int ae_menu_image_shift_up(void);
int ae_menu_image_shift_down(void);

int ae_menu_help_manual(void);
int ae_menu_help_about(void);

#endif
