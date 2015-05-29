#include <helper.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

char *str_replace(const char *original, const char *rep, const char *with)
{
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
	char *orig = (char*)original;
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count)
        ins = tmp + len_rep;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--)
	{
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

 
int levenshtein(const char *s1, const char *s2)
{
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int column[s1len+1];
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++)
	{
        column[0] = x;
        for (y = 1, lastdiag = x-1; y <= s1len; y++)
		{
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y-1] + 1, lastdiag + (tolower(s1[y-1]) == tolower(s2[x-1]) ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return(column[s1len]);
}

char *name_in_string( char* str, const char* name, int percentage)
{
	char *word = strtok( str, " '.,?:;");
	int required = (strlen(name) * percentage / 100);
	while(word)
	{
		if( required >= levenshtein( word, name ) )
			return word;
		word = strtok( NULL, " '.,?:;");
	}
	return NULL;
}