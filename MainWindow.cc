#include <toad/toad.hh>
#include <toad/menubar.hh>
#include <toad/macros.hh>

#include "MainWindow.hh"
#include "ToolBar.hh"
#include "Paper.hh"
#include "Simulate.hh"

TBitmap *bmpProgIcon;
TBitmap *bmpDialog;

int ToadMain()
{
	bmpProgIcon = new TBitmap();
	bmpProgIcon->Load(TOADBase::ExecutablePath()+"tsim.bmp");
	bmpDialog = new TBitmap();
	bmpDialog->Load(TOADBase::ExecutablePath()+"tsim_dlg.bmp");
	return TMainWindow(NULL, "Tiny Simulator v0.1").Run();
}

CONSTRUCTOR(TMainWindow)
{
	SetIcon(bmpProgIcon);
	SetSize(640,480);

	// order is important:	
	TPaper *p = new TPaper(this, "paper");
	TToolBar *tb = new TToolBar(this, "toolbar");

	TMenuBar *mb = new TMenuBar(this, "menubar");
	TMenuItem *mi;
	mb->BgnPulldown("File");
		mi = mb->AddItem("New");
		CONNECT(p,p->New, mi,mi->sigActivate);
		mi = mb->AddItem("Open..");
		CONNECT(p,p->Open, mi,mi->sigActivate);
		mi = mb->AddItem("Save");
		CONNECT(p,p->Save, mi,mi->sigActivate);
		mi = mb->AddItem("Save As..");
		CONNECT(p,p->SaveAs, mi,mi->sigActivate);
		mi = mb->AddItem("Quit");
		CONNECT(this,closeRequest, mi,mi->sigActivate);
	mb->EndPulldown();
	mb->BgnPulldown("Edit");
		mi = mb->AddItem("Delete");
		CONNECT(this,menuDelete, mi,mi->sigActivate);
	mb->EndPulldown();
	mb->BgnPulldown("Simulate");
		mi = mb->AddItem("Run");
		CONNECT(this,menuRun, mi,mi->sigActivate);
	mb->EndPulldown();
	mb->BgnPulldown("Help");
		mi = mb->AddItem("About");
		CONNECT(this, menuAbout, mi,mi->sigActivate);
	mb->EndPulldown();

	
	Attach(mb, LEFT | RIGHT | TOP);
	Attach(tb, TOP, mb);
	Attach(tb, LEFT | RIGHT);
	Attach(p, TOP, tb);
	Attach(p, LEFT | RIGHT | BOTTOM);
}

void TMainWindow::menuRun()
{
	Simulate(TPaper::Current()->_shape, TPaper::Current()->_connection);
}

void TMainWindow::menuAbout()
{
	MessageBox(this, 
		"About", 
		"Tiny Simulator v0.1\n"
		"Copyright © 1998 Mark-André Hopf\n"
		"eMail: hopf@informatik.uni-rostock.de\n"
		"www   : www.informatik.uni-rostock.de/~hopf/toad/\n"
		"\n"
		"This program is free software; you can redistribute it and/or modify "
		"it under the terms of the GNU General Public License as published by "
		"the Free Software Foundation; either version 2 of the License, or "
		"(at your option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		"GNU General Public License for more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License "
		"along with this program; if not, write to the Free Software "
		"Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n"
		"\n"
		"Developed with the GNU/Linux operating system 2.0.33, the GNU C++ Compiler 2.7.2.2, "
		"XFree 3.2, the TOAD C++ GUI toolkit 0.0.33, POV-Ray 3.0 and the JOE 2.8 texteditor."
		,MB_OK, bmpDialog);
}

void TMainWindow::menuDelete()
{
	TPaper::Current()->DeleteSelection();
}
