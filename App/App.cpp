#include "App.h"
#include "Windows/ProgressWindow.h"
#include "Windows/SettingsWindow.h"
#include "Windows/CompositePackagePicker.h"
#include "Windows/BulkImportWindow.h"
#include "Windows/REDialogs.h"
#include "Windows/DcToolDialog.h"
#include "Windows/WelcomeDialog.h"

#include <wx/mimetype.h>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/fileconf.h>
#include <wx/mstream.h>

#include <filesystem>
#include <execution>

#include <sys/stat.h>

#include <Utils/ALog.h>
#include <Tera/FPackage.h>
#include <Tera/FStream.h>
#include <Utils/ALDevice.h>

#if BNS
const char* APP_NAME = "BnS Editor";
const char* VENDOR_NAME_32 = "BnS Editor";
#else
const char* APP_NAME = "Real Editor";
const char* VENDOR_NAME_32 = "Real Editor x32";
const char* VENDOR_NAME_64 = "Real Editor x64";
#endif

wxIMPLEMENT_APP(App);

wxDEFINE_EVENT(DELAY_LOAD, wxCommandEvent);
wxDEFINE_EVENT(OPEN_PACKAGE, wxCommandEvent);
wxDEFINE_EVENT(LOAD_CORE_ERROR, wxCommandEvent);
wxDEFINE_EVENT(OBJECT_LOADED, wxCommandEvent);
wxDEFINE_EVENT(REGISTER_MIME, wxCommandEvent);
wxDEFINE_EVENT(UNREGISTER_MIME, wxCommandEvent);
wxDEFINE_EVENT(SHOW_FINAL_INIT, wxCommandEvent);

wxString GetConfigPath()
{
  wxString path = wxStandardPaths::Get().GetUserLocalDataDir() + wxFILE_SEP_PATH;
  if (!wxDirExists(path))
  {
    wxMkDir(path);
  }
  path += wxS("RE.cfg");
  return path;
}

wxString GetS1Game32Path()
{
  wxString path = wxStandardPaths::Get().GetUserLocalDataDir() + wxFILE_SEP_PATH + wxS("x86") + wxFILE_SEP_PATH;
  if (!wxDirExists(path))
  {
    wxMkdir(path);
  }
  path += wxS("S1Game");
  if (!wxDirExists(path))
  {
    wxMkdir(path);
  }
  wxString cookedPc = path + wxFILE_SEP_PATH + wxS("CookedPC");
  if (!wxDirExists(cookedPc))
  {
    wxMkdir(cookedPc);
  }
  return path;
}

inline wxString wxFrameToString(const wxPoint& pos, const wxSize& size)
{
  wxString result = wxT("$FRAME$");
  result << pos.x << L':' << pos.y << L':' << size.x << L':' << size.y << L'$';
  return result;
}

inline void wxStringToFrame(const wxString& str, wxPoint& position, wxSize& size)
{
  auto pos = str.find('$', 7);
  if (pos == wxString::npos)
  {
    return;
  }
  position.x = wxAtoi(str.Mid(7, pos - 7));
  auto epos = str.find(':', pos + 1);
  if (epos == wxString::npos)
  {
    return;
  }
  position.y = wxAtoi(str.Mid(pos + 1, epos - pos - 1));
  pos = str.find(':', epos + 1);
  if (pos == wxString::npos)
  {
    return;
  }
  size.x = wxAtoi(str.Mid(epos + 1, pos - epos - 1));
  epos = str.find('$', pos + 1);
  if (epos == wxString::npos)
  {
    return;
  }
  size.y = wxAtoi(str.Mid(pos + 1, epos - pos - 1));
}

wxString UnpackGpkPath(const wxString& path, wxString& frameInfo, wxString& selection)
{
  auto pos = path.find("$FRAME$");
  wxString fixedPath = path;
  if (pos != wxString::npos)
  {
    auto pos2 = path.find('$', pos + 7);
    frameInfo = path.Mid(pos, pos2 - pos + 1);
    fixedPath.Replace(frameInfo, wxEmptyString);
  }
  pos = fixedPath.find("$SEL$");
  if (pos != wxString::npos)
  {
    auto pos2 = fixedPath.find('$', pos + 5);
    wxString tmpSelection = fixedPath.Mid(pos, pos2 - pos + 1);
    fixedPath.Replace(tmpSelection, wxEmptyString);
    tmpSelection = tmpSelection.Mid(5, tmpSelection.size() - 6);
    if (tmpSelection.size())
    {
      selection = tmpSelection;
    }
  }
  return fixedPath;
}

wxString PackGpkPath(const wxString& path, const wxPoint& pos, const wxSize& size, const wxString& selection)
{
  wxString packed = path;
  packed += wxFrameToString(pos, size);
  packed += wxT("$SEL$") + selection + wxT("$");
  return packed;
}

void RegisterFileType(const wxString& extension, const wxString& description, const wxString& appPath, wxMimeTypesManager& man)
{
  wxFileTypeInfo info = wxFileTypeInfo("application/octet-stream");
  info.AddExtension(extension);
  info.SetDescription(description);
  info.SetOpenCommand(wxS("\"") + appPath + wxS("\""));
  info.SetIcon(appPath, -115);
  if (wxFileType* type = man.Associate(info))
  {
    delete type;
  }
}

void UnregisterFileType(const wxString& extension, wxMimeTypesManager& man)
{
  if (wxFileType* type = man.GetFileTypeFromExtension(extension))
  {
    man.Unassociate(type);
    delete type;
  }
}

bool CheckFileType(const wxString& appPath, const wxString& extension, wxMimeTypesManager& man)
{
  if (wxFileType* type = man.GetFileTypeFromExtension(extension))
  {
    wxString cmd = type->GetOpenCommand(wxT("test") + extension);
    delete type;
    return cmd.size() ? appPath.size() ? cmd.Mid(1).StartsWith(appPath) : true : false;
  }
  return false;
}

