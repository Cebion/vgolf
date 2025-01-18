#ifndef AEGUI_H
#define AEGUI_H

extern char ce_help_text[4096];
extern char ce_edit_text[4][1024];

extern MENU ce_file_menu[];
extern MENU ce_edit_menu[];
extern MENU ce_help_menu[];

extern DIALOG ce_main_dialog[];
extern DIALOG ce_help_dialog[];
extern DIALOG ce_tags_dialog[];

void ce_prepare_menus(void);

#endif
