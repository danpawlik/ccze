/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-plugin.c -- Plugin interface for CCZE.
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

#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ccze.h"
#include "ccze-plugin.h"

#define PLUGIN_LIBPATH PKGLIBDIR "/"

static ccze_plugin_t **plugins;
static size_t plugins_alloc, plugins_len;

static void
_ccze_plugin_add (ccze_plugin_t *plugin)
{
  plugins[plugins_len] = plugin;
  plugins_len++;
  if (plugins_len >= plugins_alloc)
    {
      plugins_alloc *= 2;
      plugins = (ccze_plugin_t **)realloc
	(plugins, plugins_alloc * sizeof (ccze_plugin_t *));
    }
}

void
ccze_plugin_init (void)
{
  plugins_alloc = 10;
  plugins_len = 0;
  plugins = (ccze_plugin_t **)calloc (plugins_alloc, sizeof (ccze_plugin_t *));
}

void
ccze_plugin_load (const char *name)
{
  ccze_plugin_t *plugin;
  char *tmp;
  char *home;
  
  plugin = (ccze_plugin_t *)malloc (sizeof (ccze_plugin_t));

  if ((home = getenv ("HOME")) != NULL)
    {
      asprintf (&tmp, "%s/.ccze/%s.so", home, name);
      if (access (tmp, F_OK))
	{
	  free (tmp);
	  asprintf (&tmp, PLUGIN_LIBPATH "%s.so", name);
	}
    }
  else
    asprintf (&tmp, PLUGIN_LIBPATH "%s.so", name);

  plugin->dlhandle = dlopen (tmp, RTLD_LAZY);
  free (tmp);
  if (dlerror () || !plugin->dlhandle)
    {
      free (plugin);
      return;
    }

  plugin->name = strdup (name);
  asprintf (&tmp, "ccze_%s_setup", name);
  plugin->startup = (ccze_plugin_startup_t)dlsym (plugin->dlhandle,
						  tmp);
  free (tmp);
  if (dlerror ())
    {
      free (plugin->name);
      dlclose (plugin->dlhandle);
      free (plugin);
      return;
    }
  
  asprintf (&tmp, "ccze_%s_shutdown", name);
  plugin->shutdown = (ccze_plugin_shutdown_t)dlsym (plugin->dlhandle,
						    tmp);
  free (tmp);
  if (dlerror ())
    {
      free (plugin->name);
      dlclose (plugin->dlhandle);
      free (plugin);
      return;
    }
  
  asprintf (&tmp, "ccze_%s_handle", name);
  plugin->handler = (ccze_plugin_handle_t)dlsym (plugin->dlhandle, tmp);
  free (tmp);
  if (dlerror ())
    {
      free (plugin->name);
      dlclose (plugin->dlhandle);
      free (plugin);
      return;
    }

  _ccze_plugin_add (plugin);
}

static int
_ccze_plugin_select (const struct dirent *de)
{
  if (strstr (de->d_name, ".so"))
    return 1;
  return 0;
}

static int
_ccze_plugin_loaded (const char *name)
{
  size_t i;
  
  for (i = 0; i < plugins_len; i++)
    {
      if (!strcmp (plugins[i]->name, name))
	return 1;
    }
  return 0;
}

static void
_ccze_plugin_load_set (struct dirent ***namelist, int nn)
{
  int m, n = nn;

  m = 0;
  while (m < n)
    {
      char *tmp = strdup ((*namelist)[m]->d_name);
      char *tmp2 = strstr (tmp, ".so");
      tmp2[0] = '\0';

      if (!_ccze_plugin_loaded (tmp))
	ccze_plugin_load (tmp);
      free (tmp);
      free ((*namelist)[m]);
      m++;
    }
  free (*namelist);
}
		       
void
ccze_plugin_load_all (void)
{
  struct dirent **namelist;
  int n;
  char *homeplugs, *home;
  
  if ((home = getenv ("HOME")) != NULL)
    {
      asprintf (&homeplugs, "%s/.ccze/", home);
      n = scandir (homeplugs, &namelist, _ccze_plugin_select, alphasort);
      if (n != -1)
	_ccze_plugin_load_set (&namelist, n);
      free (homeplugs);
    }

  n = scandir (PLUGIN_LIBPATH, &namelist, _ccze_plugin_select, alphasort);
  if (n != -1)
    _ccze_plugin_load_set (&namelist, n);
}

void
ccze_plugin_finalise (void)
{
  plugins[plugins_len] = NULL;
}

ccze_plugin_t **
ccze_plugins (void)
{
  return plugins;
}

void
ccze_plugin_shutdown (void)
{
  size_t i;

  for (i = 0; i < plugins_len; i++)
    {
      (*(plugins[i]->shutdown)) ();
      free (plugins[i]->name);
      dlclose (plugins[i]->dlhandle);
      free (plugins[i]);
    }
  free (plugins);
}
