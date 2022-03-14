#include "base/gl_context_win.h"

#include "GL/glew.h"
#include "GL/glut.h"

namespace media {
GLContextWin::GLContextWin(): 
  hwnd_(0), hdc_(0), hrc_(0), format_(0) {
}

void GLContextWin::SetupPixelFormat(HDC hdc) {
  int pixelFormat;
  PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),  // size
      1,                              // version
      PFD_SUPPORT_OPENGL |            // OpenGL window
          PFD_DRAW_TO_WINDOW |        // render to window
          //          PFD_DOUBLEBUFFER,           // support double-buffering
          PFD_TYPE_RGBA,  // color type
      32,                 // prefered color depth
      0,
      0,
      0,
      0,
      0,
      0,  // color bits (ignored)
      0,  // no alpha buffer
      0,  // alpha bits (ignored)
      0,  // no accumulation buffer
      0,
      0,
      0,
      0,               // accum bits (ignored)
      16,              // depth buffer
      0,               // no stencil buffer
      0,               // no auxiliary buffers
      PFD_MAIN_PLANE,  // main layer
      0,               // reserved
      0,
      0,
      0,  // no layer, visible, damage masks
  };

  pixelFormat = ChoosePixelFormat(hdc_, &pfd);
  SetPixelFormat(hdc_, pixelFormat, &pfd);
}

void GLContextWin::Initialize(HWND hwnd, HDC hdc) {
  hwnd_ = hwnd;
  hdc_ = hdc;
  SetupPixelFormat(hdc_);
  hrc_ = wglCreateContext(hdc_);
  wglMakeCurrent(hdc_, hrc_);
  glewExperimental = GL_TRUE;
  glewInit();
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_TEXTURE_2D);
}

void GLContextWin::SetCurrentGLContext() {
  wglMakeCurrent(hdc_, hrc_);  //ִ��OpenGLָ���б��е�ָ��
}

} // namespace media