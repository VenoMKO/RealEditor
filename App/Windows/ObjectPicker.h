#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>

#include <Tera/Core.h>
#include <Tera/FPackage.h>

class ObjectPicker : public wxDialog {
public:
  ObjectPicker(wxWindow* parent, const wxString& title, bool allowDifferentPackage, const wxString& packageName, PACKAGE_INDEX selection = 0, const std::vector<FString>& allowedClasses = {});
  ObjectPicker(wxWindow* parent, const wxString& title, bool allowDifferentPackage, std::shared_ptr<FPackage> package, PACKAGE_INDEX selection = 0, const std::vector<FString>& allowedClasses = {});
  ~ObjectPicker();

  void SetCanChangePackage(bool flag);

  inline bool IsValid() const
  {
    return Package != nullptr;
  }

  UObject* GetSelectedObject() const
  {
    return Selection;
  }

  void SetAllowRootExport(bool flag)
  {
    AllowRootExport = flag;
  }

protected:
  void OnObjectSelected(wxDataViewEvent& event);
  void OnPackageClicked(wxCommandEvent& event);
  void OnOkClicked(wxCommandEvent& event);
  void OnCancelClicked(wxCommandEvent& event);
  void LoadObjectTree();
  void UpdateTableTitle();

protected:
  UObject* Selection = nullptr;
  std::shared_ptr<FPackage> Package = nullptr;
  std::vector<FString> Filter;
  bool AllowDifferentPackage = false;
  bool AllowRootExport = false;

  wxString TableTitle;
  wxDataViewCtrl* ObjectTreeCtrl = nullptr;
  wxButton* PackageButton = nullptr;
  wxButton* OkButton = nullptr;
  wxButton* CancelButton = nullptr;
};

class ObjectNameDialog : public wxDialog {
public:
  ObjectNameDialog(wxWindow* parent, const wxString& objectName = wxEmptyString);
  ~ObjectNameDialog();

  typedef std::function<bool(const wxString&)> Validator;
  void SetValidator(Validator& validator);
  wxString GetObjectName() const;

protected:
  void OnName(wxCommandEvent&);
  void OnNameEnter(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);
  void OnCancelClicked(wxCommandEvent&);

protected:
  wxTextCtrl* NameField = nullptr;
  wxButton* OkButton = nullptr;
  wxButton* CancelButton = nullptr;
  std::function<bool(const wxString&)> ValidatorFunc;
};