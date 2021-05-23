#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>
#include <map>

#include <Tera/UObject.h>

class DependsResolveDialog : public wxDialog{
public:
  DependsResolveDialog(wxWindow* parent, const std::map<class UObject*, UObject*>& objects, class FPackage* destPackage);

  std::map<UObject*, UObject*> GetResult() const;

protected:
  void OnDataViewValueChanged(wxDataViewEvent& event);
  void OnSelectionChanged(wxDataViewEvent& event);
  void OnAllClicked(wxCommandEvent& event);
  void OnEditClicked(wxCommandEvent& event);
  void OnOkClicked(wxCommandEvent& event);
  void OnCancelClicked(wxCommandEvent& event);

  void UpdateOkButton();

protected:
  wxDataViewCtrl* DataView = nullptr;
  wxButton* AllButton = nullptr;
  wxButton* EditButton = nullptr;
  wxButton* OkButton = nullptr;
  wxButton* CancelButton = nullptr;

  class FPackage* DestinationPackage = nullptr;
};