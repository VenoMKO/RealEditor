#pragma once
#include "GenericEditor.h"

#include <Tera/UAnimation.h>

class AnimSetEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnExportClicked(wxCommandEvent& e) override;
};

class AnimSequenceEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void OnExportClicked(wxCommandEvent& e) override;
};