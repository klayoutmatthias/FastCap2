
#if !defined(zbufSort_H)
#define zbufSort_H

#include <cstdio>

struct ssystem;
struct face;

void getAdjGraph(ssystem *sys, face **faces, int numfaces, double *view, double rhs, double *normal);
void dumpCycles(ssystem *sys, face **faces, int numfaces);
face **depthSortFaces(ssystem *sys, face **faces, int numfaces);
double getPlane(double *normal, double *p1, double *p2, double *p3);
double dot(double *vec1, double *vec2);
void crossProd(double *out, double *in1, double *in2);

#endif
