/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-compat.c -- OS compatibility stuff for CCZE
 * Copyright (C) 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
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

#include "system.h"
#include "ccze-compat.h"

#include <ctype.h>
#include <errno.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define XSREALLOC(ptr, type, nmemb) \
	ptr = (type*) realloc (ptr, nmemb * sizeof (type))

#ifndef HAVE_STRNDUP
char *
strndup (const char *s, size_t size)
{
  char *ns = (char *)malloc (size + 1);
  memcpy (ns, s, size);
  ns[size] = '\0';
  return ns;
}
#endif

#ifndef HAVE_ARGP_PARSE
error_t
argp_parse (const struct argp *argps, int argc, char **argv,
	    unsigned flags, int arg_index, void *input)
{
  char *options;
  int optpos = 0, optionspos = 0, optionssize = 30;
  struct argp_state *state;
  int c;
  
  state = (struct argp_state *)malloc (sizeof (struct argp_state));
  state->input = input;

  options = (char *)calloc (optionssize, sizeof (char *));
  while (argps->options[optpos].name != NULL)
  {
    if (optionspos >= optionssize)
      {
	optionssize *= 2;
	XSREALLOC (options, char, optionssize);
      }
    options[optionspos++] = (char) argps->options[optpos].key;
    if (argps->options[optpos].arg)
      options[optionspos++] = ':';
    optpos++;
  }
  if (optionspos + 4 >= optionssize)
    {
      optionssize += 5;
      XSREALLOC (options, char, optionssize);
    }
  options[optionspos++] = 'V';
  options[optionspos++] = '?';
  options[optionspos] = '\0';

  while ((c = getopt (argc, argv, options)) != -1)
    {
      switch (c)
	{
	case '?':
	  if ((optopt != c) && (optopt != '?'))
	    {
	      fprintf (stderr, "Try `%s -?' for more information.\n",
		       argp_program_name);
	      exit (1);
	    }
	  printf ("Usage: %s [OPTION...]\n%s\n\n", argp_program_name,
		  argps->doc);
	  optpos = 0;
	  while (argps->options[optpos].name != NULL)
	    {
	      if (!(argps->options[optpos].flags & OPTION_HIDDEN))
		printf ("  -%c %s\t\t%s\n", argps->options[optpos].key,
			(argps->options[optpos].arg) ?
			argps->options[optpos].arg : "",
			argps->options[optpos].doc);
	      optpos++;
	    }
	  printf ("\nReport bugs to %s.\n", argp_program_bug_address);
	  exit (0);
	  break;
	case 'V':
	  printf ("%s\n", argp_program_version);
	  exit (0);
	  break;
	default:
	  argps->parser (c, optarg, state);
	  break;
	}
    }
  free (state);
  free (options);
  return 0;
}

