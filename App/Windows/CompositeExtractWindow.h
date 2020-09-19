#pragma once
#include <wx/wx.h>

class CompositeExtractWindow : public wxDialog
{
public:
	CompositeExtractWindow(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Extract composite packages"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(682, 296), long style = wxDEFAULT_DIALOG_STYLE);
	~CompositeExtractWindow();

protected:
	void OnBrowseClicked(wxCommandEvent& event);
	void OnSearchClicked(wxCommandEvent& event);
	void OnObjectNameText(wxCommandEvent& event);
	void OnObjectClassText(wxCommandEvent& event);
	void OnObjectNameEnter(wxCommandEvent& event);
	void OnImportClicked(wxCommandEvent& event);
	void OnImportClearClicked(wxCommandEvent& event);
	void OnExtractClicked(wxCommandEvent& event);

private:
	wxTextCtrl* DumpTextField = nullptr;
	wxButton* BrowseButton = nullptr;
	wxTextCtrl* ObjectClassTextField = nullptr;
	wxTextCtrl* ObjectTextField = nullptr;
	wxButton* SearchButton = nullptr;
	wxTextCtrl* ImportTextField = nullptr;
	wxButton* ImportButton = nullptr;
	wxButton* ClearButton = nullptr;
	wxStaticText* ResultLabel = nullptr;
	wxButton* ExtractButton = nullptr;
	std::vector<std::pair<std::string, int>> Found;
};