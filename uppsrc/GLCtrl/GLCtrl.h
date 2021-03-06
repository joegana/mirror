#ifndef _GLCtrl_GLCtrl_h
#define _GLCtrl_GLCtrl_h

#include <CtrlCore/CtrlCore.h>

#ifdef GUI_X11
#define Time    XTime
#define Font    XFont
#define Display XDisplay
#define Picture XPicture
#define Status  int
#endif

#include <plugin/glew/glew.h>

#ifdef PLATFORM_WIN32
#include <plugin/glew/wglew.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef GUI_X11

#include <GL/glx.h>

#undef  Status
#undef  Picture
#undef  Time
#undef  Font
#undef  Display
#endif

namespace Upp {

void InitializeGlew();

#ifdef GUI_GTK

class GLCtrl : public Ctrl {
public:
	virtual void Paint(Draw& w);

public:
	Event<> WhenGLPaint;

	virtual void GLPaint();

	void StdView();
};

#else

class GLCtrl : public Ctrl {
	typedef GLCtrl CLASSNAME;

public:
	virtual Image  MouseEvent(int event, Point p, int zdelta, dword keyflags);
	
private:
	class GLPicking
	{
	private:
		static int const _bufferSize = 512;
		bool _isPicking;
		Point _pickPoint;
		
		Vector<int> ParseHits(GLuint *buffer, int hits);
		
	public:
		void InitPickMatrix();
		Vector<int> Pick(int x, int y, Callback resizeCallback, Callback paintCallback);
		
		GLPicking() : _isPicking(false) {}
	};
	
#ifdef GUI_X11
	class GLPane : public DHCtrl {
		friend class GLCtrl;
		
		GLCtrl *ctrl;
		GLXContext WindowContext;
		
		// Ovverridden method to choose the correct visual
		virtual XVisualInfo *CreateVisual(void);
		
		// Overridden method for attribute setting
		virtual void SetAttributes(unsigned long &ValueMask, XSetWindowAttributes &attr);
		
		// Overridden method to create and destroy OpenGL context
		virtual void AfterInit(bool Error);
		virtual void BeforeTerminate(void);
		
		// Overridden method to resize GL windows
		virtual void Layout();
		
		// Paint method - with graphic context
		// Called from DHCtrl - Graphic context is *not* used
		virtual void Paint(Draw &/*draw*/);
		
	public:
		GLPane() : WindowContext(NULL) { NoWantFocus(); }
		~GLPane();
		
		virtual Image MouseEvent(int event, Point p, int zdelta, dword keyflags);
		
		void ActivateContext();
	};
#else
	struct GLPane : DHCtrl {
		friend class GLCtrl;
		
	//	HDC    hDC;
	//	HGLRC  hRC;
		GLCtrl *ctrl;
		
	public:
		GLPane()/* : hDC(NULL), hRC(NULL)*/ { NoWantFocus(); }
		
		virtual void    State(int reason);
		virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
		virtual Image   MouseEvent(int event, Point p, int zdelta, dword keyflags);
		
		void Init();
		void Destroy();
		
		void ActivateContext();
	};
#endif
	
	GLPicking picking;
	GLPane pane;

	static int  depthSize;
	static int  stencilSize;
	static bool doubleBuffering;
	static int  numberOfSamples;
	
	static Size current_viewport; // because we need to set different viewports in drawing code

	Ptr<Ctrl> mouseTarget = NULL;

protected:
	// Called on paint events
	virtual void GLPaint() { WhenGLPaint(); }
	virtual void GLPickingPaint() {}
	
	void Init();

public:
	Callback WhenGLPaint;

	static void SetDepthBits(int n)               { depthSize = n; }
	static void SetStencilBits(int n)             { stencilSize = n; }
	static void SetDoubleBuffering(bool b = true) { doubleBuffering = b; }
	static void SetMSAA(int n = 4)                { numberOfSamples = n; }
	
	static void CreateContext();
	
	static Size CurrentViewport()                 { return current_viewport; }
	static void SetCurrentViewport(); // intended to restore viewport after changing it in e.g. TextureDraw
	
	GLCtrl& RedirectMouse(Ctrl *target)    { mouseTarget = target; return *this; }

	GLCtrl()                                      { Init(); }

	void InitPickMatrix() { picking.InitPickMatrix(); }
	void Refresh()        { pane.Refresh(); }
	
	Vector<int> Pick(int x, int y);

	// deprecated (these settings are now static, as we have just single context)
	GLCtrl& DepthBits(int n)               { depthSize = n; return *this; }
	GLCtrl& StencilBits(int n)             { stencilSize = n; return *this; }
	GLCtrl& DoubleBuffering(bool b = true) { doubleBuffering = b; return *this; }
	GLCtrl& MSAA(int n = 4)                { numberOfSamples = n; return *this; }

	GLCtrl(int  depthsize, int  stencilsize = 8, bool doublebuffer = true,
	       bool multisamplebuffering = false, int  numberofsamples = 0)
	{ Init(); DepthBits(depthsize).StencilBits(stencilsize).DoubleBuffering(doublebuffer).MSAA(numberofsamples); }

	// deprecated (fixed pipeline is so out of fashion...)
	void StdView();
};

#endif

}

#endif
