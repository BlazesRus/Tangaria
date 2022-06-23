/*
 * File: ui-context.c
 * Purpose: Show player and terrain context menus
 *
 * Copyright (c) 2011 Brett Reid
 * Copyright (c) 2022 MAngband and PWMAngband Developers
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */


#include "c-angband.h"


#define ADD_LABEL(text, cmd, valid) \
    cmdkey = cmd_lookup_key_unktrl((cmd), mode); \
    menu_dynamic_add_label_valid(m, (text), cmdkey, (cmd), labels, (valid))


/*
 * Additional constants for menu item values in context_menu_object(). The values must not collide
 * with the cmd_code enum, since those are the main values for these menu items.
 */
enum context_menu_object_value_e
{
    MENU_VALUE_INSPECT = CMD_MAX,
    MENU_VALUE_DROP_ALL
#if !defined(DisableMouseEvents)&&defined(ENABLEFeatureExtraContextMenus)
	MENU_VALUE_LOOK,
    MENU_VALUE_SELECTCURRENTABILITY,//Use for selecting ability to use for casting on target
	//MENU_VALUE_RECALL,
	//MENU_VALUE_REST,
	//MENU_VALUE_INVENTORY,
	//MENU_VALUE_CENTER_MAP,
	//MENU_VALUE_FLOOR,
	//MENU_VALUE_CHARACTER,
	//MENU_VALUE_OTHER,
	//MENU_VALUE_KNOWLEDGE,
	//MENU_VALUE_MAP,
	//MENU_VALUE_MESSAGES,
	//MENU_VALUE_OBJECTS,
	//MENU_VALUE_MONSTERS,
	//MENU_VALUE_TOGGLE_IGNORED,
	//MENU_VALUE_OPTIONS,
	//MENU_VALUE_HELP,
#endif
};


static int cmd_execute(int selected, struct object *obj)
{
    struct command cmd;

    memset(&cmd, 0, sizeof(cmd));
    cmd.code = (cmd_code)selected;

    switch (selected)
    {
        /* Wield the item */
        case CMD_WIELD:
        {
            cmd_set_arg_item(&cmd, "item", obj);
            return Send_wield(&cmd);
        }

        /* Take the item off */
        case CMD_TAKEOFF:
        {
            cmd_set_arg_item(&cmd, "item", obj);
            return Send_take_off(&cmd);
        }

        /* Inscribe the item */
        case CMD_INSCRIBE:
        {
            cmd_set_arg_item(&cmd, "item", obj);
            return Send_inscribe(&cmd);
        }

        /* Uninscribe the item */
        case CMD_UNINSCRIBE:
        {
            cmd_set_arg_item(&cmd, "item", obj);
            return Send_uninscribe(&cmd);
        }

        /* Pick the item up */
        case CMD_PICKUP:
        {
            cmd_set_arg_number(&cmd, "number", 1);
            cmd_set_arg_item(&cmd, "item", obj);
            Send_pickup(&cmd);
            break;
        }

        /* Drop the item */
        case CMD_DROP:
        {
            cmd_set_arg_item(&cmd, "item", obj);
            return Send_drop(&cmd);
        }

        /* Use the item to refill a light source */
        case CMD_REFILL:
        {
            if (!obj_refill_pre()) return 0;
            cmd_set_arg_item(&cmd, "item", obj);
            return Send_fill(&cmd);
        }

        /* Throw the item */
        case CMD_THROW:
        {
            cmd_set_arg_item(&cmd, "item", obj);
            return Send_throw(&cmd);
        }

        /* Use the item */
        case CMD_USE_WAND:
        case CMD_USE_ROD:
        case CMD_USE_STAFF:
        case CMD_READ_SCROLL:
        case CMD_QUAFF:
        case CMD_EAT:
        case CMD_ACTIVATE:
        case CMD_FIRE:
        case CMD_USE:
        {
            cmd_set_arg_item(&cmd, "item", obj);
            return Send_use_any(&cmd);
        }
    }

    return 1;
}


static bool object_is_carried(const struct object *obj)
{
    int i, size = z_info->pack_size + player->body.count + z_info->quiver_size;

    for (i = 0; i < size; i++)
    {
        struct object *pile_obj = &player->gear[i];

        if (obj == pile_obj) return true;
    }

    return false;
}

