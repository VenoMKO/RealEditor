#include "ProgressWindow.h"

wxDEFINE_EVENT(UPDATE_MAX_PROGRESS, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_PROGRESS, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_PROGRESS_ADV, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_PROGRESS_DESC, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_PROGRESS_FINISH, wxCommandEvent);

class ActionTextCtrl : public wxTextCtrl {
public:
  using wxTextCtrl::wxTextCtrl;

  bool AcceptsFocus() const wxOVERRIDE
  {
    return false;
  }
  bool AcceptsFocusFromKeyboard() const wxOVERRIDE
  {
    return false;
  }
};

ProgressWindow::ProgressWindow(wxWindow* parent, const wxString& title, const wxString& cancel)
  : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(500, 115), wxCAPTION | wxTAB_TRAVERSAL)
{
  SetIcon(wxICON(#114));
  SetSizeHints(wxDefaultSize, wxDefaultSize);
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxHORIZONTAL);

  ActionLabel = new ActionTextCtrl(this, wxID_ANY, wxT("Preparing..."), wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxBORDER_NONE);
  ActionLabel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_CAPTIONTEXT));
  ActionLabel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  bSizer3->Add(ActionLabel, 1, wxALL | wxALIGN_BOTTOM, 5);


  bSizer1->Add(bSizer3, 1, wxEXPAND, 5);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  ProgressBar = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
  ProgressBar->SetValue(0);
  bSizer2->Add(ProgressBar, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  CancelButton = new wxButton(this, wxID_ANY, cancel, wxDefaultPosition, wxDefaultSize, 0);
  bSizer2->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


  bSizer1->Add(bSizer2, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);


  SetSizer(bSizer1);
  Layout();

  Centre(wxBOTH);

  ActionLabel->SetFocus();
  ActionLabel->SetDoubleBuffered(true);
  ActionLabel->ShowNativeCaret(false);
  ActionLabel->HideNativeCaret();

  // Connect Events
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ProgressWindow::OnCancelClicked), NULL, this);
}

void ProgressWindow::SetActionText(const wxString& text)
{
  if (!Canceled.load())
  {
    ActionLabel->SetFocus();
    ActionLabel->SetLabelText(text);
    ActionLabel->ShowNativeCaret(false);
    ActionLabel->HideNativeCaret();
  }
}

void ProgressWindow::AdvanceProgress()
{
  ProgressBar->SetValue(ProgressBar->GetValue() + 1);
}

void ProgressWindow::SetCurrentProgress(int progress)
{
  if (Canceled.load())
  {
    return;
  }
  if (progress < 0)
  {
    ProgressBar->Pulse();
  }
  else
  {
    ProgressBar->SetValue(progress);
  }
}


void ProgressWindow::SetMaxProgress(int max)
{
  ProgressBar->SetRange(max);
}

void ProgressWindow::SetCanCancel(bool flag)
{
  CancelButton->Enable(flag);
}

void ProgressWindow::OnCancelClicked(wxCommandEvent&)
{
  CancelButton->Enable(false);
  Canceled.store(true);
  ActionLabel->SetLabelText("Stopping...");
  ProgressBar->Pulse();
}

void ProgressWindow::OnUpdateMaxProgress(wxCommandEvent& e)
{
  SetMaxProgress(e.GetInt());
}

void ProgressWindow::OnUpdateProgress(wxCommandEvent& e)
{
  SetCurrentProgress(e.GetInt());
}

void ProgressWindow::OnAdvanceProgress(wxCommandEvent& e)
{
  AdvanceProgress();
}

void ProgressWindow::OnUpdateProgressDescription(wxCommandEvent& e)
{
  SetActionText(e.GetString());
}

void ProgressWindow::OnUpdateProgressFinish(wxCommandEvent& e)
{
  SetCurrentProgress(0);
  CancelButton->Enable(false);
  if (IsModal())
  {
    EndModal(e.GetInt());
  }
  else
  {
    Close();
  }
}

wxBEGIN_EVENT_TABLE(ProgressWindow, wxDialog)
EVT_COMMAND(wxID_ANY, UPDATE_PROGRESS, OnUpdateProgress)
EVT_COMMAND(wxID_ANY, UPDATE_PROGRESS_ADV, OnAdvanceProgress)
EVT_COMMAND(wxID_ANY, UPDATE_MAX_PROGRESS, OnUpdateMaxProgress)
EVT_COMMAND(wxID_ANY, UPDATE_PROGRESS_DESC, OnUpdateProgressDescription)
EVT_COMMAND(wxID_ANY, UPDATE_PROGRESS_FINISH, OnUpdateProgressFinish)
wxEND_EVENT_TABLE()

void ProgressWindow::OnSetFocus(wxFocusEvent& event)
{
  event.Skip();
  ActionLabel->ShowNativeCaret(false);
  ActionLabel->HideNativeCaret();
}
