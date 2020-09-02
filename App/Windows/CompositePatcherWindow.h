#pragma once
#include <wx/wx.h>
#include <Utils/CompositePatcher.h>

class CompositePatcherWindow : public wxDialog {
public:
	CompositePatcherWindow(wxWindow* parent);

protected:
	void OnSourceFieldText(wxCommandEvent&);
	void OnSourceFieldEnter(wxCommandEvent& e);
	void OnSelectClicked(wxCommandEvent&);
	void OnPatchClicked(wxCommandEvent&);

protected:
	CompositePatcher Patcher;

	wxTextCtrl* SourceField = nullptr;
	wxButton* SelectButton = nullptr;
	wxTextCtrl* ContainerField = nullptr;
	wxTextCtrl* CompositeNameField = nullptr;
	wxTextCtrl* ObjectField = nullptr;
	wxTextCtrl* OffsetField = nullptr;
	wxTextCtrl* SizeField = nullptr;
	wxButton* PatchButton = nullptr;
	wxButton* CancelButton = nullptr;
	DECLARE_EVENT_TABLE();
};