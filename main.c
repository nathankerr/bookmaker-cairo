#include "all.h"

double starttime(char* message) {
	clock_t start = clock();
	printf("%s", message);
	return start;
}

void finishtime(clock_t start) {
	printf(" %fs\n", (double)(clock() - start)/CLOCKS_PER_SEC);
}

int main(int argc, char** argv) {
	struct options_t options = parse_options(argc, argv);
	// print_options(options);

	printf("Creating a PDF for a ");
	switch (options.type) {
	case chapbook:
		printf("chapbook");
		break;
	case perfect:
		printf("perfect bound book");
		break;
	default:
		NOT_IMPLEMENTED();
	}
	printf(" on ");
	switch(options.paper) {
	case a4:
		printf("a4");
		break;
	case letter:
		printf("letter");
		break;
	default:
		NOT_IMPLEMENTED();
	}
	printf(" paper\n");

	clock_t start = starttime("Inspecting PDF");

	switch (options.paper) {
	case a4:
		// 1 pt = 1/27 in
		// 1 in = 2.54 cm
		// A4 210x297mm = 595.224x841.824 
		options.paper_height = 595.224;
		options.paper_width = 841.824;
		break;
	case letter:
		NOT_IMPLEMENTED();
		break;
	default:
		NOT_IMPLEMENTED();
	}

	// create the input and output documents
	PopplerDocument *popplerDocument = open_document(options.input_filename);
	cairo_surface_t* surface = cairo_pdf_surface_create(options.output_filename, options.paper_width, options.paper_height);
	exit_if_cairo_surface_status_not_success(surface, __FILE__, __LINE__);
	cairo_t *cr = cairo_create(surface);
	exit_if_cairo_status_not_success(cr, __FILE__, __LINE__);

	// figure out which pages to layout
	struct pages_t *pages = all_pages(popplerDocument, options);

	// get the crop boxes for the pages
	switch (options.trim) {
	case even_odd:
		add_even_odd_cropboxes(popplerDocument, pages);
		break;
	case document:
		NOT_IMPLEMENTED();
		break;
	case per_page:
		break;
	default:
		NOT_IMPLEMENTED();
	}
	finishtime(start);

	start = starttime("Creating Book");
	// layout the pages
	layout(popplerDocument, surface, cr, pages, options);

	// cleanup and finish
	g_object_unref(popplerDocument);

	exit_if_cairo_status_not_success(cr, __FILE__, __LINE__);
	cairo_destroy(cr);

	cairo_surface_destroy(surface);
	exit_if_cairo_surface_status_not_success(surface, __FILE__, __LINE__);
	finishtime(start);

	if(options.print) {
		NOT_IMPLEMENTED();
	}

	printf("Done\n");
	return 0;
}
