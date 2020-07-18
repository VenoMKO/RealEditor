#pragma once
#include <wx/msw/msvcrt.h>
#include <wx/wx.h>
#include <wx/snglinst.h>
#include <vector>
#include "RpcCom.h"
#include "PackageWindow.h"

#include <Tera/Core.h>
#include <Tera/AConfiguration.h>

#define WIN_POS_FULLSCREEN INT_MIN
#define WIN_POS_CENTER INT_MIN + 1

wxDECLARE_EVENT(DELAY_LOAD, wxCommandEvent);
wxDECLARE_EVENT(OPEN_PACKAGE, wxCommandEvent);

class ProgressWindow;
class App : public wxApp {
public:
  ~App();
  bool OpenPackage(const wxString& path);
  bool OpenDialog(const wxString& rootDir = wxEmptyString);
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
  bool OnCmdLineParsed(wxCmdLineParser& parser);

  // Build DirCache and load class packages
  void LoadCore(ProgressWindow*);
  // Create windows for loaded packages
  void DelayLoad(wxCommandEvent&);
  // Ask user for the S1Game/CookedPC path
  wxString RequestS1GameFolder();

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

