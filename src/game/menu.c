#include <allegro.h>
#include "menu.h"

MENU_ITEM * create_menu_item(int x, int y, int type, char * txt, char * sub_txt, char select){
        MENU_ITEM * mi;
	
        mi = (MENU_ITEM *)malloc(sizeof(MENU_ITEM));
        if(!mi){
		return NULL;
                }
        mi->x = x;
        mi->y = y;
        mi->item_type = type;
        mi->selectable = select;
        mi->text = (char *)malloc(strlen(txt)+1);
        strcpy(mi->text, txt);
        mi->subtext = (char *)malloc(strlen(sub_txt)+1);
        strcpy(mi->subtext, sub_txt);

        return mi;
}

//copy a menu
CMENU * copy_menu(CMENU * mn){
        CMENU * mn2;
	
        mn2 = (CMENU *)malloc(sizeof(CMENU));
        if(!mn2)
	{
		return NULL;
	}
        mn2->x = mn->x;
        mn2->y = mn->y;
        mn2->w = mn->w;
        mn2->h = mn->h;
        mn2->cur_item = mn->cur_item;
        mn2->root_menu = mn->root_menu;
        mn2->num_items = mn->num_items;
        mn2->num_submenus = mn->num_submenus;
        strcpy(mn2->label, mn->label);

        return mn2;
}

CMENU * create_menu(int w, int h, int x, int y, char * label, CMENU * root){
        CMENU * mn;
	
        mn = (CMENU *)malloc(sizeof(CMENU));
        if(!mn){
		return NULL;
                }
        mn->w = w;
        mn->h = h;
        mn->x = x;
        mn->y = y;
        mn->cur_item = 0;
        mn->root_menu = root;
        mn->num_items = 0;
        mn->num_submenus = 0;
        strcpy(mn->label, label);

        return mn;
}

void destroy_menu_item(MENU_ITEM * mi){
	int i;
	
        free(mi->text);
        free(mi->subtext);
        free(mi);
        
        for(i = 0; i < mi->num_refs; i++){
	        free(mi->c_ref_list[i]);
        	}
}

void destroy_menu(CMENU * mn){
        int i;

        for(i = 0; i < mn->num_items; i++){
                destroy_menu_item(mn->menu_item[i]);
                }

        for(i = 0; i < mn->num_submenus; i++){
                destroy_menu(mn->submenu[i]);
                }

        free(mn);
}

void set_menu_item_reflist(MENU_ITEM * mi, char * first, ...){
	va_list argp;
	char * p = first;
	
	mi->num_refs = 0;	
	mi->item_ref_set = mi->item_ref_end = 0;
	if(first == NULL) return;
	
	va_start(argp, first);
	
	while(p != NULL && p[0] != '\0'){
		//printf("%s, ", p);
		mi->c_ref_list[mi->num_refs++] = p;
		//strcpy(mi->c_ref_list[mi->num_refs++], p);
	 	p = va_arg(argp, char *);
		}
			
	va_end(argp);

	mi->num_refs--;	
}

void set_menu_item_refs(MENU_ITEM * mi, int ref, int ref_start, int ref_end){
        mi->item_ref = ref;
        mi->item_ref_set = ref_start;
        mi->item_ref_end = ref_end;
}

void menu_add_item(CMENU * mn, MENU_ITEM * mi){
        mn->menu_item[mn->num_items] = mi;
        mn->num_items++;
}

void menu_add_submenu(CMENU * mn, CMENU * sub_mn){
        mn->submenu[mn->num_submenus] = sub_mn;
        mn->num_submenus++;
}

void activate_menu(CMENU * mn){
        if(!mn->menu_item[mn->cur_item]->selectable){
                mn->cur_item++;
                }
        mn->active = 1;
}

void deactivate_menu(CMENU * mn){
        mn->active = 0;
}

