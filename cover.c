#include "all.h"

// uses the first page for the cover
void add_cover(PopplerDocument *document, cairo_surface_t* surface, cairo_t *cr, struct pages_t *pages, struct options_t options) {
	cairo_save(cr);

	GError *error = NULL;

	int cover_page_number = pages[0].pages[0].num;
	PopplerPage *cover = poppler_document_get_page(document, cover_page_number);
	if (cover == NULL) {
		printf("%s:%d: %s\n", __FILE__, __LINE__, error->message);
		exit(1);		
	}

	cairo_translate(cr, options.paper_width/2.0, 0);
	poppler_page_render_for_printing(cover, cr);

	cairo_surface_show_page(surface);
	g_object_unref(cover);
	cairo_restore(cr);

	//draw the fold marks
	cairo_save(cr);
	int num_pages_to_layout = get_num_pages_to_layout(pages->npages);
	const double PAPER_THICKNESS = 0.324; // in pt, half the thickness of a folded over page
	double fold_distance = (num_pages_to_layout * PAPER_THICKNESS) / 2.0;
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 0.5);

	cairo_move_to(cr, fold_distance, 0.0);
	cairo_rel_line_to(cr, 0, 10);
	cairo_stroke(cr);

	cairo_move_to(cr, options.paper_width-fold_distance, options.paper_height);
	cairo_rel_line_to(cr, 0, -10);
	cairo_stroke(cr);

	cairo_surface_show_page(surface);
	cairo_restore(cr);
}