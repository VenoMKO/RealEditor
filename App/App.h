#pragma once
#define WXDEBUG 1
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <wx/msw/msvcrt.h>
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/snglinst.h>
#include <vector>
#include "Misc/RpcCom.h"
#include "Misc/WXDialog.h"
#include "Windows/PackageWindow.h"

#include <Tera/Core.h>
#include <Utils/AConfiguration.h>
#include <Utils/ALDevice.h>

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

extern wxString GetS1Game32Path();

wxDECLARE_EVENT(DELAY_LOAD, wxCommandEvent);
wxDECLARE_EVENT(OPEN_PACKAGE, wxCommandEvent);
wxDECLARE_EVENT(LOAD_CORE_ERROR, wxCommandEvent);
wxDECLARE_EVENT(OBJECT_LOADED, wxCommandEvent);
wxDECLARE_EVENT(REGISTER_MIME, wxCommandEvent);
wxDECLARE_EVENT(UNREGISTER_MIME, wxCommandEvent);
wxDECLARE_EVENT(SHOW_FINAL_INIT, wxCommandEvent);

class ProgressWindow;
class BulkImportWindow;
class App 
  : public wxApp
  , public WXDialogObserver {
public:
  friend struct REScopedDialogCounter;
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
    return GetSharedApp()->GetConfig().GetLastFilePackagePath().WString();
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

  static ALDevice* GetSharedAudioDevice()
  {
    return GetSharedApp()->AudioDevice ? GetSharedApp()->AudioDevice : GetSharedApp()->InitAudioDevice();
  }

  static void AddRecentFile(const wxString& path);

  static void SavePackageSavePath(const wxString& path)
  {
    GetSharedApp()->GetConfig().LastPkgSavePath = path.ToStdWstring();
    GetSharedApp()->SaveConfig();
  }

  static void InstallS1Game32();

  ~App();
  bool OpenPackage(std::shared_ptr<FPackage> package);
  bool OpenPackage(const wxString& path, const wxString selection = wxEmptyString);
  bool OpenNamedPackage(const wxString& name, const wxString selection = wxEmptyString);
  wxString ShowOpenDialog(const wxString& rootDir = wxEmptyString);
  wxString ShowOpenCompositeDialog(wxWindow* parent);
  wxString ShowOpenByNameDialog(wxWindow* parent);
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
  void Restart(bool keepOpenList = false);
  void RestartElevated(bool keepWindows = true);

  void OnFatalException() override;

  void SetDcToolIsOpen(bool flag)
  {
    DcToolIsOpen = flag;
  }

  const wxArrayString& GetCompositePackageNames() const;

  bool CheckMimeTypes(bool strict) const;

  FAppConfig& GetConfig()
  {
    return Config;
  }

  void SaveConfig();

  bool IsShuttingDown() const
  {
    return ShuttingDown;
  }

  void SaveAndReopenPackage(std::shared_ptr<FPackage> package, const FString& tmp, const FString dest);

  void OnExitClicked();

  void DumpCompositeObjects();

  void OnDialogWillOpen(WXDialog* dialog) override;

  void OnDialogDidClose(WXDialog* dialog) override;

  void OnActivateApp(wxActivateEvent& e);

  wxFrame* GetLogConsole();

  void LogConsoleWillClose();

private:
  bool OnInit() override;
  int OnRun() override;
  int OnExit() override;
  void OnInitCmdLine(wxCmdLineParser& parser) override;
  void OnLoadError(wxCommandEvent& e);
  bool OnCmdLineParsed(wxCmdLineParser& parser) override;
  void OnObjectLoaded(wxCommandEvent& e);
  void ShowWelcomeBeforeExit(wxCommandEvent&);
  ALDevice* InitAudioDevice();
  class LogWindow* CreateLogConsole();
  void DecreaseREDialogsCount()
  {
    REDialogsCount--;
  }
  void IncreaseREDialogsCount()
  {
    REDialogsCount++;
  }

  // Build DirCache and load class packages
  void LoadCore(ProgressWindow*);
  // Create windows for loaded packages
  void DelayLoad(wxCommandEvent&);

  wxDECLARE_EVENT_TABLE();
private:
  FAppConfig Config;
  class WelcomeDialog* InitScreen = nullptr;
  class LogWindow* LogConsole = nullptr;
  BulkImportWindow* BulkImporter = nullptr;
  ALDevice* AudioDevice = nullptr;
  wxSingleInstanceChecker* InstanceChecker = nullptr;
  RpcServer* Server = nullptr;
  bool IsReady = false;
  bool ShowedStartupCfg = false;
  std::vector<PackageWindow*> PackageWindows;
  std::vector<wxString> OpenList;
  std::vector<WXDialog*> Dialogs;
  std::atomic_int REDialogsCount = { 0 };

  wxArrayString CompositePackageNames;
  wxArrayString FilePackageNames;

  bool NeedsRestart = false;
  bool ShuttingDown = false;
  bool DcToolIsOpen = false;
  bool NeedsInitialScreen = false;
  bool IgnoreWinClose = false;
};

struct REScopedDialogCounter {
  REScopedDialogCounter(const REScopedDialogCounter&) = delete;

  REScopedDialogCounter()
  {
    App::GetSharedApp()->IncreaseREDialogsCount();
  }

  ~REScopedDialogCounter()
  {
    App::GetSharedApp()->DecreaseREDialogsCount();
  }
};