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

#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ccze.h"
#include "ccze-plugin.h"

#define PLUGIN_LIBPATH PKGLIBDIR

static ccze_plugin_t **plugins;
static size_t plugins_alloc, plugins_len;

void
ccze_plugin_add (ccze_plugin_t *plugin)
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

static void
_ccze_plugin_load (const char *name, const char *path)
{
  ccze_plugin_t *plugin;
  char *tmp;
  void *dlhandle;

  dlhandle = dlopen (path, RTLD_LAZY);
  if (dlerror () || !dlhandle)
    return;

  asprintf (&tmp, "ccze_%s_info", name);
  plugin = (ccze_plugin_t *)dlsym (dlhandle, tmp);
  free (tmp);
  if (dlerror () || !plugin)
    {
      dlclose (dlhandle);
      return;
    }
  plugin->dlhandle = dlhandle;
  
  ccze_plugin_add (plugin);
}

void
ccze_plugin_load (const char *name)
{
  char *home;
  char *path;

  if ((home = getenv ("HOME")) != NULL)
    {
      asprintf (&path, "%s/.ccze/%s.so", home, name);
      if (access (path, F_OK))
	{
	  free (path);
	  asprintf (&path, PLUGIN_LIBPATH "/%s.so", name);
	}
    }
  else
    asprintf (&path, PLUGIN_LIBPATH "/%s.so", name);

  _ccze_plugin_load (name, path);
  free (path);
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
_ccze_plugin_load_set (struct dirent ***namelist, int nn, const char *base)
{
  int m, n = nn;

  m = 0;
  while (m < n)
    {
      char *tmp = strdup ((*namelist)[m]->d_name);
      char *tmp2 = strstr (tmp, ".so");
      char *path;
      tmp2[0] = '\0';

      if (!_ccze_plugin_loaded (tmp))
	{
	  asprintf (&path, "%s/%s.so", base, tmp);
	  _ccze_plugin_load (tmp, path);
	  free (path);
	}
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
      asprintf (&homeplugs, "%s/.ccze", home);
      n = scandir (homeplugs, &namelist, _ccze_plugin_select, alphasort);
      if (n != -1)
	_ccze_plugin_load_set (&namelist, n, homeplugs);
      free (homeplugs);
    }

  n = scandir (PLUGIN_LIBPATH, &namelist, _ccze_plugin_select, alphasort);
  if (n != -1)
    _ccze_plugin_load_set (&namelist, n, PLUGIN_LIBPATH);
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
      if (plugins[i])
	{
	  (*(plugins[i]->shutdown)) ();
	  if (plugins[i]->dlhandle)
	    dlclose (plugins[i]->dlhandle);
	}
    }
  free (plugins);
}

void
ccze_plugin_run (ccze_plugin_t **pluginset, char *subject, size_t subjlen,
		 char **rest, ccze_plugin_type_t type, int *handled,
		 int *status)
{
  int i = 0;
  
  while (pluginset[i])
    {
      if (pluginset[i]->type == type)
	if ((*handled = (*(pluginset[i]->handler))
	     (subject, subjlen, rest)) != 0)
	  {
	    *status = *handled;
	    break;
	  }
      i++;
    }
}
