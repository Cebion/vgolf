#include "main.h"
#include "undo.h"

char * ae_undo_filename[AE_MAX_UNDO] = {"ae.undo0", "ae.undo1", "ae.undo2", "ae.undo3", "ae.undo4", "ae.undo5", "ae.undo6", "ae.undo7"};
int ae_undo_index[AE_MAX_UNDO] = {0};
int ae_undo_type[AE_MAX_UNDO] = {0};
int ae_undo_last_type = 0;
int ae_undo_current_index = 0;
int ae_undo_count = 0;
int ae_redo_count = 0;
int ae_undo_toggle = 0;
int ae_redo_toggle = 0;

int ae_undo_load_state(const char * fn)
{
	if(ae_avatar)
	{
		vgolf_destroy_avatar(ae_avatar);
	}
	ae_avatar = vgolf_load_avatar(fn, NULL);
	return 1;
}

void ae_undo_reset(void)
{
	int i;
	
	for(i = 0; i < AE_MAX_UNDO; i++)
	{
		ae_undo_index[i] = 0;
	}
	ae_undo_current_index = 0;
	ae_undo_count = 0;
	ae_redo_count = 0;
}

int ae_undo_add(int type)
{
	ae_undo_last_type = type;
	vgolf_save_avatar(ae_avatar, ae_palette, ae_undo_filename[ae_undo_current_index]);
	ae_undo_type[ae_undo_current_index] = type;
	ae_undo_current_index++;
	if(ae_undo_current_index >= 8)
	{
		ae_undo_current_index = 0;
	}
	ae_undo_count++;
	if(ae_undo_count >= 8)
	{
		ae_undo_count = 8;
	}
	return 1;
}

int ae_undo_apply(void)
{
	if(ae_undo_count > 0)
	{
		vgolf_save_avatar(ae_avatar, ae_palette, "ae.redo");
		ae_undo_current_index--;
		if(ae_undo_current_index < 0)
		{
			ae_undo_current_index = 7;
		}
		ae_undo_load_state(ae_undo_filename[ae_undo_current_index]);
		ae_undo_count--;
		ae_redo_count = 1;
		ae_change_count--;
		if(ae_change_count == 0)
		{
			ae_changes = 0;
		}
		else
		{
			ae_changes = 1;
		}
		ae_undo_last_type = 0;
		return 1;
	}
	return 0;
}

void ae_redo_apply(void)
{
	if(ae_redo_count > 0)
	{
		vgolf_save_avatar(ae_avatar, ae_palette, ae_undo_filename[ae_undo_current_index]);
		ae_undo_current_index++;
		if(ae_undo_current_index >= 8)
		{
			ae_undo_current_index = 0;
		}
		ae_undo_load_state("ae.redo");
		ae_undo_count++;
		if(ae_undo_count >= 8)
		{
			ae_undo_count = 8;
		}
		ae_redo_count = 0;
		ae_change_count++;
		if(ae_change_count == 0)
		{
			ae_changes = 0;
		}
		else
		{
			ae_changes = 1;
		}
	}
}