#if !defined(DisableMouseEvents)&&defined(ENABLEFeatureExtraContextMenus)
int context_menu_player(int mx, int my)
{//Editing to only enable partial context menu options(when user right clicks?)
	struct menu *m;
	int selected;
	char *labels;
	bool allowed = true;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;
	struct object *obj;

	m = menu_dynamic_new();
	if (!m) {
		return 0;
	}

	labels = string_make(lower_case);
	m->selections = labels;

	ADD_LABEL("Use", CMD_USE, MN_ROW_VALID);

	/* if player can cast, add casting option */
	if (player_can_cast(player, false)) {
		ADD_LABEL("Cast", CMD_CAST, MN_ROW_VALID);
	}

	/* if player is on stairs add option to use them */
	if (square_isupstairs(cave, player->grid)) {
		ADD_LABEL("Go Up", CMD_GO_UP, MN_ROW_VALID);
	}
	else if (square_isdownstairs(cave, player->grid)) {
		ADD_LABEL("Go Down", CMD_GO_DOWN, MN_ROW_VALID);
	}

	/* Looking has different keys, but we don't have a way to look them up
	 * (see ui-game.c). */
	cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'l' : 'x';
	menu_dynamic_add_label(m, "Look", cmdkey, MENU_VALUE_LOOK, labels);

	///* 'R' is used for resting in both keymaps. */
	//menu_dynamic_add_label(m, "Rest", 'R', MENU_VALUE_REST, labels);

	/* 'i' is used for inventory in both keymaps. */
	menu_dynamic_add_label(m, "Inventory", 'i', MENU_VALUE_INVENTORY, labels);

	/* if object under player add pickup option */
	obj = square_object(cave, player->grid);
	if (obj && !ignore_item_ok(player, obj)) {
			menu_row_validity_t valid;

			/* 'f' isn't in rogue keymap, so we can use it here. */
  			menu_dynamic_add_label(m, "Floor", 'f', MENU_VALUE_FLOOR, labels);
			valid = (inven_carry_okay(obj)) ? MN_ROW_VALID : MN_ROW_INVALID;
			ADD_LABEL("Pick up", CMD_PICKUP, valid);
	}

	///* 'C' is used for the character sheet in both keymaps. */
	//menu_dynamic_add_label(m, "Character", 'C', MENU_VALUE_CHARACTER, labels);

	//if (!OPT(player, center_player)) {
	//	menu_dynamic_add_label(m, "^Center Map", 'L', MENU_VALUE_CENTER_MAP,
	//						   labels);
	//}

	menu_dynamic_add_label(m, "Other", ' ', MENU_VALUE_OTHER, labels);

	/* Hack -- no flush needed */
	msg_flag = false;
	screen_save();

	menu_dynamic_calc_location(m, mx, my);
	region_erase_bordered(&m->boundary);

	prt("(Enter to select, ESC) Command:", 0, 0);
	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	cmdkey = cmd_lookup_key(selected, mode);

	/* Check the command to see if it is allowed. */
	switch(selected) {
		case -1:
			/* User cancelled the menu. */
			return 3;

		case CMD_USE:
		case CMD_CAST:
		case CMD_GO_UP:
		case CMD_GO_DOWN:
		case CMD_PICKUP:
			/* Only check for ^ inscriptions, since we don't have an object
			 * selected (if we need one). */
			allowed = key_confirm_command(cmdkey);
			break;

		//case MENU_VALUE_REST:
		//	allowed = key_confirm_command('R');
		//	break;

		//case MENU_VALUE_INVENTORY:
		case MENU_VALUE_LOOK:
		//case MENU_VALUE_CHARACTER:
		case MENU_VALUE_OTHER:
		case MENU_VALUE_FLOOR:
		//case MENU_VALUE_CENTER_MAP:
			allowed = true;
			break;

		default:
			/* Invalid command; prevent anything from happening. */
			bell();
			allowed = false;
			break;
	}

	if (!allowed)
		return 1;

	/* Perform the command. */
	switch(selected) {
		case CMD_USE:
		case CMD_CAST:
			cmdkey = cmd_lookup_key(selected, mode);
			Term_keypress(cmdkey, 0);
			break;//Cast only saved spell instead

		case CMD_GO_UP:
		case CMD_GO_DOWN:
		case CMD_PICKUP:
			cmdq_push(selected);
			break;

		//case MENU_VALUE_REST:
		//	Term_keypress('R', 0);
		//	break;

		//case MENU_VALUE_INVENTORY:
		//	Term_keypress('i', 0);
		//	break;

		case MENU_VALUE_LOOK:
			if (target_set_interactive(TARGET_LOOK, player->grid.x, player->grid.y))
				msg("Target Selected.");
			break;

		//case MENU_VALUE_CHARACTER:
		//	Term_keypress('C', 0);
		//	break;

		case MENU_VALUE_OTHER:
			context_menu_player_2(mx, my);
			break;

		case MENU_VALUE_FLOOR:
			context_menu_player_display_floor();
			break;

		//case MENU_VALUE_CENTER_MAP:
		//	do_cmd_center_map();
		//	break;

		default:
			break;
	}

	return 1;
}

