#ifndef _GLDrawDemo_Ugl_h_
#define _GLDrawDemo_Ugl_h_

namespace Upp {

struct GL_TIMING_FINISH__ { ~GL_TIMING_FINISH__() { glFinish(); } };

#define GL_TIMING(x)    // RTIMING(x); GL_TIMING_FINISH__ COMBINE(sGlTiming, __LINE__)

#ifdef _DEBUG
#define GLCHK(x) do { \
	x; \
	int err = glGetError(); \
	if(err) LOG("ERROR " << err << " (" << __LINE__ << "): " << #x); \
	LOG((const char *)gluErrorString(err)); \
} while(0)
#endif

struct GLContext2D { // TODO: This should be changed to regular matrix (later)
	Sizef  vs;
	double alpha = 1;
	
	void Set(Size sz)    { vs = Sizef(2.0 / sz.cx, -2.0 / sz.cy); }
	
	GLContext2D(Size sz) { Set(sz); }
	GLContext2D()        {}
};

struct GLCode : GLProgram {
	GLCode(const char *vertex_shader, const char *pixel_shader);

	int operator[](const char *id)                                             { return GetUniform(id); }
	
	GLCode& Uniform(int i, double a);
	GLCode& Uniform(int i, double a, double b);
	GLCode& Uniform(int i, double a, double b, double c);
	GLCode& Uniform(int i, double a, double b, double c, double d);

	GLCode& Uniform(const char *id, double a);
	GLCode& Uniform(const char *id, double a, double b);
	GLCode& Uniform(const char *id, double a, double b, double c);
	GLCode& Uniform(const char *id, double a, double b, double c, double d);

	GLCode& operator()(const char *id, double a)                               { return Uniform(id, a); }
	GLCode& operator()(const char *id, double a, double b)                     { return Uniform(id, a, b); }
	GLCode& operator()(const char *id, double a, double b, double c)           { return Uniform(id, a, b, c); }
	GLCode& operator()(const char *id, double a, double b, double c, double d) { return Uniform(id, a, b, c, d); }

	GLCode& operator()(int i, double a)                                        { return Uniform(i, a); }
	GLCode& operator()(int i, double a, double b)                              { return Uniform(i, a, b); }
	GLCode& operator()(int i, double a, double b, double c)                    { return Uniform(i, a, b, c); }
	GLCode& operator()(int i, double a, double b, double c, double d)          { return Uniform(i, a, b, c, d); }

	GLCode& operator()(const char *id, Pointf p)                               { return Uniform(id, p.x, p.y); }
	GLCode& operator()(int i, Pointf p)                                        { return Uniform(i, p.x, p.y); }
	GLCode& operator()(const char *id, Sizef sz)                               { return Uniform(id, sz.cx, sz.cy); }
	GLCode& operator()(int i, Sizef sz)                                        { return Uniform(i, sz.cx, sz.cy); }
	GLCode& operator()(const char *id, Color c, double alpha = 1);
	GLCode& operator()(int i, Color c, double alpha = 1);
};

class GLTexture {
	struct Data {
		int     refcount = 1;
		GLuint  textureid = 0;
		Point   hotspot = Point(0, 0);
		Size    sz = Size(0, 0);
	};
	
	Data    *data = NULL;
	
	void Set(GLuint texture, Size sz, Point hotspot = Point(0, 0));
	
	friend class GLTextureDraw;
	
public:
	void     Clear();
	void     Set(const Image& img, dword flags = TEXTURE_LINEAR|TEXTURE_MIPMAP);
	
	void     Bind() const       { if(data) glBindTexture(GL_TEXTURE_2D, data->textureid); }
	int      GetID() const      { return data ? data->textureid : 0; }
	operator GLuint() const     { return GetID(); }
	Size     GetSize() const    { return data ? data->sz : Size(0, 0); }
	Point    GetHotSpot() const { return data ? data->hotspot : Point(0, 0); }

	GLTexture()                 {}
	GLTexture(const Image& img, dword flags = TEXTURE_LINEAR|TEXTURE_MIPMAP) { Set(img, flags); }
	~GLTexture()                { Clear(); }

	GLTexture(const GLTexture& src);
	GLTexture& operator=(const GLTexture& src);
};

void GLBind(const Image& img, dword style = TEXTURE_LINEAR|TEXTURE_MIPMAP);

void GLDrawTexture(const GLContext2D& dd, const Rectf& rect, int textureid);
void GLDrawTexture(const GLContext2D& dd, const Rectf& rect, const Image& img);
void GLDrawImage(const GLContext2D& dd, const Rectf& rect, const Image& img);

void GLDrawTexture(const GLContext2D& dd, const Rectf& rect, int textureid, Size tsz, const Rect& src);
void GLDrawTexture(const GLContext2D& dd, const Rectf& rect, const GLTexture& img, const Rect& src);
void GLDrawImage(const GLContext2D& dd, const Rectf& rect, const Image& img, const Rect& src);

class GLTextureDraw {
	GLuint framebuffer = 0;
    GLuint texture = 0;
	GLuint rbo = 0;
	Size   sz;
	int    msaa = 0;

public:
	void      Clear();
	void      Create(Size sz, int msaa = 0);
	GLTexture GetResult();
	operator  GLTexture()                { return GetResult(); }

