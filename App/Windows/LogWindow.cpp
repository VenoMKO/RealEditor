#include "../App.h"
#include "LogWindow.h"
#include <wx/richtext/richtextctrl.h>

#include <Utils/ALog.h>
#include <Utils/AConfiguration.h>

wxDEFINE_EVENT(PUMP_LOG_WINDOW, wxCommandEvent);

#define MAX_LINES 100000
#define CLEAN_LINES 70000

LogWindow::LogWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
  : wxFrame(parent, id, wxTheApp->GetAppDisplayName() + wxT(" ") + GetAppVersion() + wxT(" - Log"), pos, size, style)
{
  SetIcon(wxICON(#114));
  SetSizeHints(wxSize(700, 300), wxDefaultSize);

  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
  LogCtrl = new wxRichTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxVSCROLL | wxHSCROLL | wxNO_BORDER | wxWANTS_CHARS);
  LogCtrl->SetBackgroundColour(wxColour(16, 16, 16));
  bSizer1->Add(LogCtrl, 1, wxEXPAND | wxALL, 0);

  SetSizer(bSizer1);
  Layout();

  Centre(wxBOTH);
}

LogWindow::~LogWindow()
{
}

void LogWindow::OnCloseWindow(wxCloseEvent& event)
{
  if (!App::GetSharedApp()->IsShuttingDown())
  {
    FAppConfig& config = App::GetSharedApp()->GetConfig();
    config.LogConfig.ShowLog = false;
    App::GetSharedApp()->SaveConfig();
  }
  Logger->OnLogClose();
  wxFrame::OnCloseWindow(event);
}

void LogWindow::PumpMessages(wxCommandEvent&)
{
  std::vector<ALogEntry> entries;
  Logger->GetEntries(entries, LastMessageIndex);
  LogCtrl->Freeze();
  LogCtrl->SetInsertionPointEnd();
  if (LogCtrl->GetValue().size() >= MAX_LINES)
  {
    LogCtrl->SetValue(LogCtrl->GetValue().substr(CLEAN_LINES));
  }
  for (ALogEntry& e : entries)
  {
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

bool LogWindow::Show(bool show /*= true*/)
{
  bool result = wxFrame::Show(show);
  if (show && !MonitorFromWindow(GetHandle(), MONITOR_DEFAULTTONULL))
  {
    Center();
  }
  return result;
}

wxBEGIN_EVENT_TABLE(LogWindow, wxFrame)
EVT_COMMAND(wxID_ANY, PUMP_LOG_WINDOW, LogWindow::PumpMessages)
EVT_CLOSE(OnCloseWindow)
wxEND_EVENT_TABLE()