/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-color.h -- Color-handling routine prototypes for CCZE
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

#ifndef _CCZE_COLOR_H
#define _CCZE_COLOR_H 1

typedef enum
{
  CCZE_COLOR_DATE = 0,
  CCZE_COLOR_HOST,
  CCZE_COLOR_PROC,
  CCZE_COLOR_PID,
  CCZE_COLOR_PIDB,
  CCZE_COLOR_DEFAULT,
  CCZE_COLOR_EMAIL,
  CCZE_COLOR_SUBJECT,
  CCZE_COLOR_DIR,
  CCZE_COLOR_SIZE,
  CCZE_COLOR_USER,
  CCZE_COLOR_HTTPCODES,
  CCZE_COLOR_GETSIZE,
  CCZE_COLOR_HTTP_GET,
  CCZE_COLOR_HTTP_POST,
  CCZE_COLOR_HTTP_HEAD,
  CCZE_COLOR_HTTP_PUT,
  CCZE_COLOR_HTTP_CONNECT,
  CCZE_COLOR_HTTP_TRACE,
  CCZE_COLOR_UNKNOWN,
  CCZE_COLOR_GETTIME,
  CCZE_COLOR_URI,
  CCZE_COLOR_IDENT,
  CCZE_COLOR_CTYPE,
  CCZE_COLOR_ERROR,
  CCZE_COLOR_PROXY_MISS,
  CCZE_COLOR_PROXY_HIT,
  CCZE_COLOR_PROXY_DENIED,
  CCZE_COLOR_PROXY_REFRESH,
  CCZE_COLOR_PROXY_SWAPFAIL,
  CCZE_COLOR_DEBUG,
  CCZE_COLOR_WARNING,
  CCZE_COLOR_PROXY_DIRECT,
  CCZE_COLOR_PROXY_PARENT,
  CCZE_COLOR_SWAPNUM,
  CCZE_COLOR_PROXY_CREATE,
  CCZE_COLOR_PROXY_SWAPIN,
  CCZE_COLOR_PROXY_SWAPOUT,
  CCZE_COLOR_PROXY_RELEASE,
  CCZE_COLOR_MAC,
  CCZE_COLOR_VERSION,
  CCZE_COLOR_ADDRESS,
  CCZE_COLOR_NUMBERS,
  CCZE_COLOR_SIGNAL,
  CCZE_COLOR_SERVICE,
  CCZE_COLOR_PROT,
  CCZE_COLOR_BADWORD,
  CCZE_COLOR_GOODWORD,
  CCZE_COLOR_SYSTEMWORD,
  CCZE_COLOR_INCOMING,
  CCZE_COLOR_OUTGOING,
  CCZE_COLOR_UNIQN,
  CCZE_COLOR_REPEAT,
  CCZE_COLOR_FIELD,
  CCZE_COLOR_CHAIN,
  CCZE_COLOR_PERCENTAGE,
  CCZE_COLOR_LAST
} ccze_color_t;

int ccze_color (ccze_color_t idx);
void ccze_color_init (void);
void ccze_color_load (const char *fn);

int ccze_color_strip_attrib (int color);
char *ccze_color_to_name_simple (int color);
char *ccze_color_lookup_name (ccze_color_t color);
char *ccze_color_to_css (int cidx);
void ccze_colors_to_css (void);
char *ccze_cssbody_color (void);

#endif /* !_CCZE_COLOR_H */
