#include "App.h"
#include "RootPathWindow.h"

#include <wx/mimetype.h>
#include <wx/cmdline.h>

#include <Tera/FPackage.h>

wxIMPLEMENT_APP(App);

void CreateFileType(const wxString& extension, const wxString& description, const wxString& appPath, wxMimeTypesManager* man)
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
  CreateFileType(".gpk", "Tera Package", appPath, man);
  CreateFileType(".gmp", "Tera Map Package", appPath, man);
  CreateFileType(".u", "Tera Script Package", appPath, man);
  delete man;
}

void UnregisterMimeType()
{
  wxMimeTypesManager* man = new wxMimeTypesManager();
  wxFileType* t = man->GetFileTypeFromExtension(".gpk");
  if (t)
  {
    man->Unassociate(t);
    delete t;
  }
  t = man->GetFileTypeFromExtension(".gmp");
  if (t)
  {
    man->Unassociate(t);
    delete t;
  }
  t = man->GetFileTypeFromExtension(".u");
  if (t)
  {
    man->Unassociate(t);
    delete t;
  }
  delete man;
}

void App::OnRpcOpenFile(const wxString& path)
{
  if (IsReady && !path.empty())
  {
    OpenPackage(path);
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
    package = FPackage::LoadPackage(W2A(path.ToStdWstring()));
  }
  catch (const std::exception& e)
  {
    wxMessageBox(e.what(), "Failed to open the package!", wxICON_ERROR);
    return false;
  }

  if (package == nullptr)
  {
    wxMessageBox("Unknown error!", "Failed to open the package!", wxICON_ERROR);
    return false;
  }

  PackageWindow* window = new PackageWindow(package, this);
  PackageWindows.push_back(window);
  window->Show();
  return true;
}

bool App::OpenDialog(const wxString& rootDir)
{
  wxString extensions = wxS("Package files (*.gpk; *.gmp; *.u)|*.gpk;*.gmp;*.u");
  wxString packagePath = wxFileSelector("Open a package", rootDir, wxEmptyString, extensions, extensions, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (packagePath.size())
  {
    return OpenPackage(packagePath);
  }
  return false;
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
}

bool App::OnInit()
{
#ifdef _DEBUG
  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
  SetConsoleOutputCP(CP_UTF8);
  FPackage::SetRootPath("D:\\Program Files (x86)\\Destiny\\TERA\\S1Game\\CookedPC");
  LastWindowPosition = wxPoint(WIN_POS_CENTER, 0);
  InstanceChecker = new wxSingleInstanceChecker;
  return wxApp::OnInit();
}

int App::OnRun()
{
  if (IsReady)
  {
    FPackage::LoadDefaultClassPackages();
    Server = new RpcServer;
    Server->RunWithDelegate(this);
    RegisterMimeTypes(argv[0]);
    return wxApp::OnRun();
  }
  return 0;
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
  if (InstanceChecker->IsAnotherRunning())
  {
    RpcClient::SendRequest("open", paramsCount ? parser.GetParam(paramsCount - 1) : wxEmptyString);
    return false;
  }
  else if (paramsCount)
  {
    IsReady = OpenPackage(parser.GetParam(paramsCount - 1));
  }
  return IsReady;
}
