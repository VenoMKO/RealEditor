#include "PackageWindow.h"

#include <Tera/ALog.h>
#include <Tera/UObject.h>

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
  if (!Object->IsLoaded())
  {
    // TODO: Load in background
    PERF_START(UI_LOAD_OBJECT);
    Object->Load();
    PERF_END(UI_LOAD_OBJECT);
  }
}
