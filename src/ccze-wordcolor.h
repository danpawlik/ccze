/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-wordcolor.h -- Word-coloriser prototypes
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

#ifndef _CCZE_WORDCOLOR_H
#define _CCZE_WORDCOLOR_H 1

#include <pcre.h>

void ccze_wordcolor_process (const char *msg, int wcol, int slookup);
void ccze_wordcolor_setup (void);
void ccze_wordcolor_shutdown (void);

#endif /* !_CCZE_WORDCOLOR_H */
