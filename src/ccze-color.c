/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-color.c -- Color-handling routines for CCZE
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ccze-color.h"

#define BOLD A_BOLD |
#define BLACK COLOR_PAIR (0)
#define RED COLOR_PAIR (1)
#define GREEN COLOR_PAIR (2)
#define YELLOW COLOR_PAIR (3)
#define BLUE COLOR_PAIR (4)
#define CYAN COLOR_PAIR (5)
#define MAGENTA COLOR_PAIR (6)
#define WHITE COLOR_PAIR (7)

static int ccze_color_table [CCZE_COLOR_LAST];

typedef struct _ccze_colorname_t
{
  char *name;
  int value;
} ccze_colorname_t;
static ccze_colorname_t ccze_colorname_map[] = {
  {"black", 0},
  {"red", 1},
  {"green", 2},
  {"yellow", 3},
  {"blue", 4},
  {"cyan", 5},
  {"magenta", 6},
  {"white", 7},
  {"on_black", 0},
  {"on_red", 1},
  {"on_green", 2},
  {"on_yellow", 3},
  {"on_blue", 4},
  {"on_cyan", 5},
  {"on_magenta", 6},
  {"on_white", 7}
};

typedef struct _ccze_color_keyword_t
{
  char *keyword;
  ccze_color_t idx;
  char *comment;
} ccze_color_keyword_t;
static ccze_color_keyword_t ccze_color_keyword_map[] = {
  {"default", CCZE_COLOR_DEFAULT, "Default (not colorised)"},
  {"unknown", CCZE_COLOR_UNKNOWN, "Unknown message"},
  {"date", CCZE_COLOR_DATE, "Dates and times"},
  {"host", CCZE_COLOR_HOST, "Host names and IP numbers"},
  {"mac", CCZE_COLOR_MAC, "MAC addresses"},
  {"pid", CCZE_COLOR_PID, "PIDs (Process IDs)"},
  {"pid-sqbr", CCZE_COLOR_PIDB, "Brackets around PIDs"},
  {"get", CCZE_COLOR_HTTP_GET, "HTTP GET"},
  {"post", CCZE_COLOR_HTTP_POST, "HTTP POST"},
  {"head", CCZE_COLOR_HTTP_HEAD, "HTTP HEAD"},
  {"put", CCZE_COLOR_HTTP_PUT, "HTTP PUT"},
  {"connect", CCZE_COLOR_HTTP_CONNECT, "HTTP CONNECT"},
  {"trace", CCZE_COLOR_HTTP_TRACE, "HTTP TRACE"},
  {"httpcodes", CCZE_COLOR_HTTPCODES, "HTTP status codes (200, 404, etc)"},
  {"gettime", CCZE_COLOR_GETTIME, "Transfer times"},
  {"getsize", CCZE_COLOR_GETSIZE, "Transfer sizes"},
  {"debug", CCZE_COLOR_DEBUG, "Debug messages"},
  {"error", CCZE_COLOR_ERROR, "Error messages"},
  {"warning", CCZE_COLOR_WARNING, "Warnings"},
  {"bad", CCZE_COLOR_BADWORD, "\"Bad words\""},
  {"good", CCZE_COLOR_GOODWORD, "\"Good words\""},
  {"system", CCZE_COLOR_SYSTEMWORD, "\"System words\""},
  {"process", CCZE_COLOR_PROC, "Sender process"},
  {"dir", CCZE_COLOR_DIR, "Directory names"},
  {"prot", CCZE_COLOR_PROT, "Protocols"},
  {"service", CCZE_COLOR_SERVICE, "Services"},
  {"email", CCZE_COLOR_EMAIL, "E-mail addresses"},
  {"size", CCZE_COLOR_SIZE, "Sizes"},
  {"version", CCZE_COLOR_VERSION, "Version numbers"},
  {"address", CCZE_COLOR_ADDRESS, "Memory addresses"},
  {"uri", CCZE_COLOR_URI, "URIs (http://, ftp://, etc)"},
  {"miss", CCZE_COLOR_PROXY_MISS, "Proxy MISS"},
  {"parent", CCZE_COLOR_PROXY_PARENT, "Proxy PARENT"},
  {"direct", CCZE_COLOR_PROXY_DIRECT, "Proxy DIRECT"},
  {"hit", CCZE_COLOR_PROXY_HIT, "Proxy HIT"},
  {"deny", CCZE_COLOR_PROXY_DENIED, "Proxy DENIED"},
  {"ident", CCZE_COLOR_IDENT, "Remote user (proxy/http)"},
  {"refresh", CCZE_COLOR_PROXY_REFRESH, "Proxy REFRESH"},
  {"swapfail", CCZE_COLOR_PROXY_SWAPFAIL, "Proxy SWAPFAIL"},
  {"ctype", CCZE_COLOR_CTYPE, "Content type (http/proxy)"},
  {"create", CCZE_COLOR_PROXY_CREATE, "Proxy CREATE"},
  {"swapin", CCZE_COLOR_PROXY_SWAPIN, "Proxy SWAPIN"},
  {"swapout", CCZE_COLOR_PROXY_SWAPOUT, "Proxy SWAPOUT"},
  {"release", CCZE_COLOR_PROXY_RELEASE, "Proxy RELEASE"},
  {"swapnum", CCZE_COLOR_SWAPNUM, "Proxy swap number"},
  {"user", CCZE_COLOR_USER, "Usernames"},
  {"numbers", CCZE_COLOR_NUMBERS, "Numbers"},
  {"subject", CCZE_COLOR_SUBJECT, "Subject lines (procmail)"},
  {"signal", CCZE_COLOR_SIGNAL, "Signal names"},
  {"incoming", CCZE_COLOR_INCOMING, "Incoming mail (exim)"},
  {"outgoing", CCZE_COLOR_OUTGOING, "Outgoing mail (exim)"},
  {"uniqn", CCZE_COLOR_UNIQN, "Unique ID (exim)"},
  {"repeat", CCZE_COLOR_REPEAT, "'last message repeated N times'"},
  {"field", CCZE_COLOR_FIELD, "RFC822 Field"},
  {"chain", CCZE_COLOR_CHAIN, "Chain names (ulogd)"},
  {"percentage", CCZE_COLOR_PERCENTAGE, "Percentages"},
};

