
#if !defined(mulDo_H)
#define mulDo_H

struct ssystem;

void printops(ssystem *sys);
void mulPrecond(ssystem *sys, int type);
void mulDirect(ssystem *sys);
void mulUp(ssystem *sys);
void mulDown(ssystem *sys);
void mulEval(ssystem *sys);

#endif
