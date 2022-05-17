#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>

#include <Tera/UMaterialFunction.h>
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

  void SetOutput(const std::vector<FExpressionInput>& input) override
  {
    MFOutputs = input;
  }

  void SetEditorPosition(int32 x, int32 y) override
  {
    Position = wxPoint(x, y);
  }

  void SetEditorSize(int32 x, int32 y) override
  {
    Size = wxSize(x, y);
  }

  void SetIsFinalNode(bool flag = true) override
  {
    IsFinal = flag;
  }

  wxString Title;
  wxString TextValue;
  wxString Description;
  std::vector<FExpressionInput> Inputs;
  wxPoint Position;
  wxSize Size;

  bool NeedsMultipleOutputs = false;
  wxPoint PosRGB;
  wxPoint PosR;
  wxPoint PosG;
  wxPoint PosB;
  wxPoint PosA;
  std::vector<wxPoint> InputsPositions;
  UMaterialExpression* Expression = nullptr;
  std::vector<FExpressionInput> MFOutputs;
  std::vector<wxPoint> MFOutputsPositions;
  bool IsFinal = false;
};

// Parent MUST be a wxScrolledWindow!.
class DragableCanvas : public wxPanel {
public:
  using wxPanel::wxPanel;

  void OnMouseDown(wxMouseEvent& e);
  void OnMouseUp(wxMouseEvent& e);
  void OnMouseMove(wxMouseEvent& e);

protected:
  bool IsDragging = false;
  wxPoint MouseStart;
  wxPoint CanvasStart;
  DECLARE_EVENT_TABLE();
};

class UDKMaterialGraph : public DragableCanvas {
public:
  UDKMaterialGraph(wxWindow* parent, UMaterial* material);
  UDKMaterialGraph(wxWindow* parent, UMaterialFunction* func);

  void Render(wxMemoryDC& dc);

protected:
  void OnPaint(wxPaintEvent& e);
  void OnEraseBg(wxEraseEvent&)
  {}

protected:
  wxString ObjectName;
  int32 CanvasOffsetX = 0;
  int32 CanvasOffsetY = 0;
  std::vector<MaterialExpressionInfo> GraphNodes;
  std::vector<FExpressionInput> MaterialInputs;
  bool NeedsPositionCalculation = true;
  std::map<UMaterialExpression*, size_t> ExpressionMap;
  bool DcError = false;
  DECLARE_EVENT_TABLE()
};