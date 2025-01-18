#include <allegro.h>
#include "../modules/ncds.h"
#include "commentary.h"

//load a message bank from file
VGOLF_COMMENTARY * load_vgolf_commentary_script(const char * filename)
{
	char done = 0;
	int count = 0;
	int i, j;
	int cur_type = -1;
	VGOLF_COMMENTARY * cp;
	char tfilename[1024];
	
	cp = malloc(sizeof(VGOLF_COMMENTARY));
	if(!cp)
	{
		return NULL;
	}
	
	PACKFILE * in_file = pack_fopen(filename, "r");
	//file doesn't exist, return error
	if(in_file == NULL) return NULL;

	//reset the string counters for each type	
	for(i = 0; i < MAX_MESSAGE_STRING_TYPES; i++)
	{
		cp->comments[i].count = 0;
	}
	
	while(!done)
	{
//  		int nbytes = 100;
  		char temp_string[100];
  		int temp_pos = 0;
  		int line_pos = 0;
  		char k = -1;

  		//allocate memory for the temp string
//  		temp_string = (char *) malloc (nbytes + 1);
  		//grab a line from the text file (fgets should get all characters up till an EOF or EOL character)
  		temp_pos = 0;
  		temp_string[0] = 0;
  		while(!pack_feof(in_file) && k != '\n' && k != '\r' && k != '\0')
  		{
	  		k = pack_getc(in_file);
  			temp_string[temp_pos] = k;
  			temp_pos++;
  		}
  		if(pack_feof(in_file))
  		{
	  		done = 1;
  		}
  		temp_string[temp_pos - 1] = '\0';
//		textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), makecol(0, 0, 0), "%s", temp_string);
//		clear_keybuf();
//		readkey();
//  		if(!pack_fgets(temp_string, 100, in_file))
 // 		{
//	  		done = 1;
  //		}

      		//printf("current string is '%s' with length %i\n", temp_string, strlen(temp_string));
      		switch(temp_string[0])
      		{
	      		case '#':   //comment line was found.. ignore this string
	      			//printf("comment at line %i: '%s'\n", count, temp_string);
      				break;
	      		case ':':   //this tells to look for the string type
	      			if(!strcmp(temp_string, ":good finish")) cur_type = MESSAGE_TYPE_FINISH_GOOD;
	      			else if(!strcmp(temp_string, ":better finish")) cur_type = MESSAGE_TYPE_FINISH_BETTER;
	      			else if(!strcmp(temp_string, ":great finish")) cur_type = MESSAGE_TYPE_FINISH_GREAT;
	      			else if(!strcmp(temp_string, ":excellent finish")) cur_type = MESSAGE_TYPE_FINISH_EXCELLENT;
	      			else if(!strcmp(temp_string, ":poor finish")) cur_type = MESSAGE_TYPE_FINISH_POOR;
	      			else if(!strcmp(temp_string, ":very poor finish")) cur_type = MESSAGE_TYPE_FINISH_VERYPOOR;
	      			else if(!strcmp(temp_string, ":zapped")) cur_type = MESSAGE_TYPE_ZAPPED;
	      			else if(!strcmp(temp_string, ":close")) cur_type = MESSAGE_TYPE_CLOSE;
	      			else if(!strcmp(temp_string, ":water")) cur_type = MESSAGE_TYPE_WATER;
	      			else if(!strcmp(temp_string, ":crushed")) cur_type = MESSAGE_TYPE_CRUSHED;
	      			
		      		//printf("current type is %i\n", cur_type);
	      			break;
	      		case '\"':  //this tells it to start reading a new message..
	      			//break if no message type is currently set
	      			if(cur_type == -1)
	      			{
      					textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), makecol(0, 0, 0), "Incorrect type!");
		      			break;
	      			}
	      			
	      			//copy only the characters between the quotation marks..
//	      			for(i = 1; i < strlen(temp_string)-1; i++)
	      			for(i = 1; temp_string[i] != '\"'; i++)
	      			{
		      			cp->comments[cur_type].comment[cp->comments[cur_type].count].string[i - 1] = temp_string[i];
//		      			mbank->message_string[cur_type][mbank->num_strings[cur_type]][i-1] = temp_string[i];
	      			}
   	//				set_palette(default_palette);
   	//				clear_bitmap(screen);
//  					textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), makecol(0, 0, 0), "%s", cp->comments[cur_type].comment[cp->comments[cur_type].count].string);
  // 					clear_keybuf();
   	//				readkey();
	      			
	      			/* see if we have a sound also */
	      			cp->comments[cur_type].comment[cp->comments[cur_type].count].voice = NULL;
	      			i++;
	      			if(i < strlen(temp_string) - 1)
	      			{
		      			if(temp_string[i] == ',')
		      			{
			      			strcpy(tfilename, "commentaries/");
			      			line_pos = strlen(tfilename);
			      			for(i = i + 1; i < strlen(temp_string); i++)
	      					{
		      					tfilename[line_pos] = temp_string[i];
		      					line_pos++;
	      					}
	      					tfilename[line_pos] = '\0';
//	      					set_palette(default_palette);
//	      					clear_bitmap(screen);
//	      					textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), makecol(0, 0, 0), "%s", tfilename);
//	      					clear_keybuf();
//	      					readkey();
							strcpy(cp->comments[cur_type].comment[cp->comments[cur_type].count].voice_file, tfilename);
		      			}
		      			else
		      			{
							strcpy(cp->comments[cur_type].comment[cp->comments[cur_type].count].voice_file, "No Voice");
		      			}
	      			}
	      			
	      			//make sure the string is nullified at the right place
	      			cp->comments[cur_type].comment[cp->comments[cur_type].count].string[strlen(temp_string) - 3] = '\0';
//				mbank->message_string[cur_type][mbank->num_strings[cur_type]][strlen(temp_string)-3] = '\0';	      				
	      			//sscanf(temp_string, "\"%s\"", mbank->message_string[cur_type][mbank->num_strings[cur_type]]);
	      			
	      			//printf("on line %i: type %i string #%i: '%s'\n", count, cur_type, mbank->num_strings[cur_type], mbank->message_string[cur_type][mbank->num_strings[cur_type]]);
	      			//make sure we don't get more than the max # of strings
	      			cp->comments[cur_type].count++;
	      			if(cp->comments[cur_type].count > MAX_MESSAGE_STRINGS)
	      			{
		      			cp->comments[cur_type].count = MAX_MESSAGE_STRINGS;
	      			}
	      				
	      			break;
	      		default:
	      			break;
      		}
      		//increment the line counter
    		count++;
		}
		
	//close the file stream	
	pack_fclose(in_file);
	
	for(i = 0; i < MAX_MESSAGE_STRING_TYPES; i++)
	{
		for(j = 0; j < cp->comments[i].count; j++)
		{
			if(stricmp(cp->comments[i].comment[j].voice_file, "No Voice"))
			{
      			cp->comments[i].comment[j].voice = load_wav(cp->comments[i].comment[j].voice_file);
//      			play_sample(
//   			set_palette(default_palette);
//   			textprintf_ex(screen, font, 0, 0, makecol(255, 255, 255), makecol(0, 0, 0), "file : %s          ", cp->comments[i].comment[j].voice_file);
//   			clear_keybuf();
//  			readkey();
//        	play_sample(cp->comments[i].comment[j].voice, 255, 128, 1000, 0);
			}
			else
			{
				cp->comments[i].comment[j].voice = NULL;
			}
		}
	}
	
	//file was successfully loaded, happy happy joy joy
	return cp;
}

