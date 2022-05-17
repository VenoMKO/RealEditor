#include "MaterialView.h"

#include <wx/scrolwin.h>
#include <wx/graphics.h>

#include <Tera/Cast.h>

#include <sstream>

#include "../resource.h"

#include "../Windows/REDialogs.h"

const int32 CanvasPadding = 200;


#define MAKE_IDB(name) "#" wxSTRINGIZE(## name)

UDKMaterialGraph::UDKMaterialGraph(wxWindow* parent, UMaterial* material)
  : DragableCanvas(parent)
  , ObjectName(material->GetObjectNameString().String())
{
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  for (FPropertyTag* tag : material->MaterialInputs)
  {
    FExpressionInput& in = MaterialInputs.emplace_back(tag);
    in.Title = tag->Name.String();
  }
  auto expressions = material->GetExpressions();

  if (expressions.empty())
  {
    SetSize(FromDIP(500), FromDIP(500));
    return;
  }

  int32 minX = INT_MAX;
  int32 minY = INT_MAX;
  int32 maxX = INT_MIN;
  int32 maxY = INT_MIN;
  for (UMaterialExpression* exp : expressions)
  {
    if (exp->GetPosX() < minX)
    {
      minX = exp->GetPosX();
    }
    if (exp->GetPosX() > maxX)
    {
      maxX = exp->GetPosX();
    }
    if (exp->GetPosY() < minY)
    {
      minY = exp->GetPosY();
    }
    if (exp->GetPosY() > maxY)
    {
      maxY = exp->GetPosY();
    }
  }

  if (minX == maxX || minY == maxY)
  {
    SetSize(FromDIP(500), FromDIP(500));
  }

  int canvasWidth = FromDIP(abs(minX) + abs(maxX) + (CanvasPadding * 2));
  int canvasHeight = FromDIP(abs(minY) + abs(maxY) + (CanvasPadding * 2));

  if (!canvasWidth)
  {
    canvasWidth = FromDIP(500);
  }
  if (!canvasHeight)
  {
    canvasHeight = FromDIP(500);
  }

  parent->SetVirtualSize(canvasWidth, canvasHeight);

  if (minX < 0)
  {
    CanvasOffsetX = FromDIP(abs(minX));
  }
  CanvasOffsetX += FromDIP(CanvasPadding);
  if (minY < 0)
  {
    CanvasOffsetY = FromDIP(abs(minY));
  }
  CanvasOffsetY += FromDIP(CanvasPadding);

  for (UMaterialExpression* exp : expressions)
  {
    ExpressionMap[exp] = GraphNodes.size();
    MaterialExpressionInfo& i = GraphNodes.emplace_back();
    exp->AcceptVisitor(i);
    i.Expression = exp;
    i.Position.x = FromDIP(i.Position.x) + CanvasOffsetX;
    i.Position.y = FromDIP(i.Position.y) + CanvasOffsetY;
    if (!i.Size.x || !i.Size.y)
    {
      i.Size = FromDIP(wxSize(80, 94));
    }
  }

  for (MaterialExpressionInfo& test : GraphNodes)
  {
    for (MaterialExpressionInfo& info : GraphNodes)
    {
      if (&test != &info)
      {
        for (const FExpressionInput& in : info.Inputs)
        {
          if (in.Expression == test.Expression && in.Mask)
          {
            test.NeedsMultipleOutputs = true;
            break;
          }
        }
      }
      if (!test.NeedsMultipleOutputs)
      {
        for (const FExpressionInput& in : MaterialInputs)
        {
          if (in.Expression == test.Expression && in.Mask)
          {
            test.NeedsMultipleOutputs = true;
            break;
          }
        }
      }
    }
  }

  SetSize(canvasWidth, canvasHeight);
  ((wxScrolledWindow*)parent)->Scroll(CanvasOffsetX / 50, CanvasOffsetY / 50);
}

