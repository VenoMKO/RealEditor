#pragma once
#include "GenericEditor.h"
#include <Tera/UClass.h>

#include <wx/stc/stc.h>

class wxNotebook;
class wxPropertyGridManager;
class ClassEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  ClassEditor(wxPanel* parent, PackageWindow* window);

  void OnObjectLoaded() override;

protected:
  void LoadClassProperties();

protected:
  wxNotebook* Notebook = nullptr;
  wxStaticText* SuperField = nullptr;
  wxStaticText* FlagsField = nullptr;
  wxStaticText* HeaderField = nullptr;
  wxStaticText* DLLField = nullptr;
  wxPropertyGridManager* PropertiesList = nullptr;
  wxStyledTextCtrl* CppView = nullptr;
  wxStyledTextCtrl* ScriptView = nullptr;

  bool NeedsSetup = true;
};