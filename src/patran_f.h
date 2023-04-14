
#if !defined(patran_f_H)
#define patran_f_H

#include <cstdio>

#include "vector.h"
#include "matrix.h"

struct ssystem;
struct charge;

charge *patfront(ssystem *sys, FILE *stream, const char *header, int surf_type, const Matrix3d &rot, const Vector3d &trans, const char *name_suffix, char **title);

#endif
