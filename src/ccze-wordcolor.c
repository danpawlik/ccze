/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-wordcolor.c -- Word-coloriser functions
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

#include <ctype.h>
#include <curses.h>
#include <netdb.h>
#include <pcre.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

#include "ccze.h"
#include "ccze-wordcolor.h"

static pcre *reg_pre, *reg_post, *reg_host, *reg_mac, *reg_email;
static pcre *reg_uri, *reg_size, *reg_ver, *reg_time, *reg_addr;
static pcre *reg_num, *reg_sig;

static char *words_bad[] = {
  "warn", "restart", "exit", "stop", "end", "shutting", "down", "close",
  "unreach", "can't", "cannot", "skip", "deny", "disable", "ignored",
  "miss", "oops", "su", "not", "backdoor", "blocking", "ignoring",
  "unable", "readonly", "offline", "terminate"
};

static char *words_good[] = {
  "activ", "start", "ready", "online", "load", "ok", "register", "detected",
  "configured", "enable", "listen", "open", "complete", "attempt", "done",
  "check", "listen", "connect", "finish"
};

static char *words_error[] = {
  "error", "crit", "invalid", "fail", "false", "alarm"
};

static char *words_system[] = {
  "ext2-fs", "reiserfs", "vfs", "iso", "isofs", "cslip", "ppp", "bsd",
  "linux", "tcp/ip", "mtrr", "pci", "isa", "scsi", "ide", "atapi",
  "bios", "cpu", "fpu"
};

static char *
xstrdup (const char *str)
{
  if (!str)
    return NULL;
  else
    return strdup (str);
}

static char *
_stolower (const char *str)
{
  char *newstr = strdup (str);
  size_t i;

  for (i = 0; i < strlen (newstr); i++)
    newstr[i] = tolower (str[i]);

  return newstr;
}

