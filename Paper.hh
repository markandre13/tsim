#ifndef TPaper
#define TPaper TPaper

#include <toad/scrolledarea.hh>
#include "ShapeBuffer.hh"
#include "ConnectionBuffer.hh"

class TDropShape;

enum
{
	TOOL_SELECT,
	TOOL_CONNECT
};


BGN_CLASS(TPaper, TScrolledArea)
		static TPaper* _current;
		int _tool;
		int _mx, _my;
		string filename;
	public:
		static TFont *font1;
		static TFont *font2;
		static TPaper* Current(){return _current;}
		void SetTool(int t);
		TShape* GetShape(int x, int y);
		void Connect(TShape* src, TShape *dst);
 		TShapeBuffer _shape;
 		TConnectionBuffer _connection;
 		
 		void DeleteSelection();
 		void New();
 		void Open();
 		void Save();
 		void SaveAs();
	private:
		void paint();
		void paintBackground();
		void mouseLDown(int,int,unsigned);
		void mouseRDown(int,int,unsigned);
		void mouseLUp(int,int,unsigned);
		void mouseMove(int,int,unsigned);

		void dropShape(TDropShape*);

		TShape* _selection;	// points to shape in _shape
		TShape* _clipboard; // stand-alone shape
		
		int mx, my;
END_CLASS

#endif
