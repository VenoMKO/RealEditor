#pragma once
#include <wx/wx.h>
#include <Tera/FPackage.h>
#include "WXDialog.h"

class CookingOptionsWindow : public WXDialog
{
public:
	CookingOptionsWindow(wxWindow* parent, FPackage* package, wxWindowID id = wxID_ANY, const wxString& title = wxT("Save options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(521, 378), long style = wxDEFAULT_DIALOG_STYLE);
	~CookingOptionsWindow();

	void ConfigureSaveContext(PackageSaveContext& ctx);

	int ShowModal() wxOVERRIDE;

private:
	void OnSaveClicked(wxCommandEvent& event);
	void OnCancelClicked(wxCommandEvent& event);

protected:
	FPackage* Package = nullptr;

	wxCheckBox* EnableCompositeInfoButton = nullptr;
	wxCheckBox* DisableTextureCachingButton = nullptr;
	wxCheckBox* PreserveOffsetsButton = nullptr;
	wxCheckBox* CompressPackageButton = nullptr;
	wxButton* SaveButton = nullptr;
	wxButton* CancelButton = nullptr;
};