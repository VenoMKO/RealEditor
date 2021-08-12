#pragma once

#include "GenericEditor.h"
#include <Utils/ALDevice.h>
#include <Tera/USoundNode.h>

class SoundCueEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void OnExportClicked(wxCommandEvent&) override;
};