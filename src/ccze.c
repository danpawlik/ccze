/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze.c -- CCZE itself
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

#include <argp.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ccze.h"
#include "ccze-color.h"
#include "ccze-wordcolor.h"
#include "ccze-plugin.h"

struct
{
  int scroll;
  int convdate;
  int wcol;
  int slookup;
  char *rcfile;
  char **pluginlist;
  size_t pluginlist_alloc, pluginlist_len;
} ccze_config;

static short colors[] = {COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
			 COLOR_BLUE, COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE};
 
const char *argp_program_version = "ccze 0.1." PATCHLEVEL;
const char *argp_program_bug_address = "<algernon@bonehunter.rulez.org>";
static struct argp_option options[] = {
  {NULL, 0, NULL, 0, "", 1},
  {"rcfile", 'F', "FILE", 0, "Read configuration from FILE", 1},
  {"scroll", 's', NULL, 0, "Enable scrolling", 1},
  {"options", 'o', "OPTIONS...", 0, "Toggle some options\n"
   "(such as scroll, wordcolor and lookups)", 1},
  {"convert-date", 'C', NULL, 0, "Convert UNIX timestamps to readable format", 1},
  {"plugin", 'p', "PLUGIN", 0, "Load PLUGIN", 1},
  {NULL, 0, NULL, 0,  NULL, 0}
};
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static struct argp argp =
  {options, parse_opt, 0, "ccze -- cheer up 'yer logs.", NULL, NULL, NULL};

enum
{
  CCZE_O_SUBOPT_SCROLL = 0,
  CCZE_O_SUBOPT_NOSCROLL,
  CCZE_O_SUBOPT_WORDCOLOR,
  CCZE_O_SUBOPT_NOWORDCOLOR,
  CCZE_O_SUBOPT_LOOKUPS,
  CCZE_O_SUBOPT_NOLOOKUPS,
  CCZE_O_SUBOPT_END
};

