#include "MaterialEditor.h"
#include "../CustomViews/MaterialView.h"

#include "../App.h"
#include "../Windows/REDialogs.h"
#include "../Windows/ProgressWindow.h"

#include <Tera/Cast.h>

#include <thread>

MaterialEditor::MaterialEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  wxBoxSizer* bSizer13;
  bSizer13 = new wxBoxSizer(wxVERTICAL);

  Canvas = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxHSCROLL | wxVSCROLL);
  Canvas->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
  Canvas->SetScrollRate(50, 50);
  bSizer13->Add(Canvas, 1, wxEXPAND | wxALL, FromDIP(5));

  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  SetSizer(bSizer13);
  Layout();
}

void MaterialEditor::OnObjectLoaded()
{
  if ((Loading || NeedsGraph) && Cast<UMaterial>(Object))
  {
    NeedsGraph = false;
    BuildGraph();
  }
  GenericEditor::OnObjectLoaded();
}

void MaterialEditor::OnExportClicked(wxCommandEvent&)
{
  if (Graph)
  {
    wxAutoBufferedPaintDC dc(Graph);
    if (dc.GetSelectedBitmap().IsOk())
    {
      wxString path = IODialog::SaveImageDialog(this, Object->GetObjectNameString().String());
      if (path.size())
      {
        ProgressWindow progress(this, "Please wait...");
        progress.SetCurrentProgress(-1);
        progress.SetCanCancel(false);
        progress.SetActionText("Saving the image");
        progress.Layout();

        std::thread([&] {
          Graph->Render(dc);
          dc.GetSelectedBitmap().SaveFile(path, wxBITMAP_TYPE_PNG);
          SendEvent(&progress, UPDATE_PROGRESS_FINISH, true);
        }).detach();
       
        progress.ShowModal();
      }
    }
  }
}

void MaterialEditor::BuildGraph()
{
  Graph = new UDKMaterialGraph(Canvas, Cast<UMaterial>(Object));
}
