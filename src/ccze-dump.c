/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-dump.c -- Dump internal color table
 * Copyright (C) 2002, 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
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

#define CCZE_DUMP 1
#include "ccze-color.c"
#include <ncurses.h>

static char *
ccze_dump_color_get_attrib (int color)
{
  char *str = (char *)calloc (100, sizeof (char));
  
  if (color & A_BOLD)
    strcat (str, "bold ");
  if (color & A_UNDERLINE)
    strcat (str, "underline ");
  if (color & A_REVERSE)
    strcat (str, "reverse ");
  if (color & A_BLINK)
    strcat (str, "blink ");
  
  return str;
}

static char *
ccze_dump_lookup_name (ccze_color_t color)
{
  size_t cidx;
  
  for (cidx = 0; cidx < sizeof (ccze_color_keyword_map); cidx++)
    if (ccze_color_keyword_map[cidx].idx == color)
      return ccze_color_keyword_map[cidx].keyword;
  return NULL;
}

static char *
ccze_dump_color_to_name (int color)
{
  int my_color = ccze_color_strip_attrib (color);

  if (my_color < COLOR_PAIR (8))
    return ccze_color_to_name_simple (my_color);
  else
    {
      int i,j;
      char * str;

      j = (my_color >> 8) % 8;
      i = (my_color >> 8) / 8;
      asprintf (&str, "%s on_%s",
		ccze_color_to_name_simple (COLOR_PAIR (j)),
		ccze_color_to_name_simple (COLOR_PAIR (i)));
      return str;
    }
}

static char *
ccze_dump_color_comment (ccze_color_t cidx)
{
  return ccze_color_keyword_map[cidx].comment;
}

static int
ccze_dump_color_to_idx (ccze_color_t color)
{
  size_t cidx;
  
  for (cidx = 0; cidx < sizeof (ccze_color_keyword_map); cidx++)
    if (ccze_color_keyword_map[cidx].idx == color)
      return cidx;
  return 0;
}

int
main (void)
{
  ccze_color_t cidx;
  
  ccze_color_init ();

  printf ("# Configuration file for ccze\n#\n");
  printf ("# Available 'pre' attributes: bold, underline, underscore, "
	  "blink, reverse\n");
  printf ("# Available colors:  black, red, green, yellow, blue, magenta, "
	  "cyan, white\n");
  printf ("# Available bgcolors: on_black, on_red, on_green, on_yellow, "
	  "on_blue, on_magenta, on_cyan, on_white\n#\n");
  printf ("# You can also use item names in color definition, like:\n#\n");
  printf ("# default   blue\n# date      'default'\n#\n");
  printf ("# Here you defined default color to blue, and date color to "
	  "default value's color, so\n");
  printf ("# your date color is blue. (You can only use predefined item "
	  "names!)\n\n");
  printf ("# item          color                   # comment (what is "
	  "color, or why it's that ;)\n\n");
  
  for (cidx = CCZE_COLOR_DATE; cidx < CCZE_COLOR_LAST; cidx++)
    {
      int color = ccze_color (cidx);
      char line[256];
      
      strcpy (line, ccze_dump_lookup_name (cidx));
      memset (&line[strlen(line)], ' ', 16 - strlen (line));
      line[16]='\0';
      strcat (line, ccze_dump_color_get_attrib (color));
      strcat (line, ccze_dump_color_to_name (color));
      memset (&line[strlen(line)], ' ', 42 - strlen (line));
      line[40]='#';
      line[42]='\0';
      strcat (line, ccze_dump_color_comment (ccze_dump_color_to_idx (cidx)));
      
      printf ("%s\n", line);      
    }
  
  return 0;
}
