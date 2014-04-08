#include "all.h"

// method: draw all the pages to appropriate recording surfaces and then get ink extents
void evenodd_cropboxes(PopplerDocument *document, cairo_rectangle_t *odd_page_crop_box, cairo_rectangle_t *even_page_crop_box) {
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

void add_even_odd_cropboxes(PopplerDocument *document, struct pages_t *pages) {
	cairo_rectangle_t *odd_page_crop_box = malloc(sizeof(cairo_rectangle_t));
	cairo_rectangle_t *even_page_crop_box = malloc(sizeof(cairo_rectangle_t));

	evenodd_cropboxes(document, odd_page_crop_box, even_page_crop_box);

	int page_num;
	for (page_num = 0; page_num < pages->npages; page_num++) {
		struct page_t *page = &pages->pages[page_num];

		if (page_num % 2 == 1) {
			page->crop_box = even_page_crop_box;
		} else {
			page->crop_box = odd_page_crop_box;
		}
	}
}