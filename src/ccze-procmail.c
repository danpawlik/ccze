/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-procmail.c -- procmail-related colorizers for CCZE
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
#include "ccze-procmail.h"

char *
ccze_procmail_process (const char *str, int *offsets, int match)
{
  char *header = NULL, *value = NULL, *space1 = NULL;
  char *space2 = NULL, *extra = NULL;
  int handled = 0;
  int col = 0;

  pcre_get_substring (str, offsets, match, 1, (const char **)&space1);
  pcre_get_substring (str, offsets, match, 2, (const char **)&header);
  pcre_get_substring (str, offsets, match, 3, (const char **)&value);
  pcre_get_substring (str, offsets, match, 4, (const char **)&space2);
  pcre_get_substring (str, offsets, match, 5, (const char **)&extra);
  
  if (!strcasecmp ("from", header) || !strcasecmp (">from", header))
    {
      col = CCZE_COLOR_EMAIL;
      handled = 1;
    }
  if (!strcasecmp ("subject:", header))
    {
      col = CCZE_COLOR_SUBJECT;
      handled = 1;
    }
  if (!strcasecmp ("folder:", header))
    {
      col = CCZE_COLOR_DIR;
      handled = 1;
    }

  if (!handled)
    {
      free (header);
      free (value);
      free (extra);
      return strdup (str);
    }

  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, space1);
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, header);
  ccze_space ();

  CCZE_ADDSTR (col, value);
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, space2);

  if (!strcasecmp ("folder:", header))
    attrset (CCZE_COLOR_SIZE);
  if (!strcasecmp ("from", header))
    attrset (CCZE_COLOR_DATE);

  addstr (extra);
  addstr ("\n");
  
  free (extra);
  free (header);
  free (value);

  return NULL;
}

void
ccze_procmail_setup (pcre **r, pcre_extra **h)
{
  const char *error;
  int errptr;

  *r = pcre_compile
    ("^(\\s*)(>?From|Subject:|Folder:)?\\s(\\S+)(\\s+)(.*)$", 0,
     &error, &errptr, NULL);
  *h = pcre_study (*r, 0, &error);
}
