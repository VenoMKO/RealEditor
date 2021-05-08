#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>

class UObject;
class FString;
class MaterialMapperDialog : public wxDialog {
public:
  // Returns true if all fbxMaterials names perfectly match objectMaterials.
  static bool AutomaticallyMapMaterials(std::vector<class FString>& fbxMaterials, const std::vector<UObject*>& objectMaterials, std::vector<std::pair<class FString, class UObject*>>& output);

  MaterialMapperDialog(wxWindow* parent, const std::vector<std::pair<FString, UObject*>>& map, const std::vector<UObject*>& objectMaterials);
  ~MaterialMapperDialog()
  {}

  std::vector<std::pair<FString, UObject*>> GetResult() const;

protected:
  wxDataViewListCtrl* MaterialsList = nullptr;
  std::vector<UObject*> ObjectMaterials;

  void OnEditClicked(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);
  void OnCancelClicked(wxCommandEvent&);
};