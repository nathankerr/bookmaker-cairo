// implement the bookmaker option parsing in c

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "all.h"

/* option description from python version:
usage: bookmaker [-h] [--paper {a4,letter}] [--type {chapbook,perfect}]
                 [--trim {even-odd,document,per-page}] [--nopagenumbers]
                 [--print] [--printer PRINTER] [--version]
                 input [output]

DIY Books from PDFs

positional arguments:
  input                 PDF to convert into a book
  output                Where to store the new book

optional arguments:
  -h, --help            show this help message and exit
  --paper {a4,letter}   Size of paper to be printed on. Default is a4
  --type {chapbook,perfect}
                        Type of imposition to make. Auto automatically chooses
                        between the two. Default is chapbook
  --trim {even-odd,document,per-page}
                        Controls how whitespace is trimmed off. Default is
                        even-odd.
  --nopagenumbers       suppress additional page numbers
  --print               send result to default printer
  --printer PRINTER     print result to specific printer (implies --print)
  --version             prints the version string and exits
*/

void usage(char *executable_name) {
	printf("USAGE: %s [options] input.pdf [output.pdf]\n", executable_name);
	exit(1);
}

char *create_output_filename(char *input_filename) {
	size_t base_size = strlen(input_filename) + 1;
	char *base = malloc(base_size);
	strncpy(base, input_filename, base_size);

	// remove .pdf extension from base
	char *extension = rindex(base, '.');
	if (strcasecmp(extension, ".pdf") == 0) {
		base[extension - base] = 0;
	}

	char *output_filename;
	int n = 2;
	asprintf(&output_filename, "%s.book.pdf", base);
	while(stat(output_filename, NULL) == -1 && errno != ENOENT) {
		free(output_filename);
		asprintf(&output_filename, "%s.book_%d.pdf", base, n);
		n++;
	}

	free(base);
	return output_filename;
}

int main(int argc, char** argv) {
	// values we are trying to set
	char *executable_name = argv[0];
	char *input_filename = NULL;
	char *output_filename = NULL;

	const char *optstring = "h";
	const struct option longopts[] = {
		{"help", no_argument, NULL, 'h'}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
		switch (opt) {
		case 'h': // same as default
		default:
			usage(executable_name);
		}
	}

	argc -= optind;
	argv += optind;

	switch (argc) {
	case 2:
		output_filename = argv[1];
	case 1:
		input_filename = argv[0];
		break;
	default:
		usage(executable_name);
		exit(0);
	}

	// makeup a suitable output filename if none exists
	if (output_filename == NULL) {
		output_filename = create_output_filename(input_filename);
	}

	printf("INPUT: %s\n", input_filename);
	printf("OUTPUT: %s\n", output_filename);

	chapbook(input_filename, output_filename);

	return 0;
}
