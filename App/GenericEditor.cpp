#include "PackageWindow.h"

GenericEditor* GenericEditor::CreateEditor(wxPanel* parent, UObject* object)
{
  GenericEditor* editor = new GenericEditor(parent);
  editor->Object = object;
  return editor;
}

GenericEditor::GenericEditor(wxPanel* parent)
  : wxPanel(parent, wxID_ANY)
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
}

void GenericEditor::LoadObject()
{
}