void LoadMeta(const wxString& source, std::unordered_map<FString, std::unordered_map<FString, AMetaDataEntry>>& output)
{
  wxString configText;
  {
    wxTextFile tfile;
    tfile.Open(source);
    configText = tfile.GetFirstLine();
    while (!tfile.Eof())
    {
      configText += wxT('\n') + tfile.GetNextLine();
    }
    configText.Replace(wxT("\""), wxT("'"));
  }
  if (configText.empty())
  {
    LogE("Failed to load stripped meta data!");
    return;
  }
  wxMemoryInputStream s((const void*)configText.c_str(), configText.size());
  wxMBConvUTF8 conv;
  wxFileConfig config(s, conv);
  wxString groupName;
  long groupIndex = 0;
  bool hasGroup = config.GetFirstGroup(groupName, groupIndex);
  if (!hasGroup)
  {
    LogE("Failed to parse stripped meta data!");
    return;
  }
  while (hasGroup)
  {
    config.SetPath(groupName);
    wxString entryName;
    long entryIndex = 0;
    bool hasEntry = config.GetFirstEntry(entryName, entryIndex);
    while (hasEntry)
    {
      wxString value = config.Read(entryName, "");
      int pos = entryName.Find('.');
      if (pos > 0)
      {
        output[groupName.ToStdWstring()][entryName.substr(0, pos).ToStdWstring()].Tooltip = value.ToStdWstring();
      }
      else
      {
        output[groupName.ToStdWstring()][entryName.ToStdWstring()].Name = value.ToStdWstring();
      }
      hasEntry = config.GetNextEntry(entryName, entryIndex);
    }
    config.SetPath("/");
    hasGroup = config.GetNextGroup(groupName, groupIndex);
  }
}

void App::OnRpcOpenFile(const wxString& path)
{
  if (path.size() && InitScreen)
  {
    InitScreen->OnExternalOpen(path);
  }
  else if (IsReady && !path.empty())
  {
    wxCommandEvent* event = new wxCommandEvent(OPEN_PACKAGE);
    event->SetString(path);
    wxQueueEvent(this, event);
  }
  if (wxWindow* top = GetTopWindow())
  {
    top->Raise();
  }
}

void App::SetLastWindowPosition(const wxPoint& pos)
{
  Config.WindowRect.Min = { pos.x, pos.y };
}

void App::SetLastWindowSize(const wxSize& size)
{
  Config.WindowRect.Max = { size.x, size.y };
}

void App::SetLastWindowObjectSash(const int32& sash, const int32& width)
{
  Config.SashPos.Min.X = sash;
  Config.SashPos.Min.Y = width;
}

void App::SetLastWindowPropertiesSash(const int32& sash, const int32& width)
{
  if (!width)
  {
    return;
  }
  Config.SashPos.Max.X = sash;
  Config.SashPos.Max.Y = width;
}

wxPoint App::GetLastWindowPosition() const
{
  return wxPoint(Config.WindowRect.Min.X, Config.WindowRect.Min.Y);
}

wxSize App::GetLastWindowSize() const
{
  return wxSize(Config.WindowRect.Max.X, Config.WindowRect.Max.Y);
}

wxSize App::GetLastWindowObjectSash() const
{
  return wxSize(Config.SashPos.Min.X, Config.SashPos.Min.Y);
}

wxSize App::GetLastWindowPropSash() const
{
  return wxSize(Config.SashPos.Max.X, Config.SashPos.Max.Y);
}

void App::OnOpenPackage(wxCommandEvent& e)
{
  wxString path = e.GetString();
  if (OpenPackage(path) && path.length())
  {
    App::AddRecentFile(path.ToStdWstring());
  }
}

void App::OnShowSettings(wxCommandEvent& e)
{
  FAppConfig newConfig;
  SettingsWindow win(Config, newConfig, true);
  if (win.ShowModal() == wxID_OK)
  {
    if (Config.RootDir.Size() && Config.RootDir != newConfig.RootDir)
    {
      wxMessageDialog dialog(nullptr, wxT("Application must restart to apply changes.\nClick \"OK\" to restart!"), wxT("Restart ") + GetAppDisplayName() + wxT("?"), wxOK | wxCANCEL | wxICON_EXCLAMATION);
      if (dialog.ShowModal() != wxID_OK)
      {
        return;
      }

      Config = newConfig;
      Config.LastPkgOpenPath.Clear();
      Config.LastFilePackages.clear();
      AConfiguration cfg = AConfiguration(W2A(GetConfigPath().ToStdWstring()));
      cfg.SetConfig(Config);
      cfg.Save();

      Restart();
      return;
    }
    Config = newConfig;
  }
}

PackageWindow* App::GetPackageWindow(FPackage* package) const
{
  if (!package)
  {
    return nullptr;
  }
  for (PackageWindow* window : PackageWindows)
  {
    if (window->GetPackage().get() == package)
    {
      return window;
    }
  }
  return nullptr;
}

void App::AddRecentFile(const wxString& path)
{
  wxString frameInfo;
  wxString selection;
  wxString cleanPath = UnpackGpkPath(path, frameInfo, selection);
  GetSharedApp()->GetConfig().AddLastFilePackagePath(cleanPath.ToStdWstring());
  GetSharedApp()->SaveConfig();
  for (PackageWindow* window : GetSharedApp()->PackageWindows)
  {
    window->UpdateRecent(cleanPath);
  }
}

void App::InstallS1Game32()
{
  wxString path = GetS1Game32Path() + wxFILE_SEP_PATH;

  auto InstallResource = [&](int resId, const char* name) {
    HRSRC hRes = FindResource(0, MAKEINTRESOURCE(resId), RT_RCDATA);
    HGLOBAL hMem = LoadResource(0, hRes);
    void* pMem = LockResource(hMem);
    DWORD size = SizeofResource(0, hRes);

    std::wstring item = path + L"CookedPC\\" + A2W(name);
    struct _stat64 stat;
    if (_wstat64(item.c_str(), &stat) || stat.st_size != size)
    {
      std::ofstream s(item.c_str(), std::ios::out | std::ios::binary);
      s.write((const char*)pMem, size);
    }

    UnlockResource(hMem);
    FreeResource(hMem);
  };

  InstallResource(500, "Core.u");
  InstallResource(501, "Engine.u");
  InstallResource(502, "S1Game.u");
  InstallResource(503, "GameFramework.u");
  InstallResource(504, "Editor.u");
  InstallResource(505, "GFxUI.u");
  InstallResource(506, "GFxUIEditor.u");
  InstallResource(507, "IpDrv.u");
  InstallResource(508, "UnrealEd.u");
}

App::~App()
{
  delete AudioDevice;
  delete InstanceChecker;
  delete Server;
}

