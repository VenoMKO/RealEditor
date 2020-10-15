#pragma once
#include "GenericEditor.h"
#include <Tera/UObjectRedirector.h>

class ObjectRedirectorEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  ObjectRedirectorEditor(wxPanel* parent, PackageWindow* window);

  void OnObjectLoaded() override;

  void PopulateToolBar(wxToolBar* toolbar) override;

  void OnToolBarEvent(wxCommandEvent& e) override;

private:
  UObjectRedirector* Redirector = nullptr;
  UObject* Source = nullptr;
  wxStaticText* ObjectPath = nullptr;
  wxPanel* Container = nullptr;
};