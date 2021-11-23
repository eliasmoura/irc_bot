#include "string.h"
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#define MAX_INT (int)((unsigned int)(~(int)0) >> 1)

/* struct string *string_init() { */
/*   struct string *string = {0}; */
/*   string->data = */
/*       (int *)calloc((size_t)100+ 1, sizeof(*string->data)); */
/*   return string; */
/* } */

void string_init_capacity(struct string *string, int cap) {
  string->size = 0;
  assert(cap >0);
  if(cap <= 0) return;
  string->capacity = cap;
  string->data =
      (char *)calloc((size_t)string->capacity + 2, sizeof(*string->data));
  assert(string->data != NULL);
}

void string_append(struct string *string, struct string *value) {
  if ((string->size + value->size) > string->capacity)
    string_resize(string, string->size + (int)(value->size * 1.5));
  for (size_t i = 0; i < value->size; i++)
    string->data[string->size + i] = value->data[i];
  string->size += 1;
}

void string_copy(struct string *dest, char *start, char *end) {
  size_t size = (int)(end - start) + dest->size + 2;
  if (dest->capacity < size) {
    char *tmp =
        (char *)realloc(dest->data, sizeof(char) * (size_t)(size + 1));
    if (tmp != NULL) {
      dest->data = tmp;
      dest->capacity = size;
    }
  }
  assert(dest->data != NULL);
  for (; start <= end && dest->size <= dest->capacity; ++start, ++dest->size)
    dest->data[dest->size] = *start;
  dest->data[dest->size] = '\0';
}

void string_trim(struct string *src, int side){
  // side: 1 left, 2 right, 0 both
  if(src->size) {
    if(side == 2 || side == 0) {
      for(int i = src->size-1; i >= 0; --i) {
        if(is_white_space(&src->data[i])) {
          src->data[i] = '\0';
          --src->size;
        } else break;
      }
    }

    if(side == 1 || side == 0) {
      size_t move = 0;
      for(size_t i = 0; i < src->size; ++i) {
        if(is_white_space(&src->data[i])) {
          ++move;
          --src->size;
        } else break;
      }
      if(move) {
        struct string tmp;
        string_copy(&tmp, src->data+move, src->data+move+src->size);
        string_free(src);
        string_copy(src, tmp.data+move, tmp.data+move+tmp.size);
        string_free(&tmp);
      }
    }
  }
}

bool is_white_space(char *str) {
  bool result = false;
  // NOTE(elias): 65279 is a zero-width no-break space
  if(*str == ' ' || *str == '\n' || *str == '\r' || *str == '\t')// || *str == 65279)
    result = true;
  return result;
}

struct string string_copy_from_char(char *c) {
  struct string str;
  int count = 0;
  while (c[count] != '\0')
    ++count;
  str.size = 0;
  if (!count)
    return str;
  str.capacity = count;
  str.data =
      (char *)malloc(sizeof(*str.data) * str.capacity);
  assert(str.data != NULL);
  for (size_t i = 0; i < str.size; i++)
    str.data[i] = (c[i]);
  return str;
}

void string_append_char(struct string *string, char c) {
  if (string->size >= string->capacity)
    string_resize(string, (int)((string->size + 1) * 1.5));
  string->data[string->size] = c;
  string->size += 1;
}
void string_append_chars_sz(struct string *string, char *c, size_t size) {
  if ((string->size + size) > string->capacity)
    string_resize(string, (int)((string->size + size) * 1.5));
  for (size_t i = 0; i < size; i++)
    string->data[string->size++] = c[i];
  string->data[string->size] = 0;
}
void string_append_chars(struct string *string, char *c) {
  int count = 0;
  while (c[count] != '\0')
    ++count;
  if ((string->size + count) > string->capacity)
    string_resize(string, (int)((string->size + count) * 1.5));
  for (int i = 0; i < count; i++)
    string->data[string->size++] = c[i];
}
int string_compair(struct string *lstring, struct string *rstring) {
  int size = lstring->size > rstring->size ? rstring->size : lstring->size;
  int equality = 0;
  for (int i = 0; i < size; i++)
    if (lstring->data[i] != rstring->data[i]) {
      equality = -1;
      break;
    }
  return equality;
}
int string_compair_with_chars(struct string *lstring, char *c) {
  size_t char_size = 0;
  while (c[char_size] != '\0')
    char_size++;
  size_t size = lstring->size > char_size ? char_size : lstring->size;
  int equality = 0;
  for (size_t i = 0; i < size; i++)
    if (lstring->data[i] != c[i]) {
      equality = -1;
      break;
    }
  return equality;
}
int string_compair_chars(char *lstring, char *end, char *c) {
  size_t char_size = strlen(c);
  size_t lstring_size = 0;
  if (lstring < end)
    lstring_size = (size_t)(end - lstring);
  else
    lstring_size = (size_t)(lstring - end);
  ++lstring_size;
  assert(lstring_size > 0);
  if (lstring_size != char_size)
    return -1;
  for (size_t i = 0; i < char_size; i++)
    if (lstring[i] != c[i]) {
      return -1;
    }
  return 0;
}
char *string_get(struct string *string, int index) {
  assert(index >= 0 && index <= MAX_INT);
  if (string->size > 0)
    return &string->data[index];
  return 0;
}
int string_pop(struct string *string) {
  assert(string->size > 0 && string->size <= MAX_INT);
  int popedNode = (string->data[string->size]);
  string->data[string->size] = 0;
  string->size -= 1;
  return popedNode;
}
void remove_char(struct string *string, char c) {
  struct string tmp;
  tmp.size = string->size;
  string_init_capacity(&tmp, string->size);
  int tmpIndex = 0;
  for (size_t i = 0; i < string->size; i++) {
    if (string->data[i] != c)
      continue;
    tmp.data[tmpIndex] = string->data[tmpIndex];
    ++tmpIndex;
  }
  free(string->data);
  string->data = tmp.data;
}
void string_resize(struct string *string, int size) {
  int new_cap = size * 2;
  char *tmp =
      (char *)realloc(string->data, sizeof(char) * (size_t)new_cap + 1);
  if (tmp != NULL && tmp != string->data) {
    string->data = tmp;
    string->capacity = new_cap;
    for (size_t i = string->size; i < string->capacity; i++)
      string->data[i] = '\0';
  }
}
void string_free(struct string *string) {
  free(string->data);
  string->size = 0;
  string->capacity = 0;
}

