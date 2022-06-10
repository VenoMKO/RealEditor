#pragma once
#include <wx/wx.h>

class BakeModDialog : public wxDialog {
public:
	BakeModDialog(wxWindow* parent);

	wxString GetSource() const;
	wxString GetDestination() const;

private:
	void OnBrowseModClicked(wxCommandEvent&);
	void OnBrowseDestClicked(wxCommandEvent&);
	void OnBakeClicked(wxCommandEvent&);
	void OnCancelClicked(wxCommandEvent&);

	void UpdateOk();

private:
	wxTextCtrl* ModField = nullptr;
	wxTextCtrl* DestField = nullptr;
	wxButton* BakeButton = nullptr;
};