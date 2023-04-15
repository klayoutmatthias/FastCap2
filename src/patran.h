
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

/* Provides all information the PATRAN reader needs */
struct PTState
{
  NAME *start_name;             //  conductor name linked list head
  NAME *current_name;           //  conductor name linked list tail
  NAME *start_name_this_time;   //  cond name list for the current surface

  int ID;
  int IV;
  int KC;
  int N1;
  int N2;
  int N3;
  int N4;
  int N5;
  int number_nodes;
  int number_elements;
  int number_grids;
  int number_patches;
  NODE *list_nodes;
  NODE *current_node;
  NODE **node_search_table;
  ELEMENT *list_elements;
  ELEMENT *current_element;
  ELEMENT **element_search_table;
  GRID *start_grid;
  PATCH *start_patch;
  CFEG *start_cfeg;

  /* these are now only used for temporary name storage for patran surfaces */
  int conductor_count;

  /* these flags added to allow multiple calls; used to reset static variables */
  int first_grid;                 /* note that current_name static is not */
  int first_patch;                /*   reset since the name list must */
  int first_cfeg;                 /*   be preserved as new files are read */
};

#endif
