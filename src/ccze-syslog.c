/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-syslog.c -- Syslog-related colorizers for CCZE
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
#include <string.h>
#include <stdlib.h>

#include "ccze.h"
#include "ccze-syslog.h"

char *
ccze_syslog_process (const char *str, int *offsets, int match)
{
  char *date = NULL, *host = NULL, *send = NULL, *process = NULL;
  char *msg = NULL, *pid = NULL, *tmp = NULL, *toret;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&host);
  pcre_get_substring (str, offsets, match, 3, (const char **)&send);
  
  if ((strstr (send, "last message repeated") && strstr (send, "times")) ||
      (strstr (send, "-- MARK --")))
    msg = strdup (send);
  else
    {
      pcre_get_substring (str, offsets, match, 4, (const char **)&process);
      pcre_get_substring (str, offsets, match, 5, (const char **)&msg);
    }
      
  if (process)
    {
      char *t;
      if ((t = strchr (process, '[')))
	{
	  char *t2 = strchr (t, ']');

	  pid = strndup (&t[1], (size_t)(t2 - t - 1));
	  tmp = strndup (process, (size_t)(t - process));
	  free (process);
	  process = tmp;
	}
    }

  CCZE_ADDSTR (CCZE_COLOR_DATE, date);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_HOST, host);
  ccze_space ();
  
  if (process)
    {
      CCZE_ADDSTR (CCZE_COLOR_PROC, process);
      if (pid)
	{
	  CCZE_ADDSTR (CCZE_COLOR_PIDB, "[");
	  CCZE_ADDSTR (CCZE_COLOR_PID, pid);
	  CCZE_ADDSTR (CCZE_COLOR_PIDB, "]");
	  CCZE_ADDSTR (CCZE_COLOR_PROC, ":");
	}
      ccze_space ();
      toret = strdup (msg);
    }
  else
    toret = strdup (send);

  free (date);
  free (host);
  free (send);
  free (process);
  free (msg);
  free (pid);

  return toret;
}

void
ccze_syslog_setup (pcre **r, pcre_extra **h)
{
  const char *error;
  int errptr;

  *r = pcre_compile ("^(\\S*\\s{1,2}\\d{1,2}\\s\\d\\d:\\d\\d:\\d\\d)"
		     "\\s(\\S+)\\s((\\S+:?)\\s(.*))$", 0, &error,
		     &errptr, NULL);
  *h = pcre_study (*r, 0, &error);
}
