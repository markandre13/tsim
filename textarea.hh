#include <toad/toad.hh>
#include <toad/scrolledarea.hh>
#include <toad/macros.hh>

BGN_CLASS(TTextArea, TScrolledArea)
	public:
		~TTextArea();
		void SetValue(const string&);
	protected:
		void keyDown(TKey,char*,int);
		void paint();
	private:
		int _cx, _cy;				// cursor position in text
		int _sy, _sx;				// cursor position on screen
		int _tw, _th;				// text height & width
		string _line;				// current line at _cy
		bool _line_modified;		// 'true' when line was modified
		char *_buffer;				// that's where all the text is stored (line ends with 0)
		size_t _buffer_size;		// (_buffer!=NULL)
		size_t _buffer_allocated;	// (_buffer!=NULL)
		char *_by;					// start of line _cy (_buffer!=NULL)
		
		TFont font;
		
		// line editing methods
		void _return();
		void _delete();
		void _backspace();
		void _cursor_right();
		void _cursor_left();
		void _cursor_up();
		void _cursor_down();
		void _cursor_home();
		void _cursor_end();
		
		void _store_line();
		void _store_line(const string&, const string&);
		void _set_line_size(int);
		void _alloc_buffer();
		void _calculate_sliders();
END_CLASS
