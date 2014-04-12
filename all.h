#ifndef _ALL_H
#define _ALL_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <poppler.h>

#define VERSION "0.3a"

// toggle to display boxes for debugging
//#define DISPLAY_BOXES

#define NOT_IMPLEMENTED() printf("NOT_IMPLEMENTED %s:%d\n", __FILE__, __LINE__); exit(1);

enum paper_t {a4, letter};
enum type_t {chapbook, perfect};
enum trim_t {even_odd, document, per_page};
struct options_t {
	char *executable_name;
	char *input_filename;
	char *output_filename;
	enum paper_t paper;
	enum type_t type;
	enum trim_t trim;
	int print_page_numbers;
	int print;
	char* printer;
	double paper_width;
	double paper_height;
};

struct options_t parse_options(int, char**);
void print_options(struct options_t);

struct page_t {
	int num;
	cairo_rectangle_t *crop_box;
};

struct pages_t {
	struct page_t *pages;
	int npages;
};

struct pages_t* all_pages(PopplerDocument*, struct options_t);

void exit_if_cairo_status_not_success(cairo_t* cr, char* file, int line);
void write_surface_to_file_showing_crop_box(char* filename, cairo_surface_t *recording_surface, cairo_rectangle_t *crop_box);
PopplerDocument* open_document(char* filename);

void add_even_odd_cropboxes(PopplerDocument *document, struct pages_t *pages);
void add_document_cropboxes(PopplerDocument *document, struct pages_t *pages);
void add_per_page_cropboxes(PopplerDocument *document, struct pages_t *pages);

void exit_if_cairo_surface_status_not_success(cairo_surface_t* surface, char* file, int line);


void make_chapbook(char*, char*);
void layout(PopplerDocument *document, cairo_surface_t* surface, cairo_t *cr, struct pages_t *pages, struct options_t options);

#endif /* _ALL_H */