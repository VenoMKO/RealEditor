#include "App.h"
#include "Windows/ProgressWindow.h"
#include "Windows/SettingsWindow.h"
#include "Windows/CompositePackagePicker.h"

#include <wx/mimetype.h>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/fileconf.h>
#include <wx/mstream.h>

#include <Tera/ALog.h>
#include <Tera/FPackage.h>
#include <Tera/FStream.h>

const char* APP_NAME = "Real Editor";

wxIMPLEMENT_APP(App);

wxDEFINE_EVENT(DELAY_LOAD, wxCommandEvent);
wxDEFINE_EVENT(OPEN_PACKAGE, wxCommandEvent);
wxDEFINE_EVENT(LOAD_CORE_ERROR, wxCommandEvent);
wxDEFINE_EVENT(OBJECT_LOADED, wxCommandEvent);
wxDEFINE_EVENT(REGISTER_MIME, wxCommandEvent);
wxDEFINE_EVENT(UNREGISTER_MIME, wxCommandEvent);

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

void RegisterFileType(const wxString& extension, const wxString& description, const wxString& appPath, wxMimeTypesManager& man)
{
  wxFileTypeInfo info = wxFileTypeInfo("application/octet-stream");
  info.AddExtension(extension);
  info.SetDescription(description);
  info.SetOpenCommand(wxS("\"") + appPath + wxS("\" \"%1\""));
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

bool CheckFileType(const wxString& extension, wxMimeTypesManager& man)
{
  if (wxFileType* type = man.GetFileTypeFromExtension(extension))
  {
    wxString cmd = type->GetOpenCommand(wxT("test") + extension);
    delete type;
    return cmd.size();
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
  if (IsReady && !path.empty())
  {
    wxCommandEvent* event = new wxCommandEvent(OPEN_PACKAGE);
    event->SetString(path);
    wxQueueEvent(this, event);
  }
  else
  {
    GetTopWindow()->Raise();
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
  OpenPackage(e.GetString());
}

void App::OnShowSettings(wxCommandEvent& e)
{
  FAppConfig newConfig;
  SettingsWindow win(Config, newConfig, true);
  if (win.ShowModal() == wxID_OK)
  {
    if (Config.RootDir != newConfig.RootDir)
    {
      wxMessageDialog dialog(nullptr, wxT("Application must be restarted to apply changes. If you click \"OK\" the app will restart!"), wxT("Restart ") + GetAppDisplayName() + wxT("?"), wxOK | wxCANCEL | wxICON_EXCLAMATION);
      if (dialog.ShowModal() != wxID_OK)
      {
        return;
      }

      Config = newConfig;

      AConfiguration cfg = AConfiguration(W2A(GetConfigPath().ToStdWstring()));
      cfg.SetConfig(Config);
      cfg.Save();

      NeedsRestart = true;

      OpenList.clear();
      for (auto window : PackageWindows)
      {
        OpenList.push_back(window->GetPackagePath());
      }

      ExitMainLoop();
    }
    Config = newConfig;
  }
}

App::~App()
{
  delete InstanceChecker;
  delete Server;

  if (NeedsRestart)
  {
    wxString cmd = argv[0];
    for (const wxString& path: OpenList)
    {
      cmd += wxT(" \"") + path + wxT("\"");
    }
    wxExecute(cmd);
  }
}

wxString App::ShowOpenDialog(const wxString& rootDir)
{
  wxString extensions = wxS("Package files (*.gpk; *.gmp; *.u; *.umap; *.upk)|*.gpk;*.gmp;*.u;*.umap;*.upk");
  return wxFileSelector("Open a package", rootDir, wxEmptyString, extensions, extensions, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
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

bool App::OpenPackage(const wxString& path)
{
  for (const auto window : PackageWindows)
  {
    if (window->GetPackagePath() == path)
    {
      window->Raise();
      return true;
    }
  }

  std::shared_ptr<FPackage> package = nullptr;
  try
  {
    package = FPackage::GetPackage(W2A(path.ToStdWstring()));
  }
  catch (const std::exception& e)
  {
    LogE("Failed to open the package: %s", e.what());
    wxMessageBox(e.what(), "Failed to open the package!", wxICON_ERROR);
    return false;
  }

  if (package == nullptr)
  {
    LogE("Failed to open the package: Unknow error");
    wxMessageBox("Unknown error!", "Failed to open the package!", wxICON_ERROR);
    return false;
  }

  PackageWindow* window = new PackageWindow(package, this);
  PackageWindows.push_back(window);
  window->Show();

  if (!package->IsReady())
  {
    std::thread([package, window]() {
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
      if (!package->IsOperationCancelled())
      {
        SendEvent(window, PACKAGE_READY);
      }
    }).detach();
  }
  else
  {
    SendEvent(window, PACKAGE_READY);
  }
  return true;
}

bool App::OpenNamedPackage(const wxString& name, const wxString selection)
{
  for (const auto window : PackageWindows)
  {
    if (window->GetPackage()->GetPackageName().String() == name)
    {
      window->Raise();
      return true;
    }
  }

  std::shared_ptr<FPackage> package = nullptr;
  try
  {
    package = FPackage::GetPackageNamed(name.ToStdString());
  }
  catch (const std::exception& e)
  {
    LogE("Failed to open the package: %s", e.what());
    wxMessageBox(e.what(), "Failed to open the package!", wxICON_ERROR);
    return false;
  }

  if (package == nullptr)
  {
    LogE("Failed to open the package: Unknow error");
    wxMessageBox("Unknown error!", "Failed to open the package!", wxICON_ERROR);
    return false;
  }

  PackageWindow* window = new PackageWindow(package, this);
  PackageWindows.push_back(window);
  window->Show();

  if (!package->IsReady())
  {
    std::thread([package, window]() {
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
      if (!package->IsOperationCancelled())
      {
        SendEvent(window, PACKAGE_READY);
      }
    }).detach();
  }
  else
  {
    SendEvent(window, PACKAGE_READY);
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
  if (PackageWindows.empty())
  {
    ALog::Show(false);
  }
}

bool App::OnInit()
{
#ifdef _DEBUG
  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
  
  // If the executable name changes our AppData storage path will change too. We don't want that.
  // So we set AppName manually. This will keep paths consistent.
  SetAppName(APP_NAME);
  SetAppDisplayName(APP_NAME);

  AConfiguration cfg = AConfiguration(W2A(GetConfigPath().ToStdWstring()));
  if (cfg.Load())
  {
    Config = cfg.GetConfig();
  }
  if (Config.RootDir.Empty())
  {
    FAppConfig newConfig;
    SettingsWindow win(Config, newConfig, false);
    if (win.ShowModal() != wxID_OK)
    {
      return false;
    }
    Config = newConfig;
    cfg.SetConfig(Config);
    cfg.Save();
    Config.WindowRect.Min = { WIN_POS_CENTER, 0 };
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
    wxInitAllImageHandlers();
    Server = new RpcServer;
    Server->RunWithDelegate(this);
    if (Config.LogConfig.ShowLog)
    {
      ALog::SharedLog()->Show();
    }
    ProgressWindow* progressWindow = new ProgressWindow(nullptr);
    progressWindow->SetActionText(wxS("Loading..."));
    progressWindow->SetCurrentProgress(-1);
    progressWindow->Show();
    std::thread([this, progressWindow] { LoadCore(progressWindow); }).detach();
    return wxApp::OnRun();
  }
  return 0;
}

void App::LoadCore(ProgressWindow* pWindow)
{
  PERF_START(AppLoad);
  SendEvent(pWindow, UPDATE_PROGRESS_DESC, "Enumerating root folder...");
  PERF_START(DirCacheLoad);
  FPackage::SetRootPath(Config.RootDir);
  PERF_END(DirCacheLoad);

  if (pWindow->IsCancelled())
  {
    ExitMainLoop();
    return;
  }

  SendEvent(pWindow, UPDATE_PROGRESS_DESC, "Loading stripped meta data...");
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
  }

  const std::vector<FString> classPackageNames = { "Core.u", "Engine.u", "GameFramework.u", "S1Game.u", "GFxUI.u", "WinDrv.u", "IpDrv.u", "OnlineSubsystemPC.u", "UnrealEd.u", "GFxUIEditor.u" };
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
    catch (const std::exception& e)
    {
      SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
      SendEvent(this, LOAD_CORE_ERROR, e.what());
      return;
    }
    if (pWindow->IsCancelled())
    {
      ExitMainLoop();
      return;
    }
  }
  PERF_END(ClassPackagesLoad);

  SendEvent(pWindow, UPDATE_PROGRESS_DESC, "Loading persistent data...");
  try
  {
    PERF_START(PersistentStorageLoad);
    FPackage::LoadPersistentData();
    PERF_END(PersistentStorageLoad);
  }
  catch (const std::exception& e)
  {
    SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
    SendEvent(this, LOAD_CORE_ERROR, e.what());
    return;
  }

  if (pWindow->IsCancelled())
  {
    ExitMainLoop();
    return;
  }

  if (FPackage::GetCoreVersion() > VER_TERA_CLASSIC)
  {
    SendEvent(pWindow, UPDATE_PROGRESS_DESC, "Loading Mappers...");
    std::mutex errorMutex;
    FString error;
    std::thread pkgMapper([&errorMutex, &error] {
      try
      {
        PERF_START(PkgMapperLoad);
        FPackage::LoadPkgMapper();
        PERF_END(PkgMapperLoad);
      }
      catch (const std::exception& e)
      {
        std::scoped_lock<std::mutex> l(errorMutex);
        if (error.Empty())
        {
          error = e.what();
        }
      }
    });

    std::thread compositMapper([&errorMutex, &error] {
      try
      {
        PERF_START(CompositeMapperLoad);
        FPackage::LoadCompositePackageMapper();
        PERF_END(CompositeMapperLoad);
      }
      catch (const std::exception& e)
      {
        std::scoped_lock<std::mutex> l(errorMutex);
        if (error.Empty())
        {
          error = e.what();
        }
      }
    });

    std::thread objectRedirectorMapper([&errorMutex, &error] {
      try
      {
        PERF_START(RedirectorMapperLoad);
        FPackage::LoadObjectRedirectorMapper();
        PERF_END(RedirectorMapperLoad);
      }
      catch (const std::exception& e)
      {
        std::scoped_lock<std::mutex> l(errorMutex);
        if (error.Empty())
        {
          error = e.what();
        }
      }
    });

    pkgMapper.join();
    compositMapper.join();
    objectRedirectorMapper.join();

    const auto& compositeMap = FPackage::GetCompositePackageMap();
    CompositePackageNames.reserve(compositeMap.size());
    for (const auto& pair : compositeMap)
    {
      CompositePackageNames.push_back(pair.first.String());
    }

    if (error.Size())
    {
      SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
      SendEvent(this, LOAD_CORE_ERROR, error.String());
      pWindow->Destroy();
      ExitMainLoop();
    }

    if (pWindow->IsCancelled())
    {
      wxQueueEvent(this, new wxCloseEvent());
      pWindow->Destroy();
      ExitMainLoop();
    }
    PERF_END(AppLoad);
  }
  

  SendEvent(pWindow, UPDATE_PROGRESS_FINISH);
  SendEvent(this, DELAY_LOAD);
  pWindow->Destroy();
}

void App::DelayLoad(wxCommandEvent&)
{
  bool anyLoaded = false;
  for (const wxString& path : OpenList)
  {
    if (OpenPackage(path))
    {
      anyLoaded = true;
    }
  }
  OpenList.clear();
  if (!anyLoaded)
  {
    ExitMainLoop();
  }
}

int App::OnExit()
{
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
    { wxCMD_LINE_PARAM,  NULL, NULL, "Package path", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE }
  };
  parser.SetDesc(cmdLineDesc);
}

void App::OnLoadError(wxCommandEvent& e)
{
  wxMessageBox(e.GetString(), "Error!", wxICON_ERROR);
  FPackage::UnloadDefaultClassPackages();
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
      OpenList.push_back(parser.GetParam(idx));
    }
    return true;
  }

  // If we have no input args, we want to show a Root selector window
  FAppConfig newConfig;
  SettingsWindow win(Config, newConfig, false);
  if (win.ShowModal() == wxID_OK)
  {
    Config = newConfig;
    AConfiguration cfg = AConfiguration(W2A(GetConfigPath().ToStdWstring()));
    cfg.SetConfig(Config);
    cfg.Save();

    wxString path = ShowOpenDialog();
    if (path.size())
    {
      OpenList.push_back(path);
      return true;
    }
  }
  return false;
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

const wxArrayString& App::GetCompositePackageNames() const
{
  return CompositePackageNames;
}

bool App::CheckMimeTypes() const
{
  wxMimeTypesManager man;
  std::vector<wxString> extensions = { wxS(".gpk"), wxS(".gmp"), wxS(".u"), wxS(".upk"), wxS(".umap") };
  for (const wxString& extension : extensions)
  {
    if (!CheckFileType(extension, man))
    {
      return false;
    }
  }
  return true;
}

wxBEGIN_EVENT_TABLE(App, wxApp)
EVT_COMMAND(wxID_ANY, DELAY_LOAD, App::DelayLoad)
EVT_COMMAND(wxID_ANY, OPEN_PACKAGE, App::OnOpenPackage)
EVT_COMMAND(wxID_ANY, LOAD_CORE_ERROR, App::OnLoadError)
EVT_COMMAND(wxID_ANY, OBJECT_LOADED, App::OnObjectLoaded)
EVT_COMMAND(wxID_ANY, REGISTER_MIME, App::OnRegisterMime)
EVT_COMMAND(wxID_ANY, UNREGISTER_MIME, App::OnUnregisterMime)
wxEND_EVENT_TABLE()