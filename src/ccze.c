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
#include <pcre.h>

#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ccze.h"
#include "ccze-httpd.h"
#include "ccze-procmail.h"
#include "ccze-squid.h"
#include "ccze-syslog.h"
#include "ccze-vsftpd.h"

struct
{
  int scroll;
} ccze_config;

const char *argp_program_version = "ccze/0.1";
const char *argp_program_bug_address = "<algernon@bonehunter.rulez.org>";
static struct argp_option options[] = {
  {NULL, 0, NULL, 0, "", 1},
  {"scroll", 's', NULL, 0, "Enable scrolling", 1},
  {"no-scroll", -100, NULL, 0, "Disable scrolling", 1},
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

static void sigint_handler (int sig) __attribute__ ((noreturn));
static void
sigint_handler (int sig)
{
  endwin ();
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
  char subject[4096];
  int match, offsets[99];
  pcre *regc_syslog, *regc_procmail_log, *regc_httpd_access_log;
  pcre *regc_squid_access_log, *regc_vsftpd_log, *regc_squid_cache_log;
  pcre *regc_squid_store_log;
  pcre_extra *hints_syslog, *hints_procmail_log, *hints_httpd_access_log;
  pcre_extra *hints_squid_access_log, *hints_vsftpd_log;
  pcre_extra *hints_squid_cache_log, *hints_squid_store_log;
  
  ccze_config.scroll = 1;
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

  ccze_squid_setup (&regc_squid_access_log, &regc_squid_cache_log,
		    &regc_squid_store_log, &hints_squid_access_log,
		    &hints_squid_cache_log, &hints_squid_store_log);
  ccze_syslog_setup (&regc_syslog, &hints_syslog);
  ccze_procmail_setup (&regc_procmail_log, &hints_procmail_log);
  ccze_httpd_setup (&regc_httpd_access_log, &hints_httpd_access_log);
  ccze_vsftpd_setup (&regc_vsftpd_log, &hints_vsftpd_log);
  
  while (1)
    {
      int handled = CCZE_MATCH_NONE;
      char *rest = NULL;
            
      if (fgets (subject, sizeof (subject), stdin) == NULL)
	break;

      /** Procmail **/
      if ((match = pcre_exec (regc_procmail_log, hints_procmail_log,
			      subject, strlen (subject), 0, 0, offsets,
			      99)) >= 0)
	{
	  rest = ccze_procmail_process (subject, offsets, match);
	  handled = CCZE_MATCH_PROCMAIL_LOG;
	}

      /** HTTPd access.log **/
      if ((match = pcre_exec (regc_httpd_access_log, hints_httpd_access_log,
			      subject, strlen (subject), 0, 0, offsets,
			      99)) >= 0 && handled == CCZE_MATCH_NONE)
	{
	  rest = ccze_httpd_access_log_process (subject, offsets, match);
	  handled = CCZE_MATCH_HTTPD_ACCESS_LOG;
	}

      /** Squid access.log **/
      if ((match = pcre_exec (regc_squid_access_log, hints_squid_access_log,
			      subject, strlen (subject), 0, 0, offsets,
			      99)) >= 0 && handled == CCZE_MATCH_NONE)
	{
	  rest = ccze_squid_access_log_process (subject, offsets, match);
	  handled = CCZE_MATCH_SQUID_ACCESS_LOG;
	}

      /** Squid store.log **/
      if ((match = pcre_exec (regc_squid_store_log, hints_squid_store_log,
			      subject, strlen (subject), 0, 0, offsets,
			      99)) >= 0 && handled == CCZE_MATCH_NONE)
	{
	  rest = ccze_squid_store_log_process (subject, offsets, match);
	  handled = CCZE_MATCH_SQUID_STORE_LOG;
	}
      
      /** Squid cache.log **/
      if ((match = pcre_exec (regc_squid_cache_log, hints_squid_cache_log,
			      subject, strlen (subject), 0, 0, offsets,
			      99)) >= 0 && handled == CCZE_MATCH_NONE)
	{
	  rest = ccze_squid_cache_log_process (subject, offsets, match);
	  handled = CCZE_MATCH_SQUID_CACHE_LOG;
	}
      
      /** VSFTPd log **/
      if ((match = pcre_exec (regc_vsftpd_log, hints_vsftpd_log, subject,
			      strlen (subject), 0, 0, offsets, 99)) >= 0 &&
	  handled == CCZE_MATCH_NONE)
	{
	  rest = ccze_vsftpd_log_process (subject, offsets, match);
	  handled = CCZE_MATCH_VSFTPD_LOG;
	}
      
      /** Syslog **/
      if ((match = pcre_exec (regc_syslog, hints_syslog, subject,
			      strlen (subject), 0, 0, offsets, 99)) >= 0 &&
	  handled == CCZE_MATCH_NONE)
	{
	  rest = ccze_syslog_process (subject, offsets, match);
	  handled = CCZE_MATCH_SYSLOG;
	}
                        
      /** Common. Goodword coloring should come here **/
      if (rest)
	{
	  CCZE_ADDSTR (CCZE_COLOR_DEFAULT, rest);
	  CCZE_NEWLINE ();
	  free (rest);
	}

      if (handled == CCZE_MATCH_NONE)
	CCZE_ADDSTR (CCZE_COLOR_DEFAULT, subject);
            
      refresh ();
    }

  refresh ();

  free (regc_syslog);
  free (hints_syslog);
  free (regc_procmail_log);
  free (hints_procmail_log);
  free (regc_httpd_access_log);
  free (hints_httpd_access_log);
  free (regc_squid_access_log);
  free (hints_squid_access_log);
  free (regc_vsftpd_log);
  free (hints_vsftpd_log);
  free (regc_squid_cache_log);
  free (hints_squid_cache_log);
  
  sigint_handler (0);
  
  return 0;
}