VGOLF_COMMENTARY * load_vgolf_commentary(const char * fn)
{
	VGOLF_COMMENTARY * cp = NULL;
	PACKFILE * fp;
	char header[16];
	int i, j, m;
	
	fp = pack_fopen(fn, "rp");
	if(!fp)
	{
		return NULL;
	}
	
	cp = malloc(sizeof(VGOLF_COMMENTARY));
	if(!cp)
	{
		pack_fclose(fp);
		return NULL;
	}
	
	pack_fread(header, 16, fp);
	if(stricmp(header, "vGolf_vc"))
	{
		return NULL;
	}
	
	pack_fread(cp->name, 128, fp);
	pack_fread(cp->author, 256, fp);
	pack_fread(cp->comment, 1024, fp);
	
	switch(header[15])
	{
		case 1:
		{
			m = pack_igetl(fp);
			for(i = 0; i < m; i++)
			{
				cp->comments[i].count = pack_igetl(fp);
				for(j = 0; j < cp->comments[i].count; j++)
				{
					if(pack_getc(fp))
					{
						cp->comments[i].comment[j].voice = ncds_load_wav_fp(fp);
					}
				}
			}
			break;
		}
	}
	pack_fclose(fp);
	
	return cp;
}

VGOLF_COMMENTARY * load_vgolf_commentary_info(const char * fn)
{
	VGOLF_COMMENTARY * cp = NULL;
	PACKFILE * fp;
	char header[16];
	int i;
	
	fp = pack_fopen(fn, "rp");
	if(!fp)
	{
		return NULL;
	}
	
	cp = malloc(sizeof(VGOLF_COMMENTARY));
	if(!cp)
	{
		pack_fclose(fp);
		return NULL;
	}
	
	pack_fread(header, 16, fp);
	if(stricmp(header, "vGolf_vc"))
	{
		return NULL;
	}
	
	pack_fread(cp->name, 128, fp);
	pack_fread(cp->author, 256, fp);
	pack_fread(cp->comment, 1024, fp);
	
	for(i = 0; i < MAX_MESSAGE_STRING_TYPES; i++)
	{
		cp->comments[i].count = 0;
	}
	
	pack_fclose(fp);
	
	return cp;
}

int save_vgolf_commentary(VGOLF_COMMENTARY * cp, const char * fn)
{
	PACKFILE * fp;
	char header[16] = "vGolf_vc\0";
	int i, j;
	
	header[15] = VGOLF_COMMENTARY_VERSION;
	fp = pack_fopen(fn, "wp"); // use compression
	if(!fp)
	{
		return 0;
	}
	
	/* save commentary data */
	pack_fwrite(header, 16, fp);
	pack_fwrite(cp->name, 128, fp);
	pack_fwrite(cp->author, 256, fp);
	pack_fwrite(cp->comment, 1024, fp);
	pack_iputl(MAX_MESSAGE_STRING_TYPES, fp);	

	for(i = 0; i < MAX_MESSAGE_STRING_TYPES; i++)
	{
		pack_iputl(cp->comments[i].count, fp);
		for(j = 0; j < cp->comments[i].count; j++)
		{
			if(cp->comments[i].comment[j].voice)
			{
				pack_putc(1, fp);
				ncds_save_wav_fp(cp->comments[i].comment[j].voice, fp);
			}
			else
			{
				pack_putc(0, fp);
			}
		}
	}
	pack_fclose(fp);

	return 1;
}

void destroy_vgolf_commentary(VGOLF_COMMENTARY * cp)
{
	int i, j;
	
	for(i = 0; i < MAX_MESSAGE_STRING_TYPES; i++)
	{
		for(j = 0; j < cp->comments[i].count; j++)
		{
			if(cp->comments[i].comment[j].voice)
			{
				destroy_sample(cp->comments[i].comment[j].voice);
			}
		}
	}
	free(cp);
}
