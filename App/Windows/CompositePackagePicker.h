#pragma once
#include <wx/wx.h>

class CompositePackagePicker : public wxDialog {
public:
  CompositePackagePicker(wxWindow* parent, const wxString& title);

  wxString GetResult() const;

protected:
  void OnText(wxCommandEvent&);
  void OnTextEnter(wxCommandEvent&);

protected:
  wxTextCtrl* CompositeName = nullptr;
  wxButton* CancelButton = nullptr;
  wxButton* OpenButton = nullptr;

  wxDECLARE_EVENT_TABLE();
};