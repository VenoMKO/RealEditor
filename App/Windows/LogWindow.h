#pragma once
#include <wx/wx.h>

wxDECLARE_EVENT(PUMP_LOG_WINDOW, wxCommandEvent);

class ALog;
class wxRichTextCtrl;
class LogWindow : public wxFrame
{
public:
	LogWindow(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(700, 300), long style = wxSYSTEM_MENU | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxTAB_TRAVERSAL);
	~LogWindow();

	void SetLogger(ALog* logger)
	{
		Logger = logger;
	}

	void OnCloseWindow(wxCloseEvent& event);
	void PumpMessages(wxCommandEvent&);

	wxDECLARE_EVENT_TABLE();

private:
	wxRichTextCtrl* LogCtrl = nullptr;
	ALog* Logger = nullptr;
	size_t LastMessageIndex = 0;
};