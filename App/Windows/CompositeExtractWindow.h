#pragma once
#include <wx/wx.h>
#include <sstream>
#include "../Misc/CompositeExtractModel.h"

class wxDataViewCtrl;
class CompositeExtractWindow : public wxDialog
{
public:
	CompositeExtractWindow(wxWindow* parent);
	CompositeExtractWindow(wxWindow* parent, const wxString& objClass, const wxString objName);
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
	void OnContextMenu(wxDataViewEvent& event);

	void ExtractTextures();
	void ExtractSounds();
	void ExtractUntyped();

	int GetResultsCount();
	int GetEnabledResultsCount();
	std::vector<CompositeExtractModelNode> GetSearchResult();

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
	wxDataViewCtrl* ResultList = nullptr;
	wxButton* ExtractButton = nullptr;
	wxString LoadedPath;
	std::stringstream LoadedBuffer;

	wxDECLARE_EVENT_TABLE();
};