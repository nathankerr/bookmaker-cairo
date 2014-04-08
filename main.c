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

	printf("\nOPTIONS:\n");
	printf("\t--help, -h\t\tThis help information\n");
	printf("\t--paper {a4,letter}\tSize of paper to be printed on. Default is a4\n");
	printf("\t--type {chapbook,perfect}\n\t\t\t\tType of imposition to make. Default is chapbook\n");
	printf("\t--trim {even-odd,document,per-page}\n\t\t\t\tControls how whitespace is trimmed off.\n\t\t\t\tDefault is even-odd.\n");
	printf("\t--nopagenumbers\t\tsuppress additional page numbers\n");
	printf("\t--print\t\t\tsend result to default printer\n");
	printf("\t--printer PRINTER\tprint result to specific printer\n\t\t\t\t(implies --print)\n");
	printf("\t--version\t\tprints the version string and exits\n");
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
	enum {a4, letter} paper = a4;
	enum {chapbook, perfect} type = chapbook;
	enum {even_odd, document, per_page} trim = even_odd;
	int print_page_numbers = TRUE;
	int print = FALSE;

	enum options {
		paper_option,
		type_option,
		trim_option,
		no_page_numbers_option,
		print_option,
		version_option
	};
	const char *optstring = "h";
	const struct option longopts[] = {
		{"help", no_argument, NULL, 'h'},
		{"paper", required_argument, NULL, paper_option},
		{"type", required_argument, NULL, type_option},
		{"trim", required_argument, NULL, trim_option},
		{"nopagenumbers", no_argument, NULL, no_page_numbers_option},
		{"print", no_argument, NULL, print_option},
		{"version", no_argument, NULL, version_option}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
		switch (opt) {
		case 'h': // same as default
		case paper_option:
			if (strcasecmp(optarg, "a4") == 0) {
				paper = a4;
			} else if (strcasecmp(optarg, "letter") == 0) {
				paper = letter;
			} else {
				printf("ERROR: Unknown paper size: %s\n\n", optarg);
				usage(executable_name);
			}
			break;
		case type_option:
			if (strcasecmp(optarg, "chapbook") == 0) {
				type = chapbook;
			} else if (strcasecmp(optarg, "perfect") == 0) {
				type = perfect;
			} else {
				printf("ERROR: Unknown binding type: %s\n\n", optarg);
				usage(executable_name);
			}
			break;
		case trim_option:
			if (strcasecmp(optarg, "even-odd") == 0) {
				trim = even_odd;
			} else if (strcasecmp(optarg, "document") == 0) {
				trim = document;
			} else if (strcasecmp(optarg, "per-page") == 0) {
				trim = per_page;
			} else {
				printf("ERROR: Unknown trim type: %s\n\n", optarg);
				usage(executable_name);
			}
			break;
		case no_page_numbers_option:
			print_page_numbers = FALSE;
			break;
		case print_option:
			print = TRUE;
			break;
		case version_option:
			printf("%s\n", VERSION);
			exit(0);
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
	printf("PAPER: ");
	switch (paper) {
	case a4:
		printf("a4\n");
		break;
	case letter:
		printf("letter\n");
		break;
	default:
		printf("ERROR\n");
	}
	printf("TYPE: ");
	switch (type) {
	case chapbook:
		printf("chapbook\n");
		break;
	case perfect:
		printf("perfect\n");
		break;
	default:
		printf("ERROR\n");
	}
	printf("TRIM: ");
	switch (trim) {
	case even_odd:
		printf("even-odd\n");
		break;
	case document:
		printf("document\n");
		break;
	case per_page:
		printf("per-page\n");
		break;
	default:
		printf("ERROR\n");
	}
	printf("PAGE NUMBERS: ");
	if (print_page_numbers) {
		printf("yes\n");
	} else {
		printf("no\n");
	}
	printf("PRINT: ");
	if (print) {
		printf("yes\n");
	} else {
		printf("no\n");
	}

	make_chapbook(input_filename, output_filename);

	return 0;
}
