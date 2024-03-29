#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>
#include "WXDialog.h"

#include <Tera/Core.h>
#include <Tera/FPackage.h>

class ObjectPicker : public WXDialog {
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

  void SetAllowNewPackage(bool flag)
  {
    AllowNewPackage = flag;
  }

protected:
  void OnObjectSelected(wxDataViewEvent& event);
  void OnPackageClicked(wxCommandEvent& event);
  void OnShowContextMenu(wxDataViewEvent& event);
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
  bool AllowNewPackage = false;

  wxString TableTitle;
  class ObjectTreeDataViewCtrl* ObjectTreeCtrl = nullptr;
  wxButton* PackageButton = nullptr;
  wxButton* OkButton = nullptr;
  wxButton* CancelButton = nullptr;
  wxStaticText* ContentsLabel = nullptr;
};

class ObjectNameDialog : public WXDialog {
public:
  typedef std::function<bool(const wxString&)> Validator;

  static Validator GetDefaultValidator(class FObjectExport* parent, FPackage* package);
  ObjectNameDialog(wxWindow* parent, const wxString& objectName = wxEmptyString);
  ~ObjectNameDialog();

  
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

class ClassPicker : public wxDialog {
public:
  ClassPicker(wxWindow* parent = nullptr, const wxString& title = wxT("Select class"));

  wxString GetSelectedClassName() const;

protected:
  wxComboBox* ClassCombo = nullptr;
  wxButton* OkButton = nullptr;
  wxButton* CancelButton = nullptr;

  void OnClassSelected(wxCommandEvent&);
  void OnClassText(wxCommandEvent&);
  void OnClassTextEnter(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);
  void OnCancelClicked(wxCommandEvent&);

private:
  wxString ClassName;
};

class DebugObjectPicker : public WXDialog {
public:
  DebugObjectPicker(wxWindow* parent);

  PACKAGE_INDEX GetResult() const;

protected:
  void OnText(wxCommandEvent&);
  void OnTextEnter(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);
  void OnCancelClicked(wxCommandEvent&);

protected:
  wxTextCtrl* CompositeName = nullptr;
  wxButton* CancelButton = nullptr;
  wxButton* OpenButton = nullptr;
  PACKAGE_INDEX Result = 0;
};