int
ccze_color (ccze_color_t idx)
{
  return ccze_color_table[idx];
}

static int
_ccze_color_keyword_lookup (const char *key)
{
  size_t i;

  for (i = 0; i < sizeof (ccze_color_keyword_map) /
	 sizeof (ccze_color_keyword_t); i++)
    if (!strcmp (key, ccze_color_keyword_map[i].keyword))
      return ccze_color_keyword_map[i].idx;
  return -1;
}

static int
_ccze_colorname_map_lookup (const char *color)
{
  size_t i;

  if (color[0] == '\'')
    {
      char *tmp = strndup (&color[1], strlen (color) - 2);
      int rval = ccze_color_table[_ccze_color_keyword_lookup (tmp)];
      free (tmp);
      return rval;
    }
  
  for (i = 0; i < sizeof (ccze_colorname_map) / sizeof (ccze_colorname_t); i++)
    if (!strcmp (color, ccze_colorname_map[i].name))
      return ccze_colorname_map[i].value;
  return -1;
}

void
ccze_color_load (const char *fn)
{
  FILE *fp;
  char *line = NULL;
  size_t len;
  struct stat stb;

  stat (fn, &stb);
  if (!S_ISREG (stb.st_mode))
    return;
  
  fp = fopen (fn, "r");
  if (!fp)
    return;
  while (getline (&line, &len, fp) != -1)
    {
      char *tmp, *keyword, *color, *pre = NULL, *bg;
      int ncolor, nkeyword, nbg, rcolor;
      
      tmp = strstr (line, "#");
      if (tmp)
	tmp[0]='\0';
      
      keyword = strtok (line, " \t\n");
      if (!keyword)
	continue;
      if ((nkeyword = _ccze_color_keyword_lookup (keyword)) == -1)
	continue;
      
      color = strtok (NULL, " \t\n");
      if (color && (!strcmp (color, "bold") || !strcmp (color, "underline") ||
		    !strcmp (color, "reverse") || !strcmp (color, "blink")))
	{
	  pre = color;
	  color = strtok (NULL, " \t\n");
	}
      if (!color)
	continue;
      if ((ncolor = _ccze_colorname_map_lookup (color)) == -1)
	continue;

      bg = strtok (NULL, " \t\n");
      if (bg)
	{
	  if ((nbg = _ccze_colorname_map_lookup (bg)) != -1)
	    ncolor += nbg*8;
	}
      
      if (color[0] == '\'')
	rcolor = ncolor;
      else
	rcolor = COLOR_PAIR (ncolor);

      if (pre)
	{
	  if (!strcmp (pre, "bold"))
	    rcolor |= A_BOLD;
	  else if (!strcmp (pre, "underline"))
	    rcolor |= A_UNDERLINE;
	  else if (!strcmp (pre, "reverse"))
	    rcolor |= A_REVERSE;
	  else if (!strcmp (pre, "blink"))
	    rcolor |= A_BLINK;
	}
      
      ccze_color_table[nkeyword] = rcolor;
    }
  free (line);
  fclose (fp);
}

