#include "all.h"

void usage(char *executable_name) {
	printf("USAGE: %s [options] input.pdf [output.pdf]\n", executable_name);

	printf("\nOPTIONS:\n");
	printf("\t--help, -h\t\tThis help information\n");
	printf("\t--paper {a4,letter}\tSize of paper to be printed on. Default is a4\n");
	printf("\t--type {chapbook,perfect}\n\t\t\t\tType of imposition to make. Default is chapbook\n");
	printf("\t--trim {even-odd,document,per-page}\n\t\t\t\tControls how whitespace is trimmed off.\n\t\t\t\tDefault is even-odd.\n");
	printf("\t--nopagenumbers\t\tsuppress additional page numbers\n");
	printf("\t--print\t\t\tsend result to default printer instead of saving to file\n");
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
	struct stat filestat;
	while(stat(output_filename, &filestat) != -1 && errno != ENOENT) {
		free(output_filename);
		asprintf(&output_filename, "%s.book_%d.pdf", base, n);
		n++;
	}

	free(base);
	return output_filename;
}

struct options_t parse_options(int argc, char** argv) {
	struct options_t options;

	// set defaults
	options.executable_name = argv[0];
	options.input_filename = NULL;
	options.output_filename = NULL;
	options.paper = a4;
	options.type = chapbook;
	options.trim = even_odd;
	options.print_page_numbers = TRUE;
	options.print = FALSE;
	options.printer = NULL;

	enum {
		paper_option,
		type_option,
		trim_option,
		no_page_numbers_option,
		print_option,
		printer_option,
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
		{"printer", required_argument, NULL, printer_option},
		{"version", no_argument, NULL, version_option}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
		switch (opt) {
		case paper_option:
			if (strcasecmp(optarg, "a4") == 0) {
				options.paper = a4;
			} else if (strcasecmp(optarg, "letter") == 0) {
				options.paper = letter;
			} else {
				printf("ERROR: Unknown paper size: %s\n\n", optarg);
				usage(options.executable_name);
			}
			break;
		case type_option:
			if (strcasecmp(optarg, "chapbook") == 0) {
				options.type = chapbook;
			} else if (strcasecmp(optarg, "perfect") == 0) {
				options.type = perfect;
			} else {
				printf("ERROR: Unknown binding type: %s\n\n", optarg);
				usage(options.executable_name);
			}
			break;
		case trim_option:
			if (strcasecmp(optarg, "even-odd") == 0) {
				options.trim = even_odd;
			} else if (strcasecmp(optarg, "document") == 0) {
				options.trim = document;
			} else if (strcasecmp(optarg, "per-page") == 0) {
				options.trim = per_page;
			} else {
				printf("ERROR: Unknown trim type: %s\n\n", optarg);
				usage(options.executable_name);
			}
			break;
		case no_page_numbers_option:
			options.print_page_numbers = FALSE;
			break;
		case printer_option:
			options.printer = optarg;
			// NO BREAK; --printer implies --print
		case print_option:
			options.print = TRUE;
			break;
		case version_option:
			printf("%s\n", VERSION);
			exit(0);
		case 'h': // same as default
		default:
			usage(options.executable_name);
		}
	}

	argc -= optind;
	argv += optind;

	switch (argc) {
	case 2:
		options.output_filename = argv[1];
	case 1:
		options.input_filename = argv[0];
		break;
	default:
		usage(options.executable_name);
		exit(0);
	}

	// exit if input file does not exist
	struct stat filestat;
	if (stat(options.input_filename, &filestat) == -1 && errno == ENOENT) {
		printf("%s not found\n", options.input_filename);
		exit(1);
	}

	// makeup a suitable output filename if none exists
	if (options.output_filename == NULL) {
		options.output_filename = create_output_filename(options.input_filename);
	}

	return options;
}

void print_options(struct options_t options) {
	printf("INPUT: %s\n", options.input_filename);
	printf("OUTPUT: %s\n", options.output_filename);
	printf("PAPER: ");
	switch (options.paper) {
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
	switch (options.type) {
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
	switch (options.trim) {
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
	if (options.print_page_numbers) {
		printf("yes\n");
	} else {
		printf("no\n");
	}
	printf("PRINT: ");
	if (options.print) {
		printf("yes\n");
	} else {
		printf("no\n");
	}
	printf("PRINTER: %s\n", options.printer);
}