int context_menu_cave(struct chunk *c, int y, int x, int adjacent, int mx,
					  int my)
{
	struct menu *m;
	int selected;
	char *labels;
	bool allowed = true;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	unsigned char cmdkey;
	struct loc grid = loc(x, y);
	struct object *square_obj = square_object(c, grid);

	m = menu_dynamic_new();
	if (!m)
		return 0;

	labels = string_make(lower_case);
	m->selections = labels;

	/* Looking has different keys, but we don't have a way to look them up
	 * (see ui-game.c). */
	cmdkey = (mode == KEYMAP_MODE_ORIG) ? 'l' : 'x';
	menu_dynamic_add_label(m, "Look At", cmdkey, MENU_VALUE_LOOK, labels);

	if (square(c, grid)->mon)
		/* '/' is used for recall in both keymaps. */
		menu_dynamic_add_label(m, "Recall Info", '/', MENU_VALUE_RECALL,
							   labels);

	ADD_LABEL("Use Item On", CMD_USE, MN_ROW_VALID);

	if (player_can_cast(player, false))
		ADD_LABEL("Cast On", CMD_CAST, MN_ROW_VALID);

	if (adjacent) {
		struct object *obj = chest_check(player, grid, CHEST_ANY);
		ADD_LABEL((square(c, grid)->mon) ? "Attack" : "Alter", CMD_ALTER,
				  MN_ROW_VALID);

		if (obj && !ignore_item_ok(player, obj)) {
			if (obj->known->pval) {
				if (is_locked_chest(obj)) {
					ADD_LABEL("Disarm Chest", CMD_DISARM, MN_ROW_VALID);
					ADD_LABEL("Open Chest", CMD_OPEN, MN_ROW_VALID);
				} else {
					ADD_LABEL("Open Disarmed Chest", CMD_OPEN, MN_ROW_VALID);
				}
			} else {
				ADD_LABEL("Open Chest", CMD_OPEN, MN_ROW_VALID);
			}
		}

		if ((square(c, grid)->mon > 0) && player_has(player, PF_STEAL)) {
			ADD_LABEL("Steal", CMD_STEAL, MN_ROW_VALID);
		}

		if (square_isdisarmabletrap(c, grid)) {
			ADD_LABEL("Disarm", CMD_DISARM, MN_ROW_VALID);
			ADD_LABEL("Jump Onto", CMD_JUMP, MN_ROW_VALID);
		}

		if (square_isopendoor(c, grid)) {
			ADD_LABEL("Close", CMD_CLOSE, MN_ROW_VALID);
		}
		else if (square_iscloseddoor(c, grid)) {
			ADD_LABEL("Open", CMD_OPEN, MN_ROW_VALID);
			ADD_LABEL("Lock", CMD_DISARM, MN_ROW_VALID);
		}
		else if (square_isdiggable(c, grid)) {
			ADD_LABEL("Tunnel", CMD_TUNNEL, MN_ROW_VALID);
		}

		ADD_LABEL("Walk Towards", CMD_WALK, MN_ROW_VALID);
	} else {
		/* ',' is used for ignore in rogue keymap, so we'll just swap letters */
		cmdkey = (mode == KEYMAP_MODE_ORIG) ? ',' : '.';
		menu_dynamic_add_label(m, "Pathfind To", cmdkey, CMD_PATHFIND, labels);

		ADD_LABEL("Walk Towards", CMD_WALK, MN_ROW_VALID);
		ADD_LABEL("Run Towards", CMD_RUN, MN_ROW_VALID);
	}

	if (player_can_fire(player, false)) {
		ADD_LABEL("Fire On", CMD_FIRE, MN_ROW_VALID);
	}

	ADD_LABEL("Throw To", CMD_THROW, MN_ROW_VALID);

	/* Hack -- no flush needed */
	msg_flag = false;
	screen_save();

	menu_dynamic_calc_location(m, mx, my);
	region_erase_bordered(&m->boundary);

	if (player->timed[TMD_IMAGE]) {
		prt("(Enter to select command, ESC to cancel) You see something strange:", 0, 0);
	} else if (square(c, grid)->mon) {
		char m_name[80];
		struct monster *mon = square_monster(c, grid);

		/* Get the monster name ("a kobold") */
		monster_desc(m_name, sizeof(m_name), mon, MDESC_IND_VIS);

		prt(format("(Enter to select command, ESC to cancel) You see %s:",
				   m_name), 0, 0);
	} else if (square_obj && !ignore_item_ok(player, square_obj)) {
		char o_name[80];

		/* Obtain an object description */
		object_desc(o_name, sizeof (o_name), square_obj,
			ODESC_PREFIX | ODESC_FULL, player);

		prt(format("(Enter to select command, ESC to cancel) You see %s:",
				   o_name), 0, 0);
	} else {
		/* Feature (apply mimic) */
		const char *name = square_apparent_name(player->cave, grid);
		const char *prefix = square_apparent_look_prefix(player->cave, grid);

		prt(format("(Enter to select command, ESC to cancel) You see %s%s:", prefix, name), 0, 0);
	}

	selected = menu_dynamic_select(m);

	menu_dynamic_free(m);
	string_free(labels);

	screen_load();

	cmdkey = cmd_lookup_key(selected, mode);

	/* Check the command to see if it is allowed. */
	switch (selected) {
		case -1:
			/* User cancelled the menu. */
			return 3;

		case MENU_VALUE_LOOK:
		case MENU_VALUE_RECALL:
		case CMD_PATHFIND:
			allowed = true;
			break;

		case CMD_ALTER:
		case CMD_STEAL:
		case CMD_DISARM:
		case CMD_JUMP:
		case CMD_CLOSE:
		case CMD_OPEN:
		case CMD_TUNNEL:
		case CMD_WALK:
		case CMD_RUN:
		case CMD_CAST:
		case CMD_FIRE:
		case CMD_THROW:
		case CMD_USE:
			/* Only check for ^ inscriptions, since we don't have an object
			 * selected (if we need one). */
			allowed = key_confirm_command(cmdkey);
			break;

		default:
			/* Invalid command; prevent anything from happening. */
			bell();
			allowed = false;
			break;
	}

	if (!allowed)
		return 1;

	/* Perform the command. */
	switch (selected) {
		case MENU_VALUE_LOOK:
			/* Look at the spot */
			if (target_set_interactive(TARGET_LOOK, x, y)) {
				msg("Target Selected.");
			}
			break;

		case MENU_VALUE_RECALL: {
			/* Recall monster Info */
			struct monster *mon = square_monster(c, grid);
			if (mon) {
				struct monster_lore *lore = get_lore(mon->race);
				lore_show_interactive(mon->race, lore);
			}
		}
			break;

		case CMD_PATHFIND:
			cmdq_push(selected);
			cmd_set_arg_point(cmdq_peek(), "point", loc(x, y));
			break;

		case CMD_ALTER:
		case CMD_STEAL:
		case CMD_DISARM:
		case CMD_JUMP:
		case CMD_CLOSE:
		case CMD_OPEN:
		case CMD_TUNNEL:
		case CMD_WALK:
		case CMD_RUN:
			cmdq_push(selected);
			cmd_set_arg_direction(cmdq_peek(), "direction",
								  motion_dir(player->grid, loc(x, y)));
			break;

		case CMD_CAST:
		case CMD_FIRE:
		case CMD_THROW:
		case CMD_USE:
			cmdq_push(selected);
			cmd_set_arg_target(cmdq_peek(), "target", DIR_TARGET);
			break;

		default:
			break;
	}

	return 1;
}
#endif

