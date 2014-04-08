#ifndef _ALL_H
#define _ALL_H

#define VERSION "0.3a"

#define TRUE 1
#define FALSE 0

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
};

struct options_t* parse_options(int, char**);
void print_options(struct options_t*);

void make_chapbook(char*, char*);

#endif /* _ALL_H */