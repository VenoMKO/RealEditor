#include "App.h"
#include "RootPathWindow.h"
#include "ProgressWindow.h"

#include <wx/mimetype.h>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>

#include <Tera/ALog.h>
#include <Tera/FPackage.h>

const char* APP_NAME = "Real Editor";

wxIMPLEMENT_APP(App);

wxDEFINE_EVENT(DELAY_LOAD, wxCommandEvent);
wxDEFINE_EVENT(OPEN_PACKAGE, wxCommandEvent);

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

void RegisterFileType(const wxString& extension, const wxString& description, const wxString& appPath, wxMimeTypesManager* man)
{
  wxFileTypeInfo info = wxFileTypeInfo("application/octet-stream");
  info.AddExtension(extension);
  info.SetDescription(description);
  info.SetOpenCommand(wxS("\"") + appPath + wxS("\" \"%1\""));
  if (wxFileType* type = man->Associate(info))
  {
    delete type;
  }
}

void RegisterMimeTypes(const wxString& appPath)
{
  wxMimeTypesManager* man = new wxMimeTypesManager();
  RegisterFileType(".gpk", "Tera Package", appPath, man);
  RegisterFileType(".gmp", "Tera Map Package", appPath, man);
  RegisterFileType(".u", "Tera Script Package", appPath, man);
  RegisterFileType(".tfc", "Texture File Cache", appPath, man);
  RegisterFileType(".upk", "Unreal Package", appPath, man);
  RegisterFileType(".umap", "Unreal Map", appPath, man);
  delete man;
}

void UnegisterFileType(const wxString& extension, wxMimeTypesManager* man)
{
  wxFileType* type = man->GetFileTypeFromExtension(".gpk");
  if (type)
  {
    man->Unassociate(type);
    delete type;
  }
}

void UnregisterMimeTypes()
{
  wxMimeTypesManager* man = new wxMimeTypesManager();
  UnegisterFileType(".gpk", man);
  UnegisterFileType(".gmp", man);
  UnegisterFileType(".u", man);
  UnegisterFileType(".tfc", man);
  UnegisterFileType(".upk", man);
  UnegisterFileType(".umap", man);
  delete man;
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
  LastWindowPosition = pos;
}

wxPoint App::GetLastWindowPosition() const
{
  return LastWindowPosition;
}

App::~App()
{
  delete InstanceChecker;
  delete Server;
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
    package = FPackage::GetPackage(W2A(path.ToStdWstring()), true);
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

  std::thread([package, window]() {
    try
    {
      if (!package->IsReady())
      {
        package->Load();
      }
    }
    catch (const std::exception& e)
    {
      LogE("Failed to load the package. %s.", e.what());
      wxCommandEvent* event = new wxCommandEvent(PACKAGE_ERROR);
      event->SetString(e.what());
      wxQueueEvent(window, event);
      return;
    }
    if (!package->IsOperationCancelled())
    {
      wxQueueEvent(window, new wxCommandEvent(PACKAGE_READY));
    }
  }).detach();
  return true;
}

