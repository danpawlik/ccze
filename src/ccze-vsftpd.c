/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-vstpd.c -- VSFTPd-related colorizers for CCZE
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
#include "ccze-vsftpd.h"

char *
ccze_vsftpd_log_process (const char *str, int *offsets, int match)
{
  char *date, *sspace, *pid, *user, *other;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&sspace);
  pcre_get_substring (str, offsets, match, 3, (const char **)&pid);
  pcre_get_substring (str, offsets, match, 5, (const char **)&user);
  pcre_get_substring (str, offsets, match, 6, (const char **)&other);
  
  CCZE_ADDSTR (CCZE_COLOR_DATE, date);
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, sspace);

  CCZE_ADDSTR (CCZE_COLOR_PIDB, "[");
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, "pid ");
  CCZE_ADDSTR (CCZE_COLOR_PID, pid);
  CCZE_ADDSTR (CCZE_COLOR_PIDB, "]");
  ccze_space();

  if (*user)
    {
      CCZE_ADDSTR (CCZE_COLOR_PIDB, "[");
      CCZE_ADDSTR (CCZE_COLOR_USER, user);
      CCZE_ADDSTR (CCZE_COLOR_PIDB, "]");
      ccze_space ();
    }

  free (date);
  free (sspace);
  free (pid);
  free (user);
  
  return other;
}

void
ccze_vsftpd_setup (pcre **r, pcre_extra **h)
{
  const char *error;
  int errptr;

  *r = pcre_compile
    ("^(\\S+\\s+\\S+\\s+\\d{1,2}\\s+\\d{1,2}:\\d{1,2}:\\d{1,2}\\s+\\d+)"
     "(\\s+)\\[pid (\\d+)\\]\\s+(\\[(\\S+)\\])?\\s*(.*)$", 0, &error,
     &errptr, NULL);
  *h = pcre_study (*r, 0, &error);
}
