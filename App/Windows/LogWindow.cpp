#include "../App.h"
#include "LogWindow.h"
#include <wx/richtext/richtextctrl.h>

#include <Utils/ALog.h>
#include <Utils/AConfiguration.h>

#define POLL_INTERVAL 1000
#define MAX_LINES 500
#define CLEAN_LINES 400

LogWindow::LogWindow(const wxPoint& pos, const wxSize& size)
  : wxFrame(nullptr, wxID_ANY, wxTheApp->GetAppDisplayName() + wxT(" ") + GetAppVersion() + wxT(" - Log"), pos, size, wxCAPTION | wxSTAY_ON_TOP | wxCLOSE_BOX | wxFRAME_TOOL_WINDOW | wxTAB_TRAVERSAL)
{
  DesiredPosition = pos;
  SetSize(FromDIP(GetSize()));
  SetIcon(wxICON(#114));
  SetSizeHints(GetSize(), wxDefaultSize);

  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
  LogCtrl = new wxRichTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxVSCROLL | wxHSCROLL | wxNO_BORDER | wxWANTS_CHARS);
  LogCtrl->SetBackgroundColour(wxColour(16, 16, 16));
  bSizer1->Add(LogCtrl, 1, wxEXPAND | wxALL, 0);

  SetSizer(bSizer1);
  Layout();

  Centre(wxBOTH);

  PollTimer.Bind(wxEVT_TIMER, &LogWindow::OnTick, this);
  PollTimer.Start(POLL_INTERVAL);
}

void LogWindow::OnCloseWindow(wxCloseEvent& event)
{
  if (!App::GetSharedApp()->IsShuttingDown())
  {
    FAppConfig& config = App::GetSharedApp()->GetConfig();
    config.LogConfig.ShowLog = false;
    App::GetSharedApp()->SaveConfig();
  }
  wxFrame::OnCloseWindow(event);
}

void LogWindow::PumpMessages()
{
  std::vector<ALogEntry> entries;
  if (DisplayedMessages > MAX_LINES)
  {
    DisplayedMessages = 0;
    LogCtrl->Clear();
    LastMessageIndex -= CLEAN_LINES;
  }
  ALog::SharedLog()->GetEntries(entries, LastMessageIndex);

  if (entries.empty())
  {
    return;
  }

  LogCtrl->Freeze();
  LogCtrl->SetInsertionPointEnd();
  size_t eIdx = 0;
  if (entries.size() > MAX_LINES)
  {
    eIdx = entries.size() - CLEAN_LINES;
  }
  for (; eIdx < entries.size(); ++eIdx)
  {
    DisplayedMessages++;
    ALogEntry& e = entries[eIdx];
    std::string msg = e.Text;
    if (msg.back() != '\n')
    {
      msg += "\n";
    }
    std::tm* tm = std::localtime(&e.Time);
    char buffer[32];
    std::strftime(buffer, 32, "[%H:%M:%S] ", tm);
    msg = std::string(buffer) + msg;
    switch (e.Channel)
    {
    case ALogEntry::Type::ERR:
      LogCtrl->BeginTextColour(wxColour(255, 30, 30));
      break;
    case ALogEntry::Type::WARN:
      LogCtrl->BeginTextColour(wxColour(255, 120, 0));
      break;
    case ALogEntry::Type::INFO:
    default:
      LogCtrl->BeginTextColour(wxColour(160, 160, 160));
      break;
    }
    LogCtrl->WriteText(A2W(msg));
    LogCtrl->EndTextColour();
  }
  LogCtrl->Thaw();
  LogCtrl->ScrollIntoView(LogCtrl->GetCaretPosition(), WXK_PAGEDOWN);
}

void LogWindow::OnTick(wxTimerEvent& e)
{
  PumpMessages();
}

bool LogWindow::Show(bool show)
{
  if (show && DesiredPosition.x && DesiredPosition.y)
  {
    SetPosition(DesiredPosition);
    DesiredPosition = wxPoint(0, 0);
  }
  bool result = wxFrame::Show(show);
  if (show)
  {
    PumpMessages();
    if (!MonitorFromWindow(GetHandle(), MONITOR_DEFAULTTONULL))
    {
      Center();
    }
  }
  return result;
}

bool LogWindow::Destroy()
{
  App::GetSharedApp()->LogConsoleWillClose();
  return wxFrame::Destroy();
}

wxBEGIN_EVENT_TABLE(LogWindow, wxFrame)
EVT_CLOSE(OnCloseWindow)
wxEND_EVENT_TABLE()