void
ccze_color_init (void)
{
  ccze_color_table[CCZE_COLOR_DATE] = (BOLD CYAN);
  ccze_color_table[CCZE_COLOR_HOST] = (BOLD BLUE);
  ccze_color_table[CCZE_COLOR_PROC] = (GREEN);
  ccze_color_table[CCZE_COLOR_PID] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PIDB] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_DEFAULT] = (CYAN);
  ccze_color_table[CCZE_COLOR_EMAIL] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_SUBJECT] = (MAGENTA);
  ccze_color_table[CCZE_COLOR_DIR] = (BOLD CYAN);
  ccze_color_table[CCZE_COLOR_SIZE] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_USER] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_HTTPCODES] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_GETSIZE] = (MAGENTA);
  ccze_color_table[CCZE_COLOR_HTTP_GET] = (GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_POST] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_HEAD] = (GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_PUT] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_CONNECT] = (GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_TRACE] = (GREEN);
  ccze_color_table[CCZE_COLOR_UNKNOWN] = ccze_color_table[CCZE_COLOR_DEFAULT];
  ccze_color_table[CCZE_COLOR_GETTIME] = (BOLD MAGENTA);
  ccze_color_table[CCZE_COLOR_URI] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_IDENT] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_CTYPE] = (WHITE);
  ccze_color_table[CCZE_COLOR_ERROR] = (BOLD RED);
  ccze_color_table[CCZE_COLOR_PROXY_MISS] = (RED);
  ccze_color_table[CCZE_COLOR_PROXY_HIT] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_PROXY_DENIED] = (BOLD RED);
  ccze_color_table[CCZE_COLOR_PROXY_REFRESH] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPFAIL] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_DEBUG] = (WHITE);
  ccze_color_table[CCZE_COLOR_WARNING] = (RED);
  ccze_color_table[CCZE_COLOR_PROXY_DIRECT] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_PARENT] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_SWAPNUM] = COLOR_PAIR (4 + 7*8);
  ccze_color_table[CCZE_COLOR_PROXY_CREATE] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPIN] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPOUT] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_RELEASE] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_MAC] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_VERSION] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_ADDRESS] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_NUMBERS] = (WHITE);
  ccze_color_table[CCZE_COLOR_SIGNAL] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_SERVICE] = (BOLD MAGENTA);
  ccze_color_table[CCZE_COLOR_PROT] = (MAGENTA);
  ccze_color_table[CCZE_COLOR_BADWORD] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_GOODWORD] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_SYSTEMWORD] = (BOLD CYAN);
  ccze_color_table[CCZE_COLOR_INCOMING] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_OUTGOING] = (WHITE);
  ccze_color_table[CCZE_COLOR_UNIQN] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_REPEAT] = (WHITE);
  ccze_color_table[CCZE_COLOR_FIELD] = (GREEN);
  ccze_color_table[CCZE_COLOR_CHAIN] = (CYAN);
  ccze_color_table[CCZE_COLOR_PERCENTAGE] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_LAST] = (CYAN);
}
