#include "CreateModWindow.h"
#include "../App.h"

#include <Tera/FString.h>

enum ControlElementId {
	Name = wxID_HIGHEST + 1,
	Author
};

CreateModWindow::CreateModWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText1;
	m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText1->Wrap(-1);
	bSizer2->Add(m_staticText1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	NameField = new wxTextCtrl(this, ControlElementId::Name, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(NameField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	wxStaticText* m_staticText2;
	m_staticText2 = new wxStaticText(this, wxID_ANY, wxT("Author:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText2->Wrap(-1);
	bSizer2->Add(m_staticText2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	AuthorField = new wxTextCtrl(this, ControlElementId::Author, App::GetSharedApp()->GetConfig().LastModAuthor.WString(), wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(AuthorField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer1->Add(bSizer2, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* m_panel1;
	m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	bSizer3->Add(m_panel1, 1, wxEXPAND | wxALL, 5);

	CreateButton = new wxButton(this, wxID_OK, wxT("Save to..."), wxDefaultPosition, wxDefaultSize, 0);
	CreateButton->Enable(false);
	bSizer3->Add(CreateButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	CancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer3->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer1->Add(bSizer3, 1, wxEXPAND, 5);


	this->SetSizer(bSizer1);
	this->Layout();

	this->Centre(wxBOTH);
}

wxString CreateModWindow::GetName() const
{
	return NameField->GetValue();
}

wxString CreateModWindow::GetAuthor() const
{
	return AuthorField->GetValue();
}

void CreateModWindow::OnTextEvent(wxCommandEvent&)
{
	bool ok = false;
	FString testString = GetName().ToStdWstring();
	if (testString.Size())
	{
		ok = testString.IsAnsi();
	}
	if (ok)
	{
		testString = GetAuthor().ToStdWstring();
		ok = testString.Size() && testString.IsAnsi();
	}
	CreateButton->Enable(ok);
}

wxBEGIN_EVENT_TABLE(CreateModWindow, wxDialog)
EVT_TEXT(ControlElementId::Author, CreateModWindow::OnTextEvent)
EVT_TEXT(ControlElementId::Name, CreateModWindow::OnTextEvent)
wxEND_EVENT_TABLE()