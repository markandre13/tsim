#include <toad/toad.hh>
#include <toad/dialog.hh>
#include <toad/pushbutton.hh>
#include <toad/textfield.hh>

#define log c_log
#include <math.h>
#undef log

#include "Simulate.hh"
#include "Shape.hh"
#include "Act.hh"

// font1 & font2
#include <toad/macros.hh>
#include "Paper.hh"

#include <iomanip.h>


// TShape
//-------------------------------------------------------------------
TShape::TShape()
{
	x=y=0;
	w=h=32;
}

TShape::~TShape()
{
	cerr << "DESTROYING SHAPE " << title;
}

void TShape::paint(TPen &pen)
{
	if (title.size()) {
		pen.SetFont(TPaper::font2);
		pen.SetColor(0,0,0);
		pen.SetBackground(255,255,255);
		int tw = pen.Width(title);
		int th = pen.Height();
		int tx = x+(w-tw)/2;
		pen.FillRectangle(tx-2, y+h, tw+4, th+4);
		pen.DrawString(tx, y+h+2, title);
		pen.SetFont(TPaper::font1);
	}
}

double TShape::Distance(int tx, int ty) const
{
	return (x <= tx && tx<=x+w && y<=ty && ty<=y+h) ? RANGE : OUT_OF_RANGE;
}

void TShape::GetCenter(TPoint *p)
{
	p->x = x+(w>>1);
	p->y = y+(h>>1);
}

bool TShape::AcceptSend()
{
	return true;
}

bool TShape::AcceptRecv()
{
	return true;
}

void TShape::Send(Txact xact)
{
	double p = 0.0;
	double q = (double)rand()/RAND_MAX;
	double m = 0.0;
	
	TConnectionBuffer::iterator c;

	c = dst.begin();
	while(c!=dst.end()) {
		m+=(*c)->p;
		c++;
	}
	
	c = dst.begin();
	while(c!=dst.end()) {
		p+=(*c)->p / m;
		if (q<=p) {
			(*c)->dst->recv(xact);
			return;
		}
		c++;
	}
	cerr << "FATAL: FAILED TO TRIGGER CONNECTION" << endl;
}

void TShape::recv(Txact)
{
}

void TShape::paintSelection(TPen &pen)
{
	pen.SetColor(0,0,0);
	pen.DrawRectangle(x+1,y+1,w-2,w-2);
}

// TShapeDlg
//---------------------------------------------------------------------------
BGN_CLASS(TShapeDlg, TDialog)
	public:
		void paint();
		virtual void paint(TPen&);
		void setup(TShape*);
END_CLASS

CONSTRUCTOR(TShapeDlg)
{
	SetSize(320,30+30);
}

void TShapeDlg::setup(TShape *s)
{
	TTextField *tf;
	TPushButton *btn;
	tf = NewTextField(this, "shapetitle", &s->title);
		tf->SetShape(60,5,Width()-60-5,19);

	int x = Width()/2 - 85;
	btn = new TPushButton(this, "Cancel");
		btn->SetShape(x,Height()-25,80,20);
		CONNECT(this,closeRequest, btn,btn->sigActivate);
	btn = new TPushButton(this, "Ok");
		btn->SetShape(x+90,Height()-25,80,20);
		CONNECT(this,Ok, btn,btn->sigActivate);
}

void TShapeDlg::paint()
{
	TPen pen(this);
	paint(pen);
}

void TShapeDlg::paint(TPen &pen)
{
	pen.DrawString(5,5+2, "Name");
}

void TShape::edit()
{
	TShapeDlg dlg(NULL, "Shape Attributes");
	dlg.setup(this);
	dlg.DoModal();
}

void TShape::Store(TFile &file)
{
	file.WriteDWord(x);
	file.WriteDWord(y);
	file.WriteDWord(title.size());
	file.WriteString(title);
}

void TShape::Restore(TFile &file)
{
	unsigned long l;
	file.ReadDWord(&l); x = l;
	file.ReadDWord(&l); y = l;
	file.ReadDWord(&l);
	char buffer[l+1];
	file.ReadString(buffer,l);
	buffer[l]=0;
	title = buffer;
}

// TShapeWaitDlg
//---------------------------------------------------------------------------
BGN_CLASS(TShapeWaitDlg, TShapeDlg)
	public:
		void paint(TPen&);
		void setup(TShapeWait*);
END_CLASS

CONSTRUCTOR(TShapeWaitDlg)
{
	SetSize(320,30+55+30);
}

void TShapeWaitDlg::setup(TShapeWait *s)
{
	super::setup(s);
	TTextField *tf;
	tf = NewTextField(this, "shapetitle", &s->interval);
		tf->SetShape(60,30,Width()-60-5,19);
	tf = NewTextField(this, "shapetitle", &s->range);
		tf->SetShape(60,55,Width()-60-5,19);
}

