
#if !defined(mulStruct_H)
#define mulStruct_H

#include "patran.h"
#include "heap.h"
#include "vector.h"
#include "matrix.h"

#include <cstdio>
#include <set>

struct SurfaceData;
struct ssystem;

/* used to build linked list of conductor names */
struct Name {
  Name();
  char *name;
  Name *next;
  Name *alias_list;
  void add_alias(const ssystem *sys, const char *alias);
  const char *last_alias() const;
  bool match(const char *n) const;
};

/* used to make linked lists of iteration or conductor #s */
struct ITER {
  int iter;
  ITER *next;
};

struct Surface {                /* a surface file and its permittivities */
  Surface();
  int type;                     /* CONDTR, DIELEC or BOTH */
  Vector3d trans;               /* translation vector applied to surface */
  Matrix3d rot;                 /* Rotation/scale/flip matrix */
  Vector3d ref;                 /* reference point for normal alignment */
  int ref_inside;               /* TRUE=>ref point inside (on - side of) surf*/
  int end_of_chain;             /* TRUE=>end o/surf chain all w/same cond#'s */
  char *title;                  /* title inside surface file */
  char *name;                   /* surface file name (neutral or quick fmat) */
  SurfaceData *surf_data;       /* explicit surface data */
  char *group_name;             /* name of cond group (if any) surf part of */
  double outer_perm;            /* relative permitivity on side n pnts into
                                   (n is taken as the normal to 1st panel) */
  double inner_perm;            /* relative permitivity on other side
                                   (always zero for type = CONDTR) */
  struct charge *panels;        /* linked list of panels on this surface */
  int num_panels;               /* number of panels (incl dummies) this surf */
  int num_dummies;              /* number of dummy panels on this surf */
  struct Surface *next;         /* linked list pointers */
  struct Surface *prev;
};

struct charge {                 /* point charge */
  struct charge *next;          /* Next charge in linked list. */
  double corner[4][3];          /* Corner point coordinates. */
  int shape;                    /* 4=quad panel, 3 = triangular panel. */
  Vector3d *ref_pt;             /* Pointer to reference point or NULL */
  int index;                    /* charge value index in q array */
  double X[3], Y[3], Z[3];      /* Def coord system, Z is normal direction. */
  double max_diag;              /* Longest diagonal of panel. */
  double min_diag;              /* Shortest diagonal. */
  double length[4];             /* Edge lengths. */
  double area;                  /* Area of two triangluar regions. */
  double x, y, z;               /* Centroid of the quadlrilateral.  */
  double moments[16];           /* Moments of the panel. */
  double *multipole;            /* Temporarily Stores Q2M. */
  int cond;                     /* Conductor number. */
  int order;                    /* Multipole order. */
  /* things needed to do electric field evaluation by divided difference */
  int dummy;                    /* TRUE => dummy panel for elec field eval */
  struct Surface *surf;         /* surface that contains this panel */
  struct charge *pos_dummy;     /* eval pnt w/pos displacement from x,y,z */
  struct charge *neg_dummy;     /* eval pnt w/neg displacement from x,y,z */
};

struct cube {           
/* Definition variables. */
  int index;                    /* unique index */
  int level;                    /* 0 => root */
  double x, y, z;               /* Position of cube center. */
  int j, k, l;                  /* cube is cubes[level][j][k][l] */
  int flag;                     /* used for marking for tree walks */

/* Upward Pass variables. */
  int mul_exact;                /* TRUE => do not build a multipole expansn */
  struct cube *mnext;           /* Ptr to next cube on which to do multi. */
  int upnumvects;               /* 0 if empty,  1 on bot level if not empty, 
                                   else # nonempty kids. */ 
  int *upnumeles;               /* numeles[0] = # chgs on bot level, else
                                   number of terms in kid's expansion. */
  double **upvects;     /* vects[0] = chgs on bot level, else vectors
                                    of kids' expansion terms. */
  int multisize;        /* Number of terms in the expansion. */
  double *multi;        /* Vector of multi coefficients. */
  double ***upmats;     /* Matrices for chgs to multi or multi to multi.
                           upmats[i] is multisize x upnumeles[i]. */
  int *is_dummy;        /* is_dummy[i] = TRUE => panel i is a dummy panel
                           used for elec field eval - omit from upward pass */
  int *is_dielec;               /* is_dielec[i] = TRUE => panel i is on a surf
                                   of type DIELEC or BOTH */

/* Downward Pass variables. */
  int loc_exact;                /* TRUE => do not build a local expansion */
  struct cube *lnext;          /* Ptr to next cube on which to do local. */
  int downnumvects;             /* Number of cubes in iteraction list. */
  int *downnumeles;             /* # of eles in interact cube's expansion. */
  double **downvects;           /* Vects of interact cube's expansion. */