UDKMaterialGraph::UDKMaterialGraph(wxWindow* parent, UMaterialFunction* func)
  : DragableCanvas(parent)
{
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  auto expressions = func->GetExpressions();

  if (expressions.empty())
  {
    SetSize(FromDIP(500), FromDIP(500));
    return;
  }

  int32 minX = INT_MAX;
  int32 minY = INT_MAX;
  int32 maxX = INT_MIN;
  int32 maxY = INT_MIN;
  for (UMaterialExpression* exp : expressions)
  {
    if (exp->GetPosX() < minX)
    {
      minX = exp->GetPosX();
    }
    if (exp->GetPosX() > maxX)
    {
      maxX = exp->GetPosX();
    }
    if (exp->GetPosY() < minY)
    {
      minY = exp->GetPosY();
    }
    if (exp->GetPosY() > maxY)
    {
      maxY = exp->GetPosY();
    }
  }

  if (minX == maxX || minY == maxY)
  {
    SetSize(FromDIP(500), FromDIP(500));
  }

  int canvasWidth = FromDIP(abs(minX) + abs(maxX) + (CanvasPadding * 2));
  int canvasHeight = FromDIP(abs(minY) + abs(maxY) + (CanvasPadding * 2));

  if (!canvasWidth)
  {
    canvasWidth = FromDIP(500);
  }
  if (!canvasHeight)
  {
    canvasHeight = FromDIP(500);
  }

  parent->SetVirtualSize(canvasWidth, canvasHeight);

  if (minX < 0)
  {
    CanvasOffsetX = FromDIP(abs(minX));
  }
  CanvasOffsetX += FromDIP(CanvasPadding);
  if (minY < 0)
  {
    CanvasOffsetY = FromDIP(abs(minY));
  }
  CanvasOffsetY += FromDIP(CanvasPadding);

  for (UMaterialExpression* exp : expressions)
  {
    ExpressionMap[exp] = GraphNodes.size();
    MaterialExpressionInfo& i = GraphNodes.emplace_back();
    exp->AcceptVisitor(i);
    i.Expression = exp;
    i.Position.x = FromDIP(i.Position.x) + CanvasOffsetX;
    i.Position.y = FromDIP(i.Position.y) + CanvasOffsetY;
    if (!i.Size.x || !i.Size.y)
    {
      i.Size = FromDIP(wxSize(80, 94));
    }
  }

  for (MaterialExpressionInfo& test : GraphNodes)
  {
    for (MaterialExpressionInfo& info : GraphNodes)
    {
      if (&test != &info)
      {
        for (const FExpressionInput& in : info.Inputs)
        {
          if (in.Expression == test.Expression && in.Mask)
          {
            test.NeedsMultipleOutputs = true;
            break;
          }
        }
      }
      if (!test.NeedsMultipleOutputs)
      {
        for (const FExpressionInput& in : MaterialInputs)
        {
          if (in.Expression == test.Expression && in.Mask)
          {
            test.NeedsMultipleOutputs = true;
            break;
          }
        }
      }
    }
  }

  SetSize(canvasWidth, canvasHeight);
  ((wxScrolledWindow*)parent)->Scroll(CanvasOffsetX / 50, CanvasOffsetY / 50);
}

void UDKMaterialGraph::OnPaint(wxPaintEvent& e)
{
  wxAutoBufferedPaintDC dc(this);
  if (dc.GetSelectedBitmap().IsOk())
  {
    Render(dc);
  }
  else if (!DcError)
  {
    DcError = true;
    REDialog::Error("Failed to create a device context to render the graph.\nTry to reduce Scale in Windows' \"Display settings\".");
  }
}

