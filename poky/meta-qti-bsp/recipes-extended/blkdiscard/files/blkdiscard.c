/*
 * blkdiscard.c -- discard the part (or whole) of the block device.
 *
 * Copyright (C) 2012 Red Hat, Inc. All rights reserved.
 * Written by Lukas Czerner <lczerner@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This program uses BLKDISCARD ioctl to discard part or the whole block
 * device if the device supports it. You can specify range (start and
 * length) to be discarded, or simply discard the whole device.
 */


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/fs.h>

#ifndef BLKDISCARD
# define BLKDISCARD	_IO(0x12,119)
#endif

#ifndef BLKSECDISCARD
# define BLKSECDISCARD	_IO(0x12,125)
#endif

#ifndef BLKZEROOUT
# define BLKZEROOUT	_IO(0x12,127)
#endif

#ifndef PRIu64
#define PRIu64 "llu"
#endif

enum {
	ACT_DISCARD = 0,	/* default */
	ACT_ZEROOUT,
	ACT_SECURE
};

static void __attribute__((__noreturn__)) errtryhelp(int exit_code)
{
	FILE *out = stdout;
	fputs("Try blkdiscard --help\n", out);
	exit(exit_code);
}

static void __attribute__((__noreturn__)) usage(void)
{
	FILE *out = stdout;
	fputs("blkdiscard [options] <device>\n", out);
	fputs("Discard the content of sectors on a device.\n", out);
	fputs(" -s, --secure        perform secure discard\n", out);
	fputs(" -z, --zeroout       zero-fill rather than discard\n", out);
	fputs(" -v, --verbose       print aligned length and offset\n", out);
	exit(EXIT_SUCCESS);
}


int main(int argc, char **argv)
{
	char *path;
	int c, fd, verbose = 0, secsize;
	uint64_t end, blksize, step, range[2], stats[2];
	struct stat sb;
	int act = ACT_DISCARD;

	static const struct option longopts[] = {
	    { "help",      no_argument,       NULL, 'h' },
	    { "secure",    no_argument,       NULL, 's' },
	    { "verbose",   no_argument,       NULL, 'v' },
	    { "zeroout",   no_argument,       NULL, 'z' },
	    { NULL, 0, NULL, 0 }
	};

	range[0] = 0;
	range[1] = ULLONG_MAX;
	step = 0;

	while ((c = getopt_long(argc, argv, "hsvo:l:p:z", longopts, NULL)) != -1) {
		switch(c) {
		case 'h':
			usage();
			break;
		case 's':
			act = ACT_SECURE;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'z':
			act = ACT_ZEROOUT;
			break;
		default:
			errtryhelp(EXIT_FAILURE);
		}
	}

	if (optind == argc)
		errx(EXIT_FAILURE, "no device specified");

	path = argv[optind++];

	if (optind != argc) {
		warnx("unexpected number of arguments");
		errtryhelp(EXIT_FAILURE);
	}

	fd = open(path, O_WRONLY);
	if (fd < 0)
		err(EXIT_FAILURE, "cannot open %s", path);

	if (fstat(fd, &sb) == -1)
		err(EXIT_FAILURE, "stat of %s failed", path);
	if (!S_ISBLK(sb.st_mode))
		errx(EXIT_FAILURE, "%s: not a block device", path);

	if (ioctl(fd, BLKGETSIZE64, &blksize))
		err(EXIT_FAILURE, "%s: BLKGETSIZE64 ioctl failed", path);
	if (ioctl(fd, BLKSSZGET, &secsize))
		err(EXIT_FAILURE, "%s: BLKSSZGET ioctl failed", path);

	/* check offset alignment to the sector size */
	if (range[0] % secsize)
		errx(EXIT_FAILURE, "%s: offset %" PRIu64 " is not aligned "
			 "to sector size %i", path, range[0], secsize);

	/* is the range end behind the end of the device ?*/
	if (range[0] > blksize)
		errx(EXIT_FAILURE, "%s: offset is greater than device size", path);
	end = range[0] + range[1];
	if (end < range[0] || end > blksize)
		end = blksize;

	range[1] = (step > 0) ? step : end - range[0];

	/* check length alignment to the sector size */
	if (range[1] % secsize)
		errx(EXIT_FAILURE, "%s: length %" PRIu64 " is not aligned "
			 "to sector size %i", path, range[1], secsize);

	stats[0] = range[0], stats[1] = 0;

	for (/* nothing */; range[0] < end; range[0] += range[1]) {
		if (range[0] + range[1] > end)
			range[1] = end - range[0];

		switch (act) {
		case ACT_ZEROOUT:
			if (ioctl(fd, BLKZEROOUT, &range))
				 err(EXIT_FAILURE, "%s: BLKZEROOUT ioctl failed", path);
			break;
		case ACT_SECURE:
			if (ioctl(fd, BLKSECDISCARD, &range))
				err(EXIT_FAILURE, "%s: BLKSECDISCARD ioctl failed", path);
			break;
		case ACT_DISCARD:
			if (ioctl(fd, BLKDISCARD, &range))
				err(EXIT_FAILURE, "%s: BLKDISCARD ioctl failed", path);
			break;
		}

		stats[1] += range[1];
	}

	close(fd);
	return EXIT_SUCCESS;
}
