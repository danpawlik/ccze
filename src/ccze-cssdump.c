/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-cssdump.c -- Dump internal color table into css format
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

#include "ccze-color.c"
#include <ncurses.h>

static char *
ccze_dump_color_to_name (int color)
{
  int my_color = ccze_color_strip_attrib (color);
  char *str, *tmp;

  if (my_color < COLOR_PAIR (8))
    asprintf (&str, "\tcolor: %s\n", ccze_color_to_name_simple (my_color));
  else
    {
      int i,j;

      j = (my_color >> 8) % 8;
      i = (my_color >> 8) / 8;
      asprintf (&str, "\tcolor: %s\n\ttext-background: %s\n",
		ccze_color_to_name_simple (COLOR_PAIR (j)),
		ccze_color_to_name_simple (COLOR_PAIR (i)));
    }

  if (color & A_UNDERLINE)
    {
      asprintf (&tmp, "%s\ttext-decoration: underline\n", str);
      free (str);
      str = tmp;
    }
  
  return str;
}

int
main (void)
{
  ccze_color_t cidx;
  
  ccze_color_init ();

  for (cidx = CCZE_COLOR_DATE; cidx < CCZE_COLOR_LAST; cidx++)
    {
      int color = ccze_color (cidx);
      char *line;

      asprintf (&line, "%s {\n%s}\n",
		ccze_color_lookup_name (cidx),
		ccze_dump_color_to_name (color));
      
      printf ("%s\n", line);      
    }
  
  return 0;
}
