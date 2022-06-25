/*
 * File: target-ui.h
 * Purpose: UI for targeting code
 */

#ifndef TARGET_UI_H
#define TARGET_UI_H

#ifdef EnableOld_UI_TARGET_Code//Old code from UI_target in angband(target_set_interactive in both files almost same)
/**
 * Height of the help screen; any higher than 4 will overlap the health
 * bar which we want to keep in targeting mode.
 */
#define HELP_HEIGHT 3

/**
 * Size of the array that is used for object names during targeting.
 */
#define TARGET_OUT_VAL_SIZE 256

int target_dir_allow(struct keypress ch, bool allow_5);
void textui_target(void);
void textui_target_closest(void);
#endif

//target_display_help(char *help, size_t len, bool monster, bool object, bool free);already in .c file
//int target_dir(uint32_t ch);already in .c file
extern int draw_path(struct player *p, uint16_t path_n, struct loc *path_g, struct loc *grid);
extern void load_path(struct player *p, uint16_t path_n, struct loc *path_g);
extern bool target_set_interactive(struct player *p, int mode, uint32_t press, int step);

#endif /* TARGET_UI_H */
