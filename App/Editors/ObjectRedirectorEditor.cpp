#include "ObjectRedirectorEditor.h"
#include "../App.h"
#include "../Windows/PackageWindow.h"
#include <Tera/UObject.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>

#include "../resource.h"

#define SHOW_REDIRECTOR_PATH 0

enum RedirectorTab : size_t {
  RedirectorSource = 0,
  RedirectorOriginal = 1,
  RedirectorError = 2,
  RedirectorNone = 3,
};

ObjectRedirectorEditor::ObjectRedirectorEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  MessageBook = new wxSimplebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
  wxPanel* m_panel1;
  m_panel1 = new wxPanel(MessageBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel5;
  m_panel5 = new wxPanel(m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxTAB_TRAVERSAL);
  m_panel5->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(m_panel5, wxID_ANY, wxT("This is a shortcut to an object!"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  m_staticText1->Wrap(-1);
  m_staticText1->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
  m_staticText1->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));

  bSizer3->Add(m_staticText1, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));

  wxStaticText* m_staticText2;
  m_staticText2 = new wxStaticText(m_panel5, wxID_ANY, wxT("The actual object is stored in a different GPK file.\nPress \"Source\" to open the GPK with the object."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  m_staticText2->Wrap(-1);
  m_staticText2->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
  m_staticText2->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));

  bSizer3->Add(m_staticText2, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));

  EditorSourceButon = new wxButton(m_panel5, eID_Composite, wxT("Source"), wxDefaultPosition, wxDefaultSize, 0);
  EditorSourceButon->SetBitmap(wxBitmap(MAKE_IDB(IDB_FORWARD), wxBITMAP_TYPE_PNG_RESOURCE), wxRIGHT);
  EditorSourceButon->SetToolTip("Open composite package containing this object...");
  bSizer3->Add(EditorSourceButon, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, FromDIP(5));
#if SHOW_REDIRECTOR_PATH
  PathLabelSource = new wxStaticText(m_panel5, wxID_ANY, wxT("Path: None"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxST_ELLIPSIZE_MIDDLE);
  PathLabelSource->Wrap(-1);
  PathLabelSource->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));

  bSizer3->Add(PathLabelSource, 0, wxALL | wxEXPAND, FromDIP(5));
  PathLabelSource->SetAutoLayout(true);
#endif

  m_panel5->SetSizer(bSizer3);
  m_panel5->Layout();
  bSizer3->Fit(m_panel5);
  bSizer2->Add(m_panel5, 1, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  m_panel1->SetSizer(bSizer2);
  m_panel1->Layout();
  bSizer2->Fit(m_panel1);
  MessageBook->AddPage(m_panel1, wxT("a page"), false);
  wxPanel* m_panel2;
  m_panel2 = new wxPanel(MessageBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer21;
  bSizer21 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel4;
  m_panel4 = new wxPanel(m_panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxTAB_TRAVERSAL);
  m_panel4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  wxBoxSizer* bSizer31;
  bSizer31 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText11;
  m_staticText11 = new wxStaticText(m_panel4, wxID_ANY, wxT("This is a shortcut to an object!"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  m_staticText11->Wrap(-1);
  m_staticText11->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
  m_staticText11->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));

  bSizer31->Add(m_staticText11, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));

  wxStaticText* m_staticText21;
  m_staticText21 = new wxStaticText(m_panel4, wxID_ANY, wxT("The actual object is stored in a different location.\nPress \"Original\" to select the object."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  m_staticText21->Wrap(-1);
  m_staticText21->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
  m_staticText21->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));

  bSizer31->Add(m_staticText21, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));

  EditorOriginButon = new wxButton(m_panel4, eID_Origin, wxT("Original"), wxDefaultPosition, wxDefaultSize, 0);
  EditorOriginButon->SetBitmap(wxBitmap(MAKE_IDB(IDB_FORWARD), wxBITMAP_TYPE_PNG_RESOURCE));
  EditorOriginButon->SetToolTip("Show original object...");
  bSizer31->Add(EditorOriginButon, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, FromDIP(5));
#if SHOW_REDIRECTOR_PATH
  PathLabelOriginal = new wxStaticText(m_panel4, wxID_ANY, wxT("Path: None"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxST_ELLIPSIZE_MIDDLE);
  PathLabelOriginal->Wrap(-1);
  PathLabelOriginal->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));

  bSizer31->Add(PathLabelOriginal, 0, wxALL | wxEXPAND, FromDIP(5));
  PathLabelOriginal->SetAutoLayout(true);