/* FIXME: strtok cuts multiple spaces, which is not good! */
void
ccze_wordcolor_process (const char *msg, int wcol, int slookup)
{
  char *word, *tmp;
  char *msg2 = strdup (msg);
  char *pre = NULL, *post = NULL;
  int offsets[99];
  int col;
  int match;

  if (!wcol)
    {
      CCZE_ADDSTR (CCZE_COLOR_DEFAULT, msg);
      free (msg2);
      return;
    }
  
  word = xstrdup (strtok (msg2, " "));
  if (!word)
    {
      CCZE_ADDSTR (CCZE_COLOR_DEFAULT, msg);
      free (msg2);
      return;
    }
  
  do
    {
      size_t wlen;
      free (pre);
      free (post);
      col = CCZE_COLOR_DEFAULT;

      /** prefix **/
      if ((match = pcre_exec (reg_pre, NULL, word, strlen (word), 0, 0,
			      offsets, 99)) >= 0)
	{
	  pcre_get_substring (word, offsets, match, 1, (const char **)&pre);
	  pcre_get_substring (word, offsets, match, 2, (const char **)&tmp);
	  free (word);
	  word = tmp;
	}
      else
	pre = NULL;

      /** postfix **/
      if ((match = pcre_exec (reg_post, NULL, word, strlen (word), 0, 0,
			      offsets, 99)) >= 0)
	{
	  pcre_get_substring (word, offsets, match, 1, (const char **)&tmp);
	  pcre_get_substring (word, offsets, match, 2, (const char **)&post);
	  free (word);
	  word = tmp;
	}
      else
	post = NULL;

      wlen = strlen (word);
      
      /** Host **/
      if (pcre_exec (reg_host, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
	col = CCZE_COLOR_HOST;
      /** MAC address **/
      else if (pcre_exec (reg_mac, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
	col = CCZE_COLOR_MAC;
      /** Directory **/
      else if (word[0] == '/')
	col = CCZE_COLOR_DIR;
      /** E-mail **/
      else if (pcre_exec (reg_email, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
	col = CCZE_COLOR_EMAIL;
      /** URI **/
      else if (pcre_exec (reg_uri, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
      	col = CCZE_COLOR_URI;
      /** Size **/
      else if (pcre_exec (reg_size, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
      	col = CCZE_COLOR_SIZE;
      /** Version **/
      else if (pcre_exec (reg_ver, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
	col = CCZE_COLOR_VERSION;
      /** Time **/
      else if (pcre_exec (reg_time, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
	col = CCZE_COLOR_DATE;
      /** Address **/
      else if (pcre_exec (reg_addr, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
	col = CCZE_COLOR_ADDRESS;
      /** Number **/
      else if (pcre_exec (reg_num, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
	col = CCZE_COLOR_NUMBERS;
      /** Signal **/
      else if (pcre_exec (reg_sig, NULL, word, wlen, 0, 0, offsets, 99) >= 0)
	col = CCZE_COLOR_SIGNAL;
      /** Service **/
      else if (slookup && getservbyname (word, NULL))
	col = CCZE_COLOR_SERVICE;
      /** Protocol **/
      else if (slookup && getprotobyname (word))
	col = CCZE_COLOR_PROT;
      /** User **/
      else if (slookup && getpwnam (word))
	col = CCZE_COLOR_USER;
      else
	{ /* Good/Bad/System words */
	  size_t i;
	  char *lword = _stolower (word);
	  
	  for (i = 0; i < sizeof (words_bad) / sizeof (char *); i++)
	    {
	      if (strstr (lword, words_bad[i]))
		col = CCZE_COLOR_BADWORD;
	    }
	  for (i = 0; i < sizeof (words_good) / sizeof (char *); i++)
	    {
	      if (strstr (lword, words_good[i]))
		col = CCZE_COLOR_GOODWORD;
	    }
	  for (i = 0; i < sizeof (words_error) / sizeof (char *); i++)
	    {
	      if (strstr (lword, words_error[i]))
		col = CCZE_COLOR_ERROR;
	    }
	  for (i = 0; i < sizeof (words_system) / sizeof (char *); i++)
	    {
	      if (strstr (lword, words_system[i]))
		col = CCZE_COLOR_SYSTEMWORD;
	    }

	  free (lword);
	}
      
      CCZE_ADDSTR (CCZE_COLOR_DEFAULT, pre);
      CCZE_ADDSTR (col, word);
      CCZE_ADDSTR (CCZE_COLOR_DEFAULT, post);
      ccze_space ();
      
      free (word);
    } while ((word = xstrdup (strtok (NULL, " "))) != NULL);

  free (msg2);
  
  return;
}

void
ccze_wordcolor_setup (void)
{
  const char *error;
  int errptr;

  reg_pre = pcre_compile ("^([`'\".,!?:;(\\[{<]+)([^`'\".,!?:;(\\[{<]\\S+)$",
			  0, &error, &errptr, NULL);
  reg_post = pcre_compile ("^(\\S+[^`'\".,!?:;)\\]}>])([`'\".,!?:;)\\]}>]+)$",
			   0, &error, &errptr, NULL);
  reg_host = pcre_compile ("^(((\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})|"
			   "(([a-z0-9-_]+\\.)+[a-z]{2,3})|(localhost)|"
			   "(\\w*::\\w+)+)(:\\d{1,5})?)$", 0, &error,
			   &errptr, NULL);
  reg_mac = pcre_compile ("^([0-9a-f]{2}:){5}[0-9a-f]{2}$", 0, &error,
			  &errptr, NULL);
  reg_email = pcre_compile ("^[a-z0-9-_]+@([a-z0-9-_\\.]+)+(\\.[a-z]{2,3})?$",
			    0, &error, &errptr, NULL);
  reg_uri = pcre_compile ("^\\w{2,}:\\/\\/(\\S+\\/?)+$", 0, &error,
			  &errptr, NULL);
  reg_size = pcre_compile ("^\\d+(\\.\\d+)?[k|K|m|M|g|G|t|T]i?B?(ytes?)?",
			   0, &error, &errptr, NULL);
  reg_ver = pcre_compile ("^v?(\\d+\\.){1}((\\d|[a-z])+\\.)*(\\d|[a-z])+$",
			  0, &error, &errptr, NULL);
  reg_time = pcre_compile ("\\d{1,2}:\\d{1,2}(:\\d{1,2})?", 0, &error,
			   &errptr, NULL);
  reg_addr = pcre_compile ("^0x(\\d|[a-f])+$", 0, &error, &errptr, NULL);
  reg_num = pcre_compile ("^-?\\d+$", 0, &error, &errptr, NULL);
  reg_sig = pcre_compile ("^SIG(HUP|INT|QUIT|ILL|ABRT|FPE|KILL|SEGV|PIPE|"
			  "ALRM|TERM|USR1|USR2|CHLD|CONT|STOP|TSTP|TIN|TOUT|"
			  "BUS|POLL|PROF|SYS|TRAP|URG|VTALRM|XCPU|XFSZ|IOT|"
			  "EMT|STKFLT|IO|CLD|PWR|INFO|LOST|WINCH|UNUSED)", 0,
			  &error, &errptr, NULL);
}

void
ccze_wordcolor_shutdown (void)
{
  free (reg_pre);
  free (reg_post);
  free (reg_host);
  free (reg_mac);
  free (reg_email);
  free (reg_uri);
  free (reg_size);
  free (reg_ver);
  free (reg_time);
  free (reg_addr);
  free (reg_num);
  free (reg_sig);
}
