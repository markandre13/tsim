#include <toad/toad.hh>

#include "Simulate.hh"
#include "Shape.hh"
#include "Act.hh"
#include <iomanip.h>

bool TAct::Ready()
{
	return true;
}

// ActWait
//---------------------------------------------------------------------------
TActWait::TActWait(TShapeWait *c, Txact xact)
{
	_component = c;
	this->xact = xact;
}

void TActWait::Start()
{
	end = 	Clock()
			+ _component->interval 
			- _component->range
			+ (int)((1.0+(_component->range<<1))*rand()/(RAND_MAX+1.0));
}

void TActWait::End()
{
	_component->wakeUp(this);
}

// ActQueueWait
//---------------------------------------------------------------------------
TActQueueWait::TActQueueWait(TShapeQueue *q, Txact a)
	:TActWait(q,a)
{
	_queue = q;
}

bool TActQueueWait::Ready()
{
	return !_queue->Lock();
}

void TActQueueWait::Start()
{
	_queue->SetLock(true);
	super::Start();
}

void TActQueueWait::End()
{
	_queue->SetLock(false);
	super::End();
}
