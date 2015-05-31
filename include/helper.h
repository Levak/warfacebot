#ifndef _HELPER_
#define _HELPER_
#include <textcolor.h>

char *get_timestamp ( );

char *str_replace ( const char *orig, const char *rep, const char *with );

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

int levenshtein(const char *s1, const char *s2);

char *name_in_string( char* str, const char* name, int percentage);

#endif