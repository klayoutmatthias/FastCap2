
#if !defined(zbufStruct_H)
#define zbufStruct_H

/* structs used by .ps file dumping part of code (zbuf)--somewhat redundant */

struct face {
  int numsides;                 /* number of sides this face has */
  double **c;                   /* corners of the face */
  double normal[3];             /* normal to the face's plane */
  double rhs;                   /* rhs for the face's plane equation */
  int index;                    /* input order index */
  int depth;                    /* depth index - lower numbers are deeper */
  int mark;                     /* flag for topological depth ordering */
  double greylev;               /* 0 (white) to 1 (black), default = GREYLEV */
  double width;                 /* line width, default = LINE */
  int numbehind;                /* number of faces this face is behind */
  struct face **behind;         /* pntrs to faces this face is behind */
  struct face *prev;
  struct face *next;
};
typedef struct face face;

struct line {
  double from[3];
  double to[3];
  int index;
  int width;
  double arrow;                 /* != 0.0 => put arrow hd on to end this sz */
  double dot;                   /* != 0.0 => put dot on to end this sz */
  struct line *prev;
  struct line *next;
};
typedef struct line line;

#endif
