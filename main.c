// implement the bookmaker option parsing in c

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "all.h"

int main(int argc, char** argv) {
	struct options_t* options = parse_options(argc, argv);
	print_options(options);

	make_chapbook(options->input_filename, options->output_filename);

	return 0;
}
