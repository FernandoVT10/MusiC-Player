#ifndef CCFUNCS_H
#define CCFUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

// Code taken from: https://github.com/tsoding/nob.h
#define DA_INIT_CAP 128

#define da_append(da, item)                                                          \
    do {                                                                             \
        if((da)->count >= (da)->capacity) {                                          \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "No enough ram");                          \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while(0)

#define da_free(da) do { free((da)->items); } while(0)

#define da_append_many(da, new_items, new_items_count)                                  \
    do {                                                                                    \
        if ((da)->count + (new_items_count) > (da)->capacity) {                               \
            if ((da)->capacity == 0) {                                                      \
                (da)->capacity = DA_INIT_CAP;                                           \
            }                                                                               \
            while ((da)->count + (new_items_count) > (da)->capacity) {                        \
                (da)->capacity *= 2;                                                        \
            }                                                                               \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "No enough ram");                          \
        }                                                                                   \
        memcpy((da)->items + (da)->count, (new_items), (new_items_count)*sizeof(*(da)->items)); \
        (da)->count += (new_items_count);                                                     \
    } while (0)
// end of taken code

// printf like function that prints the name and line of the file where it was called
#define log_error(msg, ...) _log_error(msg, __FILE__, __LINE__, __VA_ARGS__);

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} StringBuilder;

// dumps a null terminated string
char *sb_dump_str(StringBuilder *sb);

#endif // CCFUNCS_H

#ifdef CCFUNCS_IMPLEMENTATION

void _log_error(const char *msg, char *file, int line, ...) {
    printf("[ERROR]: ");

    va_list args;
    va_start(args, line);
    vprintf(msg, args);
    va_end(args);

    printf(" (at %s:%d)\n", file, line);
}

char *sb_dump_str(StringBuilder *sb) {
    char *str = malloc(sb->count + 1);
    strncpy(str, sb->items, sb->count);
    str[sb->count] = '\0';
    return str;
}

#endif // CCFUNCS_IMPLEMENTATION