void UDKMaterialGraph::Render(wxMemoryDC& dc)
{
  wxBrush panelBrush = dc.GetBackground();
  dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)));
  dc.Clear();
  dc.SetBrush(dc.GetBackground());

  auto drawLabelFunc = [&](wxString text, const wxPoint& at, unsigned maxWidth = 0) {
    wxSize size = dc.GetTextExtent(text);
    bool truncate = false;
    while (maxWidth && (unsigned)size.x > maxWidth && text.size() > 3)
    {
      text = text.substr(0, text.size() - 3);
      size = dc.GetTextExtent(text);
      truncate = true;
    }
    if (truncate)
    {
      text += "...";
    }
    dc.DrawText(text, at);
  };

  auto drawExpressionCaption = [&](const MaterialExpressionInfo& i) {
    if (i.Title.StartsWith("Comment"))
    {
      wxPoint at = i.Position;
      wxSize size = dc.GetTextExtent(i.TextValue);
      at.y -= size.y;
      drawLabelFunc(i.TextValue, at, i.Size.x);
    }
    else
    {
      wxPoint at = i.Position;
      at.x += FromDIP(2);
      drawLabelFunc(i.Title, at, i.Size.x - FromDIP(2));
    }
  };

  const int outputNodeHeight = FromDIP(7);

  if (NeedsPositionCalculation)
  {
    NeedsPositionCalculation = false;
    for (MaterialExpressionInfo& i : GraphNodes)
    {
      int width = i.Size.x + FromDIP(4);
      wxSize titleExtent = dc.GetTextExtent(i.Title);
      if (titleExtent.x > width)
      {
        i.Size.x = titleExtent.x + FromDIP(4);
      }

      int posY = titleExtent.y + i.Position.y + FromDIP(14);
      int posX = i.Size.x + i.Position.x;

      if (i.TextValue.size())
      {
        std::istringstream stream(i.TextValue.ToStdString());
        std::string line;
        while (getline(stream, line))
        {
          wxSize ext = dc.GetTextExtent(line);
          if (ext.x >= width)
          {
            width = ext.x + FromDIP(4);
          }
          posY += FromDIP(14);
        }
        posY += FromDIP(4);
      }
      if (i.MFOutputs.size())
      {
        int tmaxX = 0;
        for (const auto& o : i.MFOutputs)
        {
          wxSize ext = dc.GetTextExtent(o.InputName.String());
          if (ext.x >= tmaxX)
          {
            tmaxX = ext.x + FromDIP(4);
          }
        }
        int tmaxX2 = 0;
        for (const auto& i : i.Inputs)
        {
          wxSize ext = dc.GetTextExtent(i.InputName.String());
          if (ext.x >= tmaxX2)
          {
            tmaxX2 = ext.x + FromDIP(4);
          }
        }
        width = std::max(tmaxX + tmaxX2, width);
      }

      if (i.Size.x < width)
      {
        i.Size.x = width + FromDIP(4);
        posX = i.Size.x + i.Position.x;
      }

      for (FExpressionInput& input : i.Inputs)
      {
        i.InputsPositions.emplace_back(wxPoint(posX, posY));
        posY += outputNodeHeight * 2;
      }

      posY = titleExtent.y + i.Position.y + FromDIP(6);
      posX = i.Position.x - FromDIP(10);

      if (i.MFOutputs.size())
      {
        for (const auto& o : i.MFOutputs)
        {
          i.MFOutputsPositions.emplace_back(posX, posY);
          posY += outputNodeHeight * 2;
        }
      }
      else
      {
        i.PosRGB.x = posX;
        i.PosRGB.y = posY;

        posY += outputNodeHeight * 2;

        i.PosR.x = posX;
        i.PosR.y = posY;

        posY += outputNodeHeight * 2;

        i.PosG.x = posX;
        i.PosG.y = posY;

        posY += outputNodeHeight * 2;

        i.PosB.x = posX;
        i.PosB.y = posY;

        posY += outputNodeHeight * 2;

        i.PosA.x = posX;
        i.PosA.y = posY;
      }
    }
  }

  // Draw comments
  for (MaterialExpressionInfo& i : GraphNodes)
  {
    if (i.Title.StartsWith("Comment"))
    {
      dc.DrawRectangle(i.Position, i.Size);
      wxPoint at = i.Position;
      wxSize size = dc.GetTextExtent(i.TextValue);
      at.y -= size.y;
      drawLabelFunc(i.TextValue, at, i.Size.x);
    }
  }

  dc.SetBrush(panelBrush);

  wxPen defaultPen = dc.GetPen();

  // Draw expressions
  for (MaterialExpressionInfo& i : GraphNodes)
  {
    if (i.Title.StartsWith("Comment"))
    {
      continue;
    }

    if (i.IsFinal)
    {
      dc.SetPen(wxPen(wxColor(0, 0, 0), 2));
      dc.SetBrush(wxBrush(wxColor(120, 120, 120)));
    }
    else
    {
      dc.SetBrush(panelBrush);
      dc.SetPen(defaultPen);
    }

    wxSize titleExtent = dc.GetTextExtent(i.Title);

    dc.DrawRectangle(i.Position, wxSize(i.Size.x, titleExtent.y));
    dc.DrawRectangle(wxPoint(i.Position.x, i.Position.y + FromDIP(2) + titleExtent.y), wxSize(i.Size.x, i.Size.y - titleExtent.y - FromDIP(2)));
    drawExpressionCaption(i);

    int posY = titleExtent.y + i.Position.y + FromDIP(6);
    int posX = i.Position.x - FromDIP(10);
    int inPosY = posY;

    if (i.TextValue.size())
    {
      std::istringstream stream(i.TextValue.ToStdString());
      std::string line;
      while (getline(stream, line))
      {
        drawLabelFunc(line, wxPoint(posX + FromDIP(12), inPosY), i.Size.x);
        inPosY += FromDIP(14);
      }
      inPosY += FromDIP(4);
    }
    
    dc.SetPen(wxPen(wxColor(), 0, wxPENSTYLE_TRANSPARENT));
    // Outputs
    
    
    if (i.MFOutputs.size())
    {
      dc.SetBrush(wxBrush(wxColor(0, 0, 0)));
      for (const auto& o : i.MFOutputs)
      {
        dc.DrawRectangle(wxPoint(posX, posY), wxSize(FromDIP(10), outputNodeHeight));
        drawLabelFunc(o.InputName.String(), wxPoint(posX + FromDIP(12), posY - outputNodeHeight), i.Size.x);
        posY += outputNodeHeight * 2;
      }
    }
    else if (!i.IsFinal)
    {
      // Composite
      dc.SetBrush(wxBrush(wxColor(0, 0, 0)));
      dc.DrawRectangle(wxPoint(posX, posY), wxSize(FromDIP(10), outputNodeHeight));

      if (i.NeedsMultipleOutputs)
      {
        // Red
        posY += outputNodeHeight * 2;
        dc.SetBrush(wxBrush(wxColor(255, 0, 0)));
        dc.DrawRectangle(wxPoint(posX, posY), wxSize(FromDIP(10), outputNodeHeight));

        // Green
        posY += outputNodeHeight * 2;
        dc.SetBrush(wxBrush(wxColor(0, 255, 0)));
        dc.DrawRectangle(wxPoint(posX, posY), wxSize(FromDIP(10), outputNodeHeight));

        // Blue
        posY += outputNodeHeight * 2;
        dc.SetBrush(wxBrush(wxColor(0, 0, 255)));
        dc.DrawRectangle(wxPoint(posX, posY), wxSize(FromDIP(10), outputNodeHeight));

        // Alpha
        posY += outputNodeHeight * 2;
        dc.SetBrush(wxBrush(wxColor(255, 255, 255)));
        dc.DrawRectangle(wxPoint(posX, posY), wxSize(FromDIP(10), outputNodeHeight));
      }
    }

    // Inputs
    posY = inPosY;
    posX = i.Size.x + i.Position.x;

    for (FExpressionInput& input : i.Inputs)
    {
      dc.SetBrush(wxBrush(wxColor(0, 0, 0)));
      wxPoint inputTriangle[4] = { {posX, posY + int(outputNodeHeight * .6)}, {posX, posY + int(outputNodeHeight * .45)}, {posX + FromDIP(10), posY}, {posX + FromDIP(10), posY + outputNodeHeight} };
      dc.DrawPolygon(4, inputTriangle);
      wxString title = input.GetDescription().WString();
      wxSize ext = dc.GetTextExtent(title);
      drawLabelFunc(title, wxPoint(posX - ext.x - FromDIP(4), posY - (ext.y * .25)));
      posY += outputNodeHeight * 2;
    }
  }

  auto createConnection = [&](wxGraphicsContext* ctx, const wxPoint& start, const wxPoint& end, const wxColor* color = nullptr) {
    wxGraphicsPath path = ctx->CreatePath();
    path.MoveToPoint(start);
    path.AddCurveToPoint(start.x + FromDIP(20), start.y, end.x - FromDIP(20), end.y, end.x, end.y);
    ctx->PushState();
    if (!color)
    {
      ctx->SetPen(wxPen(*wxBLACK));
    }
    else
    {
      ctx->SetPen(wxPen(*color));
    }
    ctx->StrokePath(path);
    ctx->PopState();
  };

  // Draw expression connections
  wxGraphicsContext* ctx = wxGraphicsContext::Create(dc);
  dc.SetPen(defaultPen);
  ctx->SetPen(defaultPen);
  for (MaterialExpressionInfo& i : GraphNodes)
  {
    if (i.Title.StartsWith("Comment"))
    {
      continue;
    }

    int32 idx = 0;
    for (FExpressionInput& input : i.Inputs)
    {
      if (!input.IsConnected() || !ExpressionMap.count(input.Expression))
      {
        idx++;
        continue;
      }

      wxPoint dst = i.InputsPositions[idx];
      dst.y -= outputNodeHeight / 2 + FromDIP(2);
      dst.x += FromDIP(10);
      

      MaterialExpressionInfo& di = GraphNodes[ExpressionMap[input.Expression]];
      if (di.MFOutputs.size())
      {
        int32 outIdx = std::clamp<int32>(input.OutputIndex, 0, di.MFOutputs.size());
        wxPoint src = di.MFOutputsPositions[outIdx];
        src.y += outputNodeHeight / 2;
        createConnection(ctx, dst, src);
      }
      else if (!input.Mask || (input.MaskR && input.MaskG && input.MaskB))
      {
        wxPoint src = di.PosRGB;
        src.y += outputNodeHeight / 2;
        createConnection(ctx, dst, src);
      }
      else
      {
        if (input.MaskR)
        {
          wxPoint src = di.PosR;
          src.y += outputNodeHeight / 2;
          createConnection(ctx, dst, src, wxRED);
        }
        if (input.MaskG)
        {
          wxPoint src = di.PosG;
          src.y += outputNodeHeight / 2;
          createConnection(ctx, dst, src, wxGREEN);
        }
        if (input.MaskB)
        {
          wxPoint src = di.PosB;
          src.y += outputNodeHeight / 2;
          createConnection(ctx, dst, src, wxBLUE);
        }
        if (input.MaskA)
        {
          wxPoint src = di.PosA;
          src.y += outputNodeHeight / 2;
          createConnection(ctx, dst, src);
        }
      }

      idx++;
    }
  }

  // Draw the material node
  if (ObjectName.size())
  {
    dc.SetPen(defaultPen);
    dc.SetBrush(panelBrush);

    int matWidth = std::max<int>(dc.GetTextExtent(ObjectName).x, FromDIP(150));
    for (FExpressionInput& input : MaterialInputs)
    {
      wxSize extent = dc.GetTextExtent(input.Title.WString());
      if (extent.x > matWidth)
      {
        matWidth = extent.x;
      }
    }

    int posX = CanvasOffsetX;
    int posY = CanvasOffsetY + dc.GetTextExtent(ObjectName).y + FromDIP(4);
    dc.SetPen(wxPen(wxColor(0, 0, 0), 2));
    dc.SetBrush(wxBrush(wxColor(120, 120, 120)));
    dc.DrawRectangle(wxPoint(posX, CanvasOffsetY), wxSize(matWidth + FromDIP(4), posY - CanvasOffsetY));
    drawLabelFunc(ObjectName, wxPoint(CanvasOffsetX + FromDIP(2), CanvasOffsetY + FromDIP(2)), matWidth);

    posY += FromDIP(2);
    dc.DrawRectangle(wxPoint(posX, posY), wxSize(matWidth + FromDIP(4), std::min<int>(MaterialInputs.size() * outputNodeHeight * 3, FromDIP(300))));
    dc.SetBrush(wxBrush(wxColor(0, 0, 0)));
    dc.SetPen(wxPen(wxColor(), 0, wxPENSTYLE_TRANSPARENT));
    for (FExpressionInput& input : MaterialInputs)
    {
      wxSize extent = dc.GetTextExtent(input.Title.WString());
      dc.DrawRectangle(wxPoint(posX + matWidth + FromDIP(4), posY + FromDIP(4)), wxSize(FromDIP(10), outputNodeHeight));
      drawLabelFunc(input.Title.WString(), wxPoint(posX + matWidth - extent.x, posY));

      if (input.IsConnected() && ExpressionMap.count(input.Expression))
      {
        wxPoint dst(posX + matWidth + FromDIP(4), posY);
        dst.y += outputNodeHeight;
        dst.x += FromDIP(10);


        MaterialExpressionInfo& di = GraphNodes[ExpressionMap[input.Expression]];
        if (!input.Mask)
        {
          wxPoint src = di.PosRGB;
          src.y += outputNodeHeight / 2;
          createConnection(ctx, dst, src);
        }
        else
        {
          if (input.MaskR)
          {
            wxPoint src = di.PosR;
            src.y += outputNodeHeight / 2;
            createConnection(ctx, dst, src);
          }
          if (input.MaskG)
          {
            wxPoint src = di.PosG;
            src.y += outputNodeHeight / 2;
            createConnection(ctx, dst, src);
          }
          if (input.MaskB)
          {
            wxPoint src = di.PosB;
            src.y += outputNodeHeight / 2;
            createConnection(ctx, dst, src);
          }
          if (input.MaskA)
          {
            wxPoint src = di.PosA;
            src.y += outputNodeHeight / 2;
            createConnection(ctx, dst, src);
          }
        }
      }

      posY += outputNodeHeight * 3;
    }
  }
}

