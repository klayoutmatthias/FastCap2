
#if !defined(zbufInOut_H)
#define zbufInOut_H

#include <cstdio>

struct ssystem;
struct face;
struct line;
struct charge;

void dumpFaceText(ssystem *sys, face **faces, int numfaces);

void copyBody(FILE *fp);
void dump_line_as_ps(FILE *fp, char *psline, double x_position, double y_position, double font_size);
void dumpPs(ssystem *sys, face **faces, int numfaces, line **lines, int numlines, FILE *fp, const char **argv, int argc, int use_density, double black, double white);
face **fastcap2faces(ssystem *sys, int *numfaces, charge *chglist, double *q, int use_density, double *black, double *white);
line **getLines(ssystem *sys, const char *line_file, int *numlines);

#endif