error_t
argp_error (const struct argp_state *state, char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fprintf (stderr, "%s: ", argp_program_name);
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\nTry `%s -?' for more information.\n", argp_program_name);
  exit (1);
}
#endif

#ifndef HAVE_GETSUBOPT
int getsubopt (char **optionp, char *const *tokens,
	       char **valuep)
{
  char *endp, *vstart;
  int cnt;

  if (**optionp == '\0')
    return -1;

  /* Find end of next token.  */
  endp = strchr (*optionp, ',');
  if (!endp)
    endp = *optionp + strlen (*optionp);
   
  /* Find start of value.  */
  vstart = memchr (*optionp, '=', endp - *optionp);

  if (vstart == NULL)
    vstart = endp;

  /* Try to match the characters between *OPTIONP and VSTART against
     one of the TOKENS.  */
  for (cnt = 0; tokens[cnt] != NULL; ++cnt)
    if (memcmp (*optionp, tokens[cnt], vstart - *optionp) == 0
	&& tokens[cnt][vstart - *optionp] == '\0')
      {
	/* We found the current option in TOKENS.  */
	*valuep = vstart != endp ? vstart + 1 : NULL;

	if (*endp != '\0')
	  *endp++ = '\0';
	*optionp = endp;

	return cnt;
      }

  /* The current suboption does not match any option.  */
  *valuep = *optionp;

  if (*endp != '\0')
    *endp++ = '\0';
  *optionp = endp;

  return -1;
}
#endif

#ifndef HAVE_SCANDIR
int
scandir (const char *dir, struct dirent ***namelist,
	 int (*select)(const struct dirent *),
	 int (*compar)(const struct dirent **, const struct dirent **))
{
  DIR *d;
  struct dirent *entry;
  register int i=0;
  size_t entrysize;

  if ((d = opendir (dir)) == NULL)
    return -1;

  *namelist=NULL;
  while ((entry = readdir (d)) != NULL)
    {
      if (select == NULL || (select != NULL && (*select) (entry)))
	{
	  *namelist = (struct dirent **)realloc
	    ((void *) (*namelist),
	     (size_t)((i + 1) * sizeof (struct dirent *)));
	  if (*namelist == NULL)
	    return -1;

	  entrysize = sizeof (struct dirent) -
	    sizeof (entry->d_name) + strlen (entry->d_name) + 1;
	  (*namelist)[i] = (struct dirent *)malloc (entrysize);
	  if ((*namelist)[i] == NULL)
	    return -1;
         memcpy ((*namelist)[i], entry, entrysize);
         i++;
	}
    }
  if (closedir (d))
    return -1;
  if (i == 0)
    return -1;
  if (compar != NULL)
    qsort ((void *)(*namelist), (size_t) i, sizeof (struct dirent *),
	   compar);

   return i;
}
#endif

#ifndef HAVE_ALPHASORT
int
alphasort (const struct dirent **a, const struct dirent **b)
{
  return (strcmp ((*a)->d_name, (*b)->d_name));
}
#endif

/* getline() and getdelim() were taken from GNU Mailutils'
   mailbox/getline.c */
/* First implementation by Alain Magloire */
#ifndef HAVE_GETLINE
ssize_t
getline (char **lineptr, size_t *n, FILE *stream)
{
  return getdelim (lineptr, n, '\n', stream);
}
#endif

#ifndef HAVE_GETDELIM
/* Default value for line length.  */
static const int line_size = 128;

ssize_t
getdelim (char **lineptr, size_t *n, int delim, FILE *stream)
{
  size_t indx = 0;
  int c;

  /* Sanity checks.  */
  if (lineptr == NULL || n == NULL || stream == NULL)
    return -1;

  /* Allocate the line the first time.  */
  if (*lineptr == NULL)
    {
      *lineptr = malloc (line_size);
      if (*lineptr == NULL)
	return -1;
      *n = line_size;
    }

  while ((c = getc (stream)) != EOF)
    {
      /* Check if more memory is needed.  */
      if (indx >= *n)
	{
	  *lineptr = realloc (*lineptr, *n + line_size);
	  if (*lineptr == NULL)
	    return -1;
	  *n += line_size;
	}

      /* Push the result in the line.  */
      (*lineptr)[indx++] = c;

      /* Bail out.  */
      if (c == delim)
	break;
    }

  /* Make room for the null character.  */
  if (indx >= *n)
    {
      *lineptr = realloc (*lineptr, *n + line_size);
      if (*lineptr == NULL)
       return -1;
      *n += line_size;
    }

  /* Null terminate the buffer.  */
  (*lineptr)[indx++] = 0;

  /* The last line may not have the delimiter, we have to
   * return what we got and the error will be seen on the
   * next iteration.  */
  return (c == EOF && (indx - 1) == 0) ? -1 : (ssize_t)(indx - 1);
}
#endif

#ifndef HAVE_ASPRINTF
int
asprintf(char **ptr, const char *fmt, ...) 
{
  va_list ap;
  size_t size = 1024;
  int n;
  
  if ((*ptr = malloc (size)) == NULL)
    return -1;

  while (1)
    {
      va_start (ap, fmt);
      n = vsnprintf (*ptr, size, fmt, ap);
      va_end (ap);

      if (n > -1 && n < (long) size)
	return n;

      if (n > -1)    /* glibc 2.1 */
	size = n+1;
      else           /* glibc 2.0 */
	size *= 2;

      if ((*ptr = realloc (*ptr, size)) == NULL)
	return -1;
    }
}
#endif
