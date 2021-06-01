#pragma once
#include <wx/wx.h>
#include <atomic>

wxDECLARE_EVENT(UPDATE_MAX_PROGRESS, wxCommandEvent);
wxDECLARE_EVENT(UPDATE_PROGRESS, wxCommandEvent);
wxDECLARE_EVENT(UPDATE_PROGRESS_DESC, wxCommandEvent);
wxDECLARE_EVENT(UPDATE_PROGRESS_FINISH, wxCommandEvent);

class ProgressWindow : public wxDialog {
public:
  ProgressWindow(wxWindow* parent, const wxString& title = wxT("Loading"), const wxString& cancel = wxT("Cancel"));

  void SetActionText(const wxString& text);
  
  void SetCurrentProgress(int progress);

  void SetMaxProgress(int max);

  void SetCanCancel(bool flag);

  bool IsCanceled()
  {
    return Cancelled.load();
  }

private:
  void OnCancellClicked(wxCommandEvent&);

  void OnUpdateMaxProgress(wxCommandEvent& e);

  void OnUpdateProgress(wxCommandEvent& e);

  void OnUpdateProgressDescription(wxCommandEvent& e);

  void OnUpdateProgressFinish(wxCommandEvent& e);

  wxDECLARE_EVENT_TABLE();
private:
  wxTextCtrl* ActionLabel = nullptr;
  wxGauge* ProgressBar = nullptr;
  wxButton* CancelButton = nullptr;
  std::atomic_bool Cancelled = { false };
};