wxString App::ShowOpenDialog(const wxString& rootDir)
{
  return IODialog::OpenPackageDialog(nullptr, rootDir);
}

wxString App::ShowOpenCompositeDialog(wxWindow* parent)
{
  CompositePackagePicker picker(parent, "Open composite package...");
  picker.CenterOnParent();
  if (picker.ShowModal() == wxID_OK)
  {
    return picker.GetResult();
  }
  return wxString();
}

wxString App::ShowOpenByNameDialog(wxWindow* parent)
{
  CompositePackagePicker picker(parent, "Open package...", true);
  picker.CenterOnParent();
  while (picker.ShowModal() == wxID_OK)
  {
    wxString result = picker.GetResult();
    bool exists = FPackage::NamedPackageExists(result.ToStdWstring(), false);
    if (!exists)
    {
      ProgressWindow progress(nullptr);
      progress.SetActionText(wxS("Looking for the package..."));
      progress.SetCurrentProgress(-1);
      progress.SetCanCancel(false);
      std::thread([&]{
        exists = FPackage::NamedPackageExists(result.ToStdWstring(), true);
        SendEvent(&progress, UPDATE_PROGRESS_FINISH, exists);
      }).detach();
      progress.ShowModal();
    }
    if (exists)
    {
      return result;
    }
    REDialog::Error("The specified package file does not exists! Check the name or try a different one.", "Failed to open the package!");
  }
  return wxString();
}

void App::ShowBulkImport(wxWindow* parent, const wxString& className, const wxString& objectName)
{
  if (!BulkImporter)
  {
    BulkImporter = new BulkImportWindow(parent, className, objectName);
    BulkImporter->Show();
  }
  else
  {
    BulkImporter->SetFocus();
    BulkImporter->AddOperation(className, objectName);
  }
}

bool App::OpenPackage(const wxString& path, const wxString selectionIn)
{
  wxString frameInfo;
  wxString selection = selectionIn;
  wxString fixedName = UnpackGpkPath(path, frameInfo, selection);
  for (const auto window : PackageWindows)
  {
    if (window->GetPackagePath() == fixedName)
    {
      if (frameInfo.size())
      {
        wxPoint pos = window->GetPosition();
        wxSize size = window->GetSize();
        wxStringToFrame(frameInfo, pos, size);
        window->SetPosition(pos);
        window->SetSize(size);
      }
      window->Raise();
      if (selection.size())
      {
        window->SelectObject(selection);
      }
      return true;
    }
  }

  std::shared_ptr<FPackage> package = nullptr;
  try
  {
    package = FPackage::GetPackage(W2A(fixedName.ToStdWstring()));
  }
  catch (const std::exception& e)
  {
    LogE("Failed to open the package: %s", e.what());
    REDialog::Error(e.what(), "Failed to open the package!");
    return false;
  }
  catch (...)
  {
    LogE("Failed to open the package: Unexpected exception!");
    REDialog::Error("Unknown error.", "Failed to open the package!");
    return false;
  }

  if (package == nullptr)
  {
    LogE("Failed to open the package: Unknow error");
    REDialog::Error("Unknown error.", "Failed to open the package!");
    return false;
  }

  PackageWindow* window = new PackageWindow(package, this);
  if (frameInfo.size())
  {
    wxPoint pos = window->GetPosition();
    wxSize size = window->GetSize();
    wxStringToFrame(frameInfo, pos, size);
    window->SetPosition(pos);
    window->SetSize(size);
  }
  PackageWindows.push_back(window);
  window->Show();
  window->Raise();
  if (!package->IsReady())
  {
    std::thread([package, window]() {
      try
      {
        package->Load();
      }
      catch (const std::exception& e)
      {
        LogE("Failed to load the package: %s.", e.what());
        SendEvent(window, PACKAGE_ERROR, e.what());
        return;
      }
      catch (const char* msg)
      {
        LogE("Failed to load the package: %s.", msg);
        SendEvent(window, PACKAGE_ERROR, wxString("Failed to load the package: ") + msg);
        return;
      }
      catch (...)
      {
        LogE("Failed to load the package. Unexpected exception occurred!");
        SendEvent(window, PACKAGE_ERROR, "Failed to load the package. Unexpected exception occurred!");
        return;
      }
      if (!package->IsOperationCancelled())
      {
        SendEvent(window, PACKAGE_READY);
      }
    }).detach();
  }
  else
  {
    SendEvent(window, PACKAGE_READY);
    if (selection.size())
    {
      SendEvent(window, SELECT_OBJECT, selection);
    }
  }
  return true;
}

