#include <toad/toad.hh>
#include <toad/scrolledarea.hh>
#include <toad/macros.hh>

#include "textarea.hh"

//#include <strstream.h>
#include "membuf.hh"

/*
	When editing large text => Use EMACS
	This implementation is moving to much data but i guess,
	text written with this class won't excess 64kB anyway.
	
	The TextArea line editing functions should be moved in to
	separate class, so anyone can use them for other things.
*/


void TTextArea::SetValue(const string &str)
{
//	printf("%s", str.c_str());

	if (_buffer)
		delete _buffer;
	_buffer_size = str.size()+1;
	_buffer_allocated = 0;
	_alloc_buffer();
	_line_modified = false;
	memcpy(_buffer, str.c_str(), _buffer_size);
	char *p = _buffer+_buffer_size;
	_cx = _cy = _sy = _sx = 0;
	_tw = _th = 1;
	_by = _buffer;
	SetVisibleAreaPos(0,0);
	SetAreaSize(1,1);

	while(p>_buffer) {
		if (*p=='\n') {
			_th++;
			*p=0;
		}
		p--;
	}
	_line = _buffer;
	
	area_h = _th;
	if (IsRealized())
		pArrangeSB();
}

CONSTRUCTOR(TTextArea)
{
	_cx = _cy = _sy = _sx = 0;
	_tw = _th = 1;
	_buffer = 0;
	_line_modified = false;
	_buffer_allocated = 0;
	font.SetFont(TFont::TYPEWRITER, TFont::PLAIN, 12);
	SetItemSize(font.Width("X"), font.Height());
}

TTextArea::~TTextArea()
{
/*
	_store_line();
	if (_buffer)
	{
		char *p = _buffer;
		char *last = _buffer+_buffer_size-1;
		while(p<last)
		{
			p+=strlen(p);
			*p='\n';
			p++;
		}
		*last = 0;
		cout << _buffer << endl;
	}
*/
	if (_buffer)
		free(_buffer);
}

void TTextArea::keyDown(TKey key, char* str, int modifier)
{
	if (modifier & MK_CONTROL)
	{
	} else {
		switch(key)
		{
			case TK_RIGHT:
				_cursor_right();
				break;
			case TK_LEFT:
				_cursor_left();
				break;
			case TK_UP:
				_cursor_up();
				break;
			case TK_DOWN:
				_cursor_down();
				break;
			case TK_HOME:
				_cursor_home();
				break;
			case TK_END:
				_cursor_end();
				break;
			case TK_RETURN:
				_return();
				break;
			case TK_DELETE:
				_delete();
				break;
			case TK_BACKSPACE:
				_backspace();
				break;
			default:
				if (((unsigned char)*str)>=32)
				{
					_line.insert(_cx,str,1);
					_line_modified = true;
					_cursor_right();
				}
		}
//		cout << "line:" << _cy << " total height:" << _th << " area_y:" << area_y << " area_h:" << area_h << " visi_h:" << visi_h << endl;
	}
}

//. Break the current line _cy before cursor position _cx and insert
//. a new line after line _cy
void TTextArea::_return()
{
	string newline;
	newline = _line.substr(_cx);	// text at and after cursor
	_line.remove(_cx);
	_line_modified = true;
	_store_line(_line, newline);	// store current line and new line
	_cursor_home();
	_cursor_down();
	// Invalidate();
}

void TTextArea::_delete()
{
	if (_line.size()>0)
	{
		_line.remove(_cx,1);
		_line_modified = true;
		Invalidate();
	}
	else
	{
		// only delete line when _buffer is available and line isn't the
		// last line
		if (_buffer && _buffer+_buffer_size > _by + strlen(_by) +1 )
		{
			_set_line_size(0);
			if (_buffer && _buffer+_buffer_size > _by)
			{
				_line = _by;
			}
			_th--;
			_calculate_sliders();
			Invalidate();
		}
	}
//	cout << "buffer size:" << _buffer_size << ", cursor:" << _by-_buffer << endl;
}

void TTextArea::_backspace()
{
	if (_cx>0 && _line.size()>0) {
		_cursor_left();
		_line.remove(_cx,1);
		_line_modified = true;
		Invalidate();
	}
}

void TTextArea::_cursor_left()
{
	if (_cx>0) {
		_cx--;
		_sx-=font.GetTextWidth(_line.substr(_cx,1));
		Invalidate();
	} else {
		if (_cy>0) {
			_cursor_up();
			_cursor_end();
		}
	}
}

void TTextArea::_cursor_right()
{
	if (static_cast<unsigned>(_cx)<_line.size()) {
		_sx+=font.GetTextWidth(_line.substr(_cx,1));
		_cx++;
		Invalidate();
	} else {
		int cy = _cy;
		_cursor_down();
		if (cy!=_cy)
			_cursor_home();
	}
}

void TTextArea::_cursor_up()
{
	if (!_buffer)
		return;

	if (_cy > 0) {
		_store_line();
		_by-=2;
		while(_by >= _buffer && *_by!=0)
		{
			_by--;
		}
		_by++;
		_cy--;
		_sy-=font.Height();
		_line = _by;
		
		_cx = _sx = 0;
		Invalidate();
		
		if (_cy<area_y) {
			area_y--;
			pArrangeSB();
		}
		
	}
}

