
#if !defined(fastcap_solve_H)
#define fastcap_solve_H

struct ssystem;

double **fastcap_solve(ssystem *sys);
int capmatrix_size(const ssystem *sys);

#endif
