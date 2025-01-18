#include <allegro.h>
#include "main.h"
#include "profile.h"

int profile_mode = 0;
int profile_selected = 0;
int profile_mode_selected = -1;

int load_old_profiles_pf(PROFILE_LIST * lp, PACKFILE * fp)
{
	char cname[256] = {0};
	char cused[VGOLF_MAX_COURSES] = {0};
	int i, j, k;
	int ci = -1;
	int tc = 0;
	
	lp->items = pack_igetl(fp);
	for(i = 0; i < lp->items; i++)
	{
		pack_fread(lp->item[i].name, 128, fp);
		lp->item[i].avatar = pack_igetl(fp);
		tc = pack_igetl(fp);
		memset(cused, 0, VGOLF_MAX_COURSES);
		for(j = 0; j < tc; j++)
		{
			
			/* read course name and find which course this entry points to */
			pack_fread(cname, 256, fp);
			ci = -1;
			for(k = 0; k < vgolf_courses.items; k++)
			{
				if(!stricmp(cname, vgolf_courses.item[k].name))
				{
					ci = k;
					break;
				}
			}
			
			/* if the course name is found, load this data into correct spot */
			if(ci >= 0 && !cused[ci])
			{
				lp->item[i].course_info[ci].id = vgolf_courses.item[ci].sum;
				for(k = 0; k < 18; k++)
				{
					lp->item[i].course_info[ci].score[k] = pack_igetl(fp);
				}
				lp->item[i].course_info[ci].total_score = pack_igetl(fp);
				lp->item[i].course_info[ci].won = pack_igetl(fp);
				lp->item[i].course_info[ci].lost = pack_igetl(fp);
				lp->item[i].course_info[ci].completed = pack_igetl(fp);
				if(lp->item[i].course_info[ci].score[0] != 0)
				{
					cused[ci] = 1;
				}
			}
			
			/* otherwise skip data */
			else
			{
				for(k = 0; k < 18; k++)
				{
					pack_igetl(fp);
				}
				pack_igetl(fp);
				pack_igetl(fp);
				pack_igetl(fp);
				pack_igetl(fp);
			}
		}
		for(j = 0; j < VGOLF_MAX_PROFILE_DATA; j++)
		{
			lp->item[i].total.data[j] = pack_igetl(fp);
		}
		lp->item[i].total.time = pack_igetl(fp);
	}
	return 1;
}

int load_profiles(PROFILE_LIST * lp, char * fn)
{
	PACKFILE * fp;
	char header[4] = {'V', 'P', 'D', 1};
	char rheader[4];
	unsigned long cid = 0;
	char cused[VGOLF_MAX_COURSES] = {0};
	int i, j, k;
	int ci = -1;
	int tc = 0;
	int ret = 0;
	
	fp = pack_fopen(fn, "r");
	if(!fp)
	{
		return 0;
	}
	pack_fread(rheader, 4, fp);
	if(memcmp(header, rheader, 3))
	{
		return 0;
	}
	if(rheader[3] == 0)
	{
		ret = load_old_profiles_pf(lp, fp);
		pack_fclose(fp);
		return ret;
	}
	else
	{
		lp->items = pack_igetl(fp);
		for(i = 0; i < lp->items; i++)
		{
			pack_fread(lp->item[i].name, 128, fp);
			lp->item[i].avatar = pack_igetl(fp);
			tc = pack_igetl(fp);
			memset(cused, 0, VGOLF_MAX_COURSES);
			for(j = 0; j < tc; j++)
			{
				
				/* read course id and find which course this entry points to */
				cid = pack_igetl(fp);
				ci = -1;
				for(k = 0; k < vgolf_courses.items; k++)
				{
					if(cid == vgolf_courses.item[k].sum)
					{
						ci = k;
						break;
					}
				}
				
				/* if the course id is found, load this data into correct spot */
				if(ci >= 0 && !cused[ci])
				{
					lp->item[i].course_info[ci].id = cid;
					for(k = 0; k < 18; k++)
					{
						lp->item[i].course_info[ci].score[k] = pack_igetl(fp);
					}
					lp->item[i].course_info[ci].total_score = pack_igetl(fp);
					lp->item[i].course_info[ci].won = pack_igetl(fp);
					lp->item[i].course_info[ci].lost = pack_igetl(fp);
					lp->item[i].course_info[ci].completed = pack_igetl(fp);
					cused[ci] = 1;
				}
				
				/* otherwise skip data */
				else
				{
					for(k = 0; k < 18; k++)
					{
						pack_igetl(fp);
					}
					pack_igetl(fp);
					pack_igetl(fp);
					pack_igetl(fp);
					pack_igetl(fp);
				}
			}
			for(j = 0; j < VGOLF_MAX_PROFILE_DATA; j++)
			{
				lp->item[i].total.data[j] = pack_igetl(fp);
			}
			lp->item[i].total.time = pack_igetl(fp);
		}
		pack_fclose(fp);
		return 1;
	}
	return 0;
}

int save_profiles(PROFILE_LIST * lp, char * fn)
{
	PACKFILE * fp;
	char header[4] = {'V', 'P', 'D', 1};
	int i, j, k;
	
	fp = pack_fopen(fn, "w");
	if(!fp)
	{
		return 0;
	}
	pack_fwrite(header, 4, fp);
	
	pack_iputl(lp->items, fp);
	for(i = 0; i < lp->items; i++)
	{
		pack_fwrite(lp->item[i].name, 128, fp);
		pack_iputl(lp->item[i].avatar, fp);
		pack_iputl(vgolf_courses.items, fp);
		for(j = 0; j < vgolf_courses.items; j++)
		{
			pack_iputl(vgolf_courses.item[j].sum, fp);
			for(k = 0; k < 18; k++)
			{
				pack_iputl(lp->item[i].course_info[j].score[k], fp);
			}
			pack_iputl(lp->item[i].course_info[j].total_score, fp);
			pack_iputl(lp->item[i].course_info[j].won, fp);
			pack_iputl(lp->item[i].course_info[j].lost, fp);
			pack_iputl(lp->item[i].course_info[j].completed, fp);
		}
		for(j = 0; j < VGOLF_MAX_PROFILE_DATA; j++)
		{
			pack_iputl(lp->item[i].total.data[j], fp);
		}
		pack_iputl(lp->item[i].total.time, fp);
	}
	pack_fclose(fp);
	return 1;
}

void clear_profiles(PROFILE_LIST * lp)
{
	lp->items = 0;
}
