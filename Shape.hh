#ifndef TShape
#define TShape TShape

class TFile;
#include "ConnectionBuffer.hh"

class TShape: public TRect
{
	public:
		TShape();
		virtual ~TShape();
		const double RANGE=5.0;
		const double OUT_OF_RANGE = 100.0;
		virtual double Distance(int tx, int ty) const;
		virtual void GetCenter(TPoint*);
		virtual bool AcceptSend();
		virtual bool AcceptRecv();
		void Send(Txact);
		virtual void recv(Txact);
		virtual TShape* Clone() = 0;

		virtual void paint(TPen&) = 0;
		virtual void paintSelection(TPen&);
		
		virtual unsigned TypeID() { return 1; }
		virtual void Store(TFile&);
		virtual void Restore(TFile&);

		string title;
		virtual void edit();
		
		TConnectionBuffer dst;	// only used during Simulate(..)
		ulong number;			// only user during Paper::Save()
};

class TShapeConnection: public TShape
{
		typedef TShape super;
	public:
		TShapeConnection(TShape *s, TShape *d) {
			src = s;
			dst = d;
			p = 1.0;
		}
		TShape *src, *dst;
		double p;
		void paint(TPen&);
		TShape* Clone();
		unsigned TypeID() { return 4; }
		void Store(TFile&);
		void Restore(TFile&);
		double Distance(int tx, int ty) const;
		void edit();
};

class TShapeTerminator:	public TShape
{
		typedef TShape super;
	public:
		TShape* Clone();
		void paint(TPen&);
		void recv(Txact);
		bool AcceptSend() { return false; }
		unsigned TypeID() { return 3; }
};

class TActWait;
class TShapeWait: public TShape
{
		typedef TShape super;
	public:
		TShapeWait() {
			interval = 10;
			range    = 5;
		}
		virtual void wakeUp(TActWait*)=0;
		TShape* Clone()=0;
		unsigned interval, range;
		void edit();
		unsigned TypeID() { return 4; }
		virtual void Store(TFile&);
		virtual void Restore(TFile&);
};

class TShapeGenerator: public TShapeWait
{
		typedef TShape super;
	public:
		static unsigned xact_counter;
		bool AcceptRecv() { return false; }

		TShape* Clone();
		void paint(TPen&);
		void wakeUp(TActWait*);
		unsigned TypeID() { return 5; }
};

class TShapeAdvance: public TShapeWait
{
		typedef TShape super;
	public:
		TShape* Clone();
		void paint(TPen&);
		void recv(Txact);
		void wakeUp(TActWait*);
		unsigned TypeID() { return 6; }
};

class TShapeQueue: public TShapeWait
{
		typedef TShape super;
		bool _lock;
	public:
		TShapeQueue() { _lock = false; }
		TShape* Clone();
		void paint(TPen&);
		void recv(Txact);
		void wakeUp(TActWait*);
		unsigned TypeID() { return 7; }
		bool Lock() {return _lock;}
		void SetLock(bool lock) {_lock = lock;}
};

#endif
