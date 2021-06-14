#include "SpeedTreeEditor.h"

enum ExportMode {
  ExportSpt,
  ExportSptAndMaterials
};

void SpeedTreeEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (auto toolbarItem = toolbar->FindById(eID_Import))
  {
    toolbarItem->Enable(true);
  }
}

void SpeedTreeEditor::OnExportClicked(wxCommandEvent& e)
{
  wxMenu menu;
  menu.Append(ExportMode::ExportSptAndMaterials, wxT("Export SPT with embeded materials"));
  menu.Append(ExportMode::ExportSpt, wxT("Export unmodifed SPT"));

  void* sptData = nullptr;
  FILE_OFFSET sptDataSize = 0;
  switch (GetPopupMenuSelectionFromUser(menu))
  {
  case ExportMode::ExportSpt:
    if (!((USpeedTree*)Object)->GetSptData(&sptData, &sptDataSize, false) || !sptDataSize || !sptData)
    {
      wxMessageBox(wxT("The object appears to be empty!"), wxT("Error!"), wxICON_ERROR);
      return;
    }
    break;
  case ExportMode::ExportSptAndMaterials:
    if (!((USpeedTree*)Object)->GetSptData(&sptData, &sptDataSize, true) || !sptDataSize || !sptData)
    {
      wxMessageBox(wxT("The object appears to be empty!"), wxT("Error!"), wxICON_ERROR);
      return;
    }
    break;
  default:
    return;
  }

  wxString path = wxSaveFileSelector("SpeedTree", ".spt", Object->GetObjectNameString().WString(), this);
  if (path.empty())
  {
    free(sptData);
    return;
  }

  FWriteStream s(path.ToStdWstring());
  if (!s.IsGood())
  {
    wxMessageBox("Failed to create/open \"" + path + "\"", "Error!", wxICON_ERROR);
    free(sptData);
    return;
  }

  s.SerializeBytes(sptData, sptDataSize);
  free(sptData);
}

void SpeedTreeEditor::OnImportClicked(wxCommandEvent& e)
{
  wxString ext = wxT("SPT files (*.spt)|*.spt");
  wxString path = wxFileSelector("Import a SpeedTree", wxEmptyString, wxEmptyString, ext, ext, wxFD_OPEN, this);
  if (path.IsEmpty())
  {
    return;
  }
  FReadStream s(path.ToStdWstring());
  if (!s.IsGood())
  {
    wxMessageBox("Failed to open \"" + path + "\"", "Error!", wxICON_ERROR);
    return;
  }
  FILE_OFFSET size = s.GetSize();
  void* sptData = malloc(size);
  s.SerializeBytes(sptData, size);
  ((USpeedTree*)Object)->SetSptData(sptData, size);
  free(sptData);
}
