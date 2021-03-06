/*-
 * Copyright (c) 2003-2007 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "test.h"
__FBSDID("$FreeBSD$");

DEFINE_TEST(test_option_u)
{
	struct timeval times[2];
	char *p;
	size_t s;
	int fd;
	int r;

	/* Create a file. */
	fd = open("f", O_CREAT | O_WRONLY, 0644);
	assert(fd >= 0);
	assertEqualInt(1, write(fd, "a", 1));
	close(fd);

	/* Copy the file to the "copy" dir. */
	r = systemf("echo f | %s -pd --quiet copy >copy.out 2>copy.err",
	    testprog);
	assertEqualInt(r, 0);

	/* Check that the file contains only a single "a" */
	p = slurpfile(&s, "copy/f");
	assertEqualInt(s, 1);
	assertEqualMem(p, "a", 1);

	/* Recreate the file with a single "b" */
	fd = open("f", O_CREAT | O_TRUNC | O_WRONLY, 0644);
	assert(fd >= 0);
	assertEqualInt(1, write(fd, "b", 1));
	close(fd);

	/* Set the mtime to the distant past. */
	memset(times, 0, sizeof(times));
	times[0].tv_sec = 1; /* atime = 1.000000002 */
	times[0].tv_usec = 2;
	times[1].tv_sec = 3; /* mtime = 3.000000004 */
	times[1].tv_usec = 4;
	assertEqualInt(0, utimes("f", times));

	/* Copy the file to the "copy" dir. */
	r = systemf("echo f | %s -pd --quiet copy >copy.out 2>copy.err",
	    testprog);
	assertEqualInt(r, 0);

	/* Verify that the file hasn't changed (it wasn't overwritten) */
	p = slurpfile(&s, "copy/f");
	assertEqualInt(s, 1);
	assertEqualMem(p, "a", 1);

	/* Copy the file to the "copy" dir with -u (force) */
	r = systemf("echo f | %s -pud --quiet copy >copy.out 2>copy.err",
	    testprog);
	assertEqualInt(r, 0);

	/* Verify that the file has changed (it was overwritten) */
	p = slurpfile(&s, "copy/f");
	assertEqualInt(s, 1);
	assertEqualMem(p, "b", 1);
}