//reads a keypress to define the player's keyboard input config
void menu_get_new_key(CMENU * mn, int key_ref){
        int temp_key;

        clear_keybuf();
        temp_key = readkey() >> 8;

        mn->menu_item[key_ref]->item_ref_set = temp_key;
        key[temp_key] = 0;
}

int get_menu_keys(MENU_CURSOR * mcur, CMENU ** mnp)
{
        int i;
        int next_item = -1;
        int exit_code = 0;
        CMENU * mn = *mnp;
        
		if(key[KEY_UP]){
                if(mn->cur_item > 0){
                        for(i = mn->cur_item-1; i >= 0; i--){
                                if(mn->menu_item[i]->selectable){
                                        next_item = i;
                                        break;
                                        }
                                }
                        if(next_item != -1) mn->cur_item = next_item;
                        }
                if(mn->menu_item[mn->cur_item]->item_ref == MENU_LOAD_COURSE){  //return an exit status that tells to load a new course preview
	                exit_code = MENU_LOAD_COURSE_PREVIEW;
                	}
                else if(mn->menu_item[mn->cur_item]->item_ref == MENU_CHANGE_PLAYER_1_NAME || mn->menu_item[mn->cur_item]->item_ref == MENU_CHANGE_PLAYER_2_NAME || mn->menu_item[mn->cur_item]->item_ref == MENU_CHANGE_PLAYER_3_NAME || mn->menu_item[mn->cur_item]->item_ref == MENU_CHANGE_PLAYER_4_NAME){  //return an exit status that tells to load a new statsheet
	                exit_code = MENU_CHANGE_PLAYER_SELECTION; //tell the main program we need to load this player's statsheets (if available)
                	}
                else{
	                exit_code = MENU_CHANGE_ITEM_SELECTION;   //tell the main program we selected a new item
                	}
                key[KEY_UP] = 0;
                }
        else if(key[KEY_DOWN]){
                if(mn->cur_item < mn->num_items-1){
                        for(i = mn->cur_item+1; i <= mn->num_items-1; i++){
                                if(mn->menu_item[i]->selectable){
                                        next_item = i;
                                        break;
                                        }
                                }
                        if(next_item != -1) mn->cur_item = next_item;
                        }
                if(mn->menu_item[mn->cur_item]->item_ref == MENU_LOAD_COURSE){  //return an exit status that tells to load a new course preview
	                exit_code = MENU_LOAD_COURSE_PREVIEW;
                	}
                else if(mn->menu_item[mn->cur_item]->item_ref == MENU_CHANGE_PLAYER_1_NAME || mn->menu_item[mn->cur_item]->item_ref == MENU_CHANGE_PLAYER_2_NAME || mn->menu_item[mn->cur_item]->item_ref == MENU_CHANGE_PLAYER_3_NAME || mn->menu_item[mn->cur_item]->item_ref == MENU_CHANGE_PLAYER_4_NAME){  //return an exit status that tells to load a new statsheet
	                exit_code = MENU_CHANGE_PLAYER_SELECTION;
                	}
                else{
	                exit_code = MENU_CHANGE_ITEM_SELECTION;   //tell the main program we selected a new item
                	}
                key[KEY_DOWN] = 0;
                }
        else if(key[KEY_LEFT]){
                if(mn->menu_item[mn->cur_item]->item_type == MENUITEM_FROMLIST){
                        if(mn->menu_item[mn->cur_item]->item_ref_set > 0){
                                mn->menu_item[mn->cur_item]->item_ref_set--;
                                }
                        exit_code = mn->menu_item[mn->cur_item]->item_ref;
                        }
                else if(mn->menu_item[mn->cur_item]->item_type == MENUITEM_SLIDER || mn->menu_item[mn->cur_item]->item_type == MENUITEM_CHARPIC_SELECT){
                        if(mn->menu_item[mn->cur_item]->item_ref_set > 0){
                                mn->menu_item[mn->cur_item]->item_ref_set--;
                                }
                        exit_code = mn->menu_item[mn->cur_item]->item_ref;
                        }
                
                key[KEY_LEFT] = 0;
                }
        else if(key[KEY_RIGHT]){
                if(mn->menu_item[mn->cur_item]->item_type == MENUITEM_FROMLIST){
                        if(mn->menu_item[mn->cur_item]->item_ref_set < mn->menu_item[mn->cur_item]->num_refs){
                                mn->menu_item[mn->cur_item]->item_ref_set++;
                                }
                        exit_code = mn->menu_item[mn->cur_item]->item_ref;
                        }
                else if(mn->menu_item[mn->cur_item]->item_type == MENUITEM_SLIDER || mn->menu_item[mn->cur_item]->item_type == MENUITEM_CHARPIC_SELECT){
                        if(mn->menu_item[mn->cur_item]->item_ref_set < mn->menu_item[mn->cur_item]->item_ref_end){
                                mn->menu_item[mn->cur_item]->item_ref_set++;
                                }
                        exit_code = mn->menu_item[mn->cur_item]->item_ref;
                        }

		key[KEY_RIGHT] = 0;
                }
        else if(key[KEY_ESC]){
                if(!mn->root_menu){
                        mn->cur_item = mn->num_items-1;
                        exit_code = MENU_CHANGE_TO_LASTITEM;
                        }
                else{
                        *mnp = mn->root_menu;
                        exit_code = MENU_CHANGE_TO_ROOTMENU;
                        }
                key[KEY_ESC] = 0;
                }
        else if(key[KEY_ENTER]){
                switch(mn->menu_item[mn->cur_item]->item_type){
                        case MENUITEM_GOTO_SUBMENU:
                                //printf("going to submenu %s\n", mn->submenu[mn->menu_item[mn->cur_item]->item_ref]->label);
                                *mnp = mn->submenu[mn->menu_item[mn->cur_item]->item_ref];
                                exit_code = MENU_CHANGE_TO_SUBMENU;
                                break;
                        case MENUITEM_GOTO_ROOTMENU:
                                *mnp = mn->root_menu;
                                break;
                        case MENUITEM_GOTO_MAINMENU:
                                break;
                        case MENUITEM_KEYDEFINE:
                        	menu_get_new_key(mn, mn->cur_item);
                        	exit_code = MENUITEM_KEYDEFINE;
                                break;
                        case MENUITEM_FROMLIST:
                        	if(mn->menu_item[mn->cur_item]->item_ref >= MENU_DEFINE_KEYS_PLAYER_4 && mn->menu_item[mn->cur_item]->item_ref <= MENU_DEFINE_KEYS_PLAYER_1){
	                        	if(mn->menu_item[mn->cur_item]->item_ref_set == mn->menu_item[mn->cur_item]->num_refs){
	                        		exit_code = MENU_CUSTOM_KEY_OFFSET+mn->menu_item[mn->cur_item]->item_ref;
	                        		}
	                                else exit_code = mn->menu_item[mn->cur_item]->item_ref;
                        		}
	                        else exit_code = mn->menu_item[mn->cur_item]->item_ref;
                                break;
                        case MENUITEM_CHARNAME:
                                exit_code = MENU_CHANGE_PLAYER_NAME;
                                break;
                        case MENUITEM_LOAD_COURSE:
                        	exit_code = MENU_LOAD_COURSE;
                        	break;
                        case MENUITEM_QUITFE:
                                exit_code = MENU_QUIT_FRONTEND;
                                break;
                        default:
                                break;
                        }
                key[KEY_ENTER] = 0;
                }
	else{    //no key pressed this time..
                switch(mn->menu_item[mn->cur_item]->item_ref){
	                case MENU_LOAD_COURSE:
	                	exit_code = MENU_LOAD_COURSE_PREVIEW_NEW;  //tell the ui that we need to load a course preview
	                default:
	                	break;
                	}
		}
        
        return exit_code;
}