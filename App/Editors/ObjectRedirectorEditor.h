#pragma once
#include "GenericEditor.h"
#include <Tera/UObjectRedirector.h>

class ObjectRedirectorEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;

  void OnObjectLoaded() override;

  void PopulateToolBar(wxToolBar* toolbar) override;
private:
  UObjectRedirector* Redirector = nullptr;
};