#include "OSGView.h"

#include <wx/wx.h>

#include "../Windows/PackageWindow.h"

OSGCanvas::OSGCanvas(PackageWindow* rootWindow, wxWindow* parent, wxWindowID id,
  const wxPoint& pos, const wxSize& size, long style, const wxString& name, int* attributes)
  : wxGLCanvas(parent, id, attributes, pos, size, style | wxFULL_REPAINT_ON_RESIZE, name)
  , Window(rootWindow)
  , Context(this)
{
  PreviousCursor = *wxSTANDARD_CURSOR;
}

OSGCanvas::~OSGCanvas()
{
}

void OSGCanvas::SetContextCurrent()
{
  Context.SetCurrent(*this);
}

void OSGCanvas::ConvertPosition(int& x, int& y)
{
  wxWindow* parent = GetParent();
  {
    wxWindow* tmp = parent;
    while (tmp)
    {
      parent = tmp;
      tmp = parent->GetParent();
    }
  }
  if (parent)
  {
    ClientToScreen(&x, &y);
    int tmpX = 0;
    int tmpY = 0;
    parent->GetPosition(&tmpX, &tmpY);
    x -= tmpX;
    y -= tmpY;
  }
}

void OSGCanvas::OnPaint(wxPaintEvent&)
{
  wxPaintDC dc(this);
}

void OSGCanvas::OnSize(wxSizeEvent&)
{
  int width, height;
  GetClientSize(&width, &height);
  if (Container.valid())
  {
    Container->getEventQueue()->windowResize(0, 0, width, height);
    Container->resized(0, 0, width, height);
  }
}

void OSGCanvas::OnEraseBackground(wxEraseEvent&)
{
}

void OSGCanvas::OnChar(wxKeyEvent& event)
{
  if (Container.valid())
  {
    Container->getEventQueue()->keyPress(event.GetUnicodeKey());
  }
}

void OSGCanvas::OnKeyUp(wxKeyEvent& event)
{
  if (Container.valid())
  {
    Container->getEventQueue()->keyRelease(event.GetUnicodeKey());
  }
}

void OSGCanvas::OnMouseDown(wxMouseEvent& event)
{
  if (Container.valid())
  {
    if (!DragMap[wxMOUSE_BTN_LEFT] && !DragMap[wxMOUSE_BTN_RIGHT] && !DragMap[wxMOUSE_BTN_MIDDLE] &&
        (event.GetButton() == wxMOUSE_BTN_LEFT || event.GetButton() == wxMOUSE_BTN_RIGHT || event.GetButton() == wxMOUSE_BTN_MIDDLE))
    {
      CaptureMouse();
    }
    int x, y;
    event.GetPosition(&x, &y);
    ConvertPosition(x, y);
    DragMap[event.GetButton()] = true;
    if (LockLmb)
    {
      auto btn = event.GetButton();
      Container->getEventQueue()->mouseButtonPress(x, y, btn == 1 ? 2 : btn);
    }
    else
    {
      Container->getEventQueue()->mouseButtonPress(x, y, event.GetButton());
    }
  }
}

void OSGCanvas::OnMouseUp(wxMouseEvent& event)
{
  if (Container.valid())
  {
    if (DragMap[event.GetButton()])
    {
      DragMap[event.GetButton()] = false;
      if (!DragMap[wxMOUSE_BTN_LEFT] && !DragMap[wxMOUSE_BTN_RIGHT] && !DragMap[wxMOUSE_BTN_MIDDLE])
      {
        ReleaseMouse();
      }
    }
    int x, y;
    event.GetPosition(&x, &y);
    ConvertPosition(x, y);
    DragMap[event.GetButton()] = false;
    if (LockLmb)
    {
      auto btn = event.GetButton();
      Container->getEventQueue()->mouseButtonRelease(x, y, btn == 1 ? 2 : btn);
    }
    else
    {
      Container->getEventQueue()->mouseButtonRelease(x, y, event.GetButton());
    }
  }
}

void OSGCanvas::OnMouseMotion(wxMouseEvent& event)
{
  if (Container.valid())
  {
    int x, y;
    event.GetPosition(&x, &y);
    ConvertPosition(x, y);
    Container->getEventQueue()->mouseMotion(x, y);
  }
}

void OSGCanvas::OnMouseWheel(wxMouseEvent& event)
{
  if (!LockScroll && Container.valid())
  {
    int delta = event.GetWheelRotation() / event.GetWheelDelta() * event.GetLinesPerAction();
    Container->getEventQueue()->mouseScroll( delta > 0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN);
  }
}

void OSGCanvas::UseCursor(bool value)
{
  if (value)
  {
    SetCursor(PreviousCursor);
  }
  else
  {
    PreviousCursor = GetCursor();
    wxImage image(1, 1);
    image.SetMask(true);
    image.SetMaskColour(0, 0, 0);
    wxCursor cursor(image);
    SetCursor(cursor);
  }
}

OSGWindow::OSGWindow(OSGCanvas* canvas)
{
  Canvas = canvas;

  _traits = new GraphicsContext::Traits;

  wxPoint pos = Canvas->GetPosition();
  wxSize  size = Canvas->GetSize();

  _traits->x = pos.x;
  _traits->y = pos.y;
  _traits->width = size.x;
  _traits->height = size.y;

  Init();
}

OSGWindow::~OSGWindow()
{
}

void OSGWindow::Init()
{
  if (valid())
  {
    osg::ref_ptr<osg::State> st = new osg::State;
    setState(st);
    getState()->setGraphicsContext(this);

    if (_traits.valid() && _traits->sharedContext.valid())
    {
      getState()->setContextID(_traits->sharedContext->getState()->getContextID());
      incrementContextIDUsageCount(getState()->getContextID());
    }
    else
    {
      getState()->setContextID(osg::GraphicsContext::createNewContextID());
    }
  }
}

void OSGWindow::grabFocus()
{
  Canvas->SetFocus();
}

void OSGWindow::grabFocusIfPointerInWindow()
{
  wxPoint pos = wxGetMousePosition();
  if (wxFindWindowAtPoint(pos) == Canvas)
    Canvas->SetFocus();
}

void OSGWindow::useCursor(bool cursorOn)
{
  Canvas->UseCursor(cursorOn);
}

bool OSGWindow::makeCurrentImplementation()
{
  Canvas->SetContextCurrent();
  return true;
}

void OSGWindow::swapBuffersImplementation()
{
  Canvas->SwapBuffers();
}

BEGIN_EVENT_TABLE(OSGCanvas, wxGLCanvas)
EVT_SIZE(OSGCanvas::OnSize)
EVT_PAINT(OSGCanvas::OnPaint)
EVT_ERASE_BACKGROUND(OSGCanvas::OnEraseBackground)

EVT_CHAR(OSGCanvas::OnChar)
EVT_KEY_UP(OSGCanvas::OnKeyUp)

EVT_LEFT_DOWN(OSGCanvas::OnMouseDown)
EVT_MIDDLE_DOWN(OSGCanvas::OnMouseDown)
EVT_RIGHT_DOWN(OSGCanvas::OnMouseDown)
EVT_LEFT_UP(OSGCanvas::OnMouseUp)
EVT_MIDDLE_UP(OSGCanvas::OnMouseUp)
EVT_RIGHT_UP(OSGCanvas::OnMouseUp)
EVT_MOTION(OSGCanvas::OnMouseMotion)
EVT_MOUSEWHEEL(OSGCanvas::OnMouseWheel)
END_EVENT_TABLE()