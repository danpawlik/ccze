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
} ccze_color_keyword_t;
static ccze_color_keyword_t ccze_color_keyword_map[] = {
  {"default", CCZE_COLOR_DEFAULT},
  {"unknown", CCZE_COLOR_UNKNOWN},
  {"date", CCZE_COLOR_DATE},
  {"host", CCZE_COLOR_HOST},
  {"mac", CCZE_COLOR_MAC},
  {"pid", CCZE_COLOR_PID},
  {"pid-sqbr", CCZE_COLOR_PIDB},
  {"get", CCZE_COLOR_HTTP_GET},
  {"post", CCZE_COLOR_HTTP_POST},
  {"head", CCZE_COLOR_HTTP_HEAD},
  {"put", CCZE_COLOR_HTTP_PUT},
  {"connect", CCZE_COLOR_HTTP_CONNECT},
  {"trace", CCZE_COLOR_HTTP_TRACE},
  {"httpcodes", CCZE_COLOR_HTTPCODES},
  {"gettime", CCZE_COLOR_GETTIME},
  {"getsize", CCZE_COLOR_GETSIZE},
  {"debug", CCZE_COLOR_DEBUG},
  {"error", CCZE_COLOR_ERROR},
  {"warning", CCZE_COLOR_WARNING},
  {"bad", CCZE_COLOR_BADWORD},
  {"good", CCZE_COLOR_GOODWORD},
  {"system", CCZE_COLOR_SYSTEMWORD},
  {"process", CCZE_COLOR_PROC},
  {"dir", CCZE_COLOR_DIR},
  {"prot", CCZE_COLOR_PROT},
  {"service", CCZE_COLOR_SERVICE},
  {"email", CCZE_COLOR_EMAIL},
  {"size", CCZE_COLOR_SIZE},
  {"version", CCZE_COLOR_VERSION},
  {"address", CCZE_COLOR_ADDRESS},
  {"uri", CCZE_COLOR_URI},
  {"miss", CCZE_COLOR_PROXY_MISS},
  {"parent", CCZE_COLOR_PROXY_PARENT},
  {"direct", CCZE_COLOR_PROXY_DIRECT},
  {"hit", CCZE_COLOR_PROXY_HIT},
  {"deny", CCZE_COLOR_PROXY_DENIED},
  {"ident", CCZE_COLOR_IDENT},
  {"refresh", CCZE_COLOR_PROXY_REFRESH},
  {"swapfail", CCZE_COLOR_PROXY_SWAPFAIL},
  {"ctype", CCZE_COLOR_CTYPE},
  {"create", CCZE_COLOR_PROXY_CREATE},
  {"swapin", CCZE_COLOR_PROXY_SWAPIN},
  {"swapout", CCZE_COLOR_PROXY_SWAPOUT},
  {"release", CCZE_COLOR_PROXY_RELEASE},
  {"swapnum", CCZE_COLOR_SWAPNUM},
  {"user", CCZE_COLOR_USER},
  {"numbers", CCZE_COLOR_NUMBERS},
  {"subject", CCZE_COLOR_SUBJECT},
  {"signal", CCZE_COLOR_SIGNAL}
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
      char *tmp = strndupa (&color[1], strlen (color) - 2);
      return ccze_color_table[_ccze_color_keyword_lookup (tmp)];
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
      int ncolor, nkeyword, nbg;
      
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
            
      if (pre)
	{
	  if (!strcmp (pre, "bold"))
	    ncolor |= A_BOLD;
	  else if (!strcmp (pre, "underline"))
	    ncolor |= A_UNDERLINE;
	  else if (!strcmp (pre, "reverse"))
	    ncolor |= A_REVERSE;
	  else if (!strcmp (pre, "blink"))
	    ncolor |= A_BLINK;
	}
      
      if (color[0] == '\'')
	ccze_color_table[nkeyword] = ncolor;
      else
	ccze_color_table[nkeyword] = COLOR_PAIR (ncolor);
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
  ccze_color_table[CCZE_COLOR_LAST] = (CYAN);
}
