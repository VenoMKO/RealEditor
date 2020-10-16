#pragma once

#include "GenericEditor.h"
#include <Tera/USoundNode.h>

class SoundWaveEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnExportClicked(wxCommandEvent&) override;
  void OnImportClicked(wxCommandEvent&) override;
};