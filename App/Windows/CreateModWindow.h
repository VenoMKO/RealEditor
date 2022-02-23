#pragma once
#include <wx/wx.h>
#include "WXDialog.h"

class CreateModWindow : public WXDialog
{
public:
	CreateModWindow(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Create a mod"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(455, 133), long style = wxDEFAULT_DIALOG_STYLE);

	wxString GetName() const;
	wxString GetAuthor() const;

protected:
	void OnTextEvent(wxCommandEvent&);

protected:
	wxTextCtrl* NameField = nullptr;
	wxTextCtrl* AuthorField = nullptr;
	wxButton* CreateButton = nullptr;
	wxButton* CancelButton = nullptr;

	wxDECLARE_EVENT_TABLE();
};