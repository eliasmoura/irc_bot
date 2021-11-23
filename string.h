#ifndef string_h_INCLUDED
#define string_h_INCLUDED
#include <string.h>
#include <stdbool.h>

#define MAX_INT (int)((unsigned int)(~(int)0) >> 1)

struct string {
  size_t size;
  size_t capacity;
  char *data;
};

struct string *string_init();
void string_init_capacity(struct string *string, int cap);
void string_append(struct string *string, struct string *value);
void string_copy(struct string *dest, char *start, char *end);
struct string string_copy_from_char(char *c);
void string_append_char(struct string *string, char c);
void string_append_chars(struct string *string, char *c);
void string_append_chars_sz(struct string *string, char *c, size_t size);
int string_compair(struct string *lstring, struct string *rstring);
int string_compair_with_chars(struct string *lstring, char *c);
int string_compair_chars(char *lstring, char *end, char *c);
char *string_get(struct string *string, int index);
int string_pop(struct string *string);
void remove_char(struct string *string, char c);
void string_resize(struct string *string, int size);
void string_free(struct string *string);
bool is_white_space(char *ch);
void string_trim(struct string *src, int side);

#endif // string_h_INCLUDED