  int localsize;        /* Size of the local expansion */
  double *local;        /* Vector of local field coefs */
  double ***downmats;   /* Matrices for multi to chg, or multi to local
                           or local to local.  Downnumele x localsize. */

  struct cube **interList;      /* explicit interaction list 
                                   - for fake dwnwd passes and eval pass */
  int interSize;                /* number of elements in interList
                                   - often != downnumvects nor evalnumvects */

  /* evaluation pass variables */
  struct cube *enext;           /* pntr to next cube to evaluate */
  int evalnumvects;             /* for exact = #in inter list, o.w. = 1 */
  int *evalnumeles;             /* num of elements in inter list entry exp */
  double **evalvects;           /* multi, local, or chgs of ilist entry */

  double *eval;                 /* vector of evaluation pnt voltages in cube */
  double ***evalmats;           /* matrices for multi to potential, local to
                                   potential or charge to potential */

/* Direct portion variables. */
  struct cube *dnext;           /* Ptr to next cube on which to do direct. */
  struct cube *pnext;           /* Ptr to next cube on which to do precond. */
  struct cube *rpnext;          /* Reverse ptr to next cube to do precond. */
  int dindex;                   /* Used to determine lower triang portion. */
  int directnumvects;           /* Number of vects, self plus nbrs. */
  int *directnumeles;           /* # of elements in the nbrs chg vect. 
                                   directnumeles[0] = numchgs in cube. */
  double **directq;             /* Vecs of chg vecs, directq[0] this cube's. */
  double ***directmats;         /* Potential Coeffs in cube and neighbors. */
  double ***precondmats;        /* Precond Coeffs in cube and neighbors. */
  double **directlu;            /* Decomposed cube potential Coefficients. */
  double **precond;             /* Preconditioner. */
  double *prevectq;             /* The charge vector for the preconditioner. */
  double *prevectp;             /* The potential vector for preconditioner. */
  int presize;                  /* Size of the preconditioner. */
  int **nbr_is_dummy;           /* Dummy vectors corresponding to directq's */

/* Cube structure variables. */
  charge **chgs;          /* Array of charge ptrs. Only used lowest level. */
  struct cube **nbrs;     /* Array of ptrs to nonemptry nearest neighbors. */
  int numnbrs;            /* Number of nonempty neighbors. */
  struct cube **kids;     /* Array of children ptrs. */
  int numkids;            /* Number of kids. */
  struct cube *parent;    /* Ptr to parent cube. */
};

struct multi_mats
{
  multi_mats();

  int *localcnt, *multicnt, *evalcnt;     //  counts of builds done by level
  int **Q2Mcnt, **Q2Lcnt, **Q2Pcnt, **L2Lcnt; //  counts of xformation mats
  int **M2Mcnt, **M2Lcnt, **M2Pcnt, **L2Pcnt, **Q2PDcnt;

  double *Irn, *Mphi;           //  (1/r)^n+1, m*phi vect's
  double *Ir, *phi;             //  1/r and phi arrays, used to update above
  double *Rho, *Rhon;           //  rho and rho^n array
  double *Beta, *Betam;         //  beta and beta*m array
  double *tleg;                 //  Temporary Legendre storage.
  double **factFac;             //  factorial factor array: (n-m+1)...(n+m)

