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
#include <unistd.h>
#include <string.h>

#define BOLD A_BOLD |
#define RED COLOR_PAIR (1)
#define GREEN COLOR_PAIR (2)
#define YELLOW COLOR_PAIR (3)
#define BLUE COLOR_PAIR (4)
#define CYAN COLOR_PAIR (5)
#define MAGENTA COLOR_PAIR (6)
#define WHITE COLOR_PAIR (7)

#define CCZE_COLOR_DATE (BOLD CYAN)
#define CCZE_COLOR_HOST (BOLD BLUE)
#define CCZE_COLOR_PROC (GREEN)
#define CCZE_COLOR_PID (BOLD WHITE)
#define CCZE_COLOR_PIDB (BOLD GREEN)
#define CCZE_COLOR_DEFAULT (CYAN)
#define CCZE_COLOR_EMAIL (BOLD GREEN)
#define CCZE_COLOR_SUBJECT (MAGENTA)
#define CCZE_COLOR_DIR (BOLD CYAN)
#define CCZE_COLOR_SIZE (BOLD WHITE)
#define CCZE_COLOR_USER (BOLD YELLOW)
#define CCZE_COLOR_HTTPCODES (BOLD WHITE)
#define CCZE_COLOR_GETSIZE (MAGENTA)
#define CCZE_COLOR_HTTP_GET (GREEN)
#define CCZE_COLOR_HTTP_POST (BOLD GREEN)
#define CCZE_COLOR_HTTP_HEAD (GREEN)
#define CCZE_COLOR_HTTP_PUT (BOLD GREEN)
#define CCZE_COLOR_HTTP_CONNECT (GREEN)
#define CCZE_COLOR_HTTP_TRACE (GREEN)
#define CCZE_COLOR_UNKNOWN CCZE_COLOR_DEFAULT
#define CCZE_COLOR_GETTIME (BOLD MAGENTA)
#define CCZE_COLOR_URI (BOLD GREEN)
#define CCZE_COLOR_IDENT (BOLD WHITE)
#define CCZE_COLOR_CTYPE (WHITE)
#define CCZE_COLOR_ERROR (BOLD RED)
#define CCZE_COLOR_PROXY_MISS (RED)
#define CCZE_COLOR_PROXY_HIT (BOLD YELLOW)
#define CCZE_COLOR_PROXY_DENIED (BOLD RED)
#define CCZE_COLOR_PROXY_REFRESH (BOLD WHITE)
#define CCZE_COLOR_PROXY_SWAPFAIL (BOLD WHITE)
#define CCZE_COLOR_DEBUG (WHITE)
#define CCZE_COLOR_WARNING (RED)
#define CCZE_COLOR_PROXY_DIRECT (BOLD WHITE)
#define CCZE_COLOR_PROXY_PARENT (BOLD YELLOW)

enum
{
  CCZE_MATCH_NONE,
  CCZE_MATCH_PROCMAIL_LOG,
  CCZE_MATCH_SYSLOG,
  CCZE_MATCH_HTTPD_ACCESS_LOG,
  CCZE_MATCH_SQUID_ACCESS_LOG,
  CCZE_MATCH_VSFTPD_LOG
};

struct
{
  int scroll;
} ccze_config;

static const char *argp_program_version = "ccze/0.1";
static const char *argp_program_bug_address = "<algernon@bonehunter.rulez.org>";
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

static void
space (void)
{
  attrset (CCZE_COLOR_DEFAULT);
  addstr (" ");
}

static char *
process_syslog (const char *str, int *offsets, int match)
{
  char *date = NULL, *host = NULL, *send = NULL, *process = NULL;
  char *msg = NULL, *pid = NULL, *tmp = NULL, *toret;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&host);
  pcre_get_substring (str, offsets, match, 3, (const char **)&send);
  
  if ((strstr (send, "last message repeated") && strstr (send, "times")) ||
      (strstr (send, "-- MARK --")))
    msg = strdup (send);
  else
    {
      pcre_get_substring (str, offsets, match, 4, (const char **)&process);
      pcre_get_substring (str, offsets, match, 5, (const char **)&msg);
    }
      
  if (process)
    {
      char *t;
      if ((t = strchr (process, '[')))
	{
	  char *t2 = strchr (t, ']');

	  pid = strndup (&t[1], (size_t)(t2 - t - 1));
	  tmp = strndup (process, (size_t)(t - process));
	  free (process);
	  process = tmp;
	}
    }
  
  attrset (CCZE_COLOR_DATE);
  addstr (date);
  space ();

  attrset (CCZE_COLOR_HOST);
  addstr (host);
  space ();
  
  attrset (CCZE_COLOR_PROC);
  if (process)
    {
      addstr (process);
      if (pid)
	{
	  attrset (CCZE_COLOR_PIDB);
	  addstr ("[");
	  attrset (CCZE_COLOR_PID);
	  addstr (pid);
	  attrset (CCZE_COLOR_PIDB);
	  addstr ("]");
	  attrset (CCZE_COLOR_PROC);
	  addstr (":");
	}
      space ();
      toret = strdup (msg);
    }
  else
    toret = strdup (send);

  free (date);
  free (host);
  free (send);
  free (process);
  free (msg);
  free (pid);

  return toret;
}