/*
 * Pick the context menu options appropriate for the item
 */
int context_menu_object(struct object *obj)
{
    struct menu *m;
    int selected;
    char *labels;
    char header[120];
    bool allowed = true;
    int mode = (OPT(player, rogue_like_commands)? KEYMAP_MODE_ROGUE: KEYMAP_MODE_ORIG);
    unsigned char cmdkey;

    m = menu_dynamic_new();
    if (!m || !obj) return 0;

    strnfmt(header, sizeof(header), "(Enter to select, ESC) Command for %s:", obj->info_xtra.name);

    labels = string_make(lower_case);
    m->selections = labels;

    /* 'I' is used for inspect in both keymaps. */
    menu_dynamic_add_label(m, "Inspect", 'I', MENU_VALUE_INSPECT, labels);

    if (obj_can_browse(player, obj))
    {
        if (obj_can_cast_from(player, obj))
        {
            ADD_LABEL("Cast", CMD_CAST, MN_ROW_VALID);
        }
        if (obj_can_study(player, obj))
        {
            ADD_LABEL("Study", CMD_STUDY, MN_ROW_VALID);
        }
        ADD_LABEL("Browse", CMD_BROWSE_SPELL, MN_ROW_VALID);
    }
    else if (obj_is_useable(player, obj))
    {
        if (tval_is_wand(obj))
        {
            menu_row_validity_t valid = (obj_has_charges(obj)? MN_ROW_VALID: MN_ROW_INVALID);

            ADD_LABEL("Aim", CMD_USE_WAND, valid);
        }
        else if (tval_is_rod(obj))
        {
            menu_row_validity_t valid = (obj_can_zap(obj)? MN_ROW_VALID: MN_ROW_INVALID);

            ADD_LABEL("Zap", CMD_USE_ROD, valid);
        }
        else if (tval_is_staff(obj))
        {
            menu_row_validity_t valid = (obj_has_charges(obj)? MN_ROW_VALID: MN_ROW_INVALID);

            ADD_LABEL("Use", CMD_USE_STAFF, valid);
        }
        else if (tval_is_scroll(obj))
        {
            ADD_LABEL("Read", CMD_READ_SCROLL, MN_ROW_VALID);
        }
        else if (tval_is_potion(obj))
        {
            ADD_LABEL("Quaff", CMD_QUAFF, MN_ROW_VALID);
        }
        else if (tval_is_edible(obj))
        {
            ADD_LABEL("Eat", CMD_EAT, MN_ROW_VALID);
        }
        else if (obj_is_activatable(player, obj))
        {
            menu_row_validity_t valid = ((object_is_equipped(player->body, obj) &&
                obj_can_activate(obj))? MN_ROW_VALID: MN_ROW_INVALID);

            ADD_LABEL("Activate", CMD_ACTIVATE, valid);
        }
        else if (item_tester_hook_fire(player, obj))
        {
            ADD_LABEL("Fire", CMD_FIRE, MN_ROW_VALID);
        }
        else
        {
            ADD_LABEL("Use", CMD_USE, MN_ROW_VALID);
        }
    }
    if (obj_can_refill(player, obj))
    {
        ADD_LABEL("Refill", CMD_REFILL, MN_ROW_VALID);
    }
    if (object_is_equipped(player->body, obj) && obj_can_takeoff(player, obj))
    {
        ADD_LABEL("Take off", CMD_TAKEOFF, MN_ROW_VALID);
    }
    else if (!object_is_equipped(player->body, obj) && obj_can_wear(player, obj))
    {
        ADD_LABEL("Equip", CMD_WIELD, MN_ROW_VALID);
    }
    if (object_is_carried(obj))
    {
        ADD_LABEL("Drop", CMD_DROP, MN_ROW_VALID);
        if (obj->number > 1)
        {
            /* 'D' is used for ignore in rogue keymap, so we'll just swap letters. */
            cmdkey = ((mode == KEYMAP_MODE_ORIG)? 'D': 'k');
            menu_dynamic_add_label(m, "Drop All", cmdkey, MENU_VALUE_DROP_ALL, labels);
        }
    }
    else
    {
        menu_row_validity_t valid = (inven_carry_okay(obj)? MN_ROW_VALID: MN_ROW_INVALID);

        ADD_LABEL("Pick up", CMD_PICKUP, valid);
    }
    ADD_LABEL("Throw", CMD_THROW, MN_ROW_VALID);
    ADD_LABEL("Inscribe", CMD_INSCRIBE, MN_ROW_VALID);
    if (obj_has_inscrip(player, obj))
    {
        ADD_LABEL("Uninscribe", CMD_UNINSCRIBE, MN_ROW_VALID);
    }
    if (!obj->ignore_protect)
    {
        ADD_LABEL((obj->info_xtra.ignored? "Unignore": "Ignore"), CMD_IGNORE, MN_ROW_VALID);
    }

    menu_dynamic_calc_location(m);

    prt(header, 0, 0);
    selected = menu_dynamic_select(m);

    menu_dynamic_free(m);
    string_free(labels);

    screen_load(false);

    switch (selected)
    {
        case -1:
            /* User cancelled the menu. */
            return 3;

        case MENU_VALUE_INSPECT:
        {
            struct command cmd;

            memset(&cmd, 0, sizeof(cmd));
            cmd.code = CMD_EXAMINE;

            /* Inspect the item */
            cmd_set_arg_item(&cmd, "item", obj);
            Send_observe(&cmd);
            return 1;
        }

        case MENU_VALUE_DROP_ALL:
        {
            struct command cmd;

            memset(&cmd, 0, sizeof(cmd));
            cmd.code = CMD_DROP;

            /* Drop entire stack without confirmation. */
            cmd_set_arg_item(&cmd, "item", obj);
            cmd_set_arg_number(&cmd, "quantity", obj->number);
            Send_drop(&cmd);
            return 1;
        }

        case CMD_BROWSE_SPELL:
        case CMD_STUDY:
        case CMD_CAST:
        case CMD_IGNORE:
        case CMD_WIELD:
        case CMD_TAKEOFF:
        case CMD_INSCRIBE:
        case CMD_UNINSCRIBE:
        case CMD_PICKUP:
        case CMD_DROP:
        case CMD_REFILL:
        case CMD_THROW:
        case CMD_USE_WAND:
        case CMD_USE_ROD:
        case CMD_USE_STAFF:
        case CMD_READ_SCROLL:
        case CMD_QUAFF:
        case CMD_EAT:
        case CMD_ACTIVATE:
        case CMD_FIRE:
        case CMD_USE:
            /* PWMAngband: inscriptions are checked on the server */
            break;
        default:
        {
            /* Invalid command; prevent anything from happening. */
            bell("Invalid context menu command.");
            allowed = false;
            break;
        }
    }

    if (!allowed) return 1;

    if (selected == CMD_IGNORE)
    {
        /* Ignore or unignore the item */
        textui_cmd_ignore_menu(obj);
    }
    else if (selected == CMD_BROWSE_SPELL)
    {
        /* Browse a spellbook */
        if (!obj_browse_pre()) return 0;
        Send_track_object(obj->oidx);
        textui_book_browse(obj->info_xtra.bidx);
        return 2;
    }
    else if (selected == CMD_STUDY)
    {
        struct command cmd;

        memset(&cmd, 0, sizeof(cmd));
        cmd.code = CMD_STUDY;

        /* Study a spellbook */
        if (!obj_study_pre()) return 0;
        cmd_set_arg_item(&cmd, "item", obj);
        return Send_gain(&cmd);
    }
    else if (selected == CMD_CAST)
    {
        /* Cast a spell from the book */
        if (obj_can_browse(player, obj))
        {
            struct command cmd;

            memset(&cmd, 0, sizeof(cmd));
            cmd.code = CMD_CAST;

            if (!obj_cast_pre()) return 0;
            cmd_set_arg_item(&cmd, "item", obj);
            cmd_cast(&cmd);
        }
    }
    else
    {
        /* PWMAngband: replaces cmd_insert + cmd_set_arg_item */
        return cmd_execute(selected, obj);
    }

    return 1;
}

