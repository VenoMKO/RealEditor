#include "StaticMeshActorEditor.h"

#include <Tera/UActor.h>
#include <Tera/UStaticMesh.h>

void StaticMeshActorEditor::OnObjectLoaded()
{
  if (Loading || !Mesh)
  {
    if (((UStaticMeshActor*)Object)->StaticMeshComponent)
    {
      Mesh = ((UStaticMeshActor*)Object)->StaticMeshComponent->StaticMesh;
    }
    CreateRenderModel();
  }
  GenericEditor::OnObjectLoaded();
}
