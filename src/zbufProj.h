
#if !defined(zbufProj_H)
#define zbufProj_H

struct ssystem;
struct face;

void initFaces(face **faces, int numfaces, double *view);
void image(ssystem *sys, face **faces, int numfaces, line **lines, int numlines, double *normal, double rhs, double *view);
void flatten(ssystem *sys, face **faces, int numfaces, line **lines, int numlines, double rhs, double rotation, double *normal, double *view);
void makePos(ssystem *sys, face **faces, int numfaces, line **lines, int numlines);
void scale2d(ssystem *sys, face **faces, int numfaces, line **lines, int numlines, double scale, double *offset);
double *getAvg(ssystem *sys, face **faces, int numfaces, line **lines, int numlines, int flag);
double getSphere(ssystem *sys, double *avg, face **faces, int numfaces, line **lines, int numlines);
double getNormal(ssystem *sys, double *normal, double radius, double *avg, double *view, double distance);

#endif
