#pragma once
#include <wx/msw/msvcrt.h>
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/snglinst.h>
#include <vector>
#include "RpcCom.h"
#include "PackageWindow.h"

#include <Tera/Core.h>
#include <Tera/AConfiguration.h>

#define WIN_POS_FULLSCREEN INT_MIN
#define WIN_POS_CENTER INT_MIN + 1

class wxEventHandler;
inline void SendEvent(wxEvtHandler* obj, wxEventType type)
{
  wxQueueEvent(obj, new wxCommandEvent(type));
}

inline void SendEvent(wxEvtHandler* obj, wxEventType type, const wxString& msg)
{
  wxCommandEvent* e = new wxCommandEvent(type);
  e->SetString(msg);
  wxQueueEvent(obj, e);
}

inline void SendEvent(wxEvtHandler* obj, wxEventType type, const wxString& msg, int32 value)
{
  wxCommandEvent* e = new wxCommandEvent(type);
  e->SetString(msg);
  e->SetInt(value);
  wxQueueEvent(obj, e);
}

wxDECLARE_EVENT(DELAY_LOAD, wxCommandEvent);
wxDECLARE_EVENT(OPEN_PACKAGE, wxCommandEvent);
wxDECLARE_EVENT(LOAD_CORE_ERROR, wxCommandEvent);
wxDECLARE_EVENT(OBJECT_LOADED, wxCommandEvent);

class ProgressWindow;
class App : public wxApp {
public:
  ~App();
  bool OpenPackage(const wxString& path);
  bool ShowOpenDialog(const wxString& rootDir = wxEmptyString);
  void OnOpenPackage(wxCommandEvent& e);

  void PackageWindowWillClose(const PackageWindow* frame);
  void OnRpcOpenFile(const wxString& path);

  void SetLastWindowPosition(const wxPoint& pos);
  wxPoint GetLastWindowPosition() const;

  FAppConfig& GetConfig()
  {
    return Config;
  }

private:
  bool OnInit();
  int OnRun();
  int OnExit();
  void OnInitCmdLine(wxCmdLineParser& parser);
  void OnLoadError(wxCommandEvent& e);
  bool OnCmdLineParsed(wxCmdLineParser& parser);
  void OnObjectLoaded(wxCommandEvent& e);

  // Build DirCache and load class packages
  void LoadCore(ProgressWindow*);
  // Create windows for loaded packages
  void DelayLoad(wxCommandEvent&);
  // Ask user for the S1Game/CookedPC path
  std::string RequestRootDir();

  wxDECLARE_EVENT_TABLE();
private:
  FAppConfig Config;
  wxPoint LastWindowPosition = wxDefaultPosition;
  wxSingleInstanceChecker* InstanceChecker = nullptr;
  RpcServer* Server = nullptr;
  bool IsReady = false;
  std::vector<PackageWindow*> PackageWindows;
  std::vector<wxString> OpenList;
  std::vector<std::shared_ptr<FPackage>> OpenPackages;
};

