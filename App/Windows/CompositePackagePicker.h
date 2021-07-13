#pragma once
#include <wx/wx.h>
#include "../Misc/WXDialog.h"

class CompositePackagePicker : public WXDialog {
public:
  CompositePackagePicker(wxWindow* parent, const wxString& title, bool filePackages = false);

  wxString GetResult() const;

protected:
  void OnText(wxCommandEvent&);
  void OnTextEnter(wxCommandEvent&);

protected:
  wxTextCtrl* CompositeName = nullptr;
  wxButton* CancelButton = nullptr;
  wxButton* OpenButton = nullptr;
  bool FilePackages = false;

  wxDECLARE_EVENT_TABLE();
};