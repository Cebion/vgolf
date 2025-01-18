#include <allegro.h>
#include "../modules/agup/agup.h"
#include "../modules/ncdgui.h"
#include "guiproc.h"
#include "main.h"

char ce_help_text[4096] = {0};
char ce_edit_text[4][1024] = {{0}};

MENU ce_file_menu[] =
{
    {"&New", ce_menu_file_new, NULL, 0, NULL},
    {"&Open", ce_menu_file_load, NULL, 0, NULL},
    {"&Save", ce_menu_file_save, NULL, 0, NULL},
    {"Save &As", ce_menu_file_save_as, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&Tags", ce_menu_file_tags, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"E&xit", ce_menu_file_exit, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU ce_help_menu[] =
{
    {"&Manual", ce_menu_help_manual, NULL, 0, NULL},
    {"", NULL, NULL, 0, NULL},
    {"&About", ce_menu_help_about, NULL, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

MENU ce_main_menu[] =
{
    {"&File", NULL, ce_file_menu, 0, NULL},
    {"&Help", NULL, ce_help_menu, 0, NULL},
    {NULL, NULL, NULL, 0, NULL}
};

DIALOG ce_main_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_menu_proc, 0, 0, 464, 16, 0, 0, 0, 0, 0, 0, ce_main_menu, NULL, NULL },
   { d_agup_box_proc,    0,  16,  464, 216 - 32, 2,   23,  0,    0,      0,   0,   "Commentary",               NULL, NULL },
   { d_ce_list_proc,    16, 48 - 16,  160, 152, 2,   23,  0,    0,      0,   0,   ce_type_list_proc,               NULL, NULL },
   { d_agup_list_proc,    192,  48 - 16,  160, 152, 2,   23,  0,    0,      0,   0,   ce_voice_list_proc,               NULL, NULL },
   { d_agup_push_proc,    368,  168 - 16, 80, 32, 2, 23, 0, 0, 0, 0, "Listen", NULL, ce_voice_listen},
   { d_agup_push_proc,    368,  48 - 16, 80, 32, 2, 23, 0, 0, 0, 0, "Add", NULL, ce_voice_add},
   { d_agup_push_proc,    368,  88 - 16, 80, 32, 2, 23, 0, 0, 0, 0, "Delete", NULL, ce_voice_delete},
   { d_agup_push_proc,    368,  128 - 16, 80, 32, 2, 23, 0, 0, 0, 0, "Export", NULL, ce_voice_export},
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG ce_tags_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  376, 136, 2,   23,  0,    0,      0,   0,   "Tags",               NULL, NULL },
   { d_agup_text_proc,   8,  24 + 4,  96, 8,  2,   23,  0,    0,      0,   0,   "Title:", NULL, NULL },
   { d_agup_edit_proc,   112,  24,  256, 8,  2,   23,  0,    0,      128,   0,   ce_edit_text[0], NULL, NULL },
   { d_agup_text_proc,   8,  48 + 4,  96, 8,  2,   23,  0,    0,      0,   0,   "Author:", NULL, NULL },
   { d_agup_edit_proc,   112,  48,  256, 8,  2,   23,  0,    0,      256,   0,   ce_edit_text[1], NULL, NULL },
   { d_agup_text_proc,   8,  72 + 4,  96, 8,  2,   23,  0,    0,      0,   0,   "Comment:", NULL, NULL },
   { d_agup_edit_proc,   112,  72,  256, 8,  2,   23,  0,    0,      1024,   0,   ce_edit_text[2], NULL, NULL },
   { d_agup_button_proc, 8,  96, 360,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

DIALOG ce_help_dialog[] =
{
   /* (proc)         (x)  (y)  (w)  (h)  (fg) (bg) (key) (flags) (d1) (d2) (dp)           (dp2) (dp3) */
   { d_agup_window_proc,    0,  0,  464, 200, 2,   23,  0,    0,      0,   0,   "vGolf Commentary Editor Help",               NULL, NULL },
   { d_agup_textbox_proc,   8,  32,  448, 120,  2,   23,  0,    0,      0,   0,   ce_help_text, NULL, NULL },
   { d_agup_button_proc, 8,  160, 448,  32, 2,   23,  '\r',    D_EXIT, 0,   0,   "Okay",               NULL, NULL },
   { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
};

void ce_prepare_menus(void)
{
	ce_main_dialog[2].dp3 = &ce_main_dialog[3];
}
