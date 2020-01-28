// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 Synopsys, Inc. and/or its affiliates.
 *
 * Author: Vitor Soares <vitor.soares@synopsys.com>
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <i3c/i3cdev.h>

#define VERSION "0.1"

const char *sopts = "d:r:w:vh";
static const struct option lopts[] = {
	{"device",		required_argument,	NULL,	'd' },
	{"read",		required_argument,	NULL,	'r' },
	{"write",		required_argument,	NULL,	'w' },
	{"command",		required_argument,	NULL,	'c' },
	{"help",		no_argument,		NULL,	'h' },
	{"version",		no_argument,		NULL,	'v' },
	{0, 0, 0, 0}
};

static void print_usage(const char *name)
{
	fprintf(stderr, "usage: %s options...\n", name);
	fprintf(stderr, "  options:\n");
	fprintf(stderr, "    -d --device       <dev>          device to use.\n");
	fprintf(stderr, "    -r --read         <data length>  read data.\n");
	fprintf(stderr, "    -w --write        <data block>   Write data block.\n");
	fprintf(stderr, "    -h --help                        Output usage message and exit.\n");
	fprintf(stderr, "    -v --version                     Output the version number and exit\n");
}

static int rx_args_to_xfer(struct i3c_ioc_priv_xfer *xfer, char *arg)
{
	int len = strtol(optarg, NULL, 0);
	uint8_t *tmp;

	tmp = (uint8_t *)calloc(len, sizeof(uint8_t));
	if (!tmp)
		return -1;

	xfer->rnw = 1;
	xfer->len = len;
	xfer->data = (uintptr_t)tmp;

	return 0;
}

static int w_args_to_xfer(struct i3c_ioc_priv_xfer *xfer, char *arg)
{
	char *data_ptrs[256];
	int len, i = 0;
	uint8_t *tmp;

	data_ptrs[i] = strtok(arg, ",");

	while (data_ptrs[i] && i < 255)
		data_ptrs[++i] = strtok(NULL, ",");

	tmp = (uint8_t *)calloc(i, sizeof(uint8_t));
	if (!tmp)
		return -1;

	for (len = 0; len < i; len++)
		tmp[len] = (uint8_t)strtol(data_ptrs[len], NULL, 0);

	xfer->len = len;
	xfer->data = (uintptr_t)tmp;

	return 0;
}

static void print_rx_data(struct i3c_ioc_priv_xfer *xfer)
{
	uint8_t *tmp;
	int i;

	tmp = (uint8_t *)calloc(xfer->len, sizeof(uint8_t));
	if (!tmp)
		return;

	memcpy(tmp, (void *)(uintptr_t)xfer->data, xfer->len * sizeof(uint8_t));

	fprintf(stdout, "  received data:\n");
	for (i = 0; i < xfer->len; i++)
		fprintf(stdout, "    0x%02x\n", tmp[i]);

	free(tmp);
}

int main(int argc, char *argv[])
{
	struct i3c_ioc_priv_xfer *xfers;
	int file, ret, opt, i;
	int nxfers = 0;
	char *device;

	while ((opt = getopt_long(argc, argv,  sopts, lopts, NULL)) != EOF) {
		switch (opt) {
		case 'h':
			print_usage(argv[0]);
			exit(EXIT_SUCCESS);
			/* fall through */
		case 'v':
			fprintf(stderr, "%s - %s\n", argv[0], VERSION);
			exit(EXIT_SUCCESS);
		case 'd':
			/* fall through */
			device = optarg;
			break;
		case 'r':
		case 'w':
			nxfers++;
			break;
		default:
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (!device)
		exit(EXIT_FAILURE);

	file = open(device, O_RDWR);
	if (file < 0)
		exit(EXIT_FAILURE);

	xfers = (struct i3c_ioc_priv_xfer *)calloc(nxfers, sizeof(*xfers));
	if (!xfers)
		exit(EXIT_FAILURE);

	optind = 1;
	nxfers = 0;

	while ((opt = getopt_long(argc, argv, sopts, lopts, NULL)) != EOF) {
		switch (opt) {
		case 'h':
		case 'v':
		case 'd':
			break;
		case 'r':
			if (rx_args_to_xfer(&xfers[nxfers], optarg)) {
				ret = EXIT_FAILURE;
				goto err_free;
			}

			nxfers++;
			break;
		case 'w':
			if (w_args_to_xfer(&xfers[nxfers], optarg)) {
				ret = EXIT_FAILURE;
				goto err_free;
			}

			nxfers++;
			break;
		}
	}

	if (ioctl(file, I3C_IOC_PRIV_XFER(nxfers), xfers) < 0) {
		fprintf(stderr, "Error: transfer failed: %s\n", strerror(errno));
		ret = EXIT_FAILURE;
		goto err_free;
	}

	for (i = 0; i < nxfers; i++) {
		fprintf(stdout, "Success on message %d\n", i);
		if (xfers[i].rnw)
			print_rx_data(&xfers[i]);
	}

	ret = EXIT_SUCCESS;

err_free:
	for (i = 0; i < nxfers; i++)
		free((void *)(uintptr_t)xfers[i].data);
	free(xfers);

	return ret;
}
