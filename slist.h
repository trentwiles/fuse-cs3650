/**
 * @file slist.h
 * @author CS3650 staff
 *
 * A simple linked list of strings.
 *
 * This might be useful for directory listings and for manipulating paths.
 */
#ifndef SLIST_H
#define SLIST_H

typedef struct slist {
  char *data;
  int refs;
  struct slist *next;
} slist_t;

/**
 * Cons a string to a string list.
 *
 * @param text String to cons on to a list
 * @param rest List of strings to cons onto.
 *
 * @return List starting with the given string in front of the original list.
 */
slist_t *slist_cons(const char *text, slist_t *rest);

/** 
 * Free the given string list.
 *
 * @param xs List of strings to free.
 */
void slist_free(slist_t *xs);

/**
 * Split the given on the given delimiter into a list of strings.
 *
 * Note, that the delimiter will not be included in any of the strings.
 *
 * @param text String to be split
 * @param delim A single character to use as the delimiter.
 *
 * @return a list containing all the substrings
 */
slist_t *slist_explode(const char *text, char delim);

#endif