bool App::OpenNamedPackage(const wxString& name, const wxString selectionIn)
{
  bool composite = name.StartsWith("composite\\");
  wxString fixedName = name;
  if (composite)
  {
    fixedName = name.Mid(10);
  }
  else if (name.StartsWith("named\\"))
  {
    fixedName = name.Mid(6);
  }
  wxString frameInfo;
  wxString selection = selectionIn;
  fixedName = UnpackGpkPath(fixedName, frameInfo, selection);
  for (const auto window : PackageWindows)
  {
    if (window->GetPackage()->GetPackageName().ToUpper().String() == fixedName.Upper())
    {
      if (!window->GetPackage()->IsComposite())
      {
        App::AddRecentFile(window->GetPackage()->GetSourcePath().WString());
      }
      else
      {
        App::AddRecentFile(L"composite\\" + window->GetPackage()->GetCompositePath().WString());
      }
      if (frameInfo.size())
      {
        wxPoint pos = window->GetPosition();
        wxSize size = window->GetSize();
        wxStringToFrame(frameInfo, pos, size);
        window->SetPosition(pos);
        window->SetSize(size);
      }
      window->Raise();
      window->SelectObject(selection);
      return true;
    }
  }

  std::shared_ptr<FPackage> package = nullptr;
  try
  {
    package = FPackage::GetPackageNamed(fixedName.ToStdString());
  }
  catch (const std::exception& e)
  {
    LogE("Failed to open the package: %s", e.what());
    REDialog::Error(e.what(), "Failed to open the package!");
    return false;
  }
  catch (const char* msg)
  {
    LogE("Failed to open the package: %s.", msg);
    REDialog::Error(msg, "Failed to open the package!");
    return false;
  }
  catch (...)
  {
    LogE("Failed to open the package. Unexpected exception occurred!");
    REDialog::Error("Unexpected exception occurred!", "Failed to open the package!");
    return false;
  }

  if (package == nullptr)
  {
    LogE("Failed to open the package: Unknow error");
    wxString msg = "Possible solutions:\n";
    msg += " * Check the file name\n";
    msg += " * Press Window -> Settings and Rebuild Cache\n";
    msg += "If the package is composite:\n";
    msg += " * Open TMM and click Restore original .dat\n";
    msg += " * Rebuild ObjectDump.txt\n";
    REDialog::Error(msg, wxS("Package ") + fixedName + wxS(" not found!"));
    return false;
  }
  if (!package->IsComposite())
  {
    App::AddRecentFile(package->GetSourcePath().WString());
  }
  else
  {
    App::AddRecentFile(L"composite\\" + package->GetPackageName().WString());
  }
  PackageWindow* window = new PackageWindow(package, this);
  if (frameInfo.size())
  {
    wxPoint pos = window->GetPosition();
    wxSize size = window->GetSize();
    wxStringToFrame(frameInfo, pos, size);
    window->SetPosition(pos);
    window->SetSize(size);
  }
  PackageWindows.push_back(window);
  window->Show();

  if (!package->IsReady())
  {
    std::thread([package, window, selection]() {
      try
      {
        package->Load();
      }
      catch (const std::exception& e)
      {
        LogE("Failed to load the package. %s.", e.what());
        SendEvent(window, PACKAGE_ERROR, e.what());
        return;
      }
      catch (const char* msg)
      {
        LogE("Failed to load the package: %s.", msg);
        REDialog::Error(msg, "Failed to open the package!");
        return;
      }
      catch (...)
      {
        LogE("Failed to load the package. Unexpected exception occurred!");
        REDialog::Error("Unexpected exception occurred!", "Failed to load the package!");
        return;
      }
      if (!package->IsOperationCancelled())
      {
        SendEvent(window, PACKAGE_READY);
        SendEvent(window, SELECT_OBJECT, selection);
      }
    }).detach();
  }
  else
  {
    SendEvent(window, PACKAGE_READY);
    SendEvent(window, SELECT_OBJECT, selection);
  }
  return true;
}

void App::PackageWindowWillClose(const PackageWindow* frame)
{
  for (auto it = PackageWindows.begin(); it < PackageWindows.end(); it++)
  {
    if (*it == frame)
    {
      PackageWindows.erase(it);
      break;
    }
  }
  if (PackageWindows.empty() && !IgnoreWinClose)
  {
    SendEvent(this, SHOW_FINAL_INIT);
  }
}

