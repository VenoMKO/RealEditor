#include "ObjectRedirectorEditor.h"
#include "../Windows/PackageWindow.h"
#include <Tera/UObject.h>
#include <Tera/FPackage.h>

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

void ObjectRedirectorEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (Object && Object->GetPackage() != Window->GetPackage().get() && Object->GetPackage()->IsComposite())
  {
    CompositeObjectPath = Object->GetObjectPath().WString();
    if (CompositeObjectPath.size())
    {
      toolbar->AddTool(eID_Composite, "Source", wxBitmap("#116", wxBITMAP_TYPE_PNG_RESOURCE), "Open composite package containig this object...");
    }
  }
}
