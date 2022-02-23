#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>

#include "WXDialog.h"

#include <Tera/Core.h>
#include <Tera/FFlags.h>

class FlagsDialog : public WXDialog {
public:
  FlagsDialog(wxWindow* parent = nullptr, const wxString& title = wxEmptyString);

  static FlagsDialog* ExportFlagsDialog(EFExportFlags flags, wxWindow* parent = nullptr, const wxString& title = wxEmptyString);
  static FlagsDialog* PackageFlagsDialog(EPackageFlags flags, wxWindow* parent = nullptr, const wxString& title = wxEmptyString);
  static FlagsDialog* ObjectFlagsDialog(EObjectFlags flags, wxWindow* parent = nullptr, const wxString& title = wxEmptyString);

  template <typename T>
  T GetFlags() const
  {
    return static_cast<T>(GetUntypedFlags());
  }

protected:
  void OnDefaultClicked(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);
  void OnCancelClicked(wxCommandEvent&);

  uint64 GetUntypedFlags() const;

protected:
  wxDataViewCtrl* FlagsTable = nullptr;
  wxButton* DefaultsButton = nullptr;
  wxButton* OkButton = nullptr;
  wxButton* CancelButton = nullptr;

  uint64 InternalFlags = 0;
};