#pragma once
#include <wx/wx.h>
#include <Utils/CompositePatcher.h>
#include "../Misc/WXDialog.h"

class CompositePatcherWindow : public WXDialog {
public:
	CompositePatcherWindow(wxWindow* parent, const wxString& sourceName = wxEmptyString);

protected:
	void OnSourceFieldText(wxCommandEvent&);
	void OnSourceFieldEnter(wxCommandEvent& e);
	void OnSelectClicked(wxCommandEvent&);
	void OnPatchClicked(wxCommandEvent&);
	void OnSizeClicked(wxCommandEvent&);

protected:
	CompositePatcher Patcher;

	wxTextCtrl* SourceField = nullptr;
	wxButton* SelectButton = nullptr;
	wxTextCtrl* ContainerField = nullptr;
	wxTextCtrl* CompositeNameField = nullptr;
	wxTextCtrl* ObjectField = nullptr;
	wxTextCtrl* OffsetField = nullptr;
	wxTextCtrl* SizeField = nullptr;
	wxButton* SizeButton = nullptr;
	wxButton* PatchButton = nullptr;
	wxButton* CancelButton = nullptr;
	DECLARE_EVENT_TABLE();
};