static char *
process_procmail (const char *str, int *offsets, int match)
{
  char *header = NULL, *value = NULL, *space1 = NULL;
  char *space2 = NULL, *extra = NULL;
  int handled = 0;
  int col = 0;

  pcre_get_substring (str, offsets, match, 1, (const char **)&space1);
  pcre_get_substring (str, offsets, match, 2, (const char **)&header);
  pcre_get_substring (str, offsets, match, 3, (const char **)&value);
  pcre_get_substring (str, offsets, match, 4, (const char **)&space2);
  pcre_get_substring (str, offsets, match, 5, (const char **)&extra);
  
  if (!strcasecmp ("from", header) || !strcasecmp (">from", header))
    {
      col = CCZE_COLOR_EMAIL;
      handled = 1;
    }
  if (!strcasecmp ("subject:", header))
    {
      col = CCZE_COLOR_SUBJECT;
      handled = 1;
    }
  if (!strcasecmp ("folder:", header))
    {
      col = CCZE_COLOR_DIR;
      handled = 1;
    }

  if (!handled)
    {
      free (header);
      free (value);
      free (extra);
      return strdup (str);
    }
  
  attrset (CCZE_COLOR_DEFAULT);
  addstr (space1);
  addstr (header);
  space ();
  
  attrset (col);
  addstr (value);
  attrset (CCZE_COLOR_DEFAULT);
  addstr (space2);

  if (!strcasecmp ("folder:", header))
    attrset (CCZE_COLOR_SIZE);
  if (!strcasecmp ("from", header))
    attrset (CCZE_COLOR_DATE);

  addstr (extra);
  addstr ("\n");
  
  free (extra);
  free (header);
  free (value);

  return NULL;
}

static void
http_action (const char *method)
{
  if (!strcasecmp ("GET", method))
    attrset (CCZE_COLOR_HTTP_GET);
  else if (!strcasecmp ("POST", method))
    attrset (CCZE_COLOR_HTTP_POST);
  else if (!strcasecmp ("HEAD", method))
    attrset (CCZE_COLOR_HTTP_HEAD);
  else if (!strcasecmp ("PUT", method))
    attrset (CCZE_COLOR_HTTP_PUT);
  else if (!strcasecmp ("CONNECT", method))
    attrset (CCZE_COLOR_HTTP_CONNECT);
  else if (!strcasecmp ("TRACE", method))
    attrset (CCZE_COLOR_HTTP_TRACE);
  else
    attrset (CCZE_COLOR_UNKNOWN);
}

static char *
process_httpd_access_log (const char *str, int *offsets, int match)
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

  attrset (CCZE_COLOR_HOST);
  addstr (host);
  space ();

  addstr ("-");
  space ();

  attrset (CCZE_COLOR_USER);
  addstr (user);
  space ();

  attrset (CCZE_COLOR_DATE);
  addstr (date);
  space ();

  http_action (method);
  addstr (full_action);
  space ();

  attrset (CCZE_COLOR_HTTPCODES);
  addstr (http_code);
  space ();

  attrset (CCZE_COLOR_GETSIZE);
  addstr (gsize);
  space ();

  free (host);
  free (user);
  free (date);
  free (method);
  free (full_action);
  free (http_code);
  free (gsize);
  
  return other;
}

