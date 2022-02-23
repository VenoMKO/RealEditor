#pragma once
#include <wx/wx.h>
#include <atomic>

#include "WXDialog.h"

wxDECLARE_EVENT(UPDATE_MAX_PROGRESS, wxCommandEvent);
wxDECLARE_EVENT(UPDATE_PROGRESS, wxCommandEvent);
wxDECLARE_EVENT(UPDATE_PROGRESS_ADV, wxCommandEvent);
wxDECLARE_EVENT(UPDATE_PROGRESS_DESC, wxCommandEvent);
wxDECLARE_EVENT(UPDATE_PROGRESS_FINISH, wxCommandEvent);

class ProgressWindow : public WXDialog {
public:
  ProgressWindow(wxWindow* parent, const wxString& title = wxT("Loading"), const wxString& cancel = wxT("Cancel"));

  void SetActionText(const wxString& text);
  
  void SetCurrentProgress(int progress);

  void AdvanceProgress();

  void SetMaxProgress(int max);

  void SetCanCancel(bool flag);

  bool IsCanceled()
  {
    return Canceled.load();
  }

  void SetCanceled()
  {
    Canceled.store(true);
  }

  int ShowModal() wxOVERRIDE;

private:
  void OnCancelClicked(wxCommandEvent&);

  void OnUpdateMaxProgress(wxCommandEvent& e);

  void OnUpdateProgress(wxCommandEvent& e);

  void OnAdvanceProgress(wxCommandEvent& e);

  void OnUpdateProgressDescription(wxCommandEvent& e);

  void OnUpdateProgressFinish(wxCommandEvent& e);

  void OnSetFocus(wxFocusEvent& event);

  wxDECLARE_EVENT_TABLE();
private:
  wxTextCtrl* ActionLabel = nullptr;
  wxGauge* ProgressBar = nullptr;
  wxButton* CancelButton = nullptr;
  std::atomic_bool Canceled = { false };
};