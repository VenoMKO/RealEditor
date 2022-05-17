#include "MaterialFunctionEditor.h"
#include "../CustomViews/MaterialView.h"

#include "../App.h"
#include "../Windows/REDialogs.h"
#include "../Windows/ProgressWindow.h"

#include "../resource.h"

#include <Tera/Cast.h>

#include <thread>

MaterialFunctionEditor::MaterialFunctionEditor(wxPanel* parent, PackageWindow* window)
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

void MaterialFunctionEditor::OnObjectLoaded()
{
  if ((Loading || NeedsGraph) && Cast<UMaterialFunction>(Object))
  {
    NeedsGraph = false;
    BuildGraph();
  }
  GenericEditor::OnObjectLoaded();
}

void MaterialFunctionEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (Object && !toolbar->FindById(eID_Export))
  {
    toolbar->AddTool(eID_Export, "Export", wxBitmap(MAKE_IDB(IDB_EXPORT), wxBITMAP_TYPE_PNG_RESOURCE), "Export object data...");
    if (!Object->GetPackage()->GetPackageFlag(PKG_ROAccess))
    {
      toolbar->AddTool(eID_Import, "Import", wxBitmap(MAKE_IDB(IDB_IMPORT), wxBITMAP_TYPE_PNG_RESOURCE), "Import object data...");
      toolbar->FindById(eID_Import)->Enable(false);
    }
  }
}

void MaterialFunctionEditor::OnExportClicked(wxCommandEvent&)
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

void MaterialFunctionEditor::BuildGraph()
{
  Graph = new UDKMaterialGraph(Canvas, Cast<UMaterialFunction>(Object));
}