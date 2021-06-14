#pragma once
#include <wx/glcanvas.h>
#include <osgViewer/Viewer>

class PackageWindow;

// osg Wrapper
class OSGCanvas : public wxGLCanvas
{
public:
  OSGCanvas(PackageWindow* rootWindow, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, 
            long style = 0, const wxString& name = wxT("OSGCanvas"), int* attributes = 0);

  ~OSGCanvas() override;

  void SetGraphicsWindow(osgViewer::GraphicsWindow* window)
  {
    Container = window;
  }

  void SetLockLmb(bool lock)
  {
    LockLmb = lock;
  }

  void SetLockScroll(bool lock)
  {
    LockScroll = lock;
  }

  void OnPaint(wxPaintEvent&);
  void OnSize(wxSizeEvent& event);
  void OnEraseBackground(wxEraseEvent&);

  void OnChar(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);

  void OnMouseEnter(wxMouseEvent& event);
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);
  void OnMouseMotion(wxMouseEvent& event);
  void OnMouseWheel(wxMouseEvent& event);

  void UseCursor(bool value);

  void SetContextCurrent();
private:
  DECLARE_EVENT_TABLE()

  osg::ref_ptr<osgViewer::GraphicsWindow> Container = nullptr;

  PackageWindow* Window = nullptr;
  wxCursor PreviousCursor;
  wxGLContext Context;

  // TODO: remove this and create a proper orthographic manipulator
  bool LockLmb = false;
  bool LockScroll = false;
};

class OSGWindow : public osgViewer::GraphicsWindow
{
public:
  OSGWindow(OSGCanvas* canvas);
  ~OSGWindow();

  void Init();

  void grabFocus() override;
  void grabFocusIfPointerInWindow() override;
  void useCursor(bool cursorOn) override;

  bool makeCurrentImplementation() override;
  void swapBuffersImplementation() override;

  bool valid() const override
  {
    return Canvas;
  }
  
  bool realizeImplementation() override
  {
    return true;
  }

  bool isRealizedImplementation() const override
  {
    return Canvas ? Canvas->IsShownOnScreen() : false;
  }

  void closeImplementation() override
  {}

  bool releaseContextImplementation() override
  {
    return true;
  }

private:
  OSGCanvas* Canvas = nullptr;
};