#ifndef DisableMouseEvents
/**
 * Handle a textui mouseclick.
 */
void textui_process_click(ui_event e)
{
	int x, y;

	if (!OPT(player, mouse_movement)) return;

	y = KEY_GRID_Y(e);
	x = KEY_GRID_X(e);

	/* Check for a valid location */
	if (!square_in_bounds_fully(cave, loc(x, y))) return;

	/* XXX show context menu here */
	if (loc_eq(player->grid, loc(x, y))) {
		if (e.mouse.mods & KC_MOD_SHIFT) {
			/* shift-click - cast magic */
			if (e.mouse.button == 1) {
				cmdq_push(CMD_CAST);
			} else if (e.mouse.button == 2) {
				Term_keypress('i',0);
			}
		} else if (e.mouse.mods & KC_MOD_CONTROL) {
			/* ctrl-click - use feature / use inventory item */
			/* switch with default */
			if (e.mouse.button == 1) {
				if (square_isupstairs(cave, player->grid))
					cmdq_push(CMD_GO_UP);
				else if (square_isdownstairs(cave, player->grid))
					cmdq_push(CMD_GO_DOWN);
			} else if (e.mouse.button == 2) {
				cmdq_push(CMD_USE);
			}
		} else if (e.mouse.mods & KC_MOD_ALT) {
			/* alt-click - show char screen */
			/* XXX call a platform specific hook */
			if (e.mouse.button == 1) {
				Term_keypress('C',0);
			}
		} else {
			if (e.mouse.button == 1) {
				if (square_object(cave, loc(x, y))) {
					cmdq_push(CMD_PICKUP);
				} else {
					cmdq_push(CMD_HOLD);
				}
			} else if (e.mouse.button == 2) {
				/* Show a context menu */
				context_menu_player(e.mouse.x, e.mouse.y);
			}
		}
	} else if (e.mouse.button == 1) {
		if (player->timed[TMD_CONFUSED]) {
			cmdq_push(CMD_WALK);
		} else {
			if (e.mouse.mods & KC_MOD_SHIFT) {
				/* shift-click - run */
				cmdq_push(CMD_RUN);
				cmd_set_arg_direction(cmdq_peek(), "direction",
									  motion_dir(player->grid, loc(x, y)));
			} else if (e.mouse.mods & KC_MOD_CONTROL) {
				/* control-click - alter */
				cmdq_push(CMD_ALTER);
				cmd_set_arg_direction(cmdq_peek(), "direction",
									  motion_dir(player->grid, loc(x, y)));
			} else if (e.mouse.mods & KC_MOD_ALT) {
				/* alt-click - look */
				if (target_set_interactive(TARGET_LOOK, x, y)) {
					msg("Target Selected.");
				}
			} else {
				/* Pathfind does not work well on trap detection borders,
				 * so if the click is next to the player, force a walk step */
				if ((y - player->grid.y >= -1) && (y - player->grid.y <= 1)	&&
					(x - player->grid.x >= -1) && (x - player->grid.x <= 1)) {
					cmdq_push(CMD_WALK);
					cmd_set_arg_direction(cmdq_peek(), "direction",
										  motion_dir(player->grid, loc(x, y)));
				} else {
					cmdq_push(CMD_PATHFIND);
					cmd_set_arg_point(cmdq_peek(), "point", loc(x, y));
				}
			}
		}
	} else if (e.mouse.button == 2) {
		struct monster *m = square_monster(cave, loc(x, y));
		if (m && target_able(m)) {
			/* Set up target information */
			monster_race_track(player->upkeep, m->race);
			health_track(player->upkeep, m);
			target_set_monster(m);
		} else {
			target_set_location(y, x);
		}

		if (e.mouse.mods & KC_MOD_SHIFT) {
			/* shift-click - cast spell at target */
			cmdq_push(CMD_CAST);
			cmd_set_arg_target(cmdq_peek(), "target", DIR_TARGET);
		} else if (e.mouse.mods & KC_MOD_CONTROL) {
			/* control-click - fire at target */
			cmdq_push(CMD_USE);
			cmd_set_arg_target(cmdq_peek(), "target", DIR_TARGET);
		} else if (e.mouse.mods & KC_MOD_ALT) {
			/* alt-click - throw at target */
			cmdq_push(CMD_THROW);
			cmd_set_arg_target(cmdq_peek(), "target", DIR_TARGET);
		} else {
			/* see if the click was adjacent to the player */
			if ((y - player->grid.y >= -1) && (y - player->grid.y <= 1)	&&
				(x - player->grid.x >= -1) && (x - player->grid.x <= 1)) {
				context_menu_cave(cave,y,x,1,e.mouse.x, e.mouse.y);
			} else {
				context_menu_cave(cave,y,x,0,e.mouse.x, e.mouse.y);
			}
		}
	}
}
#endif

