#pragma once
#include <wx/wx.h>
#include <atomic>
#include <Utils/ALog.h>

class wxRichTextCtrl;
class LogWindow : public wxFrame
{
public:
	LogWindow(const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(700, 300));

	void OnCloseWindow(wxCloseEvent& event);
	void PumpMessages();

	bool Show(bool show = true) override;
	bool Destroy() override;

private:
	void OnTick(wxTimerEvent& e);

	wxDECLARE_EVENT_TABLE();

private:
	wxPoint DesiredPosition;
	wxRichTextCtrl* LogCtrl = nullptr;
	size_t LastMessageIndex = 0;
	size_t DisplayedMessages = 0;
	wxTimer PollTimer;
};