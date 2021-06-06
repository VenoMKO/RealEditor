#pragma once
#include <wx/wx.h>
#include <wx/filepicker.h>
#include <wx/radiobox.h>

class DcToolDialog : public wxDialog {
public:
  DcToolDialog(wxWindow* parent);

protected:
  void OnKeyChanged(wxCommandEvent& event);
  void OnVecChanged(wxCommandEvent& event);
  void OnFindClicked(wxCommandEvent& event);
  void OnDcFileChanged(wxFileDirPickerEvent& event);
  void OnUnpackClicked(wxCommandEvent& event);
  void OnCloseClicked(wxCommandEvent& event);

  void UpdateButtons();

protected:
  wxStaticText* ClientVersion;
  wxTextCtrl* KeyField;
  wxTextCtrl* VecField;
  wxButton* FindButton;
  wxFilePickerCtrl* DcFilePicker;
  wxRadioBox* Mode;
  wxButton* UnpackButton;
  wxButton* CloseButton;
};