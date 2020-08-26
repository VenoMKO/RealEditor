#include "ObjectRedirectorEditor.h"

void ObjectRedirectorEditor::OnObjectLoaded()
{
  if (Loading)
  {
    if (((UObjectRedirector*)Object)->GetObject())
    {
      Redirector = (UObjectRedirector*)Object;
      Object = Redirector->GetObject();
    }
  }
  GenericEditor::OnObjectLoaded();
}