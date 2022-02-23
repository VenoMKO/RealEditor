#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>
#include "WXDialog.h"

class UObject;
class FString;
class MaterialMapperDialog : public WXDialog {
public:
  // Returns true if all fbxMaterials names perfectly match objectMaterials.
  static bool AutomaticallyMapMaterials(std::vector<class FString>& fbxMaterials, const std::vector<UObject*>& objectMaterials, std::vector<std::pair<class FString, class UObject*>>& output);

  MaterialMapperDialog(wxWindow* parent, UObject* object, const std::vector<std::pair<FString, UObject*>>& map, const std::vector<UObject*>& objectMaterials);
  ~MaterialMapperDialog()
  {}

  std::vector<std::pair<FString, UObject*>> GetMaterialMap() const;

  std::vector<UObject*> GetObjectMaterials() const
  {
    return ObjectMaterials;
  }

protected:
  UObject* Object = nullptr;
  wxDataViewCtrl* MaterialsList = nullptr;
  std::vector<UObject*> ObjectMaterials;

  void OnAddClicked(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);
  void OnCancelClicked(wxCommandEvent&);
};