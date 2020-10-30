#pragma once
#include "GenericEditor.h"
#include "StaticMeshEditor.h"

class StaticMeshActorEditor : public StaticMeshEditor {
public:
  using StaticMeshEditor::StaticMeshEditor;

  void OnObjectLoaded() override;
};