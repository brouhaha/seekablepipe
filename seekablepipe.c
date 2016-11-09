/*
  seekablepipe - allows piping input to programs that seek on stdin

  Copyright 2014, 2016 Eric Smith <spacewar@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of version 3 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// The following definition is necessary for fcntl.h to provide splice()
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

const char *progname;

void fatal (int result, char *fmt, ...) __attribute__ ((noreturn));
void fatal (int result, char *fmt, ...)
{
  va_list ap;
  if (fmt)
    {
      if (result != EX_USAGE)
	fprintf (stderr, "%s: ", progname);
      va_start (ap, fmt);
      vfprintf (stderr, fmt, ap);
      va_end (ap);
    }
  exit (result);
}

void warning (char *fmt, ...)
{
  va_list ap;
  fprintf (stderr, "%s: ", progname);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
}

void usage (char *fmt, ...) __attribute__ ((noreturn));
void usage (char *fmt, ...)
{
  va_list ap;
  if (fmt)
    {
      fprintf (stderr, "%s: ", progname);
      va_start (ap, fmt);
      vfprintf (stderr, fmt, ap);
      va_end (ap);
    }
  fatal (EX_USAGE,
	 "Usage:\n"
	 "%s [-p prefix] command [arg]...\n", progname);
}

int main (int argc, char * const argv[])
{
  int temp_wr_fd;
  int temp_rd_fd;
  const char *temp_fn_prefix = "/tmp/sp";
  const char *temp_fn_suffix = "XXXXXX";
  char *temp_fn = NULL;

  progname = *(argv++);
  argc--;

  while (argc && (argv[0][0] == '-'))
    {
      if (strcmp (argv[0], "-p") == 0)
	{
	  if (argc < 1)
	    usage ("missing argument to -p option\n");
          argv++;
          argc--;
	  temp_fn_prefix = argv[0];
	}
      else
	usage ("unrecognized '%s' option\n", argv[0]);
      argv++;
      argc--;
    }

  // generate temp filename by merging prefix and suffix
  temp_fn = calloc (1, strlen (temp_fn_prefix) + strlen (temp_fn_suffix) + 1);
  if (! temp_fn)
    fatal (EX_OSERR, "unable to allocate memory\n");
  strcpy (temp_fn, temp_fn_prefix);
  strcat (temp_fn, temp_fn_suffix);

  // create temp file and open for writing (and reading, but we won't do that)
  temp_wr_fd = mkstemp (temp_fn);
  if (0 > temp_wr_fd)
    fatal (EX_CANTCREAT, "unable to create temp file: %s\n", strerror (errno));

  // open temp file as another fd for reading
  temp_rd_fd = open (temp_fn, O_RDONLY);
  if (0 > temp_wr_fd)
    fatal (EX_CANTCREAT, "unable to create temp file: %s\n", strerror (errno));

  // unlink temp file so it will be deleted when it gets closed
  // (by the program we exec)
  //if (0 > unlink (temp_fn))
  //  warning ("unlink of temp file failed: %s\n", strerror (errno));

  // copy stdin to temp file
  ssize_t splice_result;
  splice_result = splice (STDIN_FILENO,  // int fd_in
			  NULL,          // off64_t *off_in
			  temp_wr_fd,       // int fd_out
			  NULL,          // off64_t *off_out
			  ((size_t) -1) >> 1, // size_t len, but fails if MSB set, maybe kernel treats it as ssize_t?
			  0);            // unsigned int flags
  if (0 > splice_result)
    fatal (EX_IOERR, "splice error: %s\n", strerror (errno));

  // close writable temp file
  if (0 > close(temp_wr_fd))
    warning ("unable to close temp file: %s\n", strerror (errno));

  // redirect stdin from temp file
  if (0 > dup2 (temp_rd_fd, STDIN_FILENO))
    fatal (EX_OSERR, "unable to replace stdin: %s\n", strerror (errno));

  // close readable temp file fd
  if (0 > close (temp_rd_fd))
    warning ("unable to close temp file fd: %s\n", strerror (errno));

  (void) execvp (argv [0], argv);
  // execvp() won't return unless there's an error
  fatal (EX_OSERR, "execvp error: %s\n", strerror (errno));
}
