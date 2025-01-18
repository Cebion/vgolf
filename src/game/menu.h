#define MAX_MENU_ITEM_REFS 10
#define MAX_MENU_ITEMS 16
#define MAX_TITLE_PRI_MENUS 5
#define MAX_TITLE_SUB_MENUS 8
#define MAX_MENU_SUB_MENUS 8

#define MENUITEM_GOTO_SUBMENU 0
#define MENUITEM_EXIT 1
#define MENUITEM_SLIDER 2
#define MENUITEM_CHECKBOX 3
#define MENUITEM_FROMLIST 4
#define MENUITEM_KEYDEFINE 5
#define MENUITEM_GOTO_MAINMENU 6
#define MENUITEM_QUITFE 7
#define MENUITEM_LEVTILE_BOARD 9
#define MENUITEM_GAME_BOARD 10
#define MENUITEM_EDITLEVEL_BOARD 11
#define MENUITEM_PALETTE_BLOCK 12
#define MENUITEM_CREATE_LEVELPACK 13
#define MENUITEM_GET_STRING 14
#define MENUITEM_SCRATCH_PAD 15
#define MENUITEM_GOTO_CHARED_MENU 16
#define MENUITEM_TOOL_BOX 17
#define MENUITEM_CHAR_ANIM_TABLE 18
#define MENUITEM_ADD_ANIMTABLE_FRAME 19
#define MENUITEM_SUB_ANIMTABLE_FRAME 20
#define MENUITEM_CHAR_ANIMATION 21
#define MENUITEM_GET_STRING_CHARSAVE 22
#define MENUITEM_LOAD_CHARFILE 23
#define MENUITEM_GET_STRING_PICID 24
#define MENUITEM_GOTO_ROOTMENU 50
#define MENUITEM_GOTO_ATTRIB_SUBMENU 25
#define MENUITEM_GET_STRING_QUOTE 26
#define MENUITEM_GET_STRING_CHARNAME 27
#define MENUITEM_TEEOFF 30
#define MENUITEM_LOAD_COURSE 31
#define MENUITEM_CHARNAME 32
#define MENUITEM_CHARPIC_SELECT 33

#define MENU_CHANGE_ITEM_SELECTION -200
#define MENU_CHANGE_ITEMREF_SELECTION -201
#define MENU_CHANGE_TO_ROOTMENU -202
#define MENU_CHANGE_TO_SUBMENU -203
#define MENU_CHANGE_TO_LASTITEM -204
#define MENU_CHANGE_FX_SHADOWS -5
#define MENU_CHANGE_RESET_HUDS -9
#define MENU_CHANGE_SOUND_SFX_VOLUME -31
#define MENU_CHANGE_SOUND_MUSIC_VOLUME -32
#define MENU_CHANGE_SOUND_QUALITY -30
#define MENU_CHANGE_PLAYER_NAME -61
#define MENU_CHANGE_PLAYER_SELECTION -62
#define MENU_CHANGE_PLAYER_1_NAME -11
#define MENU_CHANGE_PLAYER_2_NAME -12
#define MENU_CHANGE_PLAYER_3_NAME -13
#define MENU_CHANGE_PLAYER_4_NAME -14
#define MENU_CHANGE_PLAYER_1_PIC -17
#define MENU_CHANGE_PLAYER_2_PIC -18
#define MENU_CHANGE_PLAYER_3_PIC -19
#define MENU_CHANGE_PLAYER_4_PIC -20
#define MENU_DEFINE_KEYS_PLAYER_1 -1
#define MENU_DEFINE_KEYS_PLAYER_2 -2
#define MENU_DEFINE_KEYS_PLAYER_3 -3
#define MENU_DEFINE_KEYS_PLAYER_4 -4
#define MENU_CUSTOM_KEY_OFFSET -100
#define MENU_CHANGE_KEYS -7
#define MENU_QUIT_FRONTEND -99
#define MENU_LOAD_COURSE -50
#define MENU_LOAD_COURSE_PREVIEW -51
#define MENU_LOAD_COURSE_PREVIEW_NEW -52
#define MENU_CHANGE_GAME_MODE -70
#define MENU_CHANGE_NUMBER_OF_PLAYERS -71

typedef struct{
        int x,y;                //coords for item placement
        char active;
        char selectable;
        char * text;          //text in item
        char * subtext;       //helper text
        char item_type;         //check box? slider? choose from list?
        int item_ref;           //value changed via item
        int item_ref_set;       //changed value setting
        int item_ref_end;
        char * c_ref_list[MAX_MENU_ITEM_REFS];   //list for referenced items
        int num_refs;
        }MENU_ITEM;

typedef struct CMENU{
        MENU_ITEM * menu_item[MAX_MENU_ITEMS];
        int x,y;                //coords for menu placement
        int w,h;                //dimensions of menu
        char active;            //what level of activity is this menu
        short cur_item;         //which item is currently selected
        short num_items;
        char size;              //size reference
        char label[32];
//        short root_submenu;     //tells which submenu to go back to at KEY_ESC
        short num_submenus;     //# of submenus
        struct CMENU * root_menu;      //points to the root menu for this menu
        struct CMENU * submenu[MAX_MENU_SUB_MENUS];  //the submenus
        }CMENU;

typedef struct{
        float x,y;              //coordinates
        char active;            //active?
        int offset_counter;
        int color;
        int radius;
        int cur_menu;           //current menu
        int cur_submenu;
        }MENU_CURSOR;


/* menu functions */
MENU_ITEM * create_menu_item(int x, int y, int type, char * txt, char * sub_txt, char select);
CMENU * create_menu(int w, int h, int x, int y, char * label, CMENU * root);
CMENU * copy_menu(CMENU * mn);
void destroy_menu_item(MENU_ITEM * mi);
void destroy_menu(CMENU * mn);
void set_menu_item_refs(MENU_ITEM * mi, int ref, int ref_start, int ref_end);
void set_menu_item_reflist(MENU_ITEM * mi, char * first, ...);
void menu_add_item(CMENU * mn, MENU_ITEM * mi);
void menu_add_submenu(CMENU * mn, CMENU * sub_mn);
void activate_menu(CMENU * mn);
void deactivate_menu(CMENU * mn);
void menu_get_new_key(CMENU * mn, int key_ref);
int get_menu_keys(MENU_CURSOR * mcur, CMENU ** mnp);

