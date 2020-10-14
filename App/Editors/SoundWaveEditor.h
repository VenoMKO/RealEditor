#pragma once

#include "GenericEditor.h"
#include <Tera/USoundNode.h>

class SoundWaveEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void OnExportClicked(wxCommandEvent&) override;
};