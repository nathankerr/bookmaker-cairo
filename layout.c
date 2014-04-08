#include "all.h"

void layout(PopplerDocument *document, cairo_surface_t* surface, cairo_t *cr, struct pages_t *pages, struct options_t options) {
	const double MARGIN = 15; // unprintable margin
	const double GUTTER = 36; // interior margin

	// width and height of the scaled pages
	const double PAGE_WIDTH = options.paper_width/2.0 - MARGIN - GUTTER;
	const double PAGE_HEIGHT = options.paper_height - MARGIN - MARGIN;

	// figure out how many pages to layout
	int num_pages_to_layout = pages->npages;
	if (num_pages_to_layout%4 != 0) {
		num_pages_to_layout = pages->npages + (4 - (pages->npages % 4));
	}

		// layout the pages on the paper
		int show_page = FALSE;
		for (int page_to_layout = 0; page_to_layout < num_pages_to_layout; page_to_layout++) {
			cairo_save(cr);

			// figure out the real page number
			int page_num;
			switch (options.type) {
			case chapbook:
				page_num = page_to_layout/2;
				if (page_to_layout%2 == 1) {
					// even pages, verso
					page_num = num_pages_to_layout-page_num-1;
				}
				break;
			case perfect:
				page_num = page_to_layout - 1;
				if (page_to_layout%4 == 0) {
					page_num += 4;
				}
				break;
			default:
				NOT_IMPLEMENTED();
			}

			if (page_num > pages->npages) {
				// blank page, don't try to render it
				goto FINISH_LAYOUT;
			}

			struct page_t *page_info = &pages->pages[page_num];

			// recto pages have odd page numbers
			// this correctly handles 0 based indexes for 1 based page numbers
			int is_recto = TRUE;
			if (page_num % 2 == 1) {
				is_recto = FALSE;
			}

			cairo_rectangle_t *crop_box = page_info->crop_box;

			PopplerPage *page = poppler_document_get_page(document, page_info->num);
			if (page == NULL) {
				printf("%s:%d\n", __FILE__, __LINE__);
				exit(1);		
			}

			// figure out the desired placement
			double X = 0;
			double Y = MARGIN;
			double WIDTH = PAGE_WIDTH;
			double HEIGHT = PAGE_HEIGHT;

			if (is_recto) {
				X += options.paper_width/2.0 + GUTTER;
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

			if (options.print_page_numbers) {
				// add page number
				char* page_num_text = NULL;
				asprintf(&page_num_text, "%d", page_num + 1);

				cairo_text_extents_t text_extent;
				cairo_text_extents(cr, page_num_text, &text_extent);
				if (is_recto) {
					cairo_move_to(cr,
						options.paper_width - MARGIN - text_extent.width,
						options.paper_height - MARGIN - text_extent.height);
				} else {
					cairo_move_to(cr,
						MARGIN + text_extent.width,
						options.paper_height - MARGIN - text_extent.height);
				}
				
				cairo_show_text(cr, page_num_text);
				free(page_num_text);
			}

			// draw centerline
	#ifdef DISPLAY_BOXES
			cairo_set_source_rgb(cr, 0, 0, 1.0);
			cairo_move_to(cr, options.paper_width/2.0, 0);
			cairo_line_to(cr, options.paper_width/2.0, options.paper_height);
			cairo_stroke(cr);
			cairo_set_source_rgb(cr, 0, 0, 0);
	#endif
			// emit layouts after drawing both pages
			if (show_page) {
				cairo_surface_show_page(surface);

				// flip the next page over
				cairo_rotate(cr, M_PI);
				cairo_translate(cr, -options.paper_width, -options.paper_height);
			}
			show_page = !show_page;
		}
}