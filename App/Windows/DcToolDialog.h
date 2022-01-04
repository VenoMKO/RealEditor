#pragma once
#include <wx/wx.h>
#include <wx/filepicker.h>
#include <wx/radiobox.h>
#include "../Misc/WXDialog.h"

class DcToolDialog : public WXDialog {
public:
  DcToolDialog(wxWindow* parent);
  ~DcToolDialog();

protected:
  void OnKeyChanged(wxCommandEvent& event);
  void OnVecChanged(wxCommandEvent& event);
  void OnFindClicked(wxCommandEvent& event);
  void OnDcFileChanged(wxFileDirPickerEvent& event);
  void OnUnpackClicked(wxCommandEvent& event);
  void OnCloseClicked(wxCommandEvent& event);
  void OnEditClicked(wxCommandEvent& event);

  void UpdateButtons();

protected:
  wxStaticText* ClientVersion = nullptr;
  wxTextCtrl* KeyField = nullptr;
  wxTextCtrl* VecField = nullptr;
  wxButton* FindButton = nullptr;
  wxFilePickerCtrl* DcFilePicker = nullptr;
  wxRadioBox* Mode = nullptr;
  wxRadioBox* Client = nullptr;
  wxButton* UnpackButton = nullptr;
  wxButton* CloseButton = nullptr;
  wxButton* EditButton = nullptr;
};