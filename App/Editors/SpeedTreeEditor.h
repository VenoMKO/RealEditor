#pragma once
#include "GenericEditor.h"
#include <Tera/USpeedTree.h>

class SpeedTreeEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void OnExportClicked(wxCommandEvent& e) override;
};