static void
proxy_action (const char *action)
{
  if (strstr (action, "ERR") == action)
    attrset (CCZE_COLOR_ERROR);
  else if (strstr (action, "MISS"))
    attrset (CCZE_COLOR_PROXY_MISS);
  else if (strstr (action, "HIT"))
    attrset (CCZE_COLOR_PROXY_HIT);
  else if (strstr (action, "DENIED"))
    attrset (CCZE_COLOR_PROXY_DENIED);
  else if (strstr (action, "REFRESH"))
    attrset (CCZE_COLOR_PROXY_REFRESH);
  else if (strstr (action, "SWAPFAIL"))
    attrset (CCZE_COLOR_PROXY_SWAPFAIL);
  else if (strstr (action, "NONE"))
    attrset (CCZE_COLOR_DEBUG);
  else
    attrset (CCZE_COLOR_UNKNOWN);
}

static void
proxy_hierarch (const char *hierar)
{
  if (strstr (hierar, "NO") == hierar)
    attrset (CCZE_COLOR_WARNING);
  else if (strstr (hierar, "DIRECT"))
    attrset (CCZE_COLOR_PROXY_DIRECT);
  else if (strstr (hierar, "PARENT"))
    attrset (CCZE_COLOR_PROXY_PARENT);
  else if (strstr (hierar, "MISS"))
    attrset (CCZE_COLOR_PROXY_MISS);
  else
    attrset (CCZE_COLOR_UNKNOWN);
}

static char *
process_squid_access_log (const char *str, int *offsets, int match)
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

  attrset (CCZE_COLOR_DATE);
  addstr (date);
  attrset (CCZE_COLOR_DEFAULT);
  addstr (espace);
  attrset (CCZE_COLOR_GETTIME);
  addstr (elaps);
  space ();
    
  attrset (CCZE_COLOR_HOST);
  addstr (host);
  space ();

  proxy_action (action);
  addstr (action);
  attrset (CCZE_COLOR_DEFAULT);
  addstr ("/");
  attrset (CCZE_COLOR_HTTPCODES);
  addstr (httpc);
  space ();

  attrset (CCZE_COLOR_GETSIZE);
  addstr (gsize);
  space ();

  http_action (method);
  addstr (method);
  space ();

  attrset (CCZE_COLOR_URI);
  addstr (uri);
  space ();

  attrset (CCZE_COLOR_IDENT);
  addstr (ident);
  space ();

  proxy_hierarch (hierar);
  addstr (hierar);
  attrset (CCZE_COLOR_DEFAULT);
  addstr ("/");
  attrset (CCZE_COLOR_HOST);
  addstr (fhost);
  space ();

  attrset (CCZE_COLOR_CTYPE);
  addstr (ctype);

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

static char *
process_vsftpd_log (const char *str, int *offsets, int match)
{
  char *date, *sspace, *pid, *user, *other;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&sspace);
  pcre_get_substring (str, offsets, match, 3, (const char **)&pid);
  pcre_get_substring (str, offsets, match, 5, (const char **)&user);
  pcre_get_substring (str, offsets, match, 6, (const char **)&other);
  
  attrset (CCZE_COLOR_DATE);
  addstr (date);
  attrset (CCZE_COLOR_DEFAULT);
  addstr (sspace);

  attrset (CCZE_COLOR_PIDB);
  addstr ("[");
  attrset (CCZE_COLOR_DEFAULT);
  addstr ("pid ");
  attrset (CCZE_COLOR_PID);
  addstr (pid);
  attrset (CCZE_COLOR_PIDB);
  addstr ("]");
  space();

  if (*user)
    {
      attrset (CCZE_COLOR_PIDB);
      addstr ("[");
      attrset (CCZE_COLOR_USER);
      addstr (user);
      attrset (CCZE_COLOR_PIDB);
      addstr ("]");
      space ();
    }

  free (date);
  free (sspace);
  free (pid);
  free (user);
  
  return other;
}
     
