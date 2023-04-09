
#if !defined(patran_H)
#define patran_H

/***************************************************************************

  This is the header file for patran.c.  
  
  Written by Songmin Kim, July 24, 1990.

***************************************************************************/
/* Refer to Chapter 29, Neutral System, of PATRAN manual for explanation.
   The variable names are identical to those that appear in the manual. */

struct NODE {
  int ID;
  double coord[3];
};

struct ELEMENT {
  int ID, shape, num_nodes;
  int corner[4];
};

struct GRID {
  int ID, *equiv_ID, number_equiv_grids;
  double coord[3];
  GRID *next, *prev;
};

struct CFEG {
  int ID, NELS, LPH, LPH_ID, LSHAPE, NODES, ICONF, NDIM;
  CFEG *next, *prev;
  int *element_list;
};

struct PATCH {
  int ID, corner[4], conductor_ID;
  PATCH *next, *prev;
};

struct SM_PATCH {
  int ID, conductor_ID;
  SM_PATCH *next;
};

/* intermediate name struct; used for compatability with patran i/f */
struct NAME {
  char *name;
  SM_PATCH *patch_list;
  NAME *next;
};

#endif
