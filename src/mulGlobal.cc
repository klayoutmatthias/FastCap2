
#include "mulGlobal.h"
#include "mulStruct.h"

/*
  misc global
*/
NAME *start_name = NULL;        /* conductor name linked list head */
NAME *current_name;             /* conductor name linked list tail */
NAME *start_name_this_time;     /* cond name list for the current surface */
const char *kill_name_list;     /* cond names whose columns are omitted */
ITER *kill_num_list;            /* cond numbers whose columns are omitted */
const char *kinp_name_list;     /* cond names omitted from input */
ITER *kinp_num_list;            /* cond numbers omitted from input */
const char *qpic_name_list;     /* cond column names that get q picture */
ITER *qpic_num_list;            /* cond column names that get q picture */
const char *kq_name_list;       /* cond names removed from q picture */
ITER *kq_num_list;              /* cond numbers removed from q picture */

double iter_tol;                /* iterative loop tolerence on ||r|| */

/*
  command line option variables - all have to do with ps file dumping
*/
int s_;                         /* TRUE => insert showpage in .ps file(s) */
int n_;                         /* TRUE => number faces with input ordering */
int g_;                         /* TRUE => dump depth graph and quit */
int c_;                         /* TRUE => print command line on .ps file(s) */
int x_;                         /* TRUE => axes have been specified */
int k_;
int rc_;                        /* TRUE => rm conductors in list from pic */
int rd_;                        /* TRUE => remove all dielec i/fs from pic */
int rb_;                        /* TRUE => rm BOTH-types in list from pic */
int q_;                         /* TRUE => dump shaded plots of q_iter iters */
int rk_;                        /* TRUE => rm chg den key in -q plots */
int m_;                         /* TRUE => switch to plot gen mode */
int f_;                         /* TRUE => don't fill faces (no hidden l rm) */
int dd_;                        /* TRUE => dump ttl charges to .ps pictures */
double view[3];                 /* absolute view point of 3D geometry */
// double moffset[2];           /* image offset from lower left corner */
double elevation;               /* elevation of view rel to center of object */
double azimuth;                 /* azimuth of view rel to center of object */
double rotation;                /* image rotation, degrees */
double distance;                /* relative distance from center (#radii-1) */
double linewd;                  /* postscript line width */
double scale;                   /* over all image scale factor */
double axeslen;                 /* axes lengths in 3D distance */
int up_axis;                    /* X,Y or Z => which axis is vertical in pic */
char *line_file;                /* pointer to .fig superimposed line file */
