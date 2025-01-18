#ifndef NCDGUI_H
#define NCDGUI_H

#define KEY_EITHER_ALT (key[KEY_ALT] || key[KEY_ALTGR])
#define KEY_EITHER_CTRL (key[KEY_LCONTROL] || key[KEY_RCONTROL])
#define KEY_EITHER_SHIFT (key[KEY_LSHIFT] || key[KEY_RSHIFT])

#define NCDGUI_CURSOR_OS       0
#define NCDGUI_CURSOR_SOFTWARE 1

void ncdgui_color_dialog(DIALOG * dp, int fg, int bg);
int ncdgui_popup_dialog(DIALOG * dp, int n);

void ncdgui_initialize(int cursor);

BITMAP * ncdgui_generate_image(DIALOG * dp);

#endif
