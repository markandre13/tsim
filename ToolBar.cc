#include <toad/toad.hh>
#include <toad/togglebutton.hh>
#include <toad/macros.hh>

// #include "MainWindow.hh"
#include "ToolBar.hh"
#include "Paper.hh"
#include "DNDShape.hh"

// guess what, we're using a prototype pattern:
#define NSHAPES 4
TShape* _shape[NSHAPES];

CONSTRUCTOR(TToolBar)
{
	SetBackground(TCOLOR_DIALOG);

	TToggleButton *btn;
	int y=0, h=36, w=60, ys=0, xs=w;
	_x = 0;
	SetSize(1,36);
	
	btn=new TToggleButton(this, "Select", TOOL_SELECT);
		btn->SetShape(_x,y,w,h); y+=ys; _x+=xs;
		CONNECT(this,this->toolChange, btn,btn->sigActivate);
		btn->SetDown();

	btn=new TToggleButton(this, "Connect", TOOL_CONNECT);
		btn->SetShape(_x,y,w,h); y+=ys; _x+=xs;
		CONNECT(this,this->toolChange, btn,btn->sigActivate);
		
	_x += 3;
	y = 2;
	
	xs=32+3;
	_shape[0] = new TShapeGenerator();
	_shape[0]->y = y; _shape[0]->x = _x; _x+=xs;
	_shape[1] = new TShapeAdvance();
	_shape[1]->y = y; _shape[1]->x = _x; _x+=xs;
	_shape[2] = new TShapeQueue();
	_shape[2]->y = y; _shape[2]->x = _x; _x+=xs;
	_shape[3] = new TShapeTerminator();
	_shape[3]->y = y; _shape[3]->x = _x; _x+=xs;
}

void TToolBar::toolChange(TToggleButton *tb)
{
	TPaper::Current()->SetTool(tb->ID());
}

void TToolBar::paint()
{
	TPen pen(this);
	pen.SetFont(TPaper::font1);
	for(int i=0; i<NSHAPES; i++)
		_shape[i]->paint(pen);
}

void TToolBar::mouseMDown(int x,int y,unsigned)
{
	for(int i=0; i<NSHAPES; i++) {
		if (_shape[i]->Distance(x,y)<=TShape::RANGE)
		{
			StartDrag(new TDragShape(_shape[i]->Clone()));
			return;
		}
	}
}
