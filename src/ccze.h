/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze.h -- Common macros for CCZE
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

#ifndef _CCZE_H
#define _CCZE_H 1

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
#define CCZE_COLOR_SWAPNUM (BLUE)
#define CCZE_COLOR_PROXY_CREATE (BOLD WHITE)
#define CCZE_COLOR_PROXY_SWAPIN (BOLD WHITE)
#define CCZE_COLOR_PROXY_SWAPOUT (BOLD WHITE)
#define CCZE_COLOR_PROXY_RELEASE (BOLD WHITE)
#define CCZE_COLOR_MAC (BOLD WHITE)
#define CCZE_COLOR_VERSION (BOLD WHITE)
#define CCZE_COLOR_ADDRESS (BOLD WHITE)
#define CCZE_COLOR_NUMBERS (WHITE)
#define CCZE_COLOR_SIGNAL (BOLD YELLOW)
#define CCZE_COLOR_SERVICE (BOLD MAGENTA)
#define CCZE_COLOR_PROT (MAGENTA)
#define CCZE_COLOR_BADWORD (BOLD YELLOW)
#define CCZE_COLOR_GOODWORD (BOLD GREEN)
#define CCZE_COLOR_SYSTEMWORD (BOLD CYAN)

#define CCZE_ADDSTR(col,str) { attrset (col) ; addstr (str); }
#define ccze_space() CCZE_ADDSTR (CCZE_COLOR_DEFAULT, " ")
#define CCZE_NEWLINE() addstr ("\n")

enum
{
  CCZE_MATCH_NONE,
  CCZE_MATCH_PROCMAIL_LOG,
  CCZE_MATCH_SYSLOG,
  CCZE_MATCH_HTTPD_ACCESS_LOG,
  CCZE_MATCH_HTTPD_ERROR_LOG,
  CCZE_MATCH_SQUID_ACCESS_LOG,
  CCZE_MATCH_SQUID_CACHE_LOG,
  CCZE_MATCH_SQUID_STORE_LOG,
  CCZE_MATCH_VSFTPD_LOG,
  CCZE_MATCH_SULOG,
  CCZE_MATCH_SUPER
};

int ccze_http_action (const char *method);
void ccze_print_date (const char *date);

#endif /* !_CCZE_H */
