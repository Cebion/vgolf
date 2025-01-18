#ifndef AE_UNDO_H
#define AE_UNDO_H

#define AE_MAX_UNDO                8

extern int ae_undo_count;
extern int ae_redo_count;
extern int ae_undo_last_type;
extern int ae_undo_toggle;
extern int ae_redo_toggle;

void ae_undo_reset(void);
int ae_undo_add(int type);
int ae_undo_apply(void);
void ae_redo_apply(void);

#endif
