#pragma once
#include "GenericEditor.h"

#include <wx/propgrid/manager.h>
#include <Tera/UMaterial.h>

class MaterialInstanceEditor : public GenericEditor {
public:
	MaterialInstanceEditor(wxPanel* parent, PackageWindow* window);

	void OnObjectLoaded() override;

protected:
	bool NeedsGraph = true;
	wxScrolledWindow* Canvas = nullptr;
	wxPropertyGridManager* StaticParameterOverrides = nullptr;
	int32 CanvasOffsetX = 0;
	int32 CanvasOffsetY = 0;
};