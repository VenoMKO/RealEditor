#pragma once
#include "GenericEditor.h"
#include <Tera/USkeletalMesh.h>

class SkelMeshEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void OnExportClicked(wxCommandEvent&) override;
};