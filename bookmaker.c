#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <cairo.h>
#include <cairo-pdf.h>
#include <poppler.h>

// toggle to display boxes for debugging
#define DISPLAY_BOXES

void exit_if_cairo_surface_status_not_success(cairo_surface_t* surface, char* file, int line) {
	cairo_status_t status = cairo_surface_status(surface);
	if (status != CAIRO_STATUS_SUCCESS) {
		printf("%s:%d: %s\n", file, line, cairo_status_to_string(status));
		exit(1);
	}
}

void exit_if_cairo_status_not_success(cairo_t* cr, char* file, int line) {
	cairo_status_t status = cairo_status(cr);
	if (status != CAIRO_STATUS_SUCCESS) {
		printf("%s:%d: %s\n", file, line, cairo_status_to_string(status));
		exit(1);
	}
}

void write_surface_to_file_showing_crop_box(char* filename, cairo_surface_t *recording_surface, cairo_rectangle_t *crop_box) {
	// setup
	double width = crop_box->width + crop_box->x + crop_box->x;
	double height = crop_box->height + crop_box->y + crop_box->y;
	cairo_surface_t* surface = cairo_pdf_surface_create(filename, width, height);
	exit_if_cairo_surface_status_not_success(surface, __FILE__, __LINE__);

	cairo_t *cr = cairo_create(surface);
	exit_if_cairo_status_not_success(cr, __FILE__, __LINE__);

	// draw source_surface on surface
	cairo_set_source_surface(cr, recording_surface, 0.0, 0.0);
	cairo_paint(cr);

	// draw the crop box on top
	cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
	cairo_rectangle(cr, crop_box->x, crop_box->y, crop_box->width, crop_box->height);
	cairo_stroke(cr);

	// cleanup
	exit_if_cairo_status_not_success(cr, __FILE__, __LINE__);
	cairo_destroy(cr);	

	cairo_surface_destroy(surface);
	exit_if_cairo_surface_status_not_success(surface, __FILE__, __LINE__);
}

// if PopplerDocument* is NULL, return error to user
PopplerDocument* open_document(char* filename, GError **error) {
	// resolve path names
	gchar *absolute;
	if (g_path_is_absolute(filename)) {
		absolute = g_strdup(filename);
	} else {
		gchar* dir = g_get_current_dir();
		absolute = g_build_filename(dir, filename, (gchar*)0);
		free(dir);
	}

	gchar *uri = g_filename_to_uri(absolute, NULL, error);
	free(absolute);
	if (uri == NULL) {
		return NULL;
	}

	PopplerDocument* document = poppler_document_new_from_file(uri, NULL, error);
	free(uri);
	return document;
}

