#include <toad/toad.hh>

#include "DNDShape.hh"

void TDropShape::SetFromContext(TDragContext *dc)
{
	TDragShape *ds = dynamic_cast<TDragShape*>(dc);
	if (ds) {
		_shape = ds->_shape;
		sigDrop.Trigger(this);
		delete _shape;
		_shape = 0;
	}
}
