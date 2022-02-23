#pragma once
#include <wx/wx.h>
#include <wx/evtloop.h>

class wxModalWindow : public wxFrame {
private:
  // while we are showing a modal window we disable the other windows using
  // this object
  wxWindowDisabler* m_windowDisabler;

  // modal window runs its own event loop
  wxEventLoop* m_eventLoop;

  // is modal right now?
  bool m_isShowingModal;

  //The return code of a modal window
  int m_returnCode;
public:
  wxModalWindow();
  wxModalWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE, const wxString& name = "modalwindow");
  virtual ~wxModalWindow();


  void Init();
  bool Show(bool show);
  bool IsModal() const;
  virtual int ShowModal();

  void EndModal(int retCode);
  void SetReturnCode(int retCode);
  int GetReturnCode() const;
};