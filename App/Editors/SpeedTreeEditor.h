#pragma once
#include "GenericEditor.h"
#include <Tera/USpeedTree.h>

class SpeedTreeEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnExportClicked(wxCommandEvent& e) override;
  void OnImportClicked(wxCommandEvent& e) override;
};