/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-sulog.c -- su-related colorizers for CCZE
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

#include <curses.h>
#include <pcre.h>
#include <stdlib.h>

#include "ccze.h"
#include "ccze-sulog.h"

char *
ccze_sulog_process (const char *str, int *offsets, int match)
{
  char *date, *islogin, *tty, *fromuser, *touser;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&islogin);
  pcre_get_substring (str, offsets, match, 3, (const char **)&tty);
  pcre_get_substring (str, offsets, match, 4, (const char **)&fromuser);
  pcre_get_substring (str, offsets, match, 5, (const char **)&touser);

  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, "SU ");
  CCZE_ADDSTR (CCZE_COLOR_DATE, date);
  ccze_space ();
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, islogin);
  ccze_space ();
  switch (tty[0])
    {
    case '?':
      CCZE_ADDSTR (CCZE_COLOR_UNKNOWN, tty);
      break;
    default:
      CCZE_ADDSTR (CCZE_COLOR_DIR, tty);
      break;
    }
  ccze_space ();
  CCZE_ADDSTR (CCZE_COLOR_USER, fromuser);
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, "-");
  CCZE_ADDSTR (CCZE_COLOR_USER, touser);
  
  CCZE_NEWLINE ();

  free (date);
  free (tty);
  free (fromuser);
  free (touser);

  return NULL;
}

void
ccze_sulog_setup (pcre **r, pcre_extra **h)
{
  const char *error;
  int errptr;

  *r = pcre_compile ("^SU (\\d{2}\\/\\d{2} \\d{2}:\\d{2}) ([\\+\\-]) (\\S+) "
		     "([^\\-]+)-(.*)$",
		     0, &error, &errptr, NULL);
  *h = pcre_study (*r, 0, &error);
}
