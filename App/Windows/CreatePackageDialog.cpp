#include "CreatePackageDialog.h"

#include <wx/statline.h>

CreatePackageDialog::CreatePackageDialog(int coreVer, wxWindow* parent)
  : WXDialog(parent, wxID_ANY, _("Create GPK"), wxDefaultPosition, wxSize(361, 235))
{
  CoreVersion = coreVer;
  SetSize(FromDIP(GetSize()));
  SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* fgSizer1;
  fgSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer1->AddGrowableCol(1);
  fgSizer1->SetFlexibleDirection(wxBOTH);
  fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  wxStaticText* m_staticText3;
  m_staticText3 = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText3->Wrap(-1);
  fgSizer1->Add(m_staticText3, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, FromDIP(5));

  PackageName = new wxTextCtrl(this, wxID_ANY, wxT("Untitled"), wxDefaultPosition, wxDefaultSize, 0);
  PackageName->SetToolTip(wxT("GPK file name"));

  fgSizer1->Add(PackageName, 1, wxALL | wxEXPAND, FromDIP(5));

  wxStaticText* m_staticText4;
  m_staticText4 = new wxStaticText(this, wxID_ANY, wxT("Composite:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText4->Wrap(-1);
  fgSizer1->Add(m_staticText4, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  Composite = new wxTextCtrl(this, wxID_ANY, wxT("None"), wxDefaultPosition, wxDefaultSize, 0);
  Composite->SetToolTip(wxT("Composite path. Mandatory if the GPK is going to be used with TMM!"));
  Composite->Enable(CoreVersion > VER_TERA_CLASSIC);

  fgSizer1->Add(Composite, 1, wxALL | wxEXPAND, FromDIP(5));

  wxStaticText* m_staticText5;
  m_staticText5 = new wxStaticText(this, wxID_ANY, wxT("Licensee:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText5->Wrap(-1);
  fgSizer1->Add(m_staticText5, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, FromDIP(5));

  wxString LicenseeChoices32[] = { wxT("13"), wxT("14") };
  wxString LicenseeChoices64[] = { wxT("17") };
  int LicenseeNChoices32 = sizeof(LicenseeChoices32) / sizeof(wxString);
  int LicenseeNChoices64 = sizeof(LicenseeChoices64) / sizeof(wxString);
  Licensee = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, CoreVersion == VER_TERA_CLASSIC ? LicenseeNChoices32 : LicenseeNChoices64, CoreVersion == VER_TERA_CLASSIC ? LicenseeChoices32 : LicenseeChoices64, 0);
  Licensee->SetSelection(CoreVersion == VER_TERA_CLASSIC ? 1 : 0);
  Licensee->Enable(CoreVersion == VER_TERA_CLASSIC);
  fgSizer1->Add(Licensee, 0, wxALL, FromDIP(5));

  wxStaticText* m_staticText8;
  m_staticText8 = new wxStaticText(this, wxID_ANY, wxT("Compression:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText8->Wrap(-1);
  fgSizer1->Add(m_staticText8, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  wxString CompressionChoices[] = { wxT("None"), wxT("LZO") };
  int CompressionNChoices = sizeof(CompressionChoices) / sizeof(wxString);
  Compression = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, CompressionNChoices, CompressionChoices, 0);
  Compression->SetSelection(0);
  Compression->Enable(CoreVersion == VER_TERA_CLASSIC);
  Compression->SetToolTip(wxT("Compress GPK file. (32-bit client only!)"));

  fgSizer1->Add(Compression, 0, wxALL, FromDIP(5));


  bSizer9->Add(fgSizer1, 1, wxEXPAND, FromDIP(5));

  wxStaticLine* m_staticline2;
  m_staticline2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer9->Add(m_staticline2, 0, wxEXPAND | wxTOP | wxBOTTOM, FromDIP(5));

  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);


  bSizer11->Add(0, 0, 1, wxEXPAND, FromDIP(5));

  OkButton = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer11->Add(OkButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer11->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  bSizer9->Add(bSizer11, 1, wxEXPAND, 5);


  this->SetSizer(bSizer9);
  this->Layout();

  this->Centre(wxBOTH);

  // Connect Events
  PackageName->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CreatePackageDialog::OnNameChanged), NULL, this);
  Composite->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CreatePackageDialog::OnCompositeChanged), NULL, this);
  OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CreatePackageDialog::OnOkClicked), NULL, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CreatePackageDialog::OnCancelClicked), NULL, this);
}

void CreatePackageDialog::FillSummary(FPackageSummary& summary)
{
  summary.PackageName = PackageName->GetValue().ToStdWstring();
  if (CoreVersion > VER_TERA_CLASSIC)
  {
    summary.FolderName = Composite->GetValue().ToStdWstring();
  }
  uint16 lv = -1;
  switch (Licensee->GetSelection())
  {
  case 0:
    if (CoreVersion == VER_TERA_CLASSIC)
    {
      lv = 13;
      summary.PackageFlags &= ~PKG_Cooked;
    }
    else
    {
      lv = 17;
    }
    break;
  case 1:
    lv = 14;
    break;
  }
  summary.SetFileVersion(CoreVersion, lv);
  if (Compression->GetSelection())
  {
    summary.CompressionFlags = COMPRESS_LZO;
  }
}

void CreatePackageDialog::OnNameChanged(wxCommandEvent&)
{
  OkButton->Enable(PackageName->GetValue().size());
}

void CreatePackageDialog::OnCompositeChanged(wxCommandEvent&)
{
  OkButton->Enable(Composite->GetValue().size());
}

void CreatePackageDialog::OnOkClicked(wxCommandEvent&)
{
  EndModal(wxID_OK);
}

void CreatePackageDialog::OnCancelClicked(wxCommandEvent&)
{
  EndModal(wxID_CANCEL);
}
