/*
 * Copyright (c) 2000 Carnegie Mellon University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* Generating unrolled code for WHT building blocks
   ================================================

   This program creates C code for an unrolled 
     WHT_N with stride S
   on a vector x. S is a 2-power.

   The unrolled code is recursive!

   Only 2 temporary vars are used!
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* macros for convenience */
#define push(x)           fprintf(stdout, x)
#define push1(x, y)       fprintf(stdout, x, y)
#define push2(x, y, z)    fprintf(stdout, x, y, z)
#define push3(x, y, z, a) fprintf(stdout, x, y, z, a)

#define PRINT_USAGE_AND_EXIT \
  fprintf(stderr, \
    "error, usage: whtgen -n <num>\n"); \
  exit(-1);


/* pushwht( <init>, <size>, <stride> )
   -----------------------------------
     produces the unrolled code of a WHT(2^<size>) with <stride>
     starting at index <init>.
     The produced code is recursive.
*/
void pushwht(long init, long size, long stride) {
  long i;

  if (size == 2) {
    push1("  a = x[%ld * S];\n", init);
    push1("  b = x[%ld * S];\n", init+stride);
    push1("  x[%ld * S] = a + b;\n", init);
    push1("  x[%ld * S] = a - b;\n", init+stride);
  }
  else {
    pushwht(init, size/2, stride);
    pushwht(init+size/2, size/2, stride);
    for (i = init; i < init+size/2; i++)
      pushwht(i, 2, size/2);
  }
}


int main(int argc, char *argv[])
{
  long n;
  int k;
  char c;

  /* decode argument */
  if (argc == 1) {
    printf("usage: whtgen -n <num>\n");
    exit(0);
  }
  n = 0;
  while (++n < argc) {
    if (*argv[n]++ == '-') {
      c = *argv[n];
      switch (c) {
      case 'n':
        if (argc == n+1) {
          PRINT_USAGE_AND_EXIT;
        }
        sscanf(argv[++n], "%d", &k);
        if (k <= 0) {
          PRINT_USAGE_AND_EXIT;
        }
        break;
      default:
        fprintf(stderr, "error, illegal option %c\n", c);
        exit(-1);
      }
    }
    else {
      printf("error, options must be preceded by '-'\n");
      exit(-1);
    }  
  }

  n = (long) pow(2, k);

  /* print function header */
  push("/* This function has been automatically generated by whtgen. */\n\n");
  push1("/*\n  apply_small%d( <wht>, <S>, <x> )", k);
  push("\n  ------------------------------");
  if (k >= 10) {
    push("-");
  }
  push("\n  computes");
  push1("\n    WHT_(2^%d) with stride <S>", k);
  push("\n  on the vector <x>\n*/\n\n");

  /* includes */
  push("#include \"spiral_wht.h\"\n\n");

  /* function */
  push1("void apply_small%d(Wht *W, long S, wht_value *x)\n{\n", k);

  /* variable declaration */
  push("wht_value a, b;\n");
  push("\n");
 
  /* body */
  pushwht(0, n, 1);

  /* final brace */
  push("}\n"); 

  return 0;
}
