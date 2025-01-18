#ifndef GUIPROC_H
#define GUIPROC_H

int d_ce_list_proc(int msg, DIALOG * d, int c);

int ce_menu_file_new(void);
int ce_menu_file_load(void);
int ce_menu_file_save(void);
int ce_menu_file_save_as(void);
int ce_menu_file_exit(void);

int ce_menu_help_manual(void);
int ce_menu_help_about(void);

char * ce_type_list_proc(int index, int *list_size);
char * ce_voice_list_proc(int index, int *list_size);
int ce_voice_listen(DIALOG *d);
int ce_voice_add(DIALOG *d);
int ce_voice_delete(DIALOG *d);
int ce_voice_export(DIALOG *d);
int ce_menu_file_tags(void);

#endif
