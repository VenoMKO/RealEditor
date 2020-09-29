#include "SkelMeshEditor.h"
#include "../Windows/PackageWindow.h"

#include <Utils/FbxUtils.h>

void SkelMeshEditor::OnExportClicked(wxCommandEvent&)
{
  wxString path = wxSaveFileSelector("mesh", wxT("FBX file|*.fbx"), Object->GetObjectName().WString(), Window);
  if (path.empty())
  {
    return;
  }
  FbxExportContext ctx;
  ctx.Path = path.ToStdWstring();
  FbxUtils utils;
  if (!utils.ExportSkeletalMesh((USkeletalMesh*)Object, ctx))
  {
    wxMessageBox(ctx.Error, wxT("Error!"), wxICON_ERROR);
  }
}