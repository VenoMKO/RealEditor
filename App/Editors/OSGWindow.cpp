#include "OSGWindow.h"

#include <wx/wx.h>

#include "../PackageWindow.h"

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

void OSGCanvas::OnMouseEnter(wxMouseEvent&)
{
  if (Window->IsActive())
  {
    SetFocus();
  }
}

void OSGCanvas::OnMouseDown(wxMouseEvent& event)
{
  if (Container.valid())
  {
    if (LockLmb)
    {
      auto btn = event.GetButton();
      Container->getEventQueue()->mouseButtonPress(event.GetX(), event.GetY(), btn == 1 ? 2 : btn);
    }
    else
    {
      Container->getEventQueue()->mouseButtonPress(event.GetX(), event.GetY(), event.GetButton());
    }
  }
}

void OSGCanvas::OnMouseUp(wxMouseEvent& event)
{
  if (Container.valid())
  {
    if (LockLmb)
    {
      auto btn = event.GetButton();
      Container->getEventQueue()->mouseButtonRelease(event.GetX(), event.GetY(), btn == 1 ? 2 : btn);
    }
    else
    {
      Container->getEventQueue()->mouseButtonRelease(event.GetX(), event.GetY(), event.GetButton());
    }
  }
}

void OSGCanvas::OnMouseMotion(wxMouseEvent& event)
{
  if (Container.valid())
  {
    Container->getEventQueue()->mouseMotion(event.GetX(), event.GetY());
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
    setState(new osg::State);
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

EVT_ENTER_WINDOW(OSGCanvas::OnMouseEnter)
EVT_LEFT_DOWN(OSGCanvas::OnMouseDown)
EVT_MIDDLE_DOWN(OSGCanvas::OnMouseDown)
EVT_RIGHT_DOWN(OSGCanvas::OnMouseDown)
EVT_LEFT_UP(OSGCanvas::OnMouseUp)
EVT_MIDDLE_UP(OSGCanvas::OnMouseUp)
EVT_RIGHT_UP(OSGCanvas::OnMouseUp)
EVT_MOTION(OSGCanvas::OnMouseMotion)
EVT_MOUSEWHEEL(OSGCanvas::OnMouseWheel)
END_EVENT_TABLE()