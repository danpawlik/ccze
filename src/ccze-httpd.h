/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-httpd.h -- HTTPd-related colorizer prototypes for CCZE
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

#ifndef _CCZE_HTTPD_H
#define _CCZE_HTTPD_H 1

#include <pcre.h>

char *ccze_httpd_access_log_process (const char *str, int *offsets,
				     int match);
void ccze_httpd_setup (pcre **r, pcre_extra **h);

#endif /* !_CCZE_HTTPD_H */
