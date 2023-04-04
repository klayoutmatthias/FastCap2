
#if !defined(distri_H)
#define distri_H

int disTri(
  FILE *fp, 
  int cond,                           //  conductor number 
  double edgefrac,                    //  edge cell widths =edgefrac*(inner widths)
  int ncells,                         //  number of cells on short side, > 2
  bool no_discr,                      //  true => no discr., just wr the four pnts
  double x1, double y1, double z1,    //  3 corners
  double x2, double y2, double z2, 
  double x3, double y3, double z3
);

#endif

