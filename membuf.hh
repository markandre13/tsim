// "NetView6000 / wtdriver6 Script Editor
// for the VW SMART Project
// Copyright (C) IBM Global Services
// written 1997 by Mark-André Hopf
//-------------------------------------------

// a dirty hack by MAH

#ifndef _MEMBUF_HH
#define _MEMBUF_HH

class membuf: public streambuf {
	public:
		membuf();
		~membuf();

		streamsize xsputn(const char* s, streamsize n);
		const char* c_str();

	protected:
		char *data;
		unsigned long dsize, size;
};

#endif
