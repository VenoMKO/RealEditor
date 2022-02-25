#include "CookingOptions.h"

CookingOptionsWindow::CookingOptionsWindow(wxWindow* parent, FPackage* package, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
  : WXDialog(parent, id, title, pos, size, style)
  , Package(package)
{
  SetSize(FromDIP(GetSize()));
  SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxVERTICAL);

  wxPanel* m_panel5;
  m_panel5 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxVERTICAL);

  EnableCompositeInfoButton = new wxCheckBox(m_panel5, wxID_ANY, wxT("Embed composite information"), wxDefaultPosition, wxDefaultSize, 0);
  EnableCompositeInfoButton->Enable(Package->IsComposite() && Package->GetFolderName() == "None");
  EnableCompositeInfoButton->SetValue(Package->IsComposite() || Package->GetFolderName().StartsWith("MOD:"));
  bSizer11->Add(EnableCompositeInfoButton, 0, wxALL, FromDIP(5));

  wxStaticText* m_staticText3;
  m_staticText3 = new wxStaticText(m_panel5, wxID_ANY, wxT("This option allows to save composite mapping information necessary for TMM to work."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText3->Wrap(FromDIP(480));
  bSizer11->Add(m_staticText3, 0, wxALL, FromDIP(5));


  m_panel5->SetSizer(bSizer11);
  m_panel5->Layout();
  bSizer11->Fit(m_panel5);
  bSizer10->Add(m_panel5, 0, wxALL | wxEXPAND, FromDIP(5));

  wxPanel* m_panel6;
  m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer13;
  bSizer13 = new wxBoxSizer(wxVERTICAL);

  DisableTextureCachingButton = new wxCheckBox(m_panel6, wxID_ANY, wxT("Disable texture caching"), wxDefaultPosition, wxDefaultSize, 0);
  DisableTextureCachingButton->SetValue(!package->GetNoTexturePullOnSave() && package->GetFileVersion() == VER_TERA_MODERN && !package->GetPackageFlag(PKG_NoSource));
  DisableTextureCachingButton->Enable(!package->GetNoTexturePullOnSave() && package->GetFileVersion() == VER_TERA_MODERN && !package->GetPackageFlag(PKG_NoSource));
  bSizer13->Add(DisableTextureCachingButton, 0, wxALL, FromDIP(5));

  wxStaticText* m_staticText31;
  m_staticText31 = new wxStaticText(m_panel6, wxID_ANY, wxT("Needed for region-free mods. Pull hi-res textures to this package, discard mips && texture pre-allocations, remove texture cache record."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText31->Wrap(FromDIP(480));
  bSizer13->Add(m_staticText31, 0, wxALL, FromDIP(5));


  m_panel6->SetSizer(bSizer13);
  m_panel6->Layout();
  bSizer13->Fit(m_panel6);
  bSizer10->Add(m_panel6, 0, wxALL | wxEXPAND, FromDIP(5));

  wxPanel* m_panel61;
  m_panel61 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer131;
  bSizer131 = new wxBoxSizer(wxVERTICAL);

  PreserveOffsetsButton = new wxCheckBox(m_panel61, wxID_ANY, wxT("Preserve offsets"), wxDefaultPosition, wxDefaultSize, 0);
  PreserveOffsetsButton->SetValue(true);
  PreserveOffsetsButton->Enable(false);

  bSizer131->Add(PreserveOffsetsButton, 0, wxALL, FromDIP(5));

  wxStaticText* m_staticText311;
  m_staticText311 = new wxStaticText(m_panel61, wxID_ANY, wxT("Keep unchaged objects at there original offsets."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText311->Wrap(FromDIP(480));
  bSizer131->Add(m_staticText311, 0, wxALL, FromDIP(5));


  m_panel61->SetSizer(bSizer131);
  m_panel61->Layout();
  bSizer131->Fit(m_panel61);
  bSizer10->Add(m_panel61, 0, wxALL | wxEXPAND, FromDIP(5));

  wxPanel* m_panel611;
  m_panel611 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer1311;
  bSizer1311 = new wxBoxSizer(wxVERTICAL);

  CompressPackageButton = new wxCheckBox(m_panel611, wxID_ANY, wxT("Compress package"), wxDefaultPosition, wxDefaultSize, 0);
  CompressPackageButton->SetValue(package->GetPackageFlag(PKG_NoSource) && package->GetSummary().CompressionFlags);
  CompressPackageButton->Enable(false);

  bSizer1311->Add(CompressPackageButton, 0, wxALL, FromDIP(5));

  wxStaticText* m_staticText3111;
  m_staticText3111 = new wxStaticText(m_panel611, wxID_ANY, wxT("Compress the package using LZO algorithm to save disk space."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText3111->Wrap(FromDIP(480));
  bSizer1311->Add(m_staticText3111, 0, wxALL, FromDIP(5));


  m_panel611->SetSizer(bSizer1311);
  m_panel611->Layout();
  bSizer1311->Fit(m_panel611);
  bSizer10->Add(m_panel611, 0, wxEXPAND | wxALL, FromDIP(5));

  wxPanel* m_panel13;
  m_panel13 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer18;
  bSizer18 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel14;
  m_panel14 = new wxPanel(m_panel13, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  bSizer18->Add(m_panel14, 1, wxEXPAND | wxALL, FromDIP(5));

  SaveButton = new wxButton(m_panel13, wxID_OK, wxT("Save"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer18->Add(SaveButton, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  CancelButton = new wxButton(m_panel13, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer18->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  m_panel13->SetSizer(bSizer18);
  m_panel13->Layout();
  bSizer18->Fit(m_panel13);
  bSizer10->Add(m_panel13, 1, wxALL | wxEXPAND, FromDIP(5));


  SetSizer(bSizer10);
  Layout();

  Centre(wxBOTH);

  // Connect Events
  SaveButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CookingOptionsWindow::OnSaveClicked), NULL, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CookingOptionsWindow::OnCancelClicked), NULL, this);
}

CookingOptionsWindow::~CookingOptionsWindow()
{
  SaveButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CookingOptionsWindow::OnSaveClicked), NULL, this);
  CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CookingOptionsWindow::OnCancelClicked), NULL, this);
}

void CookingOptionsWindow::ConfigureSaveContext(PackageSaveContext& ctx)
{
  ctx.Compression = CompressPackageButton->GetValue() ? COMPRESS_LZO : COMPRESS_None;
  ctx.DisableTextureCaching = DisableTextureCachingButton->GetValue();
  ctx.PreserveOffsets = PreserveOffsetsButton->GetValue();
  ctx.EmbedObjectPath = EnableCompositeInfoButton->GetValue();
}

int CookingOptionsWindow::ShowModal()
{
  if (!EnableCompositeInfoButton->IsEnabled() && !DisableTextureCachingButton->IsEnabled() &&
      !PreserveOffsetsButton->IsEnabled() && !CompressPackageButton->IsEnabled())
  {
    // No point to show this if no options available
    return wxID_OK;
  }
  SaveButton->SetFocus();
  return WXDialog::ShowModal();
}

void CookingOptionsWindow::OnSaveClicked(wxCommandEvent& event)
{
  EndModal(wxID_OK);
}

void CookingOptionsWindow::OnCancelClicked(wxCommandEvent& event)
{
  EndModal(wxID_CANCEL);
}