static char *o_subopts[] = {
  [CCZE_O_SUBOPT_SCROLL] = "scroll",
  [CCZE_O_SUBOPT_NOSCROLL] = "noscroll",
  [CCZE_O_SUBOPT_WORDCOLOR] = "wordcolor",
  [CCZE_O_SUBOPT_NOWORDCOLOR] = "nowordcolor",
  [CCZE_O_SUBOPT_LOOKUPS] = "lookups",
  [CCZE_O_SUBOPT_NOLOOKUPS] = "nolookups",
  [CCZE_O_SUBOPT_END] = NULL
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  char *subopts, *value;
  
  switch (key)
    {
    case 'p':
      ccze_config.pluginlist[ccze_config.pluginlist_len++] = arg;
      if (ccze_config.pluginlist_len >= ccze_config.pluginlist_alloc)
	{
	  ccze_config.pluginlist_alloc *= 2;
	  ccze_config.pluginlist = (char **)realloc
	    (ccze_config.pluginlist, ccze_config.pluginlist_alloc * sizeof (char *));
	}
      break;
    case 'F':
      ccze_config.rcfile = arg;
      break;
    case 'o':
      subopts = optarg;
      while (*subopts != '\0')
	{
	  switch (getsubopt (&subopts, o_subopts, &value))
	    {
	    case CCZE_O_SUBOPT_SCROLL:
	      ccze_config.scroll = 1;
	      break;
	    case CCZE_O_SUBOPT_NOSCROLL:
	      ccze_config.scroll = 0;
	      break;
	    case CCZE_O_SUBOPT_WORDCOLOR:
	      ccze_config.wcol = 1;
	      break;
	    case CCZE_O_SUBOPT_NOWORDCOLOR:
	      ccze_config.wcol = 0;
	      break;
	    case CCZE_O_SUBOPT_LOOKUPS:
	      ccze_config.slookup = 1;
	      break;
	    case CCZE_O_SUBOPT_NOLOOKUPS:
	      ccze_config.slookup = 0;
	      break;
	    default:
	      argp_error (state, "unrecognised option: `%s'", value);
	      break;
	    }
	}
      break;
    case 'C':
      ccze_config.convdate = 1;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

int
ccze_http_action (const char *method)
{
  if (!strcasecmp ("GET", method))
    return CCZE_COLOR_HTTP_GET;
  else if (!strcasecmp ("POST", method))
    return CCZE_COLOR_HTTP_POST;
  else if (!strcasecmp ("HEAD", method))
    return CCZE_COLOR_HTTP_HEAD;
  else if (!strcasecmp ("PUT", method))
    return CCZE_COLOR_HTTP_PUT;
  else if (!strcasecmp ("CONNECT", method))
    return CCZE_COLOR_HTTP_CONNECT;
  else if (!strcasecmp ("TRACE", method))
    return CCZE_COLOR_HTTP_TRACE;
  else
    return CCZE_COLOR_UNKNOWN;
}

void
ccze_print_date (const char *date)
{
  time_t ltime;
  char tmp[128];
  
  if (ccze_config.convdate)
    {
      ltime = atol (date);
      if (ltime < 0)
	{
	  CCZE_ADDSTR (CCZE_COLOR_DATE, date);
	  return;
	}
      strftime (tmp, sizeof (tmp) - 1, "%b %e %T", gmtime (&ltime));
      CCZE_ADDSTR (CCZE_COLOR_DATE, tmp);
    }
  else
    CCZE_ADDSTR (CCZE_COLOR_DATE, date);
}

static void sigint_handler (int sig) __attribute__ ((noreturn));
static void
sigint_handler (int sig)
{
  endwin ();

  ccze_wordcolor_shutdown ();
  ccze_plugin_shutdown ();
  
  exit (0);
}

static void
sigwinch_handler (int sig)
{
  endwin ();
  refresh ();
  signal (SIGWINCH, sigwinch_handler);
}

int
main (int argc, char **argv)
{
  char *subject = NULL;
  size_t subjlen = 0;
  int i, j;
  char *homerc, *home;
  ccze_plugin_t **plugins;
      
  ccze_config.scroll = 1;
  ccze_config.convdate = 0;
  ccze_config.wcol = 1;
  ccze_config.slookup = 1;
  ccze_config.rcfile = NULL;
  ccze_config.pluginlist_len = 0;
  ccze_config.pluginlist_alloc = 10;
  ccze_config.pluginlist = (char **)calloc (ccze_config.pluginlist_alloc,
					    sizeof (char *));
  argp_parse (&argp, argc, argv, 0, 0, NULL);

  initscr ();
  signal (SIGINT, sigint_handler);
  signal (SIGWINCH, sigwinch_handler);
  nonl ();
  if (ccze_config.scroll)
    {
      idlok (stdscr, TRUE);
      scrollok (stdscr, TRUE);
      leaveok (stdscr, FALSE);
    }
  
  start_color ();
  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++)
      init_pair (i*8 + j, colors[j], colors[i]);
  ccze_color_init ();

  if (ccze_config.rcfile)
    ccze_color_load (ccze_config.rcfile);
  else
    {
      ccze_color_load (SYSCONFDIR "/colorizerc");
      ccze_color_load (SYSCONFDIR "/cczerc");
      home = getenv ("HOME");
      if (home)
	{
	  asprintf (&homerc, "%s/.colorizerc", home);
	  ccze_color_load (homerc);
	  free (homerc);
	  asprintf (&homerc, "%s/.cczerc", home);
	  ccze_color_load (homerc);
	  free (homerc);
	}
    }

  ccze_wordcolor_setup ();

  ccze_plugin_init ();
  if (ccze_config.pluginlist_len == 0)
    ccze_plugin_load_all ();
  else
    {
      while (ccze_config.pluginlist_len-- > 0)
	ccze_plugin_load (ccze_config.pluginlist[ccze_config.pluginlist_len]);
    }
  ccze_plugin_finalise ();
      
  i = 0;
  plugins = ccze_plugins();
  while (plugins[i])
    (*(plugins[i++]->startup))();
        
  while (getline (&subject, &subjlen, stdin) != -1)
    {
      int handled = 0;
      int status = 0;
      char *rest = NULL;
      char *tmp = strchr (subject, '\n');

      tmp[0] = '\0';

      i = 0;
      while (plugins[i])
	{
	  if ((handled = (*(plugins[i++]->handler))
	       (subject, subjlen, &rest)) != 0)
	    {
	      status = handled;
	      break;
	    }
	}

      if (rest)
	{
	  ccze_wordcolor_process (rest, ccze_config.wcol,
				  ccze_config.slookup);
	  CCZE_NEWLINE ();
	  free (rest);
	}

      if (status == 0)
	{
	  ccze_wordcolor_process (subject, ccze_config.wcol,
				  ccze_config.slookup);
	  CCZE_NEWLINE ();
	}
            
      refresh ();
    }

  refresh ();

  free (subject);

  sigint_handler (0);
  
  return 0;
}
