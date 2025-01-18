#ifndef AE_MAIN_H
#define AE_MAIN_H

#include <allegro.h>
#include "../modules/wfsel.h"
#include "../game/commentary.h"

extern int ce_quit;
extern char ce_filename[1024];
extern char ce_wav_filename[1024];
extern int ce_change_count;
extern int ce_changes;
extern VGOLF_COMMENTARY * ce_commentary;

extern NCDFS_FILTER_LIST * ce_filter_commentary_files;
extern NCDFS_FILTER_LIST * ce_filter_sound_files;

void ce_fix_window_title(void);

#endif
