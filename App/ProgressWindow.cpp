#include "ProgressWindow.h"

wxDEFINE_EVENT(UPDATE_PROGRESS, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_PROGRESS_DESC, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_PROGRESS_FINISH, wxCommandEvent);

ProgressWindow::ProgressWindow(wxWindow* parent, const wxString& title, const wxString& cancel)
	: wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(500, 94), wxCAPTION | wxTAB_TRAVERSAL)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	ActionLabel = new wxStaticText(this, wxID_ANY, wxT("Loading..."), wxDefaultPosition, wxDefaultSize, 0);
	ActionLabel->Wrap(-1);
	bSizer1->Add(ActionLabel, 1, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	ProgressBar = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
	ProgressBar->SetValue(0);
	bSizer2->Add(ProgressBar, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxButton* m_button1 = new wxButton(this, wxID_ANY, cancel, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(m_button1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	bSizer1->Add(bSizer2, 0, wxEXPAND, 5);


	this->SetSizer(bSizer1);
	this->Layout();

	this->Centre(wxBOTH);

	// Connect Events
	m_button1->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ProgressWindow::OnCancellClicked), NULL, this);
}

void ProgressWindow::SetActionText(const wxString& text)
{
	if (!Cancelled.load())
	{
		ActionLabel->SetLabelText(text);
	}
}

void ProgressWindow::SetCurrentProgress(int progress)
{
	if (Cancelled.load())
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

void ProgressWindow::OnCancellClicked(wxCommandEvent&)
{
	Cancelled.store(true);
	ActionLabel->SetLabelText("Stopping...");
	ProgressBar->Pulse();
}

void ProgressWindow::OnUpdateProgress(wxCommandEvent& e)
{
	SetCurrentProgress(e.GetInt());
}

void ProgressWindow::OnUpdateProgressDescription(wxCommandEvent& e)
{
	SetActionText(e.GetString());
}

void ProgressWindow::OnUpdateProgressFinish(wxCommandEvent& e)
{
	Close();
}

wxBEGIN_EVENT_TABLE(ProgressWindow, wxFrame)
EVT_COMMAND(wxID_ANY, UPDATE_PROGRESS, OnUpdateProgress)
EVT_COMMAND(wxID_ANY, UPDATE_PROGRESS_DESC, OnUpdateProgressDescription)
EVT_COMMAND(wxID_ANY, UPDATE_PROGRESS_FINISH, OnUpdateProgressFinish)
wxEND_EVENT_TABLE()