void TShapeWaitDlg::paint(TPen &pen)
{
	super::paint(pen);
	pen.DrawString(5,30+2, "Duration");
	pen.DrawString(5,55+2, "Range");
}

void TShapeWait::edit()
{
	TShapeWaitDlg dlg(NULL, "ShapeWait Attributes");
	dlg.setup(this);
	dlg.DoModal();
}

void TShapeWait::Store(TFile &file)
{
	super::Store(file);
	file.WriteDWord(interval);
	file.WriteDWord(range);
}

void TShapeWait::Restore(TFile &file)
{
	super::Restore(file);
	ulong l;
	file.ReadDWord(&l); interval = l;
	file.ReadDWord(&l); range = l;
}

// TShapeConnection
//-------------------------------------------------------------------
void TShapeConnection::paint(TPen &pen)
{
	if (!src && !dst) {
		cerr << "OPEN CONNECTION" << endl;
		return;
	}

	TPoint p1, p2;
	src->GetCenter(&p1);
	dst->GetCenter(&p2);
	pen.DrawLine(p1.x,p1.y, p2.x,p2.y);

	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;

	double phi = atan( dy / dx );
	if (dx<0.0)
		phi=phi+M_PI;

	int cx=p2.x-dx/2, cy = p2.y-dy/2;
	double r=10.0, tr=3.0*M_PI_4;
	TPoint p[3];
	p[0].Set(cx, cy);
	p[1].Set(cx+r*cos(phi+tr), cy+r*sin(phi+tr));
	p[2].Set(cx+r*cos(phi-tr), cy+r*sin(phi-tr));
	pen.SetColor(255,255,255);
	pen.FillPolygon(p,3);
	pen.SetColor(0,0,0);
	pen.DrawPolygon(p,3);

	// draw title
	if (title.size()) {
		pen.SetFont(TPaper::font2);
		pen.SetColor(0,0,0);
		pen.SetBackground(255,255,255);
		int tw = pen.Width(title);
		int th = pen.Height();
		int tx = (p2.x-p1.x)/2+p1.x-tw/2;
		int ty = (p2.y-p1.y)/2+p1.y-th/2;
		pen.FillRectangle(tx-2, ty-2, tw+4, th+4);
		pen.DrawString(tx, ty, title);
		pen.SetFont(TPaper::font1);
	}
}

TShape* TShapeConnection::Clone()
{
	cerr << "MUSTN'T CLONE A CONNECTION, DOESN'T MAKE SENSE" << endl;
	return NULL;
}

static int ccw(TPoint p0, TPoint p1, TPoint p2)
{
	int dx1, dx2, dy1, dy2;
	dx1 = p1.x - p0.x; dy1 = p1.y - p0.y;
	dx2 = p2.x - p0.x; dy2 = p2.y - p0.y;
	if (dx1*dy2 > dy1*dx2) return 1;
	if (dx1*dy2 < dy1*dx2) return -1;
	if ((dx1*dx2<0) || (dy1*dy2<0)) return -1;
	if ((dx1*dx1+dy1*dy1) < (dx2*dx2+dy2*dy2)) return 1;
	return 0;
}

double TShapeConnection::Distance(int itx, int ity) const
{
	TPoint p1, p2;
	src->GetCenter(&p1);
	dst->GetCenter(&p2);
	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;

	double phi = atan( dy / dx );
	if (dx<0.0)
		phi=phi+M_PI;

	int cx=p2.x-dx/2, cy = p2.y-dy/2;
	double r=10.0, tr=3.0*M_PI_4;
	TPoint p[5];
	
	p[0].Set(itx,ity);
	p[1].Set(-10,ity);
	
	p[2].Set(cx, cy);
	p[3].Set(cx+r*cos(phi+tr), cy+r*sin(phi+tr));
	p[4].Set(cx+r*cos(phi-tr), cy+r*sin(phi-tr));

	int n=0;
	int l11,l12,l21,l22;
	l21=0;
	l22=1;
	for(int i=0; i<3; i++) {
		switch(i) {
			case 0:
				l11=2;
				l12=3;
				break;
			case 1:
				l12=4;
				break;
			case 2:
				l11=3;
				break;
		}
		if ((ccw(p[l11], p[l12], p[l21])*ccw(p[l11], p[l12], p[l22]) <= 0)
		 && (ccw(p[l21], p[l22], p[l11])*ccw(p[l21], p[l22], p[l12]) <= 0) )
			n++;
	}
	return n&1 ? RANGE : OUT_OF_RANGE;
}

void TShapeConnection::Store(TFile &file)
{
	super::Store(file);
	ulong l = (ulong)(p*1000000000.0);
	file.WriteDWord(l);
}

void TShapeConnection::Restore(TFile &file)
{
	super::Restore(file);
	ulong l;
	file.ReadDWord(&l); p = (double)l / 1000000000.0;
}

