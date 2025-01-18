#include <allegro.h>
#include "ncds.h"

int main(void)
{
	allegro_init();
	set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0);
	install_keyboard();
	ncds_install();
	while(!key[KEY_ESC])
	{
		if(key[KEY_M])
		{
			ncds_play_music("base.xm", 1);
			key[KEY_M] = 0;
		}
		ncds_update_music();
		putpixel(screen, rand() % 640, rand() % 480, rand() % 256);
	}
	return 0;
}
END_OF_MAIN();