// method: draw all the pages to appropriate recording surfaces and then get ink extents
void get_evenodd_cropboxes(PopplerDocument *document, cairo_rectangle_t *odd_page_crop_box, cairo_rectangle_t *even_page_crop_box) {
	GError *error = NULL;
	int num_document_pages = poppler_document_get_n_pages(document);

	cairo_surface_t *odd_pages = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, NULL);
	cairo_surface_t *even_pages = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, NULL);
	for (int page_num = 0; page_num < num_document_pages; page_num++) {
		cairo_surface_t *surface = odd_pages;
		if (page_num % 2 == 1) {
			surface = even_pages;
		}
		cairo_t *cr = cairo_create(surface);

		PopplerPage *page = poppler_document_get_page(document, page_num);
		if (page == NULL) {
			printf("%s:%d: %s\n", __FILE__, __LINE__, error->message);
			exit(1);		
		}

		poppler_page_render_for_printing(page, cr);
		g_object_unref(page);

		exit_if_cairo_status_not_success(cr, __FILE__, __LINE__);
		cairo_destroy(cr);
	}

	cairo_recording_surface_ink_extents(odd_pages,
		&odd_page_crop_box->x,
		&odd_page_crop_box->y,
		&odd_page_crop_box->width,
		&odd_page_crop_box->height);
	cairo_recording_surface_ink_extents(even_pages,
		&even_page_crop_box->x,
		&even_page_crop_box->y,
		&even_page_crop_box->width,
		&even_page_crop_box->height);

	// use to check extent and crop box handling
	// write_surface_to_file_showing_crop_box("odd.pdf", odd_pages, odd_page_crop_box);
	// write_surface_to_file_showing_crop_box("even.pdf", even_pages, even_page_crop_box);

	// cleanup surfaces used to get crop boxes
	cairo_surface_destroy(odd_pages);
	exit_if_cairo_surface_status_not_success(odd_pages, __FILE__, __LINE__);
	cairo_surface_destroy(even_pages);
	exit_if_cairo_surface_status_not_success(even_pages, __FILE__, __LINE__);
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage: %s <input.pdf>\n", argv[0]);
		exit(1);
	}
	char* input_filename = argv[1];

	// 1 pt = 1/27 in
	// 1 in = 2.54 cm
	// A4 210x297mm = 595.224x841.824 
	const double PAPER_HEIGHT = 595.224;
	const double PAPER_WIDTH = 841.824;

	const double MARGIN = 15; // unprintable margin
	const double GUTTER = 36; // interior margin

	// width and height of the scaled pages
	const double PAGE_WIDTH = PAPER_WIDTH/2.0 - MARGIN - GUTTER;
	const double PAGE_HEIGHT = PAPER_HEIGHT - MARGIN - MARGIN;

	// load the pdf
	GError *error = NULL;
	PopplerDocument *document = open_document(input_filename, &error);
	if (document == NULL) {
		printf("%s:%d: %s\n", __FILE__, __LINE__, error->message);
		exit(1);
	}

	// figure out how many pages to layout
	int num_document_pages = poppler_document_get_n_pages(document);
	int num_pages_to_layout = num_document_pages;
	if (num_pages_to_layout%4 != 0) {
		num_pages_to_layout = num_document_pages + (4 - (num_document_pages % 4));
	}

	// figure out the bounding boxes
	cairo_rectangle_t even_page_crop_box, odd_page_crop_box;
	get_evenodd_cropboxes(document, &even_page_crop_box, &odd_page_crop_box);

	// create a landscape surface for the paper
	cairo_surface_t* surface = cairo_pdf_surface_create("book.pdf", PAPER_WIDTH, PAPER_HEIGHT);
	exit_if_cairo_surface_status_not_success(surface, __FILE__, __LINE__);
	cairo_t *cr = cairo_create(surface);
	exit_if_cairo_status_not_success(cr, __FILE__, __LINE__);

	// layout the pages on the paper
	int show_page = FALSE;
	for (int page_to_layout = 0; page_to_layout < num_pages_to_layout; page_to_layout++) {
		cairo_save(cr);

		// determine the real page number for chapbooks
		int page_num = page_to_layout/2;
		if (page_to_layout%2 == 1) {
			// even pages, verso
			page_num = num_pages_to_layout-page_num-1;
		}

		// recto pages have odd page numbers
		// this correctly handles 0 based indexes for 1 based page numbers
		int is_recto = TRUE;
		if (page_num % 2 == 1) {
			is_recto = FALSE;
		}

		cairo_rectangle_t *crop_box = &odd_page_crop_box;
		if(is_recto) {
			crop_box = &even_page_crop_box;
		}

		if (page_num >= num_document_pages) {
			// blank page, don't try to render it
			goto FINISH_LAYOUT;
		}

		PopplerPage *page = poppler_document_get_page(document, page_num);
		if (page == NULL) {
			printf("%s:%d: %s\n", __FILE__, __LINE__, error->message);
			exit(1);		
		}

		// figure out the desired placement
		double X = 0;
		double Y = MARGIN;
		double WIDTH = PAGE_WIDTH;
		double HEIGHT = PAGE_HEIGHT;

		if (is_recto) {
			X += PAPER_WIDTH/2.0 + GUTTER;
		} else {
			X += MARGIN;
		}

		// figure out the scale factor
		double scale_factor = 1;
		double page_aspect_ratio = HEIGHT / WIDTH;
		double crop_box_aspect_ratio = crop_box->height / crop_box->width;
		if (page_aspect_ratio > crop_box_aspect_ratio) {
			scale_factor = WIDTH / crop_box->width;
		} else {
			scale_factor = HEIGHT / crop_box->height;
		}

		// draw the desired placement
#ifdef DISPLAY_BOXES
		cairo_set_source_rgb(cr, 1.0, 0, 0);
		cairo_rectangle(cr, X, Y, WIDTH, HEIGHT);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0, 0, 0);
#endif

		// scale to the size of the crop box
		double horizontal_offset = X - (crop_box->x * scale_factor);
		double vertical_offset = Y - (crop_box->y * scale_factor);

		// float verso pages toward the gutter
		if (!is_recto) {
			horizontal_offset += (WIDTH) - (crop_box->width * scale_factor);
		}

		cairo_translate(cr, horizontal_offset, vertical_offset);
		cairo_scale(cr, scale_factor, scale_factor);

		poppler_page_render_for_printing(page, cr);

		// draw the crop box around the page
#ifdef DISPLAY_BOXES
		cairo_set_source_rgb(cr, 0, 1.0, 0);
		cairo_rectangle(cr, crop_box->x, crop_box->y, crop_box->width, crop_box->height);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0, 0, 0);
#endif
		
		g_object_unref(page);

FINISH_LAYOUT:
		cairo_restore(cr);
		
		// add page number
		char* page_num_text = NULL;
		asprintf(&page_num_text, "%d", page_num + 1);

		cairo_text_extents_t text_extent;
		cairo_text_extents(cr, page_num_text, &text_extent);
		if (is_recto) {
			cairo_move_to(cr,
				PAPER_WIDTH - MARGIN - text_extent.width,
				PAPER_HEIGHT - MARGIN - text_extent.height);
		} else {
			cairo_move_to(cr,
				MARGIN + text_extent.width,
				PAPER_HEIGHT - MARGIN - text_extent.height);
		}
		
		cairo_show_text(cr, page_num_text);
		free(page_num_text);

		// draw centerline
#ifdef DISPLAY_BOXES
		cairo_set_source_rgb(cr, 0, 0, 1.0);
		cairo_move_to(cr, PAPER_WIDTH/2.0, 0);
		cairo_line_to(cr, PAPER_WIDTH/2.0, PAPER_HEIGHT);
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0, 0, 0);
#endif
		// emit layouts after drawing both pages
		if (show_page) {
			cairo_surface_show_page(surface);

			// flip the next page over
			cairo_rotate(cr, M_PI);
			cairo_translate(cr, -PAPER_WIDTH, -PAPER_HEIGHT);
		}
		show_page = !show_page;
	}

	// cleanup and finish
	g_object_unref(document);

	exit_if_cairo_status_not_success(cr, __FILE__, __LINE__);
	cairo_destroy(cr);	

	cairo_surface_destroy(surface);
	exit_if_cairo_surface_status_not_success(surface, __FILE__, __LINE__);
	return 0;
}