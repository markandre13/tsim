#include <toad/dragndrop.hh>

#include "Simulate.hh"
#include "Shape.hh"

class TDropShape: public TDropContext
{
	public:
		TDropShape(TWindow *w):TDropContext(w){};
		TShape* GetValue() const {
			return _shape;
		}
	protected:
		void SetFromContext(TDragContext*);
		TShape* _shape;
};

class TDragShape: public TDragContext
{
		friend TDropShape;
	public:
		TDragShape(TShape *s) {
			_shape = s;
		}
	protected:
		TShape* _shape;
};
