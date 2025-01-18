#ifndef AEGUI_H
#define AEGUI_H

extern char ae_help_text[4096];

extern MENU ae_file_menu[];
extern MENU ae_edit_menu[];
extern MENU ae_help_menu[];

extern DIALOG ae_main_dialog[];
extern DIALOG ae_help_dialog[];

void ae_prepare_menus(void);

#endif
