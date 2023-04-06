
#if !defined(mulMats_H)
#define mulMats_H

struct ssystem;
struct charge;

void mulMatDirect(ssystem *sys, double **trimat, double **sqrmat, int **real_index, int up_size, int eval_size);
void olmulMatPrecond(ssystem *sys);
void bdmulMatPrecond(ssystem *sys);
void mulMatUp(ssystem *sys);
void mulMatDown(ssystem *sys);
void mulMatEval(ssystem *sys);

void find_flux_density_row(ssystem *sys, double **to_mat, double **from_mat, int eval_row, int n_chg, int n_eval, int row_offset,
                      int col_offset, charge **eval_panels, charge **chg_panels, int *eval_is_dummy,
                      int *chg_is_dummy);

#endif
