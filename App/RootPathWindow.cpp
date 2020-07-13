#include "RootPathWindow.h"
#include <wx/dir.h>

namespace 
{
  const wxString FrameTitle = "Aruns Hand";
  const wxString PanelTitle = "Provide the S1Game folder path";
  const wxString PanelDescription = "This path will be used to load dependencies. You may need to run Aruns Hand as an Administrator if Tera is in a priveleged folder (e.g. Program Files).";
  const wxString SelectTitle = "Select...";
  const wxString ContinueTitle = "Continue";
  enum ControlElementId {
    Path = wxID_HIGHEST + 1,
    Select,
    Continue
  };

  bool IsS1Folder(const wxString& path)
  {
    return path.EndsWith(wxFILE_SEP_PATH + wxString("S1Game")) || path.EndsWith(wxFILE_SEP_PATH + wxString("S1Game") + wxFILE_SEP_PATH);
  }
}

RootPathWindow::RootPathWindow()
  : wxDialog(nullptr, wxID_ANY, FrameTitle, wxPoint(20, 20), wxSize(535, 211), wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
  S1GamePath = wxEmptyString;
  wxPoint pos(20, 20);
  wxPanel* content = new wxPanel(this, wxID_ANY);
  wxStaticText* title = new wxStaticText(content, wxID_ANY, PanelTitle, pos);
  {
    wxFont font = title->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(font);
  }
  pos.y += title->GetSize().y + 5;
  wxStaticText* description = new wxStaticText(content, wxID_ANY, PanelDescription, pos, wxSize(500, 32));
  pos.y += description->GetSize().y + 15;
  PathTextfield = new wxTextCtrl(content, ControlElementId::Path, wxEmptyString, pos, wxSize(385, 20), wxTE_PROCESS_ENTER);
  PathTextfield->ShowNativeCaret(true);
  pos.x += PathTextfield->GetSize().x + 5;
  pos.y -= 1;
  SelectButton = new wxButton(content, ControlElementId::Select, SelectTitle, pos, wxSize(90, 22));
  pos.y += SelectButton->GetSize().y + 20;
  ContinueButton = new wxButton(content, ControlElementId::Continue, ContinueTitle, pos, wxSize(90, 24));
  SelectButton->SetFocus();
  ContinueButton->Enable(false);
  Center();
}

wxString RootPathWindow::GetRootPath() const
{
  return S1GamePath;
}

void RootPathWindow::OnPathChanged(wxCommandEvent& e)
{
  ContinueButton->Enable(IsS1Folder(PathTextfield->GetValue()));
}

void RootPathWindow::OnSelectClicked(wxCommandEvent& e)
{
  wxDirDialog *openPanel = new wxDirDialog(this, "Select S1Game folder", PathTextfield->GetLabelText());
  openPanel->Center();
  if (openPanel->ShowModal() == wxID_OK)
  {
    const wxString path = openPanel->GetPath();
    if (IsS1Folder(path))
    {
      PathTextfield->SetLabelText(path);
    }
    else
    {
      wxMessageBox(wxT("Folder must be called S1Game"), wxT("Error: Invalid path!"), wxICON_ERROR);
    }
  }
  openPanel->Destroy();
}

void RootPathWindow::OnContinueClicked(wxCommandEvent& e)
{
  if (!ContinueButton->IsEnabled())
  {
    return;
  }
  wxDir s1Dir(PathTextfield->GetValue());
  if (!s1Dir.IsOpened())
  {
    wxMessageBox(wxT("Make sure you entered a correct path and have enough privileges to access it."), wxT("Error: Failed to open S1Game folder!"), wxICON_ERROR);
    return;
  }
  S1GamePath = PathTextfield->GetValue();
  s1Dir.Close();
  Close();
}

wxBEGIN_EVENT_TABLE(RootPathWindow, wxDialog)
EVT_BUTTON(ControlElementId::Select, OnSelectClicked)
EVT_BUTTON(ControlElementId::Continue, OnContinueClicked)
EVT_TEXT(ControlElementId::Path, OnPathChanged)
EVT_TEXT_ENTER(ControlElementId::Path, OnContinueClicked)
wxEND_EVENT_TABLE()