#ifndef _SIMULATE_HH
#define _SIMULATE_HH

#include "ShapeBuffer.hh"
#include "ConnectionBuffer.hh"
class TAct;

typedef unsigned Txact;
typedef unsigned TTime;

void Simulate(TShapeBuffer&, TConnectionBuffer&);
void Add2Boundlist(TAct*);
TTime Clock();

extern ostream *log;

#endif