bool App::OpenDialog(const wxString& rootDir)
{
  wxString extensions = wxS("Package files (*.gpk; *.gmp; *.u; *.umap; *.tfc; *.upk)|*.gpk;*.gmp;*.u;*.umap;*.tfc;*.upk");
  wxString packagePath = wxFileSelector("Open a package", rootDir, wxEmptyString, extensions, extensions, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (packagePath.size())
  {
    return OpenPackage(packagePath);
  }
  return false;
}

void App::OnOpenPackage(wxCommandEvent& e)
{
  OpenPackage(e.GetString());
}

void App::PackageWindowWillClose(const PackageWindow* frame)
{
  for (auto it = PackageWindows.begin(); it < PackageWindows.end(); it++)
  {
    if ((*it)->GetPackagePath() == frame->GetPackagePath())
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
  
  // If renames the executable our AppData storage path will change too. We don't want that.
  // So we set AppName manually. This will keep paths consistent.
  SetAppName(APP_NAME);
  SetAppDisplayName(APP_NAME);

  AConfiguration cfg = AConfiguration(W2A(GetConfigPath().ToStdWstring()));
  if (cfg.Load())
  {
    Config = cfg.GetConfig();
  }
  if (Config.RootDir.empty())
  {
    Config = cfg.GetDefaultConfig();
    RootPathWindow rtw;
    rtw.ShowModal();
    Config.RootDir = W2A(rtw.GetRootPath().ToStdWstring());
    if (Config.RootDir.empty())
    {
      return false;
    }
    cfg.SetConfig(Config);
    cfg.Save();
  }
  LastWindowPosition = wxPoint(WIN_POS_CENTER, 0);
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
    Server = new RpcServer;
    Server->RunWithDelegate(this);
    RegisterMimeTypes(argv[0]);
    if (Config.LogConfig.ShowLog)
    {
      ALog::SharedLog()->Show();
    }
    return wxApp::OnRun();
  }
  return 0;
}

void App::LoadCore(ProgressWindow* pWindow)
{
  {
    wxCommandEvent* event = new wxCommandEvent(UPDATE_PROGRESS_DESC);
    event->SetString(wxT("Enumerating root folder..."));
    wxQueueEvent(pWindow, event);
  }
  FPackage::SetRootPath(Config.RootDir);
  if (pWindow->IsCancelled())
  {
    wxQueueEvent(this, new wxCloseEvent());
    return;
  }
  {
    wxCommandEvent* event = new wxCommandEvent(UPDATE_PROGRESS_DESC);
    event->SetString(wxT("Loading class packages..."));
    wxQueueEvent(pWindow, event);
  }
  FPackage::LoadDefaultClassPackages();
  if (pWindow->IsCancelled())
  {
    wxQueueEvent(this, new wxCloseEvent());
    return;
  }
  {
    wxCommandEvent* event = new wxCommandEvent(UPDATE_PROGRESS_DESC);
    event->SetString(wxT("Loading composit package mapping..."));
    wxQueueEvent(pWindow, event);
  }
  FPackage::LoadCompositePackageMap();
  {
    wxCommandEvent* event = new wxCommandEvent(UPDATE_PROGRESS_FINISH);
    wxQueueEvent(pWindow, event);
    event = new wxCommandEvent(DELAY_LOAD);
    wxQueueEvent(this, event);
  }
}

void App::DelayLoad(wxCommandEvent&)
{
  for (const wxString& path : OpenList)
  {
    OpenPackage(path);
  }
  OpenList.clear();
}

wxString App::RequestS1GameFolder()
{
  RootPathWindow* frame = new RootPathWindow();
  frame->ShowModal();
  wxString path = frame->GetRootPath();
  frame->Destroy();
  return path;
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

bool App::OnCmdLineParsed(wxCmdLineParser& parser)
{
  int paramsCount = parser.GetParamCount();
  if (InstanceChecker && InstanceChecker->IsAnotherRunning())
  {
    RpcClient::SendRequest("open", paramsCount ? parser.GetParam((size_t)paramsCount - 1) : wxEmptyString);
    return false;
  }
  else
  {
    if (paramsCount)
    {
      for (int idx = 0; idx < paramsCount; ++idx)
      {
        OpenList.push_back(parser.GetParam(idx));
      }
    }
    ProgressWindow* progressWindow = new ProgressWindow(nullptr);
    progressWindow->SetActionText(wxS("Loading..."));
    progressWindow->SetCurrentProgress(-1);
    progressWindow->Show();
    std::thread([this, progressWindow] { LoadCore(progressWindow); }).detach();
  }
  return true;
}

wxBEGIN_EVENT_TABLE(App, wxApp)
EVT_COMMAND(wxID_ANY, DELAY_LOAD, DelayLoad)
EVT_COMMAND(wxID_ANY, OPEN_PACKAGE, OnOpenPackage)
wxEND_EVENT_TABLE()