#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>

#include <Tera/Core.h>
#include <Tera/FPackage.h>

class ObjectPicker : public wxDialog {
public:
	ObjectPicker(wxWindow* parent, const wxString& title, bool allowDifferentPackage, const wxString& packageName, PACKAGE_INDEX selection = 0);
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
	bool AllowDifferentPackage = false;

	wxString TableTitle;
	wxDataViewCtrl* ObjectTreeCtrl = nullptr;
	wxButton* PackageButton = nullptr;
	wxButton* OkButton = nullptr;
	wxButton* CancelButton = nullptr;
};