#include <toad/dragndrop.hh>

class TToggleButton;

BGN_CLASS(TToolBar, TWindow)
	void paint();
	void toolChange(TToggleButton*);
	void mouseMDown(int,int,unsigned);
	int _x;
END_CLASS
