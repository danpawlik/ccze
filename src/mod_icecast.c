/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_icecast.c -- icecast/{user,icecast}.log colorizers for CCZE
 * Copyright (C) 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
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

#include <ccze.h>
#include <string.h>
#include <stdlib.h>

#include "ccze-compat.h"

static void ccze_icecast_setup (void);
static void ccze_icecast_shutdown (void);
static int ccze_icecast_handle (const char *str, size_t length, char **rest);

static pcre *reg_icecast;
static pcre_extra *hints_icecast;

static char *
ccze_icecast_process (const char *str, int *offsets, int match)
{
  char *date = NULL, *admin = NULL, *threadno = NULL, *thread = NULL;
  char *rest = NULL;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&admin);
  pcre_get_substring (str, offsets, match, 4, (const char **)&threadno);
  pcre_get_substring (str, offsets, match, 5, (const char **)&thread);
  pcre_get_substring (str, offsets, match, 6, (const char **)&rest);
  
  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();

  if (admin && admin[0] != '\0')
    {
      ccze_addstr (CCZE_COLOR_KEYWORD, admin);
      ccze_space ();
      ccze_addstr (CCZE_COLOR_PIDB, "[");
      ccze_addstr (CCZE_COLOR_HOST, thread);
      ccze_addstr (CCZE_COLOR_PIDB, "]");
    }
  else
    {
      ccze_addstr (CCZE_COLOR_PIDB, "[");
      ccze_addstr (CCZE_COLOR_NUMBERS, threadno);
      ccze_addstr (CCZE_COLOR_DEFAULT, ":");
      ccze_addstr (CCZE_COLOR_KEYWORD, thread);
      ccze_addstr (CCZE_COLOR_PIDB, "]");
    }
  ccze_space ();

  free (date);
  free (admin);
  free (threadno);
  free (thread);
  
  return rest;
}

static void
ccze_icecast_setup (void)
{
  const char *error;
  int errptr;

  reg_icecast = pcre_compile ("^(\\[\\d+/.../\\d+:\\d+:\\d+:\\d+\\]) "
			      "(Admin)? *(\\[(\\d+)?:?([^\\]]*)\\]) (.*)$",
			  0, &error, &errptr, NULL);
  hints_icecast = pcre_study (reg_icecast, 0, &error);
}

static void
ccze_icecast_shutdown (void)
{
  free (reg_icecast);
  free (hints_icecast);
}

static int
ccze_icecast_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_icecast, hints_icecast, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_icecast_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (icecast, "icecast", FULL);
