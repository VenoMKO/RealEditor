#pragma once
#include "GenericEditor.h"

#include <wx/scrolwin.h>

#include <Tera/UMaterialFunction.h>

class MaterialFunctionEditor : public GenericEditor {
public:
  MaterialFunctionEditor(wxPanel* parent, PackageWindow* window);

  void OnObjectLoaded() override;
  void PopulateToolBar(wxToolBar* toolbar) override;
  void OnExportClicked(wxCommandEvent&) override;

protected:
  void BuildGraph();

protected:
  bool NeedsGraph = true;
  wxScrolledWindow* Canvas = nullptr;
  class UDKMaterialGraph* Graph = nullptr;
  int32 CanvasOffsetX = 0;
  int32 CanvasOffsetY = 0;
};