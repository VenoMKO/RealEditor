#include "MaterialView.h"

#include <wx/scrolwin.h>
#include <wx/graphics.h>

#include <Tera/Cast.h>

#include <sstream>

const int32 CanvasPadding = 200;

UDKMaterialGraph::UDKMaterialGraph(wxWindow* parent, UMaterial* material)
	: wxPanel(parent)
	, Material(material)
{
	for (FPropertyTag* tag : Material->MaterialInputs)
	{
		FExpressionInput& in = MaterialInputs.emplace_back(tag);
		in.Title = tag->Name.String();
	}
	auto expressions = material->GetExpressions();

	if (expressions.empty())
	{
		SetSize(500, 500);
		return;
	}

	int32 minX = INT_MAX;
	int32 minY = INT_MAX;
	int32 maxX = INT_MIN;
	int32 maxY = INT_MIN;
	for (UMaterialExpression* exp : expressions)
	{
		if (exp->MaterialExpressionEditorX < minX)
		{
			minX = exp->MaterialExpressionEditorX;
		}
		if (exp->MaterialExpressionEditorX > maxX)
		{
			maxX = exp->MaterialExpressionEditorX;
		}
		if (exp->MaterialExpressionEditorY < minY)
		{
			minY = exp->MaterialExpressionEditorY;
		}
		if (exp->MaterialExpressionEditorY > maxY)
		{
			maxY = exp->MaterialExpressionEditorY;
		}
	}

	if (minX == maxX || minY == maxY)
	{
		SetSize(500, 500);
	}

	int canvasWidth = abs(minX) + abs(maxX) + (CanvasPadding * 2);
	int canvasHeight = abs(minY) + abs(maxY) + (CanvasPadding * 2);

	if (!canvasWidth)
	{
		canvasWidth = 500;
	}
	if (!canvasHeight)
	{
		canvasHeight = 500;
	}

	parent->SetVirtualSize(canvasWidth, canvasHeight);

	if (minX < 0)
	{
		CanvasOffsetX = abs(minX);
	}
	CanvasOffsetX += CanvasPadding;
	if (minY < 0)
	{
		CanvasOffsetY = abs(minY);
	}
	CanvasOffsetY += CanvasPadding;

	for (UMaterialExpression* exp : expressions)
	{
		ExpressionMap[exp] = GraphNodes.size();
		MaterialExpressionInfo& i = GraphNodes.emplace_back();
		exp->AcceptVisitor(&i);
		i.Expression = exp;
		i.Position.x += CanvasOffsetX;
		i.Position.y += CanvasOffsetY;
		if (!i.Size.x || !i.Size.y)
		{
			i.Size = wxSize(80, 94);
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
}

void UDKMaterialGraph::OnPaint(wxPaintEvent& e)
{
	wxBufferedPaintDC dc(this);
	Render(dc);
}

void UDKMaterialGraph::Render(wxBufferedPaintDC& dc)
{
	wxBrush panelBrush = dc.GetBackground();
	dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)));
	dc.Clear();
	dc.SetBrush(dc.GetBackground());

	auto drawLabelFunc = [&dc](wxString text, const wxPoint& at, unsigned maxWidth = 0) {
		wxSize size = dc.GetTextExtent(text);
		bool truncate = false;
		while (maxWidth && size.x > maxWidth && text.size() > 3)
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

	auto drawExpressionCaption = [&dc, &drawLabelFunc](const MaterialExpressionInfo& i) {
		if (i.Title == "Comment")
		{
			wxPoint at = i.Position;
			wxSize size = dc.GetTextExtent(i.TextValue);
			at.y -= size.y;
			drawLabelFunc(i.TextValue, at, i.Size.x);
		}
		else
		{
			wxPoint at = i.Position;
			at.x += 2;
			drawLabelFunc(i.Title, at, i.Size.x - 2);
		}
	};

	const int outputNodeHeight = 7;

	if (NeedsPositionCalculation)
	{
		NeedsPositionCalculation = false;
		for (MaterialExpressionInfo& i : GraphNodes)
		{
			int width = i.Size.x + 4;
			wxSize titleExtent = dc.GetTextExtent(i.Title);
			if (titleExtent.x > width)
			{
				i.Size.x = titleExtent.x + 4;
			}

			int posY = titleExtent.y + i.Position.y + 14;
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
						width = ext.x + 4;
					}
					posY += 14;
				}
				posY += 4;
			}

			if (i.Size.x < width)
			{
				i.Size.x = width + 4;
				posX = i.Size.x + i.Position.x;
			}

			for (FExpressionInput& input : i.Inputs)
			{
				i.InputsPositions.push_back(wxPoint(posX, posY));
				posY += outputNodeHeight * 2;
			}

			posY = titleExtent.y + i.Position.y + 6;
			posX = i.Position.x - 10;

			i.PosRGBA.x = posX;
			i.PosRGBA.y = posY;

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

	// Draw comments
	for (MaterialExpressionInfo& i : GraphNodes)
	{
		if (i.Title == "Comment")
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
		if (i.Title == "Comment")
		{
			continue;
		}

		dc.SetBrush(panelBrush);
		dc.SetPen(defaultPen);

		wxSize titleExtent = dc.GetTextExtent(i.Title);

		dc.DrawRectangle(i.Position, wxSize(i.Size.x, titleExtent.y));
		dc.DrawRectangle(wxPoint(i.Position.x, i.Position.y + 2 + titleExtent.y), wxSize(i.Size.x, i.Size.y - titleExtent.y - 2));
		drawExpressionCaption(i);

		int posY = titleExtent.y + i.Position.y + 6;
		int posX = i.Position.x - 10;
		int inPosY = posY;

		if (i.TextValue.size())
		{
			std::istringstream stream(i.TextValue.ToStdString());
			std::string line;
			while (getline(stream, line))
			{
				drawLabelFunc(line, wxPoint(posX + 12, inPosY), i.Size.x);
				inPosY += 14;
			}
			inPosY += 4;
		}
		
		dc.SetPen(wxPen(wxColor(), 0, wxPENSTYLE_TRANSPARENT));
		// Outputs
		
		

		// Composite
		dc.SetBrush(wxBrush(wxColor(0, 0, 0)));
		dc.DrawRectangle(wxPoint(posX, posY), wxSize(10, outputNodeHeight));

		if (i.NeedsMultipleOutputs)
		{
			// Red
			posY += outputNodeHeight * 2;
			dc.SetBrush(wxBrush(wxColor(255, 0, 0)));
			dc.DrawRectangle(wxPoint(posX, posY), wxSize(10, outputNodeHeight));

			// Gree
			posY += outputNodeHeight * 2;
			dc.SetBrush(wxBrush(wxColor(0, 255, 0)));
			dc.DrawRectangle(wxPoint(posX, posY), wxSize(10, outputNodeHeight));

			// Blue
			posY += outputNodeHeight * 2;
			dc.SetBrush(wxBrush(wxColor(0, 0, 255)));
			dc.DrawRectangle(wxPoint(posX, posY), wxSize(10, outputNodeHeight));

			// Alpha
			posY += outputNodeHeight * 2;
			dc.SetBrush(wxBrush(wxColor(255, 255, 255)));
			dc.DrawRectangle(wxPoint(posX, posY), wxSize(10, outputNodeHeight));
		}

		// Inputs
		posY = inPosY;
		posX = i.Size.x + i.Position.x;

		for (FExpressionInput& input : i.Inputs)
		{
			dc.SetBrush(wxBrush(wxColor(0, 0, 0)));
			wxPoint inputTriangle[4] = { {posX, posY + int(outputNodeHeight * .6)}, {posX, posY + int(outputNodeHeight * .45)}, {posX + 10, posY}, {posX + 10, posY + outputNodeHeight} };
			dc.DrawPolygon(4, inputTriangle);
			wxString title = input.GetDescription().WString();
			wxSize ext = dc.GetTextExtent(title);
			drawLabelFunc(title, wxPoint(posX - ext.x - 4, posY - (ext.y * .25)));
			posY += outputNodeHeight * 2;
		}
	}

	auto createConnection = [](wxGraphicsContext* ctx, const wxPoint& start, const wxPoint& end) {
		wxGraphicsPath path = ctx->CreatePath();
		path.MoveToPoint(start);
		path.AddCurveToPoint(start.x + 20, start.y, end.x - 20, end.y, end.x, end.y);
		ctx->StrokePath(path);
	};

	// Draw expression connections
	wxGraphicsContext* ctx = wxGraphicsContext::Create(dc);
	dc.SetPen(defaultPen);
	ctx->SetPen(defaultPen);
	for (MaterialExpressionInfo& i : GraphNodes)
	{
		if (i.Title == "Comment")
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
			dst.y -= outputNodeHeight / 2 + 2;
			dst.x += 10;
			

			MaterialExpressionInfo& di = GraphNodes[ExpressionMap[input.Expression]];
			if (!input.Mask)
			{
				wxPoint src = di.PosRGBA;
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

			idx++;
		}
	}

	// Draw the material node

	dc.SetPen(defaultPen);
	dc.SetBrush(panelBrush);

	int matWidth = std::max<int>(dc.GetTextExtent(Material->GetObjectName().WString()).x, 150);
	for (FExpressionInput& input : MaterialInputs)
	{
		wxSize extent = dc.GetTextExtent(input.Title.WString());
		if (extent.x > matWidth)
		{
			matWidth = extent.x;
		}
	}
	
	int posX = CanvasOffsetX;
	int posY = CanvasOffsetY + dc.GetTextExtent(Material->GetObjectName().WString()).y + 4;
	dc.SetPen(wxPen(wxColor(0, 0, 0), 2));
	dc.SetBrush(wxBrush(wxColor(120, 120, 120)));
	dc.DrawRectangle(wxPoint(posX, CanvasOffsetY), wxSize(matWidth + 4, posY - CanvasOffsetY));
	drawLabelFunc(Material->GetObjectName().WString(), wxPoint(CanvasOffsetX + 2, CanvasOffsetY + 2), matWidth);

	posY += 2;
	dc.DrawRectangle(wxPoint(posX, posY), wxSize(matWidth + 4, std::min<int>(MaterialInputs.size() * outputNodeHeight * 3, 300)));
	dc.SetBrush(wxBrush(wxColor(0, 0, 0)));
	dc.SetPen(wxPen(wxColor(), 0, wxPENSTYLE_TRANSPARENT));
	for (FExpressionInput& input : MaterialInputs)
	{
		wxSize extent = dc.GetTextExtent(input.Title.WString());
		dc.DrawRectangle(wxPoint(posX + matWidth + 4, posY + 4), wxSize(10, outputNodeHeight));
		drawLabelFunc(input.Title.WString(), wxPoint(posX + matWidth - extent.x, posY));

		if (input.IsConnected() && ExpressionMap.count(input.Expression))
		{
			wxPoint dst(posX + matWidth + 4, posY);
			dst.y += outputNodeHeight;
			dst.x += 10;


			MaterialExpressionInfo& di = GraphNodes[ExpressionMap[input.Expression]];
			if (!input.Mask)
			{
				wxPoint src = di.PosRGBA;
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


BEGIN_EVENT_TABLE(UDKMaterialGraph, wxPanel)
EVT_PAINT(UDKMaterialGraph::OnPaint)
EVT_ERASE_BACKGROUND(UDKMaterialGraph::OnEraseBg)
END_EVENT_TABLE()
