#include "CompositePackagePicker.h"
#include "../App.h"

#include <Tera/FPackage.h>

DEFINE_EVENT_TYPE(FINISHED_POPULATING);

enum ControlElementId {
  TextField = wxID_HIGHEST + 1,
};

CompositePackagePicker::CompositePackagePicker(wxWindow* parent, const wxString& title, bool filePackages)
  : WXDialog(parent, wxID_ANY, title)
{
#if IS_TERA_BUILD
  FilePackages = filePackages;
#else
  FilePackages = true;
#endif
  SetSize(FromDIP(445), FromDIP(144));
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxPanel* m_panel1;
  m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText = new wxStaticText(m_panel1, wxID_ANY, wxT("Package name: "), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText->Wrap(-1);
  bSizer2->Add(m_staticText, 0, wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));

  CompositeName = new wxTextCtrl(m_panel1, ControlElementId::TextField, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  if (FilePackages)
  {
    wxArrayString complete;
    for (const auto& name : FPackage::FilePackageNames)
    {
      complete.push_back(name.WString());
    }
    CompositeName->AutoComplete(complete);
  }
  else
  {
    CompositeName->AutoComplete(App::GetSharedApp()->GetCompositePackageNames());
  }
  bSizer2->Add(CompositeName, 1, wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));

  m_panel1->SetSizer(bSizer2);
  m_panel1->Layout();
  bSizer2->Fit(m_panel1);
  bSizer1->Add(m_panel1, 1, wxEXPAND | wxALL, FromDIP(5));

  wxPanel* m_panel2;
  m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxHORIZONTAL);

  OpenButton = new wxButton(m_panel2, wxID_OK, wxT("Open"), wxDefaultPosition, wxDefaultSize, 0);
  OpenButton->Enable(false);
  bSizer3->Add(OpenButton, 0, wxALL, FromDIP(5));

  CancelButton = new wxButton(m_panel2, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(CancelButton, 0, wxALL, FromDIP(5));

  m_panel2->SetSizer(bSizer3);
  m_panel2->Layout();
  bSizer3->Fit(m_panel2);
  bSizer1->Add(m_panel2, 0, wxALIGN_RIGHT | wxALL, FromDIP(5));


  this->SetSizer(bSizer1);
  this->Layout();

  this->Centre(wxBOTH);
}

wxString CompositePackagePicker::GetResult() const
{
  return CompositeName->GetValue();
}

void CompositePackagePicker::OnText(wxCommandEvent&)
{
  if (FilePackages)
  {
    OpenButton->Enable(CompositeName->GetValue().size());
  }
  else
  {
    OpenButton->Enable(((App*)wxTheApp)->GetCompositePackageNames().Index(CompositeName->GetValue(), false) != wxNOT_FOUND);
  }
}

void CompositePackagePicker::OnTextEnter(wxCommandEvent&)
{
  if (OpenButton->IsEnabled())
  {
    EndModal(wxID_OK);
  }
}

wxBEGIN_EVENT_TABLE(CompositePackagePicker, wxDialog)
EVT_TEXT(ControlElementId::TextField, CompositePackagePicker::OnText)
EVT_TEXT_ENTER(ControlElementId::TextField, CompositePackagePicker::OnTextEnter)
wxEND_EVENT_TABLE()
