
#include <toad/toad.hh>
#include <vector>
#include <algorithm>
#include "Simulate.hh"
#include "Shape.hh"
#include "Act.hh"

#include "textarea.hh"
#include "membuf.hh"
#include <iostream.h>
#include <iomanip.h>

struct TCompare
{
	bool operator()(const TTime a,const TTime b) const { return a<b; }
	bool operator()(const TAct *a,const TAct *b) const { return a->end<b->end; }
};

typedef vector<TAct*> TActList;

void Clear(TActList &l)
{
	TActList::iterator p = l.begin();
	while(p!=l.end()) {
		if ( (*p)==0 ) {
			l.erase(p);
			p = l.begin();
			continue;
		}
		p++;
	}
}

static TCompare comp;
static TActList boundlist, eventlist;
static TTime clock;

ostream *log;

void Add2Boundlist(TAct *act)
{
	boundlist.push_back(act);
}

TTime Clock()
{
	return clock;
}

void Simulate(TShapeBuffer &sb, TConnectionBuffer &cb)
{
	membuf b;
	ostream log(&b);
	::log = &log;
	// ostrstream txt()		// buggy in libg++-2.7.2

		// Simulator zurücksetzen
		//------------------------
	boundlist.erase(boundlist.begin(), boundlist.end());
	eventlist.erase(eventlist.begin(), eventlist.end());

	TShapeGenerator::xact_counter = 0;
	srand(1);

	clock = 0;
	TTime end   = 100;
	
	log << setw(5) << clock << ": SIMULATION START----------------------------------------\n";

	// Connections in den Shapes notieren
	//------------------------------------
	TShapeBuffer::iterator sp;
	sp = sb.begin();
	while(sp!=sb.end()) {
		(*sp)->dst.erase(
			(*sp)->dst.begin(),
			(*sp)->dst.end()
		);
		sp++;
	}

	TConnectionBuffer::iterator cp = cb.begin();
	while(cp!=cb.end()) {
		sp = sb.find((*cp)->src);
		assert(sp!=sb.end());
		(*sp)->dst.push_back(*cp);
		cp++;
	}

	// Alle Generatoren suchen und mit Aktivitäten versehen
	//------------------------------------------------------
	sp = sb.begin();
	TShapeGenerator *gp;
	while(sp!=sb.end())	{
		gp = dynamic_cast<TShapeGenerator*>(*sp);
		if (gp) {
			TActWait *ga = new TActWait(gp,0);
			boundlist.push_back(ga);
		}
		sp++;
	}

	// Simulationsschleife
	//------------------------------------------------------
	while(clock<=end) {
		TActList::iterator p, np;
		
		// Aktivitäten starten
		p = boundlist.begin();
		while(p!=boundlist.end()) {
			if ((*p)->Ready()) {
				(*p)->Start();
				(*p)->_bound = false;
				eventlist.push_back(*p);
				(*p) = NULL;
			}
			p++;
		}
		Clear(boundlist);

		sort(eventlist.begin(), eventlist.end(), comp);

		// Aktivitäten beenden
		p = eventlist.begin();
		if (p==eventlist.end())
		{
			log << setw(5) << clock << ": NO ACTIVITIES => STOPED\n";
			break;
		}
		
		clock = (*p)->end;
		while(p!=eventlist.end() && (*p)->end == clock) {
			np = p;
			np++;
			(*p)->End();
			if ((*p)->_bound) {
				boundlist.push_back(*p);
			} else {
				delete (*p);
			}
			(*p) = NULL;
			p = np;
		}
		Clear(eventlist);
	}
	log << setw(5) << clock << ": SIMULATION END ------------------------------------------";
	TTextArea *w = new TTextArea(NULL, "Simulation Log");
	w->SetValue(b.c_str());
	w->Create();
}
