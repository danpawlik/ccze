/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-ulogd.c -- ulogd-related colorizers for CCZE
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
#include "ccze-plugin.h"
#include "ccze-wordcolor.h"

static void ccze_ulogd_setup (void);
static void ccze_ulogd_shutdown (void);
static int ccze_ulogd_handle (const char *str, size_t length, char **rest);

static pcre *reg_ulogd, *reg_ulogd_sub;
static pcre_extra *hints_ulogd;

static char *
ccze_ulogd_process (const char *str, int *offsets, int match)
{
  char *date = NULL, *host = NULL, *chain = NULL;
  char *msg = NULL, *word, *field, *value, *tmp;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&host);
  pcre_get_substring (str, offsets, match, 4, (const char **)&chain);
  pcre_get_substring (str, offsets, match, 5, (const char **)&msg);

  CCZE_ADDSTR (CCZE_COLOR_DATE, date);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_HOST, host);
  ccze_space ();
  
  CCZE_ADDSTR (CCZE_COLOR_CHAIN, chain);
  ccze_space ();

  free (date);
  free (host);
  free (chain);

  word = xstrdup (ccze_strbrk (msg, ' '));
  do
    {
      if ((tmp = strchr (word, '=')) != NULL)
	{
	  field = strndup (word, tmp - word);
	  value = strdup (tmp + 1);
	  CCZE_ADDSTR (CCZE_COLOR_FIELD, field);
	  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, "=");
	  ccze_wordcolor_process_one (value, 1);
	  free (field);
	}
      else
	{
	  CCZE_ADDSTR (CCZE_COLOR_FIELD, word);
	  ccze_space ();
	}
    } while ((word = xstrdup (ccze_strbrk (NULL, ' '))) != NULL);
  free (msg);

  return NULL;
}

static void
ccze_ulogd_setup (void)
{
  const char *error;
  int errptr;

  reg_ulogd = pcre_compile ("^(\\S*\\s{1,2}\\d{1,2}\\s\\d\\d:\\d\\d:\\d\\d)"
			     "\\s(\\S+)\\s((\\S+:?)\\s(.*))$", 0, &error,
			     &errptr, NULL);
  hints_ulogd = pcre_study (reg_ulogd, 0, &error);
  reg_ulogd_sub = pcre_compile
    ("(IN|OUT|MAC|TTL|SRC|TOS|PREC|SPT)=", 0, &error,
     &errptr, NULL);
}

static void
ccze_ulogd_shutdown (void)
{
  free (reg_ulogd);
  free (hints_ulogd);
  free (reg_ulogd_sub);
}

static int
ccze_ulogd_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
    
  if (((match = pcre_exec (reg_ulogd, hints_ulogd, str, length,
			   0, 0, offsets, 99)) >= 0) &&
      (pcre_exec (reg_ulogd_sub, NULL, str, length, 0, 0, NULL, 0) >= 0))
    {
      *rest = ccze_ulogd_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (ulogd, "ulogd", FULL);
