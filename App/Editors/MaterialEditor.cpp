#include "MaterialEditor.h"

#include <Tera/Cast.h>

MaterialEditor::MaterialEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  wxBoxSizer* bSizer13;
  bSizer13 = new wxBoxSizer(wxVERTICAL);

  Canvas = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxHSCROLL | wxVSCROLL);
  Canvas->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
  Canvas->SetScrollRate(50, 50);
  bSizer13->Add(Canvas, 1, wxEXPAND | wxALL, 5);

  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  SetSizer(bSizer13);
  Layout();
}

void MaterialEditor::OnObjectLoaded()
{
  if (Loading && Cast<UMaterial>(Object))
  {
    BuildGraph();
  }
  GenericEditor::OnObjectLoaded();
}

void MaterialEditor::BuildGraph()
{
  new UDKMaterialGraph(Canvas, Cast<UMaterial>(Object));
}
