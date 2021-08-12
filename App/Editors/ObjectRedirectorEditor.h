#pragma once
#include "GenericEditor.h"
#include <wx/simplebook.h>
#include <Tera/UObjectRedirector.h>

class ObjectRedirectorEditor : public GenericEditor {
public:
  using GenericEditor::GenericEditor;
  ObjectRedirectorEditor(wxPanel* parent, PackageWindow* window);

  void OnObjectLoaded() override;

  void PopulateToolBar(wxToolBar* toolbar) override;

  void OnToolBarEvent(wxCommandEvent& e) override;

  void OnEditorSourceClicked(wxCommandEvent& e);

  void OnEditorOriginalClicked(wxCommandEvent& e);

private:
  UObjectRedirector* Redirector = nullptr;
  UObject* Source = nullptr;
  wxPanel* Container = nullptr;
  wxSimplebook* MessageBook = nullptr;
  wxButton* EditorSourceButon = nullptr;
  wxStaticText* PathLabelSource = nullptr;
  wxButton* EditorOriginButon = nullptr;
  wxStaticText* PathLabelOriginal = nullptr;
  wxStaticText* ErrorTitle = nullptr;
  wxStaticText* ErrorDescription = nullptr;
};