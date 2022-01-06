#pragma once
#include <wx/wx.h>
#include "../Misc/WXDialog.h"

#include <Tera/FStructs.h>

class CreatePackageDialog : public WXDialog {
public:
  CreatePackageDialog(int coreVer, wxWindow* parent = nullptr);

  void FillSummary(FPackageSummary& summary);

protected:
  void OnNameChanged(wxCommandEvent&);
  void OnCompositeChanged(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);
  void OnCancelClicked(wxCommandEvent&);

protected:
  wxTextCtrl* PackageName = nullptr;
  wxTextCtrl* Composite = nullptr;
  wxChoice* Licensee = nullptr;
  wxChoice* Compression = nullptr;
  wxButton* OkButton = nullptr;
  wxButton* CancelButton = nullptr;
  int CoreVersion = 0;
};