  double *sinmkB, *cosmkB, **facFrA;
};

enum dumpps_mode {
  DUMPPS_ON,
  DUMPPS_OFF,
  DUMPPS_ALL
};

enum dmpchg_mode {
  DMPCHG_ON,
  DMPCHG_OFF,
  DMPCHG_LAST
};

struct ssystem
{
  ssystem();

  const char **argv;            //  program arguments
  int argc;

  FILE *log;                    //  log stream (0 to turn off output)

  //  patran only
  PTState pts;

  //  misc global
  const char *kill_name_list;   //  cond names whose columns are omitted
  const char *kinp_name_list;   //  cond names omitted from input
  const char *qpic_name_list;   //  cond column names that get q picture
  const char *kq_name_list;     //  cond names removed from q picture

  std::set<int> kill_num_list;  //  corresponding number lists (cached)
  std::set<int> kinp_num_list;
  std::set<int> qpic_num_list;
  std::set<int> kq_num_list;

  double iter_tol;              //  iterative loop tolerence on ||r||

  //  command line option variables - all have to do with ps file dumping
  bool s_;                      //  true => insert showpage in .ps file(s)
  bool n_;                      //  true => number faces with input ordering
  bool g_;                      //  true => dump depth graph and quit
  bool c_;                      //  true => prbool command line on .ps file(s)
  bool x_;                      //  true => axes have been specified
  bool k_;
  bool rc_;                     //  true => rm conductors in list from pic
  bool rd_;                     //  true => remove all dielec i/fs from pic
  bool rb_;                     //  true => rm BOTH-types in list from pic
  bool q_;                      //  true => dump shaded plots of q_iter iters
  bool rk_;                     //  true => rm chg den key in -q plots
  bool m_;                      //  true => switch to plot gen mode
  bool f_;                      //  true => don't fill faces (no hidden l rm)
  bool dd_;                     //  true => dump ttl charges to .ps pictures
  double view[3];               //  absolute view point of 3D geometry
  double moffset[2];            //  image offset from lower left corner
  double elevation;             //  elevation of view rel to center of object
  double azimuth;               //  azimuth of view rel to center of object
  double rotation;              //  image rotation, degrees
  double distance;              //  relative distance from center (#radii-1)
  double linewd;                //  postscript line width
  double scale;                 //  over all image scale factor
  double axeslen;               //  axes lengths in 3D distance
  int up_axis;                  //  X,Y or Z => which axis is vertical in pic
  const char *line_file;        //  pointer to .fig superimposed line file

  //  solver configuration

  bool dirsol;                  //  solve Pq=psi by Gaussian elim.
  bool expgcr;                  //  do explicit full P*q products

  //  configuration options
  bool timdat;                  //  print timing data
  bool mksdat;                  //  dump symmetrized, MKS units cap mat
  dumpps_mode dumpps;           //  ON=> dump ps file w/mulMatDirect calcp's
                                //  ALL=> dump adaptive alg calcp's as well
  bool capvew;                  //  enable ps file dumps of geometry
  bool cmddat;                  //  dump command line info to output
  bool rawdat;                  //  dump unsymm, Gaussian units cap mat
  bool itrdat;                  //  dump residuals for every iteration
  bool cfgdat;                  //  dump configuration flags to output
  bool muldat;                  //  dump brief multipole setup info
  bool dissyn;                  //  display synopsis of cubes in lists
  bool dmtcnt;                  //  display xform matrix counts by level
  bool dissrf;                  //  display input surface information
  bool namdat;                  //  dump conductor names

  //  display of transformation matrices
  bool disq2m;                  //  display Q2M matrices when built
  bool dism2m;                  //  display M2M matrices when built
  bool dism2p;                  //  display M2P matrices when built
  bool disl2p;                  //  display L2P matrices when built
  bool disq2p;                  //  display Q2P matrices when built
  bool dsq2pd;                  //  display Q2PDiag matrices > build
  bool disq2l;                  //  display Q2L matrices when built
  bool dism2l;                  //  display M2L matrices when built
  bool disl2l;                  //  display L2L matrices when built
  bool dalq2m;                  //  display all Q2M matrix build steps
  bool dalm2p;                  //  display all M2P matrix build steps
  bool dall2p;                  //  display all L2P matrix build steps
  bool dalq2l;                  //  display all Q2L matrix build steps