/*
 * Menu functions
 */


/*
 * Display an entry on a command menu
 */
static void cmd_sub_entry(struct menu *menu, int oid, bool cursor, int row, int col, int width)
{
    uint8_t attr = (cursor? COLOUR_L_BLUE: COLOUR_WHITE);
    const struct cmd_info *commands = menu_priv(menu);
    int mode = (OPT(player, rogue_like_commands)? KEYMAP_MODE_ROGUE: KEYMAP_MODE_ORIG);
    struct keypress kp;
    char buf[16];

    kp.type = EVT_KBRD;
    kp.code = commands[oid].key[mode];
    kp.mods = 0;

    /* Write the description */
    Term_putstr(col, row, -1, attr, commands[oid].desc);

    /* Include keypress */
    Term_addch(attr, ' ');
    Term_addch(attr, '(');

    /* Get readable version */
    keypress_to_readable(buf, sizeof(buf), kp);
    Term_addstr(-1, attr, buf);

    Term_addch(attr, ')');
}


/*
 * Display a list of commands.
 */
static bool cmd_menu(struct command_list *list, void *selection_p)
{
    struct menu menu;
    menu_iter commands_menu = {NULL, NULL, cmd_sub_entry, NULL, NULL};
    region area = {23, 4, 37, 13};
    ui_event evt;
    struct cmd_info **selection = selection_p;
    ui_event ea = EVENT_ABORT;

    /* Set up the menu */
    menu_init(&menu, MN_SKIN_SCROLL, &commands_menu);
    menu_setpriv(&menu, list->len, list->list);
    menu_layout(&menu, &area);

    /* Set up the screen */
    screen_save();
    window_make(21, 3, 62, 17);

    /* Select an entry */
    evt = menu_select(&menu, 0, true);

    /* Load the screen */
    screen_load(false);

    if (evt.type == EVT_SELECT) *selection = &list->list[menu.cursor];
    else if (is_abort(evt)) Term_event_push(&ea);

    return false;
}


