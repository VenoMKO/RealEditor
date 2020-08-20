#include "ObjectRedirectorEditor.h"

void ObjectRedirectorEditor::OnObjectLoaded()
{
  if (Loading)
  {
    Redirector = (UObjectRedirector*)Object;
    Object = Redirector->GetObject();
  }
  GenericEditor::OnObjectLoaded();
}