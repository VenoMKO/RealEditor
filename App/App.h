#pragma once
#define WXDEBUG 1
#include <wx/msw/msvcrt.h>
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/snglinst.h>
#include <vector>
#include "Misc/RpcCom.h"
#include "Windows/PackageWindow.h"

#include <Tera/Core.h>
#include <Utils/AConfiguration.h>

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

inline void SendEvent(wxEvtHandler* obj, wxEventType type, int32 number)
{
  wxCommandEvent* e = new wxCommandEvent(type);
  e->SetInt(number);
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
wxDECLARE_EVENT(REGISTER_MIME, wxCommandEvent);
wxDECLARE_EVENT(UNREGISTER_MIME, wxCommandEvent);

class ProgressWindow;
class BulkImportWindow;
class App : public wxApp {
public:
  static App* GetSharedApp()
  {
    return (App*)wxTheApp;
  }

  static wxString GetExportPath()
  {
    return GetSharedApp()->GetConfig().LastExportPath.WString();
  }

  static wxString GetImportPath()
  {
    return GetSharedApp()->GetConfig().LastImportPath.WString();
  }

  static wxString GetPackageOpenPath()
  {
    return GetSharedApp()->GetConfig().LastPkgOpenPath.WString();
  }

  static wxString GetPackageSavePath()
  {
    return GetSharedApp()->GetConfig().LastPkgSavePath.WString();
  }

  static void SaveExportPath(const wxString& path)
  {
    GetSharedApp()->GetConfig().LastExportPath = path.ToStdWstring();
    GetSharedApp()->SaveConfig();
  }

  static void SaveImportPath(const wxString& path)
  {
    GetSharedApp()->GetConfig().LastImportPath = path.ToStdWstring();
    GetSharedApp()->SaveConfig();
  }

  static void SavePackageOpenPath(const wxString& path)
  {
    GetSharedApp()->GetConfig().LastPkgOpenPath = path.ToStdWstring();
    GetSharedApp()->SaveConfig();
  }

  static void SavePackageSavePath(const wxString& path)
  {
    GetSharedApp()->GetConfig().LastPkgSavePath = path.ToStdWstring();
    GetSharedApp()->SaveConfig();
  }

  ~App();
  bool OpenPackage(const wxString& path, const wxString selection = wxEmptyString);
  bool OpenNamedPackage(const wxString& name, const wxString selection = wxEmptyString);
  wxString ShowOpenDialog(const wxString& rootDir = wxEmptyString);
  wxString ShowOpenCompositeDialog(wxWindow* parent);
  void ShowBulkImport(wxWindow* parent, const wxString& className = wxEmptyString, const wxString& objectName = wxEmptyString);
  void ResetBulkImport()
  {
    BulkImporter = nullptr;
  }
  void OnOpenPackage(wxCommandEvent& e);
  void OnShowSettings(wxCommandEvent& e);
  PackageWindow* GetPackageWindow(FPackage* package) const;

  void PackageWindowWillClose(const PackageWindow* frame);
  void OnRpcOpenFile(const wxString& path);

  void SetLastWindowPosition(const wxPoint& pos);
  void SetLastWindowSize(const wxSize& size);
  void SetLastWindowObjectSash(const int32& sash, const int32& width);
  void SetLastWindowPropertiesSash(const int32& sash, const int32& width);

  wxPoint GetLastWindowPosition() const;
  wxSize GetLastWindowSize() const;
  wxSize GetLastWindowObjectSash() const;
  wxSize GetLastWindowPropSash() const;

  void OnRegisterMime(wxCommandEvent&);
  void OnUnregisterMime(wxCommandEvent&);

  void OnFatalException() override;

  const wxArrayString& GetCompositePackageNames() const;

  bool CheckMimeTypes() const;

  FAppConfig& GetConfig()
  {
    return Config;
  }

  void SaveConfig();

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

  wxDECLARE_EVENT_TABLE();
private:
  FAppConfig Config;
  BulkImportWindow* BulkImporter = nullptr;
  wxSingleInstanceChecker* InstanceChecker = nullptr;
  RpcServer* Server = nullptr;
  bool IsReady = false;
  bool ShowedStartupCfg = false;
  std::vector<PackageWindow*> PackageWindows;
  std::vector<wxString> OpenList;

  wxArrayString CompositePackageNames;

  bool NeedsRestart = false;
};

