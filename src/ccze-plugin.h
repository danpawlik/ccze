/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-plugin.h -- Plugin interface for CCZE.
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

#ifndef _CCZE_PLUGIN_H
#define _CCZE_PLUGIN_H 1

#define CCZE_DEFINE_PLUGIN(name,quoted,type) \
ccze_plugin_t ccze_##name##_info = { NULL, \
				     quoted, \
				     ccze_##name##_setup, \
				     ccze_##name##_shutdown, \
				     ccze_##name##_handle, \
				     CCZE_PLUGIN_TYPE_##type }

typedef void (*ccze_plugin_startup_t) (void);
typedef void (*ccze_plugin_shutdown_t) (void);
typedef int (*ccze_plugin_handle_t) (const char *str, size_t length,
				     char **rest);

typedef enum
{
  CCZE_PLUGIN_TYPE_FULL,
  CCZE_PLUGIN_TYPE_PARTIAL,
} ccze_plugin_type_t;

typedef struct
{
  void *dlhandle;
  char *name;
  ccze_plugin_startup_t startup;
  ccze_plugin_shutdown_t shutdown;
  ccze_plugin_handle_t handler;
  ccze_plugin_type_t type;
} ccze_plugin_t;

void ccze_plugin_init (void);
ccze_plugin_t **ccze_plugins (void);
void ccze_plugin_load_all (void);
void ccze_plugin_load (const char *name);
void ccze_plugin_add (ccze_plugin_t *plugin);
void ccze_plugin_shutdown (void);
void ccze_plugin_finalise (void);
void ccze_plugin_run (ccze_plugin_t **pluginset, char *subject,
		      size_t subjlen, char **rest,
		      ccze_plugin_type_t type, int *handled,
		      int *status);
void ccze_plugin_load_all_builtins (void);

#endif /* !_CCZE_PLUGIN_H */
