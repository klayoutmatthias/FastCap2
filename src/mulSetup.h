
#if !defined(mulSetup_H)
#define mulSetup_H

struct ssystem;
struct charge;

void mulInit(ssystem *sys, int autom, int depth, int order, charge *charges);

#endif
