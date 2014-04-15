#include "all.h"

// uses the first page for the cover
void add_cover(PopplerDocument *document, cairo_surface_t* surface, cairo_t *cr, struct pages_t *pages, struct options_t options) {
	int num_pages_to_layout = get_num_pages_to_layout(pages->npages);
	double PAPER_THICKNESS = 0.324; // in pt, half the thickness of a folded over page
	double fold_distance = (num_pages_to_layout * PAPER_THICKNESS) / 2.0;
	int margin = 72/2;

	cairo_save(cr);
	if (options.title != NULL) {
		// make a cool cover
		PangoLayout *layout = pango_cairo_create_layout(cr);
		PangoFontDescription *title = pango_font_description_from_string("serif 20");
		PangoFontDescription *normal = pango_font_description_from_string("sans 10");

		double layout_x = options.paper_width/2.0 + fold_distance + margin;
		double layout_width = options.paper_width/2.0 - fold_distance - 2 * margin;

#ifdef DISPLAY_BOXES
		cairo_save(cr);

		// center line
		cairo_set_source_rgb(cr, 1.0, 0, 0);
		cairo_move_to(cr, options.paper_width/2.0, 0);
		cairo_rel_line_to(cr, 0, options.paper_height);
		cairo_stroke(cr);

		// inner margin
		cairo_set_source_rgb(cr, 0, 1.0, 0);
		cairo_move_to(cr, options.paper_width/2.0 + fold_distance, 0);
		cairo_rel_line_to(cr, 0, options.paper_height);
		cairo_stroke(cr);

		// layout area
		double layout_max_height = options.paper_height - 2*margin;
		cairo_set_source_rgb(cr, 0, 0, 1.0);
		cairo_move_to(cr, layout_x, margin);
		cairo_rectangle(cr, layout_x, margin, layout_width, layout_max_height);
		cairo_stroke(cr);

		cairo_restore(cr);
#endif

		// title
		pango_layout_set_width(layout, layout_width * PANGO_SCALE);
		pango_layout_set_font_description(layout, title);
		pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
		pango_layout_set_text(layout, options.title, -1);

		double title_top = margin;

		pango_cairo_update_layout(cr, layout);
		cairo_move_to(cr, layout_x, title_top);
		pango_cairo_show_layout(cr, layout);

		if (options.date != NULL) {
			int pango_title_height;
			pango_layout_get_size(layout, NULL, &pango_title_height);
			pango_layout_set_font_description(layout, normal);
			double date_top = title_top + ((double) pango_title_height / PANGO_SCALE);

			pango_layout_set_text(layout, options.date, -1);
			pango_cairo_update_layout(cr, layout);
			cairo_move_to(cr, layout_x, date_top);
			pango_cairo_show_layout(cr, layout);
		}

		if (options.author != NULL) {
			pango_layout_set_text(layout, options.author, -1);
			pango_layout_set_font_description(layout, normal);
			pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
			pango_cairo_update_layout(cr, layout);

			int pango_author_height;
			pango_layout_get_size(layout, NULL, &pango_author_height);

			cairo_move_to(cr, layout_x, options.paper_height - margin - ((double) pango_author_height / PANGO_SCALE));
			pango_cairo_show_layout(cr, layout);
		}

		pango_font_description_free(normal);
		pango_font_description_free(title);
		g_object_unref(layout);
	} else {
		// use the first page of the document as the cover
		// TODO: scale the cover page
		GError *error = NULL;

		int cover_page_number = pages[0].pages[0].num;
		PopplerPage *cover = poppler_document_get_page(document, cover_page_number);
		if (cover == NULL) {
			printf("%s:%d: %s\n", __FILE__, __LINE__, error->message);
			exit(1);
		}

		cairo_translate(cr, options.paper_width/2.0, 0);
		poppler_page_render_for_printing(cover, cr);

		g_object_unref(cover);
	}
	cairo_surface_show_page(surface);
	cairo_restore(cr);

	//draw the fold marks
	cairo_save(cr);
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