void DragableCanvas::OnMouseDown(wxMouseEvent& e)
{
  if (e.GetButton() != wxMOUSE_BTN_LEFT)
  {
    return;
  }
  IsDragging = true;
  MouseStart = wxGetMousePosition();
  CanvasStart = ((wxScrolledWindow*)GetParent())->GetViewStart();

  CaptureMouse();
  SetCursor(wxCursor(MAKE_IDB(IDB_CUR_GRABHAND)));
}

void DragableCanvas::OnMouseUp(wxMouseEvent& e)
{
  if (e.GetButton() != wxMOUSE_BTN_LEFT || !IsDragging)
  {
    return;
  }
  IsDragging = false;
  ReleaseMouse();
  SetCursor(wxCursor(wxCURSOR_ARROW));
}

void DragableCanvas::OnMouseMove(wxMouseEvent& e)
{
  if (!IsDragging)
  {
    return;
  }
  int unitX = 0;
  int unitY = 0;
  wxScrolledWindow* scrolledWin = (wxScrolledWindow*)GetParent();
  scrolledWin->GetScrollPixelsPerUnit(&unitX, &unitY);
  wxPoint pos = wxGetMousePosition() - MouseStart;
  if (unitX)
  {
    pos.x /= unitX;
  }
  if (unitY)
  {
    pos.y /= unitY;
  }
  pos = CanvasStart - pos;
  pos.x = std::max(pos.x, 0);
  pos.y = std::max(pos.y, 0);
  scrolledWin->Scroll(pos);
}


BEGIN_EVENT_TABLE(UDKMaterialGraph, DragableCanvas)
EVT_PAINT(UDKMaterialGraph::OnPaint)
EVT_ERASE_BACKGROUND(UDKMaterialGraph::OnEraseBg)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(DragableCanvas, wxPanel)
EVT_LEFT_DOWN(DragableCanvas::OnMouseDown)
EVT_LEFT_UP(DragableCanvas::OnMouseUp)
EVT_MOTION(DragableCanvas::OnMouseMove)
END_EVENT_TABLE()