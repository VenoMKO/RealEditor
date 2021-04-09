#include "ObjectRedirectorEditor.h"
#include "../App.h"
#include "../Windows/PackageWindow.h"
#include <Tera/UObject.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>

ObjectRedirectorEditor::ObjectRedirectorEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  wxBoxSizer* sizer;
  sizer = new wxBoxSizer(wxVERTICAL);

  Container = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  sizer->Add(Container, 1, wxEXPAND | wxALL, 5);

  wxPanel* m_panel2;
  m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  m_panel2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(m_panel2, wxID_ANY, wxT("Original:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText1->Wrap(-1);
  bSizer3->Add(m_staticText1, 0, wxALL, 5);

  ObjectPath = new wxStaticText(m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  ObjectPath->Wrap(-1);
  bSizer3->Add(ObjectPath, 0, wxALL, 5);


  m_panel2->SetSizer(bSizer3);
  m_panel2->Layout();
  bSizer3->Fit(m_panel2);
  sizer->Add(m_panel2, 0, wxALL | wxEXPAND, 0);

  SetSizer(sizer);
  Layout();
  sizer->Fit(this);

  Centre(wxBOTH);
}

void ObjectRedirectorEditor::OnObjectLoaded()
{
  if (Loading)
  {
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
    }
  }
  else
  {
    opath = "NULL";
  }

  std::replace(opath.begin(), opath.end(), '.', '\\');
  ObjectPath->SetLabel(opath);
  
  GenericEditor::OnObjectLoaded();
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
        toolbar->AddTool(eID_Composite, "Source", wxBitmap("#116", wxBITMAP_TYPE_PNG_RESOURCE), "Open composite package containing this object...");
      }
    }
  }
  
  
  if (!toolbar->FindById(eID_Composite) && Source)
  {
    toolbar->AddTool(eID_Origin, "Original", wxBitmap("#116", wxBITMAP_TYPE_PNG_RESOURCE), "Show original object");
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
