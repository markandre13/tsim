// "NetView6000 / wtdriver6 Script Editor
// for the VW SMART Project
// Copyright (C) IBM Global Services
// written 1997 by Mark-André Hopf
//-------------------------------------------

#include <typeinfo>
#include <cstdlib>
#include <iostream.h>
#include "membuf.hh"

membuf::membuf()
{
	data = NULL;
	dsize = size = 0;
}

membuf::~membuf()
{
	if (data)
		free(data);
}

streamsize membuf::xsputn(const char* s, streamsize n)
{
	unsigned long ns = size+n+1;
	unsigned long nd = (ns%4096) ? (ns/4096+1) * 4096 : (ns/4096)*4096;

	if (nd>dsize)
	{
		data = static_cast<char*>(realloc(data, nd));
		dsize = nd;
	}
	
	memcpy(data+size, s,n);
	
	size = ns - 1;
	return n;
}

const char* membuf::c_str()
{
	data[size]=0;
	return data;
}
