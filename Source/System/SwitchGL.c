// Switch GLES1 compatibility shim.
//
// libGLESv1_CM (OpenGL ES 1.1 Common, fixed-function) gives us the matrix stack,
// lighting, fog, materials, alpha test, blend and VERTEX ARRAYS -- but NOT
// immediate mode (glBegin/glEnd), nor a few desktop-only entry points
// (double-precision glOrtho, glFogi, glPolygonMode, glColorMaterial).
//
// Cro-Mag's overlay/effect draws use immediate mode, and all its GL code is
// written against desktop <SDL_opengl.h> prototypes. Instead of rewriting the
// 17 immediate-mode call sites, this file supplies the missing symbols on top of
// GLES1 so the existing code links and actually renders:
//
//   * a small immediate-mode -> vertex-array emulator: glBegin/glEnd bracket a
//     batch of glVertex/glColor/glTexCoord, flushed via glDrawArrays (or, for
//     GL_QUADS which GLES1 lacks, glDrawElements expanding each quad to 2 tris).
//   * thin redirects:  glOrtho -> glOrthof,  glFogi -> glFogf.
//   * no-ops where GLES1 already covers it: glColorMaterial (the game also calls
//     glEnable(GL_COLOR_MATERIAL), which is valid GLES1) and glPolygonMode
//     (no wireframe fill mode on GLES1).
//
// This whole file is inert on every non-Switch platform.

#ifdef __SWITCH__

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <stdio.h>

// Primitive tokens the GLES1 headers don't define. The values come from the
// desktop <SDL_opengl.h> the game itself was compiled against, so glBegin()
// receives exactly these at runtime.
#ifndef GL_QUADS
#define GL_QUADS   0x0007
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif

#define IM_MAX_VERTS 8192

static GLenum  s_mode;
static int     s_count;
static GLfloat s_pos[IM_MAX_VERTS * 3];
static GLfloat s_tex[IM_MAX_VERTS * 2];
static GLfloat s_col[IM_MAX_VERTS * 4];
static GLfloat s_curTex[2] = { 0.0f, 0.0f };
static GLfloat s_curCol[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

void glBegin(GLenum mode)
{
	s_mode  = mode;
	s_count = 0;
}

static void IM_Push(GLfloat x, GLfloat y, GLfloat z)
{
	if (s_count >= IM_MAX_VERTS)
		return;
	s_pos[s_count*3 + 0] = x;
	s_pos[s_count*3 + 1] = y;
	s_pos[s_count*3 + 2] = z;
	s_tex[s_count*2 + 0] = s_curTex[0];
	s_tex[s_count*2 + 1] = s_curTex[1];
	s_col[s_count*4 + 0] = s_curCol[0];
	s_col[s_count*4 + 1] = s_curCol[1];
	s_col[s_count*4 + 2] = s_curCol[2];
	s_col[s_count*4 + 3] = s_curCol[3];
	s_count++;
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z) { IM_Push(x, y, z); }
void glVertex2f(GLfloat x, GLfloat y)            { IM_Push(x, y, 0.0f); }

void glTexCoord2f(GLfloat u, GLfloat v)
{
	s_curTex[0] = u;
	s_curTex[1] = v;
}

// glColor4f IS GLES1-core, so we call (never define) it, both to update our
// immediate-mode current colour and to set the real pipeline colour for the
// game's non-immediate (vertex-array) draws.
void glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
	s_curCol[0] = r; s_curCol[1] = g; s_curCol[2] = b; s_curCol[3] = 1.0f;
	glColor4f(r, g, b, 1.0f);
}

void glColor4fv(const GLfloat* v)
{
	s_curCol[0] = v[0]; s_curCol[1] = v[1]; s_curCol[2] = v[2]; s_curCol[3] = v[3];
	glColor4f(v[0], v[1], v[2], v[3]);
}

void glColor3fv(const GLfloat* v)
{
	s_curCol[0] = v[0]; s_curCol[1] = v[1]; s_curCol[2] = v[2]; s_curCol[3] = 1.0f;
	glColor4f(v[0], v[1], v[2], 1.0f);
}

