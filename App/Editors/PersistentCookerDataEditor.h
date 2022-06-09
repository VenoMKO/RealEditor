#pragma once
#include "GenericEditor.h"

#include <Tera/UPersistentCookerData.h>

class PersistentCookerDataEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void LoadObject() override;
  void OnObjectLoaded() override;
  void OnExportClicked(wxCommandEvent& e) override;

protected:
  class ProgressWindow* LoadingProgress = nullptr;
};