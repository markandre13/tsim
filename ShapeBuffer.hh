#ifndef TShapeBuffer
#define TShapeBuffer TShapeBuffer

#include <vector>
class TShape;
class TShapeBuffer: public vector<TShape*>
{
	public:
		iterator find(TShape *s) {
			iterator p = begin();
			while(p!=end()) {
				if (*p==s) break;
				p++;
			}
			return p;
		}
};

#endif