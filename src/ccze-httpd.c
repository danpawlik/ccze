/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-httpd.c -- httpd-related colorizers for CCZE
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
#include "ccze-httpd.h"

char *
ccze_httpd_access_log_process (const char *str, int *offsets, int match)
{
  char *host, *user, *date, *full_action, *method, *http_code;
  char *gsize, *other;

  pcre_get_substring (str, offsets, match, 1, (const char **)&host);
  pcre_get_substring (str, offsets, match, 2, (const char **)&user);
  pcre_get_substring (str, offsets, match, 3, (const char **)&date);
  pcre_get_substring (str, offsets, match, 4, (const char **)&full_action);
  pcre_get_substring (str, offsets, match, 5, (const char **)&method);
  pcre_get_substring (str, offsets, match, 6, (const char **)&http_code);
  pcre_get_substring (str, offsets, match, 7, (const char **)&gsize);
  pcre_get_substring (str, offsets, match, 8, (const char **)&other);

  CCZE_ADDSTR (CCZE_COLOR_HOST, host);
  ccze_space ();
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, "-");
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_USER, user);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_DATE, date);
  ccze_space ();

  CCZE_ADDSTR (ccze_http_action (method), full_action);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_HTTPCODES, http_code);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_GETSIZE, gsize);
  ccze_space ();

  free (host);
  free (user);
  free (date);
  free (method);
  free (full_action);
  free (http_code);
  free (gsize);
  
  return other;
}

void
ccze_httpd_setup (pcre **r, pcre_extra **h)
{
  const char *error;
  int errptr;

  *r = pcre_compile
    ("^(\\S*)\\s-\\s(\\S+)\\s(\\[\\d{1,2}\\/\\S*"
     "\\/\\d{4}:\\d{2}:\\d{2}:\\d{2}.{0,6}\\])\\s"
     "(\"([A-Z]{3,})\\s[^\"]+\")\\s(\\d{3})\\s(\\d+|-)\\s(.*)$", 0,
     &error, &errptr, NULL);
  *h = pcre_study (*r, 0, &error);
}