// glColor4f IS a real GLES1 entry point, so the game's direct calls to it (e.g.
// the per-vertex pillarbox shadow gradient) would bypass this immediate-mode
// emulator -- and since glEnd() always feeds a colour array, those calls would be
// lost and the batch would draw with a stale colour. Wrap it (via
// -Wl,--wrap=glColor4f) to mirror the colour into s_curCol AND pass it through.
extern void __real_glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void __wrap_glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	s_curCol[0] = r; s_curCol[1] = g; s_curCol[2] = b; s_curCol[3] = a;
	__real_glColor4f(r, g, b, a);
}

void glEnd(void)
{
	if (s_count <= 0) { s_count = 0; return; }

	GLboolean wasV = glIsEnabled(GL_VERTEX_ARRAY);
	GLboolean wasT = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
	GLboolean wasC = glIsEnabled(GL_COLOR_ARRAY);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, s_pos);
	glTexCoordPointer(2, GL_FLOAT, 0, s_tex);
	glColorPointer(4, GL_FLOAT, 0, s_col);

	if (s_mode == GL_QUADS)
	{
		// GLES1 has no GL_QUADS: expand each quad (v0,v1,v2,v3) to two triangles
		// (v0,v1,v2)+(v0,v2,v3) via an index buffer; winding is preserved.
		static GLushort idx[(IM_MAX_VERTS / 4) * 6];
		int quads = s_count / 4;
		for (int q = 0; q < quads; q++)
		{
			GLushort b = (GLushort)(q * 4);
			idx[q*6 + 0] = b;       idx[q*6 + 1] = b + 1; idx[q*6 + 2] = b + 2;
			idx[q*6 + 3] = b;       idx[q*6 + 4] = b + 2; idx[q*6 + 5] = b + 3;
		}
		glDrawElements(GL_TRIANGLES, quads * 6, GL_UNSIGNED_SHORT, idx);
	}
	else if (s_mode == GL_POLYGON)
	{
		glDrawArrays(GL_TRIANGLE_FAN, 0, s_count);	// convex polygon -> fan
	}
	else
	{
		// POINTS / LINES / LINE_LOOP / LINE_STRIP / TRIANGLES / TRIANGLE_STRIP /
		// TRIANGLE_FAN all exist unchanged in GLES1.
		glDrawArrays(s_mode, 0, s_count);
	}

	// Restore the client-array enables we found, so we don't disturb the game's
	// own vertex-array bookkeeping. (Pointers are re-set by the game each draw.)
	if (!wasC) glDisableClientState(GL_COLOR_ARRAY);
	if (!wasT) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (!wasV) glDisableClientState(GL_VERTEX_ARRAY);

	s_count = 0;
}

// --- desktop-only entry points redirected onto GLES1 -------------------------

void glOrtho(double l, double r, double b, double t, double n, double f)
{
	// glOrthof is a no-op on this Tegra/Mesa GLES stack -- 2D-ortho projections
	// ended up as identity, mapping all kProjectionType2DOrtho* content (menu text,
	// HUD, etc.) far off-screen. Build the ortho matrix by hand and multiply it in
	// via glMultMatrixf (which works: the 3D path relies on it).
	GLfloat m[16];
	for (int i = 0; i < 16; i++) m[i] = 0.0f;
	m[0]  = (GLfloat)( 2.0 / (r - l));
	m[5]  = (GLfloat)( 2.0 / (t - b));
	m[10] = (GLfloat)(-2.0 / (f - n));
	m[12] = (GLfloat)(-(r + l) / (r - l));
	m[13] = (GLfloat)(-(t + b) / (t - b));
	m[14] = (GLfloat)(-(f + n) / (f - n));
	m[15] = 1.0f;
	glMultMatrixf(m);
}

void glFogi(GLenum pname, GLint param)
{
	glFogf(pname, (GLfloat)param);
}

void glPolygonMode(GLenum face, GLenum mode)
{
	(void)face; (void)mode;		// no polygon fill modes / wireframe on GLES1
}

void glColorMaterial(GLenum face, GLenum mode)
{
	(void)face; (void)mode;		// GLES1 tracks ambient+diffuse when GL_COLOR_MATERIAL
					// is enabled; the game already calls glEnable(GL_COLOR_MATERIAL).
}

#endif // __SWITCH__
