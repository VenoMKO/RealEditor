#include "../../App/Windows/LogWindow.h"
#include "../../App/App.h"
#include "Utils/ALog.h"

ALog* SharedLogger = nullptr;

wxPoint FIntPointToPoint(const FIntPoint& v)
{
  const int x = v.X;
  const int y = v.Y;
  if (x == -1 && y == -1)
  {
    return wxDefaultPosition;
  }
  return wxPoint(x, y);
}

ALog* ALog::SharedLog()
{
  if (!SharedLogger)
  {
    SharedLogger = new ALog();
  }
  return SharedLogger;
}

void ALog::Log(const std::string& msg, ALogEntry::Type channel)
{
  if (SharedLogger)
  {
    SharedLogger->Push({ std::time(nullptr), msg, channel });
  }
}

void ALog::ILog(const std::string& msg)
{
  Log(msg, ALogEntry::Type::INFO);
}

void ALog::ELog(const std::string& msg)
{
  Log(msg, ALogEntry::Type::ERR);
}

void ALog::WLog(const std::string& msg)
{
  Log(msg, ALogEntry::Type::WARN);
}

void ALog::GetConfig(FLogConfig& cfg)
{
  ALog::SharedLog()->_GetConfig(cfg);
}

void ALog::SetConfig(const FLogConfig& cfg)
{
  ALog::SharedLog()->_SetConfig(cfg);
}

void ALog::_GetConfig(FLogConfig& cfg)
{
  ALog* l = ALog::SharedLog();
  cfg.LogRect.Min = l->LastPosition;
  cfg.LogRect.Max = l->LastSize;
}

void ALog::_SetConfig(const FLogConfig& cfg)
{
  ALog::SharedLog();
  std::scoped_lock<std::recursive_mutex> locker(WLocker);
  LastPosition = cfg.LogRect.Min;
  LastSize = cfg.LogRect.Max;
  if (Window)
  {
    Window->SetPosition(FIntPointToPoint(cfg.LogRect.Min));
    Window->SetSize(wxSize(cfg.LogRect.Max.X, cfg.LogRect.Max.Y));
  }
}

ALog::ALog()
{
}

void ALog::Show(bool show)
{
  ALog::SharedLog()->_Show(show);
}

bool ALog::IsShown()
{
  return ALog::SharedLog()->_IsShown();
}

bool ALog::_IsShown()
{
  std::scoped_lock<std::recursive_mutex> locker(WLocker);
  return Window ? Window->IsShown() && !Window->IsIconized() : false;
}

void ALog::_Show(bool show)
{
  std::scoped_lock<std::recursive_mutex> lock(WLocker);
  if (Window)
  {
    if (!show)
    {
      Window->Close();
    }
    else if (Window->IsIconized())
    {
      Window->Restore();
    }
  }
  else if (show)
  {
    Window = new LogWindow(nullptr, wxID_ANY, FIntPointToPoint(LastPosition), wxSize(LastSize.X, LastSize.Y));
    Window->SetLogger(this);
    Window->SetPosition(FIntPointToPoint(LastPosition));
    Window->SetSize(wxSize(LastSize.X, LastSize.Y));
    Window->Show(show);
    UpdateWindow();
  }
}

void ALog::OnLogClose()
{
  std::scoped_lock<std::recursive_mutex> lock(WLocker);
  LastPosition.X = Window->GetPosition().x;
  LastPosition.Y = Window->GetPosition().y;
  LastSize.X = Window->GetSize().x;
  LastSize.Y = Window->GetSize().y;
  Window = nullptr;
}

void ALog::OnAppExit()
{
  if (SharedLogger)
  {
    delete SharedLogger;
    SharedLogger = nullptr;
  }
}

void ALog::GetEntries(std::vector<ALogEntry>& output, size_t& index)
{
  std::scoped_lock<std::recursive_mutex> lock(ELocker);
  while (index < Entries.size())
  {
    output.push_back(Entries[index]);
    index++;
  }
}

void ALog::Push(const ALogEntry& entry)
{
  {
    std::scoped_lock<std::recursive_mutex> lock(ELocker);
    Entries.push_back(entry);
  }
  std::scoped_lock<std::recursive_mutex> lock(WLocker);
  if (Window)
  {
    UpdateWindow();
  }
}

void ALog::UpdateWindow()
{
  std::scoped_lock<std::recursive_mutex> lock(WLocker);
  if (Window)
  {
    SendEvent(Window, PUMP_LOG_WINDOW);
  }
}
