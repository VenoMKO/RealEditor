#pragma once
#include <wx/msw/msvcrt.h>
#include <wx/wx.h>
#include <wx/snglinst.h>
#include <vector>
#include "RpcCom.h"
#include "PackageWindow.h"

#include <Tera/AConfiguration.h>


#define WIN_POS_FULLSCREEN INT_MIN
#define WIN_POS_CENTER INT_MIN + 1

class App 
  : public wxApp
{
public:
  ~App();
  bool OpenPackage(const wxString& path);
  bool OpenDialog(const wxString& rootDir = wxEmptyString);

  void PackageWindowWillClose(const PackageWindow* frame);
  void OnRpcOpenFile(const wxString& path);

  void SetLastWindowPosition(const wxPoint& pos);
  wxPoint GetLastWindowPosition() const;

private:
  bool OnInit();
  int OnRun();
  int OnExit();
  void OnInitCmdLine(wxCmdLineParser& parser);
  bool OnCmdLineParsed(wxCmdLineParser& parser);

  wxString RequestS1GameFolder();

private:
  FConfig Config;
  wxPoint LastWindowPosition = wxDefaultPosition;
  wxSingleInstanceChecker* InstanceChecker = NULL;
  RpcServer* Server = NULL;
  bool IsReady = false;
  std::vector<PackageWindow*> PackageWindows;
};

