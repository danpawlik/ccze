/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-vsftpd.h -- VSFTPd-related colorizer prototypes for CCZE
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

#ifndef _CCZE_VSFTPD_H
#define _CCZE_VSFTPD_H 1

int ccze_vsftpd_handle (const char *str, size_t length, char **rest);
void ccze_vsftpd_setup (void);
void ccze_vsftpd_shutdown (void);

#endif /* !_CCZE_VSFTPD_H */
