/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze.h -- Common macros for CCZE
 * Copyright (C) 2002 Gergely Nagy <algernon@bonehunter.rulez.org>
 *
 * This file is part of ccze.
 *
 * ccze is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ccze is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _CCZE_H
#define _CCZE_H 1

#include "ccze-color.h"
#include <curses.h>

#define CCZE_ADDSTR(col,str) { attrset (ccze_color (col)) ; addstr (str); }
#define ccze_space() CCZE_ADDSTR (CCZE_COLOR_DEFAULT, " ")
#define CCZE_NEWLINE() addstr ("\n")

int ccze_http_action (const char *method);
void ccze_print_date (const char *date);
char *ccze_strbrk (char *str, char delim);
char *xstrdup (const char *str);

#endif /* !_CCZE_H */
