/*
 * This file contains misc functions for dealing with armor
 * @(#)armor.c	4.14 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"

/*
 * wear:
 *	The player wants to wear something, so let him/her put it on.
 */
void
wear()
{
    register THING *obj;
    register char *sp;

    if ((obj = get_item("身につける", ARMOR)) == NULL)
	return;
    if (cur_armor != NULL)
    {
	if (!terse)
	    msg("すでに鎧を着ている。先に脱がなければならない。");
	else
	    msg("すでに鎧を着ている。");
	after = FALSE;
	return;
    }
    if (obj->o_type != ARMOR)
    {
	msg("それは身につけられない。");
	return;
    }
    waste_time();
    obj->o_flags |= ISKNOW;
    sp = inv_name(obj, TRUE);
    cur_armor = obj;
    msg("%s を身につけた。", sp);
}

/*
 * take_off:
 *	Get the armor off of the players back
 */
void
take_off()
{
    register THING *obj;

    if ((obj = cur_armor) == NULL)
    {
	after = FALSE;
	msg("鎧を着ていない。");
	return;
    }
    if (!dropcheck(cur_armor))
	return;
    cur_armor = NULL;
    msg("%c) %s を脱いだ。", obj->o_packch, inv_name(obj, TRUE));
}

/*
 * waste_time:
 *	Do nothing but let other things happen
 */
void
waste_time()
{
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    do_daemons(AFTER);
    do_fuses(AFTER);
}
