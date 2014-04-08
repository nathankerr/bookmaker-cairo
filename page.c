#include "all.h"

struct pages_t* all_pages(PopplerDocument *document, struct options_t options) {
	struct pages_t *pages = malloc(sizeof(struct pages_t));
	
	pages->npages = poppler_document_get_n_pages(document);

	pages->pages = malloc(sizeof(struct page_t)*pages->npages);

	int page_num;
	for (page_num = 0; page_num < pages->npages; page_num++) {
		struct page_t *page = &pages->pages[page_num];

		page->num = page_num;
	}

	return pages;
}