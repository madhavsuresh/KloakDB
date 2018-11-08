/*
 * bitonic.c: C reference implementation of Bitonic Sort
 * Chris Leger <clleger@cag.lcs.mit.edu>
 * $Id: bitonic.c,v 1.1 2002/07/30 05:18:03 clleger Exp $
 */

/*
 * Copied almost exactly from
 * http://www.iti.fh-flensburg.de/lang/algorithmen/sortieren/bitonic/bitonicen.htm
 * That URL also has a nice explanation of the code and theory.
 */
#ifdef raw
#include <raw.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif
#include "Sort.h"
#include <math.h>

#define ASCENDING 1
#define DESCENDING 0

// n>=2  and  n<=Integer.MAX_VALUE
int power_of_two_less_than(int n) {
  int k = 1;
  while (k > 0 && k < n) {
    k *= 2;
  }
  return k / 2;
}

// swap_tuples attempts to be an oblivious swap.
// Ideally the execution trace of this will be the same
// regardless if the swap occurs.
void swap_tuples(int t1, int t2, table_t *t, bool to_swap) {
  tuple_t *tup1 = get_tuple(t1, t);
  tuple_t *tup2 = get_tuple(t2, t);

  bool d1 = tup1->is_dummy;
  bool d2 = tup2->is_dummy;
  if (to_swap) {
    tup1->is_dummy = d2;
    tup2->is_dummy = d1;
  } else {
    tup1->is_dummy = d1;
    tup2->is_dummy = d2;
  }

  for (int i = 0; i < t->schema.num_fields; i++) {
    switch (t->schema.fields[i].type) {
    case UNSUPPORTED: {
      throw;
    }
    case FIXEDCHAR: {
      // TODO(madhavsuresh): This is an "oblivious swap". may need to use
      // instructions
      char f1[16], f2[16];
      memcpy(&f1, tup1->field_list[i].f.fixed_char_field.val, 16);
      memcpy(&f2, tup2->field_list[i].f.fixed_char_field.val, 16);
      if (to_swap) {
        memcpy(tup1->field_list[i].f.fixed_char_field.val, &f2, 16);
        memcpy(tup2->field_list[i].f.fixed_char_field.val, &f1, 16);
      } else {
        memcpy(tup1->field_list[i].f.fixed_char_field.val, &f1, 16);
        memcpy(tup2->field_list[i].f.fixed_char_field.val, &f2, 16);
      }
      break;
    }
    case INT: {
      uint64_t f1, f2;
      f1 = tup1->field_list[i].f.int_field.val;
      f2 = tup2->field_list[i].f.int_field.val;
      if (to_swap) {
        tup1->field_list[i].f.int_field.val = f2;
        tup2->field_list[i].f.int_field.val = f1;
      } else {
        tup1->field_list[i].f.int_field.val = f1;
        tup2->field_list[i].f.int_field.val = f2;
      }
    } break;
    }
  }
}

void compare(int i, int j, bool dir, table_t *t, sort_t *s) {
  switch (t->schema.fields[s->colno].type) {
  case UNSUPPORTED:
    throw;
  case FIXEDCHAR: {
    throw;
    // printf("UNSUPPORTED\n");
  }
  case INT: {
    uint64_t i_val = get_tuple(i, t)->field_list[s->colno].f.int_field.val;
    uint64_t j_val = get_tuple(j, t)->field_list[s->colno].f.int_field.val;
    if (dir == (i_val > j_val)) {
      swap_tuples(i, j, t, true);
      // TODO(madhavsuresh): abstract this out to a function
    } else {
      swap_tuples(i, j, t, false);
    }
  } break;
  }
}

/** The procedure bitonicMerge recursively sorts a bitonic sequence in
 * ascending order, if dir = ASCENDING, and in descending order
 * otherwise. The sequence to be sorted starts at index position lo,
 * the number of elements is cnt.
 **/
void bitonicMerge(int lo, int n, bool dir, table_t *t, sort_t *s) {
  if (n > 1) {
    int m = power_of_two_less_than(n);
    for (int i = lo; i < lo + n - m; i++) {
      compare(i, i + m, dir, t, s);
    }
    bitonicMerge(lo, m, dir, t, s);
    bitonicMerge(lo + m, n - m, dir, t, s);
  }
}

/** Procedure bitonicSort first produces a bitonic sequence by
 * recursively sorting its two halves in opposite directions, and then
 * calls bitonicMerge.
 **/
void bitonicSort(int lo, int cnt, bool dir, table_t *t, sort_t *s) {
  if (cnt > 1) {
    int k = cnt / 2;
    bitonicSort(lo, k, !dir, t, s);
    bitonicSort(lo + k, cnt - k, dir, t, s);
    bitonicMerge(lo, cnt, dir, t, s);
  }
}

table_t *sort(table_t *t, sort_t *s) {
  int len = t->num_tuples;
  bitonicSort(0, t->num_tuples, s->ascending, t, s);
  return t;
}
