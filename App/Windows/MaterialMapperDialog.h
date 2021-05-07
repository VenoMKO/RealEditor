#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>

class UObject;
struct MaterialMapperItem {
  wxString FbxName;
  UObject* Material = nullptr;
};

class MaterialMapperDialog : public wxDialog {
public:
  static std::vector<MaterialMapperItem> AutomaticallyMapMaterials(std::vector<class FString>& fbxMaterials, const std::vector<UObject*>& objectMaterials);

  MaterialMapperDialog(wxWindow* parent, const std::vector<MaterialMapperItem>& map, const std::vector<UObject*>& objectMaterials);
  ~MaterialMapperDialog()
  {}

  std::vector<MaterialMapperItem> GetResult() const;

protected:
  wxDataViewListCtrl* MaterialsList = nullptr;
  std::vector<UObject*> ObjectMaterials;

  void OnEditClicked(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);
  void OnCancelClicked(wxCommandEvent&);
};