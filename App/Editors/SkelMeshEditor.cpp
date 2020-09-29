#include "SkelMeshEditor.h"
#include "../Windows/PackageWindow.h"

#include <Utils/FbxUtils.h>

enum ExportMode {
  ExportGeometry = wxID_HIGHEST + 1,
  ExportFull
};

void SkelMeshEditor::OnExportMenuClicked(wxCommandEvent& e)
{
  FbxExportContext ctx;
  ctx.ExportSkeleton = e.GetId() == ExportMode::ExportFull;
  wxString path = wxSaveFileSelector("mesh", wxT("FBX file|*.fbx"), Object->GetObjectName().WString(), Window);
  if (path.empty())
  {
    return;
  }
  ctx.Path = path.ToStdWstring();
  FbxUtils utils;
  if (!utils.ExportSkeletalMesh((USkeletalMesh*)Object, ctx))
  {
    wxMessageBox(ctx.Error, wxT("Error!"), wxICON_ERROR);
  }
}

void SkelMeshEditor::OnExportClicked(wxCommandEvent&)
{
  wxMenu menu;
  menu.Append(ExportMode::ExportFull, wxT("Export rigged geometry"));
  menu.Append(ExportMode::ExportGeometry, wxT("Export geometry"));
  menu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(SkelMeshEditor::OnExportMenuClicked), NULL, this);
  PopupMenu(&menu);
}