#ifndef TAct
#define TAct TAct

class TAct
{
	public:
		virtual bool Ready();
		virtual void Start() = 0;
		virtual void End() {}
		void Bind() { _bound = true; }
		TTime end;
		bool _bound;
};

class TShapeWait;

class TActWait: public TAct
{
		TShapeWait *_component;
	public:
		Txact xact;
		TActWait(TShapeWait *, Txact);
		void Start();
		void End();
};

class TActQueueWait: public TActWait
{
		typedef TActWait super;
		TShapeQueue *_queue;
	public:
		Txact xact;
		TActQueueWait(TShapeQueue*, Txact);
		bool Ready();
		void Start();
		void End();
};

#endif