	GLTextureDraw()                      {}
	GLTextureDraw(Size sz, int msaa = 0) { Create(sz, msaa); }
	~GLTextureDraw()                     { Clear(); }
};

class GLVertexData {
	struct Data {
		int            refcount = 1;
		GLuint         VAO = 0;
		GLuint         EBO = 0;
		int            elements = 0;
	    
		Vector<GLuint> VBO;
	};
	
	Data    *data = NULL;
	
	void     Do();

public:
	void    Clear();

	GLVertexData& Add(const void *data, int type, int ntuple, int count);
	GLVertexData& Add(const float *data, int ntuple, int count) { return Add(data, GL_FLOAT, ntuple, count); }
	GLVertexData& Add(const Vector<float>& data, int ntuple)    { return Add(data, ntuple, data.GetCount() / ntuple); }
	GLVertexData& Add(const Vector<Pointf>& pt);
	GLVertexData& Index(const int *indices, int count);
	GLVertexData& Index(const Vector<int>& indices)             { return Index(indices, indices.GetCount()); }
	
	void    Draw(int mode = GL_TRIANGLES) const;

	void    Draw(GLCode& shaders, int mode = GL_TRIANGLES) const;
	
	operator bool() const                                       { return data; }
	bool    IsEmpty() const                                     { return !data; }

	GLVertexData()                                              {}
	~GLVertexData()                                             { Clear(); }

	GLVertexData(const GLVertexData& src);
	GLVertexData& operator=(const GLVertexData& src);
};

const GLVertexData& GLRectMesh();

template <typename Src>
void GLPolygons(GLVertexData& mesh, const Src& polygon);

void GLDrawPolygons(const GLContext2D& dd, Pointf at, const GLVertexData& mesh, Sizef scale, Color color);
void GLDrawConvexPolygons(const GLContext2D& dd, Pointf at, const GLVertexData& mesh, Sizef scale, Color color);

template <typename Src>
void GLPolylines(GLVertexData& data, const Src& polygon);

void DashPolyline(Vector<Vector<Pointf>>& polyline, const Vector<Pointf>& line,
                  const Vector<double>& pattern, double distance = 0);

void GLDrawPolylines(const GLContext2D& dd, Pointf at, const GLVertexData& mesh, Sizef scale, double width, Color color);

void GLDrawStencil(Color color, double alpha);

void GLDrawEllipse(const GLContext2D& dd, Pointf center, Sizef radius, Color fill_color,
                   double width, Color line_color, const Vector<double>& dash, double distance);
void GLDrawEllipse(const GLContext2D& dd, Pointf center, Sizef radius, Color fill_color,
                   double width, Color line_color);

GLTexture GetGlyphGLTextureCached(double angle, int chr, Font font, Color color);

void GLDrawText(const GLContext2D& dd, Pointf pos, double angle, const wchar *text, Font font,
                Color ink, int n = -1, const int *dx = NULL);

void GLArc(Vector<Vector<Pointf>>& line, const Rectf& rc, Pointf start, Pointf end);

#include "GLPainter.hpp"

class DrawGL : public Draw {
public:
	virtual dword GetInfo() const;

	virtual void BeginOp();
	virtual bool ClipOp(const Rect& r);
	virtual bool ClipoffOp(const Rect& r);
	virtual bool IntersectClipOp(const Rect& r);
	virtual void OffsetOp(Point p);
	virtual bool ExcludeClipOp(const Rect& r);
	virtual void EndOp();
	virtual bool IsPaintingOp(const Rect& r) const;

	virtual void SysDrawImageOp(int x, int y, const Image& img, Color color);
	virtual void SysDrawImageOp(int x, int y, const Image& img, const Rect& src, Color color);
	virtual void DrawRectOp(int x, int y, int cx, int cy, Color color);

	virtual void DrawTextOp(int x, int y, int angle, const wchar *text, Font font, Color ink, int n, const int *dx);

	virtual void DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color);
	virtual void DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor);
	virtual void DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color);
	virtual void DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count, const int *subpolygon_counts, int scc, const int *disjunct_polygon_counts, int dpcc, Color color, int width, Color outline, uint64 pattern, Color doxor);
	virtual void DrawPolyPolylineOp(const Point *vertices, int vertex_count, const int *counts, int count_count, int width, Color color, Color doxor);

private:
	struct Cloff : Moveable<Cloff> {
		Rect   clip;
		Pointf offset;
	};
	
	Vector<Cloff> cloff;
	GLContext2D   dd;
	Size          view_size;
	
	Pointf Offset(int x, int y);
	Pointf Offset(Point p) { return Offset(p.x, p.y); }
	Rectf  Offset(int x, int y, int cx, int cy);
	Rectf  Offset(int x, int y, Size sz);
	void   SyncScissor();
	void   DoPath(Vector<Vector<Pointf>>& poly, const Point *pp, const Point *end);
	static const Vector<double>& GetDash(int& width);
	void   ApplyDash(Vector<Vector<Pointf>>& polyline, int& width);

public:
	void Init(Size sz, double alpha = 1);

	DrawGL() {}
	DrawGL(Size sz, double alpha = 1) { Init(sz, alpha); }
	~DrawGL();
};

};

#endif