static bool cmd_list_action(struct menu *m, const ui_event *event, int oid)
{
    if (event->type == EVT_SELECT)
        return cmd_menu(&cmds_all[oid], menu_priv(m));
    return false;
}


static void cmd_list_entry(struct menu *menu, int oid, bool cursor, int row,
    int col, int width)
{
    uint8_t attr = (cursor? COLOUR_L_BLUE: COLOUR_WHITE);

    Term_putstr(col, row, -1, attr, cmds_all[oid].name);
}


static struct menu *command_menu;


static menu_iter command_menu_iter =
{
    NULL,
    NULL,
    cmd_list_entry,
    cmd_list_action,
    NULL
};


/*
 * Display a list of command types, allowing the user to select one.
 */
struct cmd_info *textui_action_menu_choose(void)
{
    region area = {21, 5, 37, 6};
    struct cmd_info *chosen_command = NULL;

    if (!command_menu) command_menu = menu_new(MN_SKIN_SCROLL, &command_menu_iter);

    menu_setpriv(command_menu, MAX_COMMAND_LIST - 1, &chosen_command);
    menu_layout(command_menu, &area);

    /* Set up the screen */
    screen_save();
    window_make(19, 4, 58, 11);

    /* Select an entry */
    menu_select(command_menu, 0, true);

    /* Load the screen */
    screen_load(true);

    return chosen_command;
}


void free_command_menu(void)
{
    mem_free(command_menu);
}
