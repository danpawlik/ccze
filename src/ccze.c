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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ccze.h"
#include "ccze-httpd.h"
#include "ccze-procmail.h"
#include "ccze-squid.h"
#include "ccze-sulog.h"
#include "ccze-super.h"
#include "ccze-syslog.h"
#include "ccze-vsftpd.h"
#include "ccze-wordcolor.h"

struct
{
  int scroll;
  int convdate;
  int wcol;
  int slookup;
} ccze_config;

const char *argp_program_version = "ccze 0.1." PATCHLEVEL;
const char *argp_program_bug_address = "<algernon@bonehunter.rulez.org>";
static struct argp_option options[] = {
  {NULL, 0, NULL, 0, "", 1},
  {"scroll", 's', NULL, 0, "Enable scrolling", 1},
  {"no-scroll", -100, NULL, 0, "Disable scrolling", 1},
  {"convert-date", 'C', NULL, 0, "Convert UNIX timestamps to readable format", 1},
  {"no-word-color", -101, NULL, 0, "Disable word coloring", 1},
  {"no-service-lookup", -102, NULL, 0, "Disable service lookups", 1},
  {NULL, 0, NULL, 0,  NULL, 0}
};
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static struct argp argp =
  {options, parse_opt, 0, "ccze -- cheer up 'yer logs.", NULL, NULL, NULL};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 's':
      ccze_config.scroll = 1;
      break;
    case -100:
      ccze_config.scroll = 0;
      break;
    case 'C':
      ccze_config.convdate = 1;
      break;
    case -101:
      ccze_config.wcol = 0;
      break;
    case -102:
      ccze_config.slookup = 0;
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
  ccze_vsftpd_shutdown ();
  ccze_syslog_shutdown ();
  ccze_super_shutdown ();
  ccze_sulog_shutdown ();  
  ccze_squid_shutdown ();
  ccze_procmail_shutdown ();
  ccze_httpd_shutdown ();

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
    
  ccze_config.scroll = 1;
  ccze_config.convdate = 0;
  ccze_config.wcol = 1;
  ccze_config.slookup = 1;
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
  init_pair (1, COLOR_RED,     COLOR_BLACK);
  init_pair (2, COLOR_GREEN,   COLOR_BLACK);
  init_pair (3, COLOR_YELLOW,  COLOR_BLACK);
  init_pair (4, COLOR_BLUE,    COLOR_BLACK);
  init_pair (5, COLOR_CYAN,    COLOR_BLACK);
  init_pair (6, COLOR_MAGENTA, COLOR_BLACK);
  init_pair (7, COLOR_WHITE,   COLOR_BLACK);

  ccze_httpd_setup ();
  ccze_procmail_setup ();
  ccze_squid_setup ();
  ccze_sulog_setup ();
  ccze_super_setup ();
  ccze_syslog_setup ();
  ccze_vsftpd_setup ();
  ccze_wordcolor_setup ();
      
  while (getline (&subject, &subjlen, stdin) != -1)
    {
      int handled = CCZE_MATCH_NONE;
      int status = CCZE_MATCH_NONE;
      char *rest = NULL;
      char *tmp = strchr (subject, '\n');

      tmp[0] = '\0';
      
      if ((handled = ccze_procmail_handle (subject, subjlen, &rest)) !=
	  CCZE_MATCH_NONE)
	status = handled;
      else if ((handled = ccze_httpd_handle (subject, subjlen, &rest)) !=
	       CCZE_MATCH_NONE)
	status = handled;
      else if ((handled = ccze_squid_handle (subject, subjlen, &rest)) !=
	       CCZE_MATCH_NONE)
	status = handled;
      else if ((handled = ccze_vsftpd_handle (subject, subjlen, &rest)) !=
	       CCZE_MATCH_NONE)
	status = handled;
      else if ((handled = ccze_sulog_handle (subject, subjlen, &rest)) !=
	       CCZE_MATCH_NONE)
	status = handled;
      else if ((handled = ccze_super_handle (subject, subjlen, &rest)) !=
	       CCZE_MATCH_NONE)
	status = handled;
      else if ((handled = ccze_syslog_handle (subject, subjlen, &rest)) !=
	       CCZE_MATCH_NONE)
	status = handled;
                        
      if (rest)
	{
	  ccze_wordcolor_process (rest, ccze_config.wcol,
				  ccze_config.slookup);
	  CCZE_NEWLINE ();
	  free (rest);
	}

      if (status == CCZE_MATCH_NONE)
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