// TShapeConnectionDlg
//---------------------------------------------------------------------------
BGN_CLASS(TShapeConnectionDlg, TShapeDlg)
	public:
		void paint(TPen&);
		void setup(TShapeConnection*);
END_CLASS

CONSTRUCTOR(TShapeConnectionDlg)
{
	SetSize(320,30+30+30);
}

void TShapeConnectionDlg::setup(TShapeConnection *s)
{
	super::setup(s);
	TTextField *tf;
	tf = NewTextField(this, "shapetitle", &s->p);
		tf->SetShape(60,30,Width()-60-5,19);
}

void TShapeConnectionDlg::paint(TPen &pen)
{
	super::paint(pen);
	pen.DrawString(5,30+2, "Weight");
}

void TShapeConnection::edit()
{
	TShapeConnectionDlg dlg(NULL, "Connection Attributes");
	dlg.setup(this);
	dlg.DoModal();
}


// TShapeGenerator
//-------------------------------------------------------------------
unsigned TShapeGenerator::xact_counter;

void TShapeGenerator::paint(TPen &pen)
{
	const char* t = "G";
	pen.SetColor(0,0,0);
	pen.SetBackground(127,191,255);
	pen.FillRectangle(this);
	pen.DrawString( ((w-pen.Width(t))>>1)+x , ((h-pen.Height())>>1)+y, t,1);
	super::paint(pen);
}

void TShapeGenerator::wakeUp(TActWait *act)
{
	(*log)	<< setw(5)	<< Clock() << ":"
			<< setw(5)	<< xact_counter << ":"
			<< setw(15) << "GENERATE " << "\""
			<< setw(20) << title << "\"\n";
	Send(xact_counter);
	xact_counter++;

	act->Bind();	// reuse activity for another wait
}

TShape* TShapeGenerator::Clone()
{
	return new TShapeGenerator(*this);
}

// TShapeTerminator
//-------------------------------------------------------------------
void TShapeTerminator::paint(TPen &pen)
{
	const char* t = "T";
	pen.SetColor(0,0,0);
	pen.SetBackground(127,127,191);
	pen.FillRectangle(this);
	pen.DrawString( ((w-pen.Width(t))>>1)+x , ((h-pen.Height())>>1)+y, t);
	super::paint(pen);
}

TShape* TShapeTerminator::Clone()
{
	return new TShapeTerminator(*this);
}

void TShapeTerminator::recv(Txact xact)
{
	(*log)	<< setw(5)	<< Clock() << ":"
			<< setw(5)	<< xact << ":"
			<< setw(15) << "TERMINATE " << "\""
			<< setw(20) << title << "\"\n";
}

// TShapeAdvance
//-------------------------------------------------------------------
void TShapeAdvance::paint(TPen &pen)
{
	const char* t = "A";
	pen.SetColor(0,0,0);
	pen.SetBackground(191,0,0);
	pen.FillRectangle(this);
	pen.DrawString( ((w-pen.Width(t))>>1)+x , ((h-pen.Height())>>1)+y, t);
	super::paint(pen);
}

TShape* TShapeAdvance::Clone()
{
	return new TShapeAdvance(*this);
}

void TShapeAdvance::recv(Txact xact)
{
	(*log)	<< setw(5)	<< Clock() << ":"
			<< setw(5)	<< xact << ":"
			<< setw(15) << "GRAB " << "\""
			<< setw(20) << title << "\"\n";
	Add2Boundlist(new TActWait(this, xact));
}

void TShapeAdvance::wakeUp(TActWait *act)
{
	(*log)	<< setw(5)	<< Clock() << ":"
			<< setw(5)	<< act->xact << ":"
			<< setw(15) << "UNGRAB " << "\""
			<< setw(20) << title << "\"\n";
	Send(act->xact);
}

// TShapeQueue
//-------------------------------------------------------------------
void TShapeQueue::paint(TPen &pen)
{
	const char* t = "Q";
	pen.SetColor(0,0,0);
	pen.SetBackground(0,191,0);
	pen.FillRectangle(this);
	pen.DrawString( ((w-pen.Width(t))>>1)+x , ((h-pen.Height())>>1)+y, t);
	super::paint(pen);
}

TShape* TShapeQueue::Clone()
{
	return new TShapeQueue(*this);
}

void TShapeQueue::recv(Txact xact)
{
	// push XACT as a waiting activity on the stack
	Add2Boundlist(new TActQueueWait(this, xact));

	(*log)	<< setw(5)	<< Clock() << ":"
			<< setw(5)	<< xact << ":"
			<< setw(15) << "QUEUE ENTRY " << "\""
			<< setw(20) << title << "\"\n";
}

void TShapeQueue::wakeUp(TActWait *act)
{
	(*log)	<< setw(5)	<< Clock() << ":"
			<< setw(5)	<< act->xact << ":"
			<< setw(15) << "QUEUE EXIT " << "\""
			<< setw(20) << title << "\"\n";
	Send(act->xact);
}
