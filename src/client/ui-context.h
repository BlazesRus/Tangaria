/*
 * File: ui-context.h
 * Purpose: Show player and terrain context menus
 */

#ifndef UI_CONTEXT_H
#define UI_CONTEXT_H

#ifndef DisableMouseEvents
#ifdef ENABLEFeatureExtraContextMenus
#include "cave.h"
#endif
#include "ui-input.h"
#endif

#if !defined(DisableMouseEvents)&&defined(ENABLEFeatureExtraContextMenus)
int context_menu_player(int mx, int my);
int context_menu_cave(struct chunk *c, int y, int x, int adjacent, int mx,
					  int my);
#endif
extern int context_menu_object(struct object *obj);
extern struct cmd_info *textui_action_menu_choose(void);
#ifndef DisableMouseEvents
//int context_menu_command(int mx, int my);
void textui_process_click(ui_event e);
#endif
extern void free_command_menu(void);

#endif /* UI_CONTEXT_H */
