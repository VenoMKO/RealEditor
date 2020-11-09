#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>

#include <Tera/UMaterialExpression.h>


class MaterialExpressionInfo : public UMaterialExpressionViewVisitor {
public:
	virtual ~MaterialExpressionInfo()
	{}

	void SetTitle(const FString& title) override
	{
		Title = title.WString();
	}

	void SetValue(const FString& value) override
	{
		TextValue = value.WString();
	}

	void SetDescription(const FString& desc) override
	{
		Description = desc.WString();
	}

	void SetInput(const std::vector<FExpressionInput>& input) override
	{
		Inputs = input;
	}

	void SetEditorPosition(int32 x, int32 y) override
	{
		Position = wxPoint(x, y);
	}

	void SetEditorSize(int32 x, int32 y) override
	{
		Size = wxSize(x, y);
	}

	wxString Title;
	wxString TextValue;
	wxString Description;
	std::vector<FExpressionInput> Inputs;
	wxPoint Position;
	wxSize Size;

	bool NeedsMultipleOutputs = false;
	wxPoint PosRGBA;
	wxPoint PosR;
	wxPoint PosG;
	wxPoint PosB;
	wxPoint PosA;
	std::vector<wxPoint> InputsPositions;
	UMaterialExpression* Expression = nullptr;
};

class UDKMaterialGraph : public wxPanel {
public:
	UDKMaterialGraph(wxWindow* parent, UMaterial* material);

protected:
	void OnPaint(wxPaintEvent& e);
	void OnEraseBg(wxEraseEvent&)
	{}

	void Render(wxBufferedPaintDC& dc);

protected:
	UMaterial* Material = nullptr;
	int32 CanvasOffsetX = 0;
	int32 CanvasOffsetY = 0;
	std::vector<MaterialExpressionInfo> GraphNodes;
	std::vector<FExpressionInput> MaterialInputs;
	bool NeedsPositionCalculation = true;
	std::map<UMaterialExpression*, size_t> ExpressionMap;
	DECLARE_EVENT_TABLE()
};