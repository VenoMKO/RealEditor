#include "RootPathWindow.h"
#include <wx/dir.h>
#include <Tera/Core.h>

namespace 
{
  const wxString RoorDir = wxS("S1Game");
  const wxString PanelTitle = wxS("Provide the \"") + RoorDir + wxS("\" folder path");
  const wxString PanelDescription = wxS("This path will be used to load dependencies. You may need to run the program as an Administrator if Tera is in a priveleged folder (e.g. Program Files).");
  const wxString SelectTitle = wxS("Select...");
  const wxString ContinueTitle = wxS("Continue");
  enum ControlElementId {
    Path = wxID_HIGHEST + 1,
    Select,
    Continue
  };

  bool IsS1Folder(const wxString& path)
  {
    return path.EndsWith(wxFILE_SEP_PATH + RoorDir) || path.EndsWith(wxFILE_SEP_PATH + RoorDir + wxFILE_SEP_PATH);
  }
}

RootPathWindow::RootPathWindow(const std::string& path)
  : wxDialog(nullptr, wxID_ANY, wxTheApp->GetAppDisplayName(), wxPoint(20, 20), wxSize(535, 211), wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
  S1CookedPC = path.size() ? A2W(path) : wxEmptyString;
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
  PathTextfield = new wxTextCtrl(content, ControlElementId::Path, S1CookedPC, pos, wxSize(385, 20), wxTE_PROCESS_ENTER);
  PathTextfield->ShowNativeCaret(true);
  pos.x += PathTextfield->GetSize().x + 5;
  pos.y -= 1;
  SelectButton = new wxButton(content, ControlElementId::Select, SelectTitle, pos, wxSize(90, 22));
  pos.y += SelectButton->GetSize().y + 20;
  ContinueButton = new wxButton(content, ControlElementId::Continue, ContinueTitle, pos, wxSize(90, 24));
  SelectButton->SetFocus();
  ContinueButton->Enable(IsS1Folder(S1CookedPC));
  Center();
}

wxString RootPathWindow::GetRootPath() const
{
  return S1CookedPC;
}

void RootPathWindow::OnPathChanged(wxCommandEvent& e)
{
  ContinueButton->Enable(IsS1Folder(PathTextfield->GetValue()));
}

void RootPathWindow::OnSelectClicked(wxCommandEvent& e)
{
  wxDirDialog *openPanel = new wxDirDialog(this, wxS("Select \"") + RoorDir + wxS("\" folder"), PathTextfield->GetLabelText());
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
      wxMessageBox(wxS("Folder must be called \"") + RoorDir + wxS("\""), wxS("Error: Invalid path!"), wxICON_ERROR);
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
    wxMessageBox(wxS("Make sure you entered a correct path and have enough privileges to access it."), wxS("Error: Failed to open \"") + RoorDir + wxS("\" folder!"), wxICON_ERROR);
    return;
  }
  ContinuePressed = true;
  S1CookedPC = PathTextfield->GetValue();
  s1Dir.Close();
  Close();
}

void RootPathWindow::OnClose(wxCloseEvent& e)
{
  if (!ContinuePressed)
  {
    S1CookedPC = wxEmptyString;
  }
  e.Skip();
}

wxBEGIN_EVENT_TABLE(RootPathWindow, wxDialog)
EVT_BUTTON(ControlElementId::Select, RootPathWindow::OnSelectClicked)
EVT_BUTTON(ControlElementId::Continue, RootPathWindow::OnContinueClicked)
EVT_TEXT(ControlElementId::Path, RootPathWindow::OnPathChanged)
EVT_TEXT_ENTER(ControlElementId::Path, RootPathWindow::OnContinueClicked)
EVT_CLOSE(OnClose)
wxEND_EVENT_TABLE()