void TTextArea::_cursor_down()
{
	if (!_buffer)
		return;

	_store_line();
	char *by = _by + strlen(_by)+1;
	if (by >= _buffer + _buffer_size)
		return;
	_cy++;
	_sy+=font.Height();
	_by = by;
	_line = _by;
	
	_cx = _sx = 0;

	if (_cy>=area_y+visi_h) {
		area_y++;
		pArrangeSB();
	}

	Invalidate();
}

void TTextArea::_cursor_home()
{
	_cx = 0;
	_sx = 0;
	Invalidate();
}

void TTextArea::_cursor_end()
{
	_cx = _line.size();
	_sx = font.GetTextWidth(_line.c_str());
	Invalidate();
}

//. store the current line in the textbuffer
void TTextArea::_store_line()
{
	// line wasn't modified => dont't waste time
	if (!_line_modified)
		return;
		
	_line_modified = false;
	const char *line = _line.c_str();
	int size = _line.size();
	
	if (!_buffer) {
		// no buffer and no data to store => don't waste time
		if (!size)
			return;
		// create a new buffer and store _cy empty lines
		// before the current (and first modified line) is stored
		_buffer_size = _cy + size+1;		// _cy empty lines + one line
		_alloc_buffer();
		_by = _buffer+_cy;
		memset(_buffer, 0, _cy);
	} else {
		_set_line_size(size+1);
	}
	strncpy(_by, line, size+1);
}

//. Store 2 lines at the position of the current line.<BR>
//. Usefull when inserting a new line.
void TTextArea::_store_line(const string &s1, const string &s2)
{
	_line_modified = false;
	const char *line1 = s1.c_str();
	const char *line2 = s2.c_str();
	int size1 = s1.size();
	int size2 = s2.size();
	
	if (!_buffer) {
		// create a new buffer and store _cy empty lines
		// before the current (and first modified line) is stored
		_buffer_size = _cy + size1 + size2 + 2 ;
		_alloc_buffer();
		_by = _buffer + _cy;
		memset(_buffer, 0, _cy);
		_th = _cy+2;
	} else {
		_set_line_size(size1+size2+2);
		_th++;
	}
	strncpy(_by, line1, size1+1);
	strncpy(_by+size1+1, line2, size2+1);
	_calculate_sliders();
}

//. Set the size of the current buffer line to <VAR>size</VAR>.<BR>
//. <VAR>size</VAR> includes the terminating '\0' so a <VAR>size</VAR>
//. of 0 will delete the current line.
void TTextArea::_set_line_size(int size)
{
	if (!_buffer)
		return;
	int osize = strlen(_by)+1;	// get size of current line (including the '\0')
	if (size < osize)
	{
		memmove(_by+size, _by+osize, _buffer_size - osize - (_by - _buffer));
		if (size)
			*(_by+size-1)='\0';
		_buffer_size += size - osize;
		return;
	}
	if (size > osize)
	{
		size_t obs = _buffer_size;
		_buffer_size += size - osize;
		_alloc_buffer();
		memmove(_by+size, _by+osize, obs - osize - (_by - _buffer));
		memset(_by+osize, ' ', size-osize);
	}
}

//. Ensure there's enough buffer for <VAR>_buffer_size</VAR> bytes.<BR>
//. <I>Note: _buffer and _by might be modified during this process.</I>
void TTextArea::_alloc_buffer()
{
	// reserve memory in 256 byte blocks
	_buffer_allocated = ((_buffer_size>>8)+(_buffer_size&0xFF?1:0)) << 8;
	int by = _by  - _buffer;
	_buffer = static_cast<char*>(realloc(_buffer, _buffer_allocated));
	_by = by + _buffer;
	if (!_buffer_allocated)
		_buffer=NULL;
}

void TTextArea::paint()
{
	TPen pen(this);
	pen.SetFont(&font);
	pen.SetOrigin(area_x, -area_y*item_h);

	if (!_buffer)
	{
		pen.DrawString(0, _sy, _line);
	}
	else
	{
		char *p = _buffer;
		int y = 0;
		int h = pen.Height();
		while(p < _buffer + _buffer_size)
		{
			if (p == _by)
				pen.DrawString(0,y,_line);
			else
				pen.DrawString(0,y,p);
			y+=h;
			p+=strlen(p)+1;
		}
	}
	
	// draw text cursor
	pen.DrawLine(_sx	,_sy, 				_sx,	_sy+pen.Height());
	pen.DrawLine(_sx-2	,_sy, 				_sx+2,	_sy );
	pen.DrawLine(_sx-2	,_sy+pen.Height(), 	_sx+2,	_sy+pen.Height());
}

void TTextArea::_calculate_sliders()
{
	SetAreaSize(_tw, _th);
	pArrangeSB();
}

/*
int ToadMain()
{
	membuf b;
	ostream txt(&b);
	// ostrstream txt(buffer,1023);
	txt << "Dieses ist ein Beispieltext, der mit Hilfe der\n"
		<< "'ostream' Klasse von C++ in den Speicher\n"
		<< "geschrieben wurde und dann anschließend in\n"
		<< "das TTextArea Widget von TOAD eingelesen wurde.\n"
		<< "\n"
		<< "Dabei fällt mir auch auf, daß das Wort 'Widget' sich\n"
		<< "eigentlich ganz gut macht. Okay, hier noch ein paar\n"
		<< "kleine Spielereien:\n"
		<< "\n"
		<< "Die Zahl 13: " << 13;
	TTextArea w(NULL, "TextArea I");
	w.SetValue(b.c_str());
	return w.Run();
}
*/