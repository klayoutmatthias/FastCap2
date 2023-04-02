
#define TOL 1e-10                /* tolerance on epsilon (machine precision) */
#define UPPEPS 1e-10              /* should be bigger than epsilon */
#define LOWEPS 1e-25             /* should be smaller than epsilon */

/*
  returns upper and lower bounds on machine precision (for doubles)
  - takes into account the limitations of the memory rep of a doubles
    by forcing the compiler to put all doubles in core
*/
void getEpsBnds(upper, lower)
double *upper, *lower;            /* current upper and lower epsilon bounds */
{
  double dif, tol, mid;
  double temp, one;

  double *difp = &dif;          /* to stop optimizer from putting */
  double *tolp = &tol;          /* variables in registers (not doing so */
  double *midp = &mid;          /* can lead to undully optomistic machine */
  double *tempp = &temp;        /* precision estimate that doesnt take */
  double *onep = &one;          /* memory storage rounding into account) */

  *upper = UPPEPS;
  *lower = LOWEPS;
  *onep = 1.0;

  *difp = *upper - *lower;
  *tolp = *difp/(*lower);
  while(*tolp > TOL) {
    *midp = (*upper + *lower)/2.0;
    *tempp = 1.0 + *midp;
    if(*tempp > *onep) *upper = *midp; 
    else *lower = *midp;
    *difp = *upper - *lower;
    *tolp = *difp/(*lower);
  }
}