bool App::OnInit()
{
  for (int32 idx = 0; idx < argc; ++idx)
  {
    if (_strcmpi(argv[idx], "-version") == 0)
    {
      {
        std::ofstream s("app_ver.txt");
        s << APP_VER_MAJOR << '.' << APP_VER_MINOR << std::endl;
      }
      exit(0);
    }
    if (_strcmpi(argv[idx], "-build") == 0)
    {
      {
        std::ofstream s("app_build.txt");
        s << BUILD_NUMBER << std::endl;
      }
      exit(0);
    }
    if (_strcmpi(argv[idx], "-s") == 0)
    {
      // This delay ensures that previous RE instance has returned
      // from the ShellExecuteEx, deleted its InstanceChecker and RpcServer
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
#ifdef _DEBUG
  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
  wxHandleFatalExceptions(true);
  // If the executable name changes our AppData storage path will change too. We don't want that.
  // So we set AppName manually. This will keep paths consistent.
  SetAppName(APP_NAME);

  // Update executable path if MIME is registered
  if (CheckMimeTypes(false))
  {
    // Check weather the app path matches the one in the registry
    if (!CheckMimeTypes(true))
    {
      // Update app path
      wxCommandEvent tmp;
      OnRegisterMime(tmp);
    }
  }

  InstallS1Game32();

  AConfiguration cfg = AConfiguration(W2A(GetConfigPath().ToStdWstring()));
  if (cfg.Load())
  {
    Config = cfg.GetConfig();
  }
  FPackage::S1DirError verr = FPackage::ValidateRootDirCandidate(Config.RootDir);
  if (verr == FPackage::S1DirError::ACCESS_DENIED)
  {
    if (REDialog::Auth())
    {
      App::GetSharedApp()->RestartElevated();
    }
    return false;
  }
  if (verr != FPackage::S1DirError::OK)
  {
    FAppConfig newConfig = Config;
    newConfig.RootDir.Clear();
    SettingsWindow win(Config, newConfig, false);
    if (win.ShowModal() != wxID_OK)
    {
      return false;
    }
    Config = newConfig;
    cfg.SetConfig(Config);
    cfg.Save();
    Config.WindowRect.Min = { WIN_POS_CENTER, 0 };
    ShowedStartupCfg = true;
  }
  else if (!Config.IsVersionGreaterOrEqual(1, 80) && CheckMimeTypes(false))
  {
    // Fix incorrect open path in the registry prior to 1.80
    try
    {
      wxCommandEvent tmp;
      OnRegisterMime(tmp);
    }
    catch (...)
    {
    }
  }
  InstanceChecker = new wxSingleInstanceChecker;
  ALog::SharedLog();
  ALog::SetConfig(Config.LogConfig);
  
  IsReady = true;
  
  return wxApp::OnInit();
}

int App::OnRun()
{
  if (IsReady)
  {
    IsReady = false;
    // Unexpected issue: Some GPKs have way too many imports.
    // Bulk exporting these GPKs leads to thousands of opened streams.
    // Proper fix would be to close FPackage::Stream after load
    // or to use MStreams instead.
    _setmaxstdio(8192);
    SetExitOnFrameDelete(false);
    wxInitAllImageHandlers();
    Server = new RpcServer;
    Server->Run();
    if (Config.LogConfig.ShowLog)
    {
      ALog::SharedLog()->Show();
    }

    if (OpenList.empty() || NeedsInitialScreen)
    {
      InitScreen = new WelcomeDialog(nullptr);
      InitScreen->Center();
      InitScreen->ShowModal();
      OpenList = InitScreen->GetOpenList();
      InitScreen->Destroy();
      InitScreen = nullptr;
      if (OpenList.empty())
      {
        return 0;
      }
    }
    
    ProgressWindow* progressWindow = new ProgressWindow(nullptr, APP_NAME);
    progressWindow->SetActionText(wxS("Loading..."));
    progressWindow->SetCurrentProgress(-1);
    progressWindow->SetCanCancel(false);
    progressWindow->Show();
    std::thread([this, progressWindow] { LoadCore(progressWindow); IsReady = true; }).detach();
    return wxApp::OnRun();
  }
  return 0;
}

void App::LoadCore(ProgressWindow* pWindow)
{
  PERF_START(LoadCore);
  FPackage::CleanCacheDir();
  SendEvent(pWindow, UPDATE_PROGRESS_DESC, "Enumerating S1Game folder contents...");
  FPackage::SetRootPath(Config.RootDir);
  if (pWindow->IsCanceled())
  {
    ExitMainLoop();
    return;
  }

  {

    wxString desc = wxS("Loading Core.u...");
    SendEvent(pWindow, UPDATE_PROGRESS_DESC, desc);
    try
    {
      FPackage::LoadClassPackage("Core.u");
    }
    catch (const std::exception& e)
    {
      SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
      SendEvent(this, LOAD_CORE_ERROR, "Failed to load Core.u file!\n\nThe file may be corrupted or does not exist.");
      return;
    }
    if (pWindow->IsCanceled())
    {
      ExitMainLoop();
      return;
    }
  }

  std::mutex mapperErrorMutex;
  FString mapperError;
  std::thread compositMapper;
  std::thread pkgMapper;
  std::thread objectRedirectorMapper;
  if (FPackage::GetCoreVersion() > VER_TERA_CLASSIC)
  {
#if !BNS
    SetAppDisplayName(VENDOR_NAME_64);
    SetVendorDisplayName(VENDOR_NAME_64);
#endif
    compositMapper = std::thread([&mapperErrorMutex, &mapperError] {
      try
      {
        FPackage::LoadCompositePackageMapper();
      }
      catch (const std::exception& e)
      {
        std::scoped_lock<std::mutex> l(mapperErrorMutex);
        if (mapperError.Empty())
        {
          mapperError = e.what();
        }
      }
    });

    pkgMapper = std::thread([&mapperErrorMutex, &mapperError] {
      try
      {
        FPackage::LoadPkgMapper();
      }
      catch (const std::exception& e)
      {
        std::scoped_lock<std::mutex> l(mapperErrorMutex);
        if (mapperError.Empty())
        {
          mapperError = e.what();
        }
      }
    });

    objectRedirectorMapper = std::thread([&mapperErrorMutex, &mapperError] {
      try
      {
        FPackage::LoadObjectRedirectorMapper();
      }
      catch (const std::exception& e)
      {
        std::scoped_lock<std::mutex> l(mapperErrorMutex);
        if (mapperError.Empty())
        {
          mapperError = e.what();
        }
      }
    });
  }
  else
  {
    SetAppDisplayName(VENDOR_NAME_32);
    SetVendorDisplayName(VENDOR_NAME_32);
  }

  SendEvent(pWindow, UPDATE_PROGRESS_DESC, "Loading stripped meta data...");
#ifndef _DEBUG
  if (FPackage::GetCoreVersion() > VER_TERA_CLASSIC)
  {
    std::unordered_map<FString, std::unordered_map<FString, AMetaDataEntry>> meta;
    try
    {
      LoadMeta(Config.RootDir.FStringByAppendingPath("..\\Engine\\Localization\\AutoGenerated.Properties"), meta);
      FPackage::SetMetaData(meta);
    }
    catch (const std::exception& e)
    {
      LogE("Can't load metadata: %s", e.what());
    }
    catch (...)
    {
      LogE("Can't load metadata!");
    }
  }
#endif

#if BNS
  const std::vector<FString> classPackageNames = { "Engine.u", "T1Game.u", "Editor.u", "UnrealEd.u"};
  const std::vector<FString> extraClassPackageNames;
#elif MINIMAL_CORE
  const std::vector<FString> classPackageNames = { "Engine.u", "S1Game.u", "GFxUI.u" };
  const std::vector<FString> extraClassPackageNames;
#else
  const std::vector<FString> classPackageNames = { "Engine.u", "GameFramework.u", "S1Game.u", "GFxUI.u", "IpDrv.u", "UnrealEd.u", "GFxUIEditor.u" };
  const std::vector<FString> extraClassPackageNames = { "WinDrv.u", "OnlineSubsystemPC.u" };
#endif

  PERF_START(ClassPackagesLoad);
  for (const FString& name : classPackageNames)
  {
    wxString desc = wxS("Loading ");
    desc += name.String() + "...";
    SendEvent(pWindow, UPDATE_PROGRESS_DESC, desc);
    try
    {
      FPackage::LoadClassPackage(name);
    }
    catch (...)
    {
      SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
      wxString errDesc = wxString::Format("Failed to load %s file!\n\nThe file may be corrupted or does not exist.", name.C_str());
      SendEvent(this, LOAD_CORE_ERROR, errDesc);
      return;
    }
    if (pWindow->IsCanceled())
    {
      ExitMainLoop();
      return;
    }
  }
  if (FPackage::GetCoreVersion() > VER_TERA_CLASSIC)
  {
    for (const FString& name : extraClassPackageNames)
    {
      wxString desc = wxS("Loading ");
      desc += name.String() + "...";
      SendEvent(pWindow, UPDATE_PROGRESS_DESC, desc);
      try
      {
        FPackage::LoadClassPackage(name);
      }
      catch (...)
      {
        // Don't care if we failed to load extra packages. They contain only runtime classes anyway.
      }
      if (pWindow->IsCanceled())
      {
        ExitMainLoop();
        return;
      }
    }
  }
  FPackage::BuildClassInheritance();
  PERF_END(ClassPackagesLoad);

#if 0
  // Don't need this
  SendEvent(pWindow, UPDATE_PROGRESS_DESC, "Loading persistent data...");
  try
  {
    FPackage::LoadPersistentData();
  }
  catch (const std::exception& e)
  {
    SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
    SendEvent(this, LOAD_CORE_ERROR, e.what());
    return;
  }
#endif

  if (pWindow->IsCanceled())
  {
    ExitMainLoop();
    return;
  }

  SendEvent(pWindow, UPDATE_PROGRESS_DESC, "Loading Mappers...");

  if (FPackage::GetCoreVersion() > VER_TERA_CLASSIC)
  {
    pkgMapper.join();
    compositMapper.join();
    objectRedirectorMapper.join();

    if (mapperError.Size())
    {
      SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
      SendEvent(this, LOAD_CORE_ERROR, mapperError.String());
      pWindow->Destroy();
      ExitMainLoop();
      return;
    }
  }

  if (FPackage::GetCoreVersion() > VER_TERA_CLASSIC)
  {
    const auto& compositeMap = FPackage::GetCompositePackageMap();
    CompositePackageNames.reserve(compositeMap.size());
    for (const auto& pair : compositeMap)
    {
      CompositePackageNames.push_back(pair.first.String());
    }

    if (pWindow->IsCanceled())
    {
      wxQueueEvent(this, new wxCloseEvent());
      pWindow->Destroy();
      ExitMainLoop();
      return;
    }
  }
  PERF_END(LoadCore);

  SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
  SendEvent(this, DELAY_LOAD);
  pWindow->Destroy();
}

void App::DelayLoad(wxCommandEvent& e)
{
  bool anyLoaded = false;
  bool needsDcTool = false;
  bool needsObjDump = false;
  for (const wxString& path : OpenList)
  {
    if (path.StartsWith("composite\\") || path.StartsWith("named\\"))
    {
      if (OpenNamedPackage(path))
      {
        anyLoaded = true;
      }
    }
    else if (path == "DCTOOL")
    {
      needsDcTool = true;
    }
    else if (path == "OBJDUMP")
    {
      needsObjDump = true;
    }
    else if (OpenPackage(path))
    {
      anyLoaded = true;
      App::AddRecentFile(path);
    }
  }
  OpenList.clear();
  if (needsDcTool)
  {
    DcToolDialog dlg(nullptr);
    dlg.ShowModal();
  }
  else if (needsObjDump)
  {
    DumpCompositeObjects();
  }
  if (!anyLoaded)
  {
    InitScreen = new WelcomeDialog(nullptr);
    InitScreen->Center();
    InitScreen->ShowModal();
    OpenList = InitScreen->GetOpenList();
    InitScreen->Destroy();
    InitScreen = nullptr;
    if (OpenList.empty())
    {
      ExitMainLoop();
      return;
    }
    DelayLoad(e);
    return;
  }
}

int App::OnExit()
{
  FPackage::GetTransactionStream().Clear();
  FPackage::UnloadDefaultClassPackages();
  ALog::GetConfig(Config.LogConfig);
  AConfiguration cfg = AConfiguration(W2A(GetConfigPath().ToStdWstring()));
  cfg.SetConfig(Config);
  cfg.Save();
  ALog::SharedLog()->OnAppExit();
  return wxApp::OnExit();
}

void App::OnInitCmdLine(wxCmdLineParser& parser)
{
  static const wxCmdLineEntryDesc cmdLineDesc[] =
  {
    { wxCMD_LINE_SWITCH, "i", "private", "for internal usage" },
    { wxCMD_LINE_SWITCH, "s", "private", "for internal usage" },
    { wxCMD_LINE_PARAM,  NULL, NULL, "Package path", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE }
  };
  parser.SetDesc(cmdLineDesc);
}

void App::OnLoadError(wxCommandEvent& e)
{
  REDialog::Error(e.GetString(), "Error!");
  FPackage::UnloadDefaultClassPackages();
  ExitMainLoop();
  // TODO: try to recover. Ask for a new root dir and reload.
  /*
  // Retry to load the core with a new path
  Config.RootDir = rootDir.ToStdWstring();
  ProgressWindow* progressWindow = new ProgressWindow(nullptr);
  progressWindow->SetActionText(wxS("Loading..."));
  progressWindow->SetCurrentProgress(-1);
  progressWindow->Show();
  std::thread([this, progressWindow] { LoadCore(progressWindow); }).detach();*/
}

bool App::OnCmdLineParsed(wxCmdLineParser& parser)
{
  int paramsCount = parser.GetParamCount();
  if (InstanceChecker && InstanceChecker->IsAnotherRunning())
  {
    RpcClient::SendRequest("open", paramsCount ? parser.GetParam((size_t)paramsCount - 1) : wxEmptyString);
    return false;
  }
  if (paramsCount)
  {
    for (int idx = 0; idx < paramsCount; ++idx)
    {
      if (parser.GetParam(idx) == "-s")
      {
        continue;
      }
      if (parser.GetParam(idx) == "-i")
      {
        NeedsInitialScreen = true;
        continue;
      }
      OpenList.push_back(parser.GetParam(idx));
    }
  }
  return true;
}

void App::OnObjectLoaded(wxCommandEvent& e)
{
  std::string id = e.GetString().ToStdString();
  for (PackageWindow* win : PackageWindows)
  {
    if (win->OnObjectLoaded(id))
    {
      return;
    }
  }
}

void App::ShowWelcomeBeforeExit(wxCommandEvent&)
{
  if (!Config.ShowWelcomeOnClose)
  {
    ExitMainLoop();
    return;
  }
  InitScreen = new WelcomeDialog(nullptr);
  if (InitScreen->ShowModal() == wxID_OK)
  {
    OpenList = InitScreen->GetOpenList();
    if (OpenList.size())
    {
      SendEvent(this, DELAY_LOAD);
      InitScreen->Destroy();
      InitScreen = nullptr;
      return;
    }
  }
  InitScreen->Destroy();
  InitScreen = nullptr;
  ShuttingDown = true;
  ALog::Show(false);
  ExitMainLoop();
}

ALDevice* App::InitAudioDevice()
{
  if (AudioDevice)
  {
    delete AudioDevice;
  }
  try
  {
    AudioDevice = new ALDevice();
  }
  catch (const std::exception& e)
  {
    AudioDevice = nullptr;
    LogE("AudioDevice init: %s", e.what());
  }
  return AudioDevice;
}

void App::OnRegisterMime(wxCommandEvent&)
{
  wxString appPath = argv[0];
  wxMimeTypesManager man;
  RegisterFileType(".gpk", "Tera Game Package", appPath, man);
  RegisterFileType(".gmp", "Tera Game Map", appPath, man);
  RegisterFileType(".u", "Unreal Script Package", appPath, man);
  RegisterFileType(".upk", "Unreal Package", appPath, man);
  RegisterFileType(".umap", "Unreal Map", appPath, man);
}

void App::OnUnregisterMime(wxCommandEvent&)
{
  wxMimeTypesManager man;
  UnregisterFileType(".gpk", man);
  UnregisterFileType(".gmp", man);
  UnregisterFileType(".u", man);
  UnregisterFileType(".upk", man);
  UnregisterFileType(".umap", man);
}

void App::Restart(bool keepOpenList)
{
  wxString cmd = argv[0];
  if (keepOpenList)
  {
    for (const wxString& path : OpenList)
    {
      cmd += wxT(" \"") + path + wxT("\"");
    }
  }
  wxExecute(cmd);
  delete Server;
  Server = nullptr;
  delete InstanceChecker;
  InstanceChecker = nullptr;
  exit(0);
}

void App::RestartElevated(bool keepWindows)
{
  std::wstring appPath = argv[0].ToStdWstring();
  std::wstring params;
  if (keepWindows)
  {
    if (InitScreen)
    {
      params += L"-i ";
    }
    for (auto window : PackageWindows)
    {
      wxString sel = window->GetSelectedObjectPath();
      if (window->GetPackage()->IsComposite())
      {
        wxString path = L"composite\\" + window->GetPackage()->GetPackageName().WString();
        params += L'\"' + PackGpkPath(path, window->GetPosition(), window->GetSize(), sel) + L"\" ";
      }
      else
      {
        params += L"\"" + PackGpkPath(window->GetPackagePath(), window->GetPosition(), window->GetSize(), sel) + L"\" ";
      }
    }
    if (DcToolIsOpen)
    {
      params += L"DCTOOL ";
    }
  }
  params += L"-s ";
  SHELLEXECUTEINFO shExInfo = { 0 };
  shExInfo.cbSize = sizeof(shExInfo);
  shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  shExInfo.hwnd = 0;
  shExInfo.lpVerb = _T("runas");
  shExInfo.lpFile = appPath.c_str();
  shExInfo.lpParameters = params.c_str();
  shExInfo.lpDirectory = 0;
  shExInfo.nShow = SW_SHOW;
  shExInfo.hInstApp = 0;
  SaveConfig();
  if (ShellExecuteEx(&shExInfo))
  {
    delete Server;
    Server = nullptr;
    delete InstanceChecker;
    InstanceChecker = nullptr;
    exit(0);
  }
}

const wxArrayString& App::GetCompositePackageNames() const
{
  return CompositePackageNames;
}

bool App::CheckMimeTypes(bool strict) const
{
  wxMimeTypesManager man;
  std::vector<wxString> extensions = { wxS(".gpk"), wxS(".gmp"), wxS(".u"), wxS(".upk"), wxS(".umap") };
  for (const wxString& extension : extensions)
  {
    if (!CheckFileType(strict ? argv[0] : wxEmptyString, extension, man))
    {
      return false;
    }
  }
  return true;
}

void App::SaveConfig()
{
  AConfiguration cfg = AConfiguration(W2A(GetConfigPath().ToStdWstring()));
  cfg.SetConfig(Config);
  cfg.Save();
}

void App::SaveAndReopenPackage(std::shared_ptr<FPackage> package, const FString& tmp, const FString dest)
{
  PackageWindow* win = nullptr;
  for (PackageWindow* window : PackageWindows)
  {
    if (window->GetPackage() == package)
    {
      win = window;
      break;
    }
  }
  if (win)
  {
    // TODO: this is incorrect. The package can be retained by other packages and closing its stream may cause bugs or crash the app
    // Need to iterate over all open packages, release this package and unload all objects
    package->GetStream().Close();
    FPackage::UnloadPackage(win->GetPackage());
    PackageWindows.erase(std::remove(PackageWindows.begin(), PackageWindows.end(), win), PackageWindows.end());
    IgnoreWinClose = true;
    win->Close();
    IgnoreWinClose = false;
    bool ok = false;
    try
    {
      std::filesystem::path backup = GetTempFilePath().WString();
      std::error_code err;
      std::filesystem::copy_file(dest.WString(), backup, err);
      std::filesystem::rename(tmp.WString(), dest.WString(), err);
      if (err)
      {
        std::filesystem::rename(backup, dest.WString());
        REDialog::Error("Check if the destination is available and have free space.", "Failed to write package to the disk!");
      }
      else
      {
        ok = true;
      }
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what(), "Failed to write package to the disk!");
    }
    if (ok)
    {
      OpenPackage(dest.WString());
    }
    else if (PackageWindows.empty())
    {
      // Show welcome if no package windows left
      SendEvent(this, DELAY_LOAD);
    }
  }
}

void App::OnExitClicked()
{
  // Does not work when a modal runloop is running
  // Application->ExitMainLoop();
  SaveConfig();
  exit(0);
}

void App::DumpCompositeObjects()
{
  wxString dest = wxSaveFileSelector("composite objects map", "txt", "ObjectDump", nullptr);
  if (dest.empty())
  {
    return;
  }

  ProgressWindow progress(nullptr, "Dumping all objects");
  progress.SetCurrentProgress(-1);
  std::mutex failedMutex;
  bool fatal = false;

  size_t totalSavedGPKs = 0;
  size_t totalSavedGpkObjects = 0;
  std::vector<std::pair<std::string, std::string>> failed;
  std::thread([&] {

    // Update the mappers

    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating package mapper..."));
    Sleep(200);
    try
    {
      FPackage::LoadPkgMapper(true);
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what());
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    if (progress.IsCanceled())
    {
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating composite mapper..."));

    try
    {
      FPackage::LoadCompositePackageMapper(true);
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what());
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    if (progress.IsCanceled())
    {
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }

    // Run dumping

    std::mutex outMutex;
    std::ofstream s(dest.ToStdWstring(), std::ios::out | std::ios::binary);

    auto compositeMap = FPackage::GetCompositePackageMap();
    const int total = (int)compositeMap.size();

    std::mutex idxMut;
    volatile int idx = 0;

    SendEvent(&progress, UPDATE_MAX_PROGRESS, total);

    PERF_START(CompositeDump);
    std::vector<FString> pools = FPackageDumpHelper::GetGpkPools();
    std::for_each(std::execution::par_unseq, pools.begin(), pools.end(), [&](const auto& pool) {
      std::vector<FString> items = FPackageDumpHelper::GetPoolItems(pool);
      if (items.empty() || progress.IsCanceled())
      {
        return;
      }
      FString path = FPackageDumpHelper::GetPoolPath(pool);
      FReadStream poolStream(path);
      if (!poolStream.IsGood())
      {
        // Add error
        std::scoped_lock<std::mutex> l(failedMutex);
        failed.emplace_back(std::make_pair(pool.UTF8(), "Failed to open the package!"));
        return;
      }
      int32 localCount = 0;
      size_t dumpApproxReserve = 0;
      std::vector<FPackageDumpHelper::CompositeDumpEntry> localDump;
      for (const FString& item : items)
      {
        FPackageDumpHelper::CompositeDumpEntry& output = localDump.emplace_back();
        try
        {
          FPackageDumpHelper::GetPoolItemInfo(item, App::GetSharedApp()->GetConfig().FastObjectDump, poolStream, output);
        }
        catch (const std::exception& exc)
        {
          std::string errpkg = pool.UTF8() + '.' + item.UTF8();
          std::scoped_lock<std::mutex> l(failedMutex);
          failed.emplace_back(std::make_pair(errpkg, std::string("Failed to dump: ") + exc.what()));
        }
        dumpApproxReserve += output.Exports.size() * 120; // Reserve 120 chars per export entry
        dumpApproxReserve += output.ObjectPath.Size() + 15; // Reserve for objectPath
        localCount++;
        if (localCount % 31 == 0)
        {
          std::scoped_lock<std::mutex> l(idxMut);
          idx += localCount;
          localCount = 0;
          SendEvent(&progress, UPDATE_PROGRESS, idx);
          SendEvent(&progress, UPDATE_PROGRESS_DESC, wxString::Format("Saving %d/%d gpks...", idx, total));
        }
        if (progress.IsCanceled())
        {
          return;
        }
      }
      if (localCount)
      {
        std::scoped_lock<std::mutex> l(idxMut);
        idx += localCount;
        localCount = 0;
        SendEvent(&progress, UPDATE_PROGRESS, idx);
        SendEvent(&progress, UPDATE_PROGRESS_DESC, wxString::Format("Saving %d/%d gpks...", idx, total));
      }
      std::string dumpStr;
      dumpStr.reserve(dumpApproxReserve);
      for (const FPackageDumpHelper::CompositeDumpEntry& entry : localDump)
      {
        if (entry.Exports.empty())
        {
          continue;
        }
        dumpStr += "// ObjectPath: " + entry.ObjectPath.UTF8() + '\n';
        for (const auto& exp : entry.Exports)
        {
          dumpStr += exp.ClassName.UTF8();
          dumpStr += '\t';
          dumpStr += std::to_string(exp.Index);
          dumpStr += '\t';
          dumpStr += exp.Path.UTF8();
          dumpStr += '\n';
        }
      }

      if (dumpStr.size())
      {
        std::scoped_lock<std::mutex> l(outMutex);
        if (s.good())
        {
          totalSavedGPKs += localDump.size();
          totalSavedGpkObjects += std::accumulate(localDump.begin(), localDump.end(), 0, [](size_t sum, const auto& i) { return sum + i.Exports.size(); });
          try
          {
            s.write(dumpStr.data(), dumpStr.size());
            s.flush();
          }
          catch (...)
          {
          }
        }
        if (!s.good())
        {
          fatal = true;
          if (!progress.IsCanceled())
          {
            progress.SetCanceled();
            REDialog::Error("Check if your drive has free space.", "Failed to write data to your disk!");
            SendEvent(&progress, UPDATE_PROGRESS_FINISH);
          }
          return;
        }
      }
    });
    PERF_END(CompositeDump);
    if (fatal)
    {
      return;
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();

  if (fatal)
  {
    return;
  }

  if (failed.size())
  {
    std::filesystem::path errDst = dest.ToStdWstring();
    errDst.replace_extension("errors.txt");
    std::ofstream s(errDst, std::ios::out | std::ios::binary);

    for (const auto& pair : failed)
    {
      s << pair.first << ": " << pair.second << '\n';
    }

    REDialog::Warning("Failed to iterate some of the packages!\nSee " + errDst.filename().string() + " file for details");
  }
  else
  {
    REDialog::Info(wxString::Format("Saved %Iu objects from %Iu gpks.", totalSavedGpkObjects, totalSavedGPKs));
  }
}

void App::OnDialogWillOpen(WXDialog* dialog)
{
  if (dialog)
  {
    Dialogs.push_back(dialog);
  }
}

void App::OnDialogDidClose(WXDialog* dialog)
{
  if (dialog)
  {
    auto it = std::find(Dialogs.begin(), Dialogs.end(), dialog);
    if (it != Dialogs.end())
    {
      Dialogs.erase(it);
    }
  }
}

void App::OnActivateApp(wxActivateEvent& e)
{
  if (e.GetActive() && Dialogs.size())
  {
    if (WXDialog* dialog = Dialogs.back())
    {
      std::vector<wxWindow*> parents;
      wxWindow* parent = dialog->GetParent();
      while (parent)
      {
        parents.emplace_back(parent);
        parent = parent->GetParent();
      }
      for (auto rit = parents.rbegin(); rit != parents.rend(); ++rit)
      {
        (*rit)->Raise();
      }
      dialog->SetFocus();
      dialog->Raise();
    }
  }
  e.Skip();
}

void App::OnFatalException()
{
  wxMessageBox("An unknown error occurred. The program will close!", "Error!", wxICON_ERROR);
}

wxBEGIN_EVENT_TABLE(App, wxApp)
EVT_ACTIVATE_APP(App::OnActivateApp)
EVT_COMMAND(wxID_ANY, DELAY_LOAD, App::DelayLoad)
EVT_COMMAND(wxID_ANY, OPEN_PACKAGE, App::OnOpenPackage)
EVT_COMMAND(wxID_ANY, LOAD_CORE_ERROR, App::OnLoadError)
EVT_COMMAND(wxID_ANY, OBJECT_LOADED, App::OnObjectLoaded)
EVT_COMMAND(wxID_ANY, REGISTER_MIME, App::OnRegisterMime)
EVT_COMMAND(wxID_ANY, UNREGISTER_MIME, App::OnUnregisterMime)
EVT_COMMAND(wxID_ANY, SHOW_FINAL_INIT, App::ShowWelcomeBeforeExit)
wxEND_EVENT_TABLE()
