/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-squid.c -- Squid-related colorizers for CCZE
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
#include "ccze-squid.h"

static int
_ccze_proxy_action (const char *action)
{
  if (strstr (action, "ERR") == action)
    return CCZE_COLOR_ERROR;
  else if (strstr (action, "MISS"))
    return CCZE_COLOR_PROXY_MISS;
  else if (strstr (action, "HIT"))
    return CCZE_COLOR_PROXY_HIT;
  else if (strstr (action, "DENIED"))
    return CCZE_COLOR_PROXY_DENIED;
  else if (strstr (action, "REFRESH"))
    return CCZE_COLOR_PROXY_REFRESH;
  else if (strstr (action, "SWAPFAIL"))
    return CCZE_COLOR_PROXY_SWAPFAIL;
  else if (strstr (action, "NONE"))
    return CCZE_COLOR_DEBUG;
  else
    return CCZE_COLOR_UNKNOWN;
}

static int
_ccze_proxy_hierarch (const char *hierar)
{
  if (strstr (hierar, "NO") == hierar)
    return CCZE_COLOR_WARNING;
  else if (strstr (hierar, "DIRECT"))
    return CCZE_COLOR_PROXY_DIRECT;
  else if (strstr (hierar, "PARENT"))
    return CCZE_COLOR_PROXY_PARENT;
  else if (strstr (hierar, "MISS"))
    return CCZE_COLOR_PROXY_MISS;
  else
    return CCZE_COLOR_UNKNOWN;
}

char *
ccze_squid_access_log_process (const char *str, int *offsets, int match)
{
  char *date, *espace, *elaps, *host, *action, *httpc, *gsize;
  char *method, *uri, *ident, *hierar, *fhost, *ctype;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&espace);
  pcre_get_substring (str, offsets, match, 3, (const char **)&elaps);
  pcre_get_substring (str, offsets, match, 4, (const char **)&host);
  pcre_get_substring (str, offsets, match, 5, (const char **)&action);
  pcre_get_substring (str, offsets, match, 6, (const char **)&httpc);
  pcre_get_substring (str, offsets, match, 7, (const char **)&gsize);
  pcre_get_substring (str, offsets, match, 8, (const char **)&method);
  pcre_get_substring (str, offsets, match, 9, (const char **)&uri);
  pcre_get_substring (str, offsets, match, 10, (const char **)&ident);
  pcre_get_substring (str, offsets, match, 11, (const char **)&hierar);
  pcre_get_substring (str, offsets, match, 12, (const char **)&fhost);
  pcre_get_substring (str, offsets, match, 13, (const char **)&ctype);

  CCZE_ADDSTR (CCZE_COLOR_DATE, date);
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, espace);
  CCZE_ADDSTR (CCZE_COLOR_GETTIME, elaps);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_HOST, host);
  ccze_space ();

  CCZE_ADDSTR (_ccze_proxy_action (action), action);
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, "/");
  CCZE_ADDSTR (CCZE_COLOR_HTTPCODES, httpc);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_GETSIZE, gsize);
  ccze_space ();

  CCZE_ADDSTR (ccze_http_action (method), method);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_URI, uri);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_IDENT, ident);
  ccze_space ();

  CCZE_ADDSTR (_ccze_proxy_hierarch (hierar), hierar);
  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, "/");
  CCZE_ADDSTR (CCZE_COLOR_HOST, host);
  ccze_space ();

  CCZE_ADDSTR (CCZE_COLOR_CTYPE, ctype);

  addstr ("\n");

  free (date);
  free (espace);
  free (elaps);
  free (host);
  free (action);
  free (httpc);
  free (gsize);
  free (method);
  free (uri);
  free (ident);
  free (hierar);
  free (fhost);
  free (ctype);
  
  return NULL;
}

char *
ccze_squid_cache_log_process (const char *str, int *offsets, int match)
{
  char *date, *other;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 3, (const char **)&other);

  CCZE_ADDSTR (CCZE_COLOR_DATE, date);
  ccze_space();

  free (date);
  return other;
}

void
ccze_squid_setup (pcre **r_access, pcre **r_cache, pcre_extra **h_access,
		  pcre_extra **h_cache)
{
  const char *error;
  int errptr;

  *r_access = pcre_compile
    ("^(\\d{9,10}\\.\\d{3})(\\s+)(\\d+)\\s(\\S+)\\s(\\w+)\\/(\\d{3})"
     "\\s(\\d+)\\s(\\w+)\\s(\\S+)\\s(\\S+)\\s(\\w+)\\/(\\S+)\\s(\\S*)$",
     0, &error, &errptr, NULL);
  *h_access = pcre_study (*r_access, 0, &error);

  *r_cache = pcre_compile
    ("^(\\d{4}\\/\\d{2}\\/\\d{2}\\s(\\d{2}:){2}\\d{2}\\|)\\s(.*)$", 0,
     &error, &errptr, NULL);
  *h_cache = pcre_study (*r_cache, 0, &error);
}
