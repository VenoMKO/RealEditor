#include "BakeModDialog.h"
#include "REDialogs.h"

#include "../App.h"

#include <wx/statline.h>

BakeModDialog::BakeModDialog(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, wxT("Bake a mod"), wxDefaultPosition, wxSize(497, 232))
{
	SetSize(FromDIP(GetSize()));
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText2;
	m_staticText2 = new wxStaticText(this, wxID_ANY, wxT("Select a TMM mod you want to bake into the client"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText2->Wrap(-1);
	bSizer1->Add(m_staticText2, 0, wxALL, FromDIP(5));

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText1;
	m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Mod:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText1->Wrap(-1);
	bSizer2->Add(m_staticText1, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

	ModField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	bSizer2->Add(ModField, 1, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

	wxButton* m_button1;
	m_button1 = new wxButton(this, wxID_ANY, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(m_button1, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


	bSizer1->Add(bSizer2, 0, wxEXPAND, FromDIP(5));

	wxStaticLine* m_staticline1;
	m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer1->Add(m_staticline1, 0, wxEXPAND | wxTOP | wxBOTTOM, FromDIP(5));

	wxStaticText* m_staticText3;
	m_staticText3 = new wxStaticText(this, wxID_ANY, wxT("Select a folder to save modified files to"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText3->Wrap(-1);
	bSizer1->Add(m_staticText3, 0, wxALL, FromDIP(5));

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText4;
	m_staticText4 = new wxStaticText(this, wxID_ANY, wxT("Out:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText4->Wrap(-1);
	bSizer3->Add(m_staticText4, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

	DestField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	bSizer3->Add(DestField, 1, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

	wxButton* m_button2;
	m_button2 = new wxButton(this, wxID_ANY, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0);
	bSizer3->Add(m_button2, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


	bSizer1->Add(bSizer3, 0, wxEXPAND, FromDIP(5));

	wxStaticLine* m_staticline11;
	m_staticline11 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer1->Add(m_staticline11, 0, wxEXPAND | wxALL, FromDIP(5));

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText5;
	m_staticText5 = new wxStaticText(this, wxID_ANY, wxT("Disable all TMM mods before you continue"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText5->Wrap(-1);
	m_staticText5->SetForegroundColour(wxColour(255, 0, 0));

	bSizer4->Add(m_staticText5, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


	bSizer4->Add(0, 0, 1, wxEXPAND, FromDIP(5));

	BakeButton = new wxButton(this, wxID_ANY, wxT("Bake"), wxDefaultPosition, wxDefaultSize, 0);
	BakeButton->Enable(false);

	bSizer4->Add(BakeButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

	wxButton* m_button4;
	m_button4 = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(m_button4, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


	bSizer1->Add(bSizer4, 1, wxEXPAND, FromDIP(5));


	SetSizer(bSizer1);
	Layout();

	Centre(wxBOTH);

	ModField->SetValue(App::GetSharedApp()->GetConfig().LastBakeMod.WString());
	DestField->SetValue(App::GetSharedApp()->GetExportPath());

	UpdateOk();

	// Connect Events
	m_button1->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BakeModDialog::OnBrowseModClicked), NULL, this);
	m_button2->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BakeModDialog::OnBrowseDestClicked), NULL, this);
	BakeButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BakeModDialog::OnBakeClicked), NULL, this);
	m_button4->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BakeModDialog::OnCancelClicked), NULL, this);
}

wxString BakeModDialog::GetSource() const
{
	return ModField->GetValue();
}

wxString BakeModDialog::GetDestination() const
{
	return DestField->GetValue();
}

void BakeModDialog::OnBrowseModClicked(wxCommandEvent&)
{
	wxString modPath = IODialog::OpenPackageDialog(this, ModField->GetValue(), "Select a mod file...");
	if (!modPath.size())
	{
		return;
	}
	ModField->SetValue(modPath);
	App::GetSharedApp()->GetConfig().LastBakeMod = modPath.ToStdWstring();
	App::GetSharedApp()->SaveConfig();
	UpdateOk();
}

void BakeModDialog::OnBrowseDestClicked(wxCommandEvent&)
{
	wxString dirPath = IODialog::OpenDirectoryDialog(this, DestField->GetValue());
	if (!dirPath.size())
	{
		return;
	}
	DestField->SetValue(dirPath);
	App::GetSharedApp()->GetConfig().LastExportPath = dirPath.ToStdWstring();
	App::GetSharedApp()->SaveConfig();
	UpdateOk();
}

void BakeModDialog::OnBakeClicked(wxCommandEvent&)
{
	EndModal(wxID_OK);
}

void BakeModDialog::OnCancelClicked(wxCommandEvent&)
{
	EndModal(wxID_CANCEL);
}

void BakeModDialog::UpdateOk()
{
	BakeButton->Enable(ModField->GetValue().size() && DestField->GetValue().size());
}
