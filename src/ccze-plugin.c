/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-plugin.c -- Plugin interface for CCZE.
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

#include <sys/types.h>
#include <ccze.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ccze-private.h"

#define PLUGIN_LIBPATH PKGLIBDIR

static ccze_plugin_t **plugins;
static size_t plugins_alloc, plugins_len;
static ccze_plugin_t **plugin_args;
static size_t plugin_args_alloc, plugin_args_len;

static int
_ccze_plugin_allow (const char *name)
{
  int i = 0;
  int rval = 0;

  if (ccze_config.pluginlist_len == 0)
    return 1;
  
  while (i < ccze_config.pluginlist_len)
    {
      if (!strcmp (ccze_config.pluginlist[i], name))
	{
	  rval = 1;
	  break;
	}
      i++;
    }

  return rval;
}

void
ccze_plugin_add (ccze_plugin_t *plugin)
{
  if (!_ccze_plugin_allow (plugin->name))
    return;
  
  plugins[plugins_len] = plugin;
  plugins_len++;
  if (plugins_len >= plugins_alloc)
    {
      plugins_alloc *= 2;
      plugins = (ccze_plugin_t **)ccze_realloc
	(plugins, plugins_alloc * sizeof (ccze_plugin_t *));
    }
}

void
ccze_plugin_init (void)
{
  plugins_alloc = 10;
  plugins_len = 0;
  plugins = (ccze_plugin_t **)ccze_calloc (plugins_alloc,
					   sizeof (ccze_plugin_t *));
}

void
ccze_plugin_argv_init (void)
{
  plugin_args_alloc = 10;
  plugin_args_len = 0;
  plugin_args = (ccze_plugin_t **)ccze_calloc (plugin_args_alloc,
					       sizeof (ccze_plugin_t *));
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
  if (plugin->abi_version != CCZE_ABI_VERSION)
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

static ccze_plugin_t *
_ccze_plugin_find (const char *name)
{
  size_t i;
  
  for (i = 0; i < plugins_len; i++)
    {
      if (!strcmp (plugins[i]->name, name))
	return plugins[i];
    }
  return NULL;
}

static int
_ccze_plugin_loaded (const char *name)
{
  if (_ccze_plugin_find (name))
    return 1;

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
	  free (plugins[i]->argv);
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
      if (pluginset[i]->type == type ||
	  pluginset[i]->type == CCZE_PLUGIN_TYPE_ANY)
	if ((*handled = (*(pluginset[i]->handler))
	     (subject, subjlen, rest)) != 0)
	  {
	    *status = *handled;
	    break;
	  }
      i++;
    }
}

char **
ccze_plugin_argv_get (const char *name)
{
  ccze_plugin_t *p = _ccze_plugin_find (name);

  if (!p)
    return NULL;
  return p->argv;
}

int
ccze_plugin_argv_set (const char *name, const char *args)
{
  ccze_plugin_t *p = NULL;
  size_t i, j = 1;
  char *args_copy, *arg;

  if (!args || !name)
    return -1;
  
  for (i = 0; i < plugin_args_len; i++)
    {
      if (!strcmp (plugin_args[i]->name, name))
	p = plugin_args[i];
    }

  if (p)
    {
      free (p->argv);
      p->argv = NULL;
    }
  else
    {
      p = (ccze_plugin_t *)ccze_malloc (sizeof (ccze_plugin_t));
      p->name = strdup (name);
      p->argv = NULL;
      i = plugin_args_len++;
      if (plugin_args_len >= plugin_args_alloc)
	{
	  plugin_args_alloc *= 2;
	  plugin_args = (ccze_plugin_t **)ccze_realloc
	    (plugin_args, plugin_args_alloc * sizeof (ccze_plugin_t *));
	}
    }

  args_copy = strdup (args);
  arg = strtok (args_copy, " \t\n");
  do
    {
      p->argv = (char **)ccze_realloc (p->argv, (j + 1) * sizeof (char *));
      p->argv[j++] = strdup (arg);
    } while ((arg = strtok (NULL, " \t\n")) != NULL);
  p->argv = (char **)ccze_realloc (p->argv, (j + 1) * sizeof (char *));
  p->argv[j] = NULL;
  plugin_args[i] = p;
  free (args_copy);
  
  return 1;
}

void
ccze_plugin_argv_finalise (void)
{
  ccze_plugin_t *p;
  size_t i;

  for (i = 0; i < plugin_args_len; i++)
    {
      p = _ccze_plugin_find (plugin_args[i]->name);
      if (p)
	p->argv = plugin_args[i]->argv;
      free (plugin_args[i]);
    }
  free (plugin_args);
}
