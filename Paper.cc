#include <toad/toad.hh>
#include <toad/filedialog.hh>
#include <toad/file.hh>
#include <toad/macros.hh>

#include "Paper.hh"
#include "DNDShape.hh"

TFont* TPaper::font1 = 0;
TFont* TPaper::font2 = 0;
TPaper* TPaper::_current = NULL;

CONSTRUCTOR(TPaper)
{
	_current = this;
	bNoBackground = true;
	SetMouseMoveMessages(TMMM_LBUTTON);
	
	TDropShape *ds = new TDropShape(this);
	CONNECT(this,dropShape, ds,ds->sigDrop);
	
	font1 = new TFont(TFont::SANSSERIF, TFont::BOLD,  24);
	font2 = new TFont(TFont::SANSSERIF, TFont::PLAIN, 12);
	_selection = NULL;
	_clipboard = NULL;
}

void TPaper::SetTool(int t)
{
	_tool = t;
	if (_selection) {
		mx = _selection->x+16;
		my = _selection->y+16;
		Invalidate(*_selection, true);
		_selection = NULL;
	}
}

void TPaper::paintBackground()
{
}

void TPaper::paint()
{
	SetOrigin(0,0);
	TBitmap bmp(Width(), Height(), TBITMAP_SERVER);
	
	TPen pen(&bmp);
	pen.SetColor(191,127,127);
	pen.FillRectangle(0,0,Width(), Height());
	pen.SetColor(0,0,0);
	pen.SetFont(font1);
	
	if (_tool==TOOL_CONNECT && _selection) {
		pen.DrawLine(_selection->x+16, _selection->y+16, mx,my);
	}

	// paint connections
	//-------------------
	TConnectionBuffer::iterator q = _connection.begin();
	while(q!=_connection.end()) {
		(*q)->paint(pen);
		q++;
	}

	// paint shapes
	//--------------
	TShapeBuffer::iterator p = _shape.begin();
	while(p!=_shape.end()) {
		(*p)->paint(pen);
		p++;
	}
	
	// paint selection marker
	//------------------------
	if (_selection) {
		_selection->paint(pen);
		_selection->paintSelection(pen);
	}
	
	TPen scr(this);
	scr.DrawBitmap(0,0, &bmp);
}

TShape* TPaper::GetShape(int x, int y)
{
	TShapeBuffer::iterator p = _shape.begin();
	while(p!=_shape.end()) {
		if ((*p)->Distance(x,y)<=TShape::RANGE) {
//			mx = x-(*p)->x; my = y-(*p)->y;
			return *p;
		}
		p++;
	}
	return NULL;
}

void TPaper::dropShape(TDropShape *ds)
{
	TShape *s = ds->GetValue()->Clone();
	s->x = ds->X();
	s->y = ds->Y();

	_shape.push_back(s);

	TPen pen(this);
	pen.SetFont(font1);
	s->paint(pen);
}

void TPaper::mouseRDown(int x,int y,unsigned)
{
	TShape *s = GetShape(x,y);
	if (!s) {
		TConnectionBuffer::iterator p = _connection.begin();
		while(p!=_connection.end()) {
			if ((*p)->Distance(x,y)<=TShape::RANGE) {
				s = *p;
				break;
			}
			p++;
		}
	}
	if (s)
		s->edit();
}

void TPaper::mouseLDown(int x,int y,unsigned)
{
	TShape *s = GetShape(x,y);
	switch(_tool)
	{
		case TOOL_SELECT:
			if (_selection != s) {
				if (_selection)
					Invalidate(*_selection, true);
				_selection = s;
				if (_selection) {
					Invalidate(*_selection, true);
					mx = x - _selection->x;
					my = y - _selection->y;
				}
			}
			break;
		case TOOL_CONNECT:
			mx = x; my = y;
			if (!s)
				break;
			if (_selection) {
				Invalidate(*_selection, true);
				_selection = NULL;
			} else {
				if (!_selection) {
					_selection = s;
					Invalidate(*_selection, true);
				} else {
					Connect(_selection, s);
					Invalidate();
					_selection = NULL;
				}
			}
			break;
	}
}

void TPaper::mouseMove(int x,int y,unsigned)
{
	switch(_tool)
	{
		case TOOL_SELECT:
			if (_selection) {
				_selection->x = x-mx;
				_selection->y = y-my;
				if (_selection->x<0) _selection->x=0;
				if (_selection->y<0) _selection->y=0;
				Invalidate();
			}
			break;
		case TOOL_CONNECT:
			if (_selection) {
				TRect r;
				r.Set(_selection->x+16, _selection->y+16,
					  mx-(_selection->x+16),my-(_selection->y+16));
				r.Adjust();
				r.w+=3; r.h+=3; r.x--; r.y--;
				Invalidate(r,true);
				mx = x;
				my = y;
				r.Set(_selection->x+16, _selection->y+16,
					  mx-(_selection->x+16),my-(_selection->y+16));
				r.Adjust();
				r.w+=3; r.h+=3; r.x--; r.y--;
				Invalidate(r,true);
			}
	}
}