int
main (int argc, char **argv)
{
  char subject[4096];
  const char *error;
  int match, offsets[99], errptr;
  pcre *regc_syslog, *regc_procmail_log, *regc_httpd_access_log;
  pcre *regc_squid_access_log, *regc_vsftpd_log;
  pcre_extra *hints_syslog, *hints_procmail_log, *hints_httpd_access_log;
  pcre_extra *hints_squid_access_log, *hints_vsftpd_log;
  
  ccze_config.scroll = 1;
  argp_parse (&argp, argc, argv, 0, 0, NULL);
  
  initscr ();
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

  regc_squid_access_log = pcre_compile
    ("^(\\d{9,10}\\.\\d{3})(\\s+)(\\d+)\\s(\\S+)\\s(\\w+)\\/(\\d{3})"
     "\\s(\\d+)\\s(\\w+)\\s(\\S+)\\s(\\S+)\\s(\\w+)\\/(\\S+)\\s(\\S*)$",
     0, &error, &errptr, NULL);
  hints_squid_access_log = pcre_study (regc_squid_access_log, 0, &error);
  regc_httpd_access_log = pcre_compile
    ("^(\\S*)\\s-\\s(\\S+)\\s(\\[\\d{1,2}\\/\\S*"
     "\\/\\d{4}:\\d{2}:\\d{2}:\\d{2}.{0,6}\\])\\s"
     "(\"([A-Z]{3,})\\s[^\"]+\")\\s(\\d{3})\\s(\\d+|-)\\s(.*)$", 0,
     &error, &errptr, NULL);
  hints_httpd_access_log = pcre_study (regc_httpd_access_log, 0, &error);
  regc_procmail_log = pcre_compile ("^(\\s*)(>?From|Subject:|Folder:)?"
				    "\\s(\\S+)(\\s+)(.*)$", 0,
				     &error, &errptr, NULL);
  hints_procmail_log = pcre_study (regc_procmail_log, 0, &error);
  regc_syslog = pcre_compile ("^(\\S*\\s{1,2}\\d{1,2}\\s\\d\\d:\\d\\d:\\d\\d)"
			      "\\s(\\S+)\\s((\\S+:?)\\s(.*))$", 0, &error,
			      &errptr, NULL);
  hints_syslog = pcre_study (regc_syslog, 0, &error);
  regc_vsftpd_log = pcre_compile
    ("^(\\S+\\s+\\S+\\s+\\d{1,2}\\s+\\d{1,2}:\\d{1,2}:\\d{1,2}\\s+\\d+)(\\s+)"
     "\\[pid (\\d+)\\]\\s+(\\[(\\S+)\\])?\\s*(.*)$", 0, &error, &errptr, NULL);
  hints_vsftpd_log = pcre_study (regc_vsftpd_log, 0, &error);
  
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
	  rest = process_procmail (subject, offsets, match);
	  handled = CCZE_MATCH_PROCMAIL_LOG;
	}

      /** HTTPd access.log **/
      if ((match = pcre_exec (regc_httpd_access_log, hints_httpd_access_log,
			      subject, strlen (subject), 0, 0, offsets,
			      99)) >= 0 && handled == CCZE_MATCH_NONE)
	{
	  rest = process_httpd_access_log (subject, offsets, match);
	  handled = CCZE_MATCH_HTTPD_ACCESS_LOG;
	}

      /** Squid access.log **/
      if ((match = pcre_exec (regc_squid_access_log, hints_squid_access_log,
			      subject, strlen (subject), 0, 0, offsets,
			      99)) >= 0 && handled == CCZE_MATCH_NONE)
	{
	  rest = process_squid_access_log (subject, offsets, match);
	  handled = CCZE_MATCH_SQUID_ACCESS_LOG;
	}

      if ((match = pcre_exec (regc_vsftpd_log, hints_vsftpd_log, subject,
			      strlen (subject), 0, 0, offsets, 99)) >= 0 &&
	  handled == CCZE_MATCH_NONE)
	{
	  rest = process_vsftpd_log (subject, offsets, match);
	  handled = CCZE_MATCH_VSFTPD_LOG;
	}
      
      /** Syslog **/
      if ((match = pcre_exec (regc_syslog, hints_syslog, subject,
			      strlen (subject), 0, 0, offsets, 99)) >= 0 &&
	  handled == CCZE_MATCH_NONE)
	{
	  rest = process_syslog (subject, offsets, match);
	  handled = CCZE_MATCH_SYSLOG;
	}
                        
      /** Common. Goodword coloring should come here **/
      if (rest)
	{
	  attrset (CCZE_COLOR_DEFAULT);
	  addstr (rest);
	  addstr ("\n");
	  free (rest);
	}

      if (handled == CCZE_MATCH_NONE)
	{
	  attrset (CCZE_COLOR_DEFAULT);
	  addstr (subject);
	}
            
      refresh ();
    }

  addstr ("\n");
  refresh ();
  endwin ();

  free (regc_syslog);
  free (hints_syslog);
  free (regc_procmail_log);
  free (hints_procmail_log);
  free (regc_httpd_access_log);
  free (hints_httpd_access_log);
  free (regc_squid_access_log);
  free (hints_squid_access_log);
  
  return 0;
}