#endif

  m_panel4->SetSizer(bSizer31);
  m_panel4->Layout();
  bSizer31->Fit(m_panel4);
  bSizer21->Add(m_panel4, 1, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  m_panel2->SetSizer(bSizer21);
  m_panel2->Layout();
  bSizer21->Fit(m_panel2);
  MessageBook->AddPage(m_panel2, wxT("a page"), false);
  wxPanel* m_panel3;
  m_panel3 = new wxPanel(MessageBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer211;
  bSizer211 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel6;
  m_panel6 = new wxPanel(m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxTAB_TRAVERSAL);
  m_panel6->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  wxBoxSizer* bSizer311;
  bSizer311 = new wxBoxSizer(wxVERTICAL);

  ErrorTitle = new wxStaticText(m_panel6, wxID_ANY, wxT("Error!"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  ErrorTitle->Wrap(-1);
  ErrorTitle->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
  ErrorTitle->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));

  bSizer311->Add(ErrorTitle, 0, wxALL | wxEXPAND, FromDIP(5));

  ErrorDescription = new wxStaticText(m_panel6, wxID_ANY, wxT("This object is a shortcut that has no destination. "), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  ErrorDescription->Wrap(-1);
  ErrorDescription->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
  ErrorDescription->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));

  bSizer311->Add(ErrorDescription, 0, wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, FromDIP(5));


  m_panel6->SetSizer(bSizer311);
  m_panel6->Layout();
  bSizer311->Fit(m_panel6);
  bSizer211->Add(m_panel6, 1, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  m_panel3->SetSizer(bSizer211);
  m_panel3->Layout();
  bSizer211->Fit(m_panel3);
  MessageBook->AddPage(m_panel3, wxT("a page"), false);
  wxPanel* m_panel7;
  m_panel7 = new wxPanel(MessageBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  MessageBook->AddPage(m_panel7, wxT("a page"), false);

  bSizer1->Add(MessageBook, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, FromDIP(5));

  MessageBook->ChangeSelection(RedirectorTab::RedirectorNone);
  SetSizer(bSizer1);
  Layout();
  MessageBook->SetAutoLayout(true);
  EditorSourceButon->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectRedirectorEditor::OnEditorSourceClicked), NULL, this);
  EditorOriginButon->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectRedirectorEditor::OnEditorOriginalClicked), NULL, this);
}

void ObjectRedirectorEditor::OnObjectLoaded()
{
  if (Loading)
  {
    if (Redirector = (UObjectRedirector*)Object)
    {
      if (Redirector->GetObject())
      {
        Source = Redirector->GetObject();
      }
    }
  }
  else if (!Redirector && Object)
  {
    // Setup editor if the redirector was loaded externally
    Redirector = (UObjectRedirector*)Object;
    if (Redirector && Redirector->GetObject())
    {
      Source = Redirector->GetObject();
    }
  }

  std::string opath;
  if (Source)
  {
    opath = Source->GetObjectPath().UTF8();
    if (Source->GetPackage() != Window->GetPackage().get() && Source->GetPackage()->IsComposite())
    {
      MessageBook->ChangeSelection(RedirectorTab::RedirectorSource);
    }
    else
    {
      MessageBook->ChangeSelection(RedirectorTab::RedirectorOriginal);
    }
  }
  else if (Redirector)
  {
    PACKAGE_INDEX idx = Redirector->GetObjectRefIndex();
    if (idx < 0)
    {
      FObjectImport* imp = Redirector->GetPackage()->GetImportObject(idx);
      opath = imp->GetObjectPath().UTF8();
    }
    else if (idx > 0)
    {
      FObjectExport* exp = Redirector->GetPackage()->GetExportObject(idx);
      opath = exp->GetObjectPath().UTF8();
    }
    else
    {
      opath = "None";
#if SHOW_REDIRECTOR_PATH
      ErrorTitle->SetLabel(wxT("Error! The redirector is empty."));
      ErrorDescription->SetLabel(wxT("Game developers left this empty object."));
      MessageBook->ChangeSelection(RedirectorTab::RedirectorError);
      MessageBook->Layout();
#endif
    }
  }
  else
  {
    opath = "NULL";
#if SHOW_REDIRECTOR_PATH
    ErrorTitle->SetLabel(wxT("Error! The redirector is empty."));
    ErrorDescription->SetLabel(wxT("Failed to find the object or its GPK file."));
    MessageBook->ChangeSelection(RedirectorTab::RedirectorError);
    MessageBook->Layout();
#endif
  }

  std::replace(opath.begin(), opath.end(), '.', '\\');
  opath = "Path: " + opath;
#if SHOW_REDIRECTOR_PATH
  PathLabelSource->SetLabel(opath);
  PathLabelOriginal->SetLabel(opath);
#endif
  GenericEditor::OnObjectLoaded();
  Layout();
}

void ObjectRedirectorEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (auto item = toolbar->FindById(eID_Export))
  {
    item->Enable(false);
  }

  if (!toolbar->FindById(eID_Composite) && Source)
  {
    if (Source->GetPackage() != Window->GetPackage().get() && Source->GetPackage()->IsComposite())
    {
      CompositeObjectPath = Source->GetObjectPath().WString();
      if (CompositeObjectPath.size())
      {
        toolbar->AddTool(eID_Composite, "Source", wxBitmap(MAKE_IDB(IDB_FORWARD), wxBITMAP_TYPE_PNG_RESOURCE), "Open composite package containing this object...");
      }
    }
  }
  
  
  if (!toolbar->FindById(eID_Composite) && Source)
  {
    toolbar->AddTool(eID_Origin, "Original", wxBitmap(MAKE_IDB(IDB_FORWARD), wxBITMAP_TYPE_PNG_RESOURCE), "Show original object");
  }
}

void ObjectRedirectorEditor::OnToolBarEvent(wxCommandEvent& e)
{
  GenericEditor::OnToolBarEvent(e);
  if (e.GetSkipped())
  {
    // The base class has processed the event. Unmark the event and exit
    e.Skip(false);
    return;
  }
  auto eId = e.GetId();
  if (eId == eID_Origin)
  {
    if (Source->GetPackage() == Window->GetPackage().get())
    {
      Window->SelectObject(Source);
    }
    else
    {
      App::GetSharedApp()->OpenPackage(Source->GetPackage()->GetSourcePath().WString(), Source->GetObjectPath().WString());
    }
  }
}

void ObjectRedirectorEditor::OnEditorSourceClicked(wxCommandEvent& e)
{
  OnToolBarEvent(e);
}

void ObjectRedirectorEditor::OnEditorOriginalClicked(wxCommandEvent& e)
{
  OnToolBarEvent(e);
}