void TPaper::mouseLUp(int x,int y,unsigned)
{
	switch(_tool)
	{
		case TOOL_CONNECT:
			if (_selection)
			{
				TShape *s = GetShape(x,y);
				if (s) {
					if (s==_selection)
						return;
					Connect(_selection,s);
				}
				Invalidate();
				_selection=NULL;
			}
	}
}

//. Create connection from <VAR>src</VAR> to <VAR>dst</VAR>.
void TPaper::Connect(TShape *src, TShape *dst)
{
	if (src->AcceptSend() && dst->AcceptRecv())
	{	
		TShapeConnection *c = new TShapeConnection(src,dst);
		_connection.push_back(c);
	}
}

//. Delete selected Shape from the shape buffer and all its connections.
void TPaper::DeleteSelection()
{
	if (_selection) {
		// remove connections from connection buffer
		TConnectionBuffer::iterator q;
		q = _connection.begin();
		while(q!=_connection.end()) {
			if ((*q)->src==_selection || (*q)->dst==_selection) {
				delete *q;
				_connection.erase(q);
			}
			else
				q++;
		}
		
		// remove selected shape from shape buffer
		TShapeBuffer::iterator p;
		p = _shape.find(_selection);
		_shape.erase(p);
		delete _selection;
		_selection = NULL;

		Invalidate();
		return;
	}
}

void TPaper::New()
{
	_selection = 0;
	_shape.erase(_shape.begin(), _shape.end());
	_connection.erase(_connection.begin(), _connection.end());
	Invalidate();
}

void TPaper::Save()
{
	if (filename.size()==0) {
		SaveAs();
		return;
	}

	TFile file;
	if (!file.Open(filename, "w")) {
		string msg = "Couldn't open file \"";
		msg+=filename;
		msg+="\" for writing.";
		MessageBox(this,"Save Error", msg, MB_ICONSTOP | MB_OK );
		return;
	}
	
	file.WriteString("TSIM0001",8);
		
	// store shapes
	//--------------
	ulong counter = 1;
	TShapeBuffer::iterator sp;
	sp = _shape.begin();
	while(sp!=_shape.end()) {
		file.WriteWord( (*sp)->TypeID() );
		(*sp)->Store(file);
		(*sp)->number = counter;
		counter++;
		sp++;
	}
	file.WriteWord(0);
	
	// store connections
	//-------------------
	TConnectionBuffer::iterator cp;
	cp = _connection.begin();
	while(cp!=_connection.end()) {
		file.WriteWord( (*cp)->TypeID() );
		(*cp)->Store(file);
		file.WriteDWord((*cp)->src->number);
		file.WriteDWord((*cp)->dst->number);
		cp++;
	}
	file.WriteWord(0);
}

void TPaper::Open()
{
	New();
	TFileDialog dlg(this, "Open..");
	dlg.AddFileType("Tiny Simulation","*.tsim", NULL);
	dlg.AddFileType("All files","*", NULL);
	dlg.DoModal();
	if (dlg.GetResult()!=IDOK)
		return;
	filename = dlg.GetFilename();
	if (filename.size()==0)
		return;
	TFile file;
	if (!file.Open(filename, "r")) {
		string msg = "Couldn't open file \"";
		msg+=filename;
		msg+="\" for reading.";
		MessageBox(this,"Open Error", msg, MB_ICONSTOP | MB_OK );
		return;
	}
	if (!file.CompareString("TSIM0001",8))
	{
		string msg = "\"";
		msg+=filename;
		msg+="\" isn't a Tiny Simulator file.";
		MessageBox(this,"Open Error", msg, MB_ICONSTOP | MB_OK );
		return;
	}
	unsigned type;
	while(true) {
		TShape *s;
		file.ReadWord(&type);
		if (type==0)
			break;
		switch(type) {
			case 3:
				s = new TShapeTerminator();
				break;
			case 5:
				s = new TShapeGenerator();
				break;
			case 6:
				s = new TShapeAdvance();
				break;
			case 7:
				s = new TShapeQueue();
				break;
		}
		s->Restore(file);
		_shape.push_back(s);
	}
	while(true)
	{
		TShapeConnection *s;
		ulong l;
		file.ReadWord(&type);
		if (type==0)
			break;
		switch(type) {
			case 4:
				s = new TShapeConnection(NULL,NULL);
				break;
		}
		s->Restore(file);
		file.ReadDWord(&l);
		s->src = _shape[l-1];
		file.ReadDWord(&l);
		cout << l << endl;
		s->dst = _shape[l-1];
		_connection.push_back(s);
	}
	Invalidate();
}


void TPaper::SaveAs()
{
	TFileDialog dlg(this, "Save..");
	dlg.AddFileType("Tiny Simulation","*.tsim", NULL);
	dlg.AddFileType("All files","*", NULL);
	dlg.DoModal();
	if (dlg.GetResult()!=IDOK)
		return;
	filename = dlg.GetFilename();
	if (filename.size()!=0)
		Save();
}
