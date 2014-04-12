#include "all.h"

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
PopplerDocument* open_document(char* filename) {
	// resolve path names
	gchar *absolute;
	if (g_path_is_absolute(filename)) {
		absolute = g_strdup(filename);
	} else {
		gchar* dir = g_get_current_dir();
		absolute = g_build_filename(dir, filename, (gchar*)0);
		free(dir);
	}

	gchar *uri = g_filename_to_uri(absolute, NULL, NULL);
	free(absolute);
	if (uri == NULL) {
		return NULL;
	}

	PopplerDocument* document = poppler_document_new_from_file(uri, NULL, NULL);
	free(uri);

	if (document == NULL) {
		printf("Could not open document %s\n", filename);
		exit(1);
	}

	return document;
}