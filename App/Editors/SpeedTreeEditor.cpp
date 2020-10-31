#include "SpeedTreeEditor.h"

enum ExportMode {
  ExportSpt,
  ExportSptAndMaterials
};

void SpeedTreeEditor::OnExportClicked(wxCommandEvent& e)
{
  wxMenu menu;
  wxMenuItem* mitem = nullptr;
  bool hasData = Object->GetDataSize() > 0 && !Object->IsDirty();
  mitem = menu.Append(ExportMode::ExportSpt, wxT("Export SPT"));
  mitem = menu.Append(ExportMode::ExportSptAndMaterials, wxT("Export SPT with embeded materials"));
  mitem->Enable(false);

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

  wxString path = wxSaveFileSelector("SpeedTree", ".spt", Object->GetObjectName().WString(), this);
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