  //  display of other intermediate results
  bool dupvec;                  //  display lev 1 upward pass vectors
  bool disfac;                  //  display factorial fractions in M2L
  bool dpsysd;                  //  display system after direct build
  bool dilist;                  //  display interaction lists
  bool dmpele;                  //  display electric flux densities
  dmpchg_mode dmpchg;           //  ON=> display all charge vector iterates
                                //  LAST=> display final charge vector

  //  misc debug
  bool ckdlst;                  //  check direct list, prnt msg if bad
  bool dmprec;                  //  dump P and Ctil to matlab file
  bool ckclst;                  //  check charge list, prnt msg if bad
  bool dpcomp;                  //  dump prec pts before&aft compression
  bool dpddif;                  //  dump divided difference components
  bool chkdum;                  //  print msg if dummy list inconsistent
  bool jacdbg;                  //  print random Jacob debug messages

  //  global variables
  char *ps_file_base;           //  pointer to base name for .ps files
  double ***axes;               //  for PS plot
  char *title;                  //  project title
  Surface *surf_list;           //  the list of surface descriptors
  int group_cnt;                //  next GROUPx number for automatic group numbering

  //  problem description
  int side;                     //  # cubes per side on lowest level.
  int depth;                    //  # of levels of cubes.
  int order;                    //  # of levels of cubes.
  int num_cond;                 //  number of conductors
  Name *cond_names;             //  conductor name list
  double perm_factor;           //  overall scale factor for permittivities
  double length;                //  Length per cube on lowest level.
  double minx, miny, minz;      //  Coordinates of one corner of the domain.
  int mul_maxq;                 //  max #panels in mul_exact cube
  int mul_maxlq;                //  max #panels in lowest level cube
  int max_panel;                //  max #panels in all cubes w/multipole
  int loc_maxq;                 //  max #evaluation points in loc_exact cube
  int loc_maxlq;                //  max #eval pnts in lowest level cube.
  int max_eval_pnt;             //  max #eval pnts in all cubes w/local exp
  double *q;                    //  The vector of lowest level charges.
  double *p;                    //  The vector of lowest level potentials.
  charge *panels;               //  linked list of charge panels in problem
  cube *****cubes;              //  The array of cube pointers.
  cube **multilist;             //  Array of ptrs to first cube in linked list
                                //    of cubes to do multi at each level.
  cube **locallist;             //  Array of ptrs to first cube in linked list
                                //    of cubes to do local at each level.
  cube *directlist;             //  head of linked lst of low lev cubes w/chg
  cube *precondlist;            //  head of linked lst of precond blks.
  cube *revprecondlist;         //  reversed linked lst of precond blks.
  int *is_dummy;                //  is_dummy[i] = TRUE => panel i is a dummy
  int *is_dielec;               //  is_dielec[i] = TRUE => panel i on dielec

  multi_mats mm;

  mutable Heap heap;            //  allocation heap

  std::set<int> get_conductor_number_set(const char *names) const;
  int get_conductor_number(const char *name);
  bool rename_conductor(const char *old_name, const char *new_name);
  int number_of(const Name *) const;
  Name *conductor_name(int i);
  const Name *conductor_name(int i) const;
  const char *conductor_name_str(int i) const;

  void reset_read();

  void msg(const char *fmt, ...) const;
  void info(const char *fmt, ...) const;
  void warn(const char *fmt, ...) const;
#if __cplusplus >= 201103L
  [[noreturn]] void error(const char *fmt, ...) const;
#else
  void error(const char *fmt, ...) const;
#endif
  void flush();

private:
  int get_unique_cond_num(const char *name, size_t nlen) const;
};

#endif

