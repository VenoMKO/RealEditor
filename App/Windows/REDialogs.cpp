#include <wx/filedlg.h>
#include <wx/statline.h>
#include <wx/statbmp.h>
#include "../App.h"
#include "../Misc/WXDialog.h"

#include <Utils/AConfiguration.h>

namespace
{
  // Looks like wxWidgets has a bug that prevents wxFileDialog to select a default filter. wxFileDialog chooses the first element in the list regardless of the input.
  // To address the issue this function rearranges extensions so that the default extension is the first one in the list.
  // If the result extension doesn't match the selected filter this function appends the correct extension.
  void SaveDialog(wxWindow* parent, const wxString& filename, const wxString& path, const wxString caption, const std::vector<std::pair<std::string, std::string>>& extensions, wxString& outPath, int32& outExt)
  {
    std::vector<std::pair<std::string, std::string>> arrangedExtensions = extensions;
    if (outExt >= arrangedExtensions.size())
    {
      outExt = 0;
    }
    else if (outExt)
    {
      std::swap(arrangedExtensions[0], arrangedExtensions[outExt]);
    }

    wxString wildcard;
    for (const auto& p : arrangedExtensions)
    {
      wildcard += wxString::Format("%s%s|*%s", wildcard.Length() ? "|" : "", p.second.c_str(), p.first.c_str());
    }

    int ext = 0;
    outPath = wxFileSelectorEx(caption, path, filename, &ext, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent);
    if (outPath.Length() && !outPath.Lower().EndsWith(arrangedExtensions[ext].first.c_str()))
    {
      outPath += arrangedExtensions[ext].first;
    }
    if (ext)
    {
      for (int32 idx = 0; idx < extensions.size(); ++idx)
      {
        if (extensions[idx].first == arrangedExtensions[ext].first)
        {
          outExt = idx;
          break;
        }
      }
    }
  }

  bool GetShieldIcon(wxBitmap& shieldBM)
  {
    HMODULE	hShellDLL;
    HRESULT	hr;
    bool isOK = true;

    HRESULT(CALLBACK * pfnSHGetStockIconInfo)(SHSTOCKICONID siid, UINT uFlags, SHSTOCKICONINFO * psii);

    hShellDLL = LoadLibraryW(L"shell32.dll");
    if (hShellDLL == nullptr)
      return false;

    (*(FARPROC*)&pfnSHGetStockIconInfo) = GetProcAddress(hShellDLL, "SHGetStockIconInfo");
    if (pfnSHGetStockIconInfo)
    {
      SHSTOCKICONINFO stockInfo;
      stockInfo.cbSize = sizeof(SHSTOCKICONINFO);
      hr = pfnSHGetStockIconInfo(SIID_SHIELD, SHGSI_ICON, &stockInfo);
      if (hr == S_OK)
      {
        wxIcon uacShieldIcon;
        uacShieldIcon.CreateFromHICON(stockInfo.hIcon);
        shieldBM.CopyFromIcon(uacShieldIcon);
        DestroyIcon(stockInfo.hIcon);
      }
      else
      {
        isOK = false;
      }
    }
    else
    {
      isOK = false;
    }
    FreeLibrary(hShellDLL);
    return isOK;
  }

  class AuthDialog : public WXDialog {
  public:
    AuthDialog(wxWindow* parent = nullptr, const wxString& title = wxEmptyString)
      : WXDialog(parent, wxID_ANY, wxTheApp->GetVendorDisplayName(), wxDefaultPosition, wxSize(526, 201))
    {
      SetSize(FromDIP(GetSize()));
      SetIcon(wxICON(#114));
      SetSizeHints(wxDefaultSize, wxDefaultSize);

      wxBoxSizer* bSizer11;
      bSizer11 = new wxBoxSizer(wxVERTICAL);

      wxPanel* m_panel4;
      m_panel4 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
      m_panel4->SetBackgroundColour(wxColour(255, 255, 255));

      wxBoxSizer* bSizer14;
      bSizer14 = new wxBoxSizer(wxHORIZONTAL);

      ShieldIcon = new wxStaticBitmap(m_panel4, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(48, 48), 0);
      bSizer14->Add(ShieldIcon, 0, wxTOP | wxBOTTOM | wxLEFT, 10);

      wxBoxSizer* bSizer15;
      bSizer15 = new wxBoxSizer(wxVERTICAL);

      wxStaticText* m_staticText9;
      m_staticText9 = new wxStaticText(m_panel4, wxID_ANY, wxT("This action requires Real Editor to have elevated permissions."), wxDefaultPosition, wxDefaultSize, 0);
      m_staticText9->Wrap(-1);
      m_staticText9->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Segoe UI Semibold")));
      m_staticText9->SetForegroundColour(wxColour(0, 45, 128));

      bSizer15->Add(m_staticText9, 0, wxALL, 10);

      wxString desc = title;
      if (desc.size())
      {
        desc += '\n';
      }
      desc += wxT("Press Restart as Administrator to restart Real Editor under different credentials.\nAll unsaved changes will be lost.");
      Description = new wxStaticText(m_panel4, wxID_ANY, desc, wxDefaultPosition, wxDefaultSize, 0);
      Description->Wrap(FromDIP(460));
      bSizer15->Add(Description, 0, wxRIGHT | wxLEFT, 10);


      bSizer14->Add(bSizer15, 1, wxEXPAND, FromDIP(5));


      m_panel4->SetSizer(bSizer14);
      m_panel4->Layout();
      bSizer14->Fit(m_panel4);
      bSizer11->Add(m_panel4, 1, wxEXPAND, FromDIP(5));

      wxBoxSizer* bSizer12;
      bSizer12 = new wxBoxSizer(wxVERTICAL);

      wxStaticLine* m_staticline2;
      m_staticline2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
      bSizer12->Add(m_staticline2, 0, wxEXPAND, FromDIP(5));

      wxPanel* m_panel5;
      m_panel5 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
      wxBoxSizer* bSizer13;
      bSizer13 = new wxBoxSizer(wxHORIZONTAL);


      bSizer13->Add(0, 0, 1, wxEXPAND, FromDIP(5));

      OkButton = new wxButton(m_panel5, wxID_ANY, wxT("Restart as Administrator"), wxDefaultPosition, wxDefaultSize, 0);
      bSizer13->Add(OkButton, 0, wxALL, FromDIP(5));

      CancelButton = new wxButton(m_panel5, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
      bSizer13->Add(CancelButton, 0, wxALL, FromDIP(5));


      m_panel5->SetSizer(bSizer13);
      m_panel5->Layout();
      bSizer13->Fit(m_panel5);
      bSizer12->Add(m_panel5, 1, wxEXPAND | wxTOP | wxBOTTOM, 10);


      bSizer11->Add(bSizer12, 0, wxEXPAND, FromDIP(5));


      this->SetSizer(bSizer11);
      this->Layout();

      this->Centre(wxBOTH);

      wxBitmap shield;
      if (GetShieldIcon(shield))
      {
        ShieldIcon->SetBitmap(shield);
      }

      wxAcceleratorEntry entries[2];
      entries[0].Set(wxACCEL_NORMAL, WXK_RETURN, wxID_OK);
      entries[1].Set(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CANCEL);
      wxAcceleratorTable accel(2, entries);
      this->SetAcceleratorTable(accel);

      OkButton->SetFocus();
      OkButton->SetAuthNeeded();
      OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AuthDialog::OnOkClicked), nullptr, this);
      CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AuthDialog::OnCancelClicked), nullptr, this);
    }

    ~AuthDialog()
    {
      OkButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AuthDialog::OnOkClicked), nullptr, this);
      CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AuthDialog::OnCancelClicked), nullptr, this);
    }

    int ShowModal() wxOVERRIDE
    {
      MessageBeep(MB_ICONINFORMATION);
      return wxDialog::ShowModal();
    }

  protected:
    wxDECLARE_EVENT_TABLE();

    void OnOkClicked(wxCommandEvent& event)
    {
      EndModal(wxID_OK);
    }

    void OnCancelClicked(wxCommandEvent& event)
    {
      EndModal(wxID_CANCEL);
    }

  protected:
    wxStaticBitmap* ShieldIcon = nullptr;
    wxStaticText* Description = nullptr;
    wxButton* OkButton = nullptr;
    wxButton* CancelButton = nullptr;
  };
}

namespace IODialog
{

  wxString GetLastTextureExtension()
  {
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    std::vector<std::pair<std::string, std::string>> extensions = { {".tga", "TGA Image (*.tga)"}, {".png", "PNG Image  (*.png)"}, {".dds", "DDS Texture (*.dds)"} };
    return extensions[cfg.LastTextureExtension].first.substr(1);
  }

  wxString SaveDatacenter(int mode, wxWindow* parent, const wxString& path, const wxString& filename)
  {
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    if (!mode)
    {
      wxString fname = filename;
      if (!fname.EndsWith(wxT(".unpacked")))
      {
        fname += wxT(".unpacked");
      }
      wxString dir = path;
      if (dir.IsEmpty())
      {
        dir = cfg.LastDcSavePath.WString();
      }
      wxString result = wxFileSelector(wxT("Save unpacked datacenter..."), dir, fname, wxT(".unpacked"), wxT("Unpacked DataCenter files (*.unpacked)|*.unpacked"), wxFD_OPEN, parent);
      if (result.size())
      {
        cfg.LastDcSavePath = result.ToStdWstring();
        App::GetSharedApp()->SaveConfig();
      }
      return result;
    }
    wxString dir = path;
    if (dir.IsEmpty())
    {
      dir = cfg.LastDcSavePath.WString();
    }
    return wxDirSelector(wxT("Directory to save datacenter..."), dir, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  }

  wxString OpenMapperForEncryption(wxWindow* parent, const wxString& filename)
  {
    wxString path;
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    if (cfg.RootDir.Length())
    {
      path = cfg.RootDir.WString();
    }
    return wxFileSelector(wxT("Mapper file to encrypt..."), path, filename, wxEmptyString, wxS("Text files (*.txt)|*.txt"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, parent);
  }

  wxString OpenMapperForDecryption(wxWindow* parent, const wxString& filename)
  {
    wxString path;
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    if (cfg.RootDir.Length())
    {
      path = cfg.RootDir.WString();
    }
    return wxFileSelector(wxT("Mapper file to decrypt..."), path, filename, wxEmptyString, wxS("Mapper files (*.dat)|*.dat"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, parent);
  }

  wxString SaveEncryptedMapperFile(wxWindow* parent, const wxString& filename)
  {
    return wxFileSelector(wxT("Save encrypted file"), wxEmptyString, filename, wxEmptyString, wxS("Mapper file (*.dat)|*.dat"), wxFD_SAVE, parent);
  }

  wxString SaveDecryptedMapperFile(wxWindow* parent, const wxString& filename)
  {
    return wxFileSelector(wxT("Save decrypted file"), wxEmptyString, filename, wxEmptyString, wxS("Text file (*.txt)|*.txt"), wxFD_SAVE, parent);
  }

  wxString OpenPackageDialog(wxWindow* parent, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      FAppConfig& cfg = App::GetSharedApp()->GetConfig();
      path = cfg.LastPkgOpenPath.WString();
      if (path.Length())
      {
        wxFileName f(path);
        path = f.GetPathWithSep();
      }
    }
    wxString result = wxFileSelector(caption, path, wxEmptyString, wxEmptyString, wxS("Tera Package files (*.gpk; *.gmp; *.u; *.umap; *.upk)|*.gpk;*.gmp;*.u;*.umap;*.upk"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, parent);
    if (result.Length())
    {
      wxFileName f(result);
      FAppConfig& cfg = App::GetSharedApp()->GetConfig();
      cfg.LastPkgOpenPath = f.GetPathWithSep().ToStdWstring();
      cfg.AddLastFilePackagePath(result.ToStdWstring());
      App::GetSharedApp()->SaveConfig();
    }
    return result;
  }

  std::vector<wxString> OpenMultiPackageDialog(wxWindow* parent, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      FAppConfig& cfg = App::GetSharedApp()->GetConfig();
      path = cfg.LastPkgOpenPath.WString();
      if (path.Length())
      {
        wxFileName f(path);
        path = f.GetPathWithSep();
      }
    }
    wxFileDialog dialog(parent, caption, path, wxEmptyString, wxT("Tera Package files (*.gpk;*.gmp;*.upk;*.umap;*.u)|*.gpk;*.gmp;*.upk;*.umap;*.u"), wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
    {
      return {};
    }
    std::vector<wxString> result;
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    wxArrayString tmp;
    dialog.GetPaths(tmp);
    for (const wxString& p : tmp)
    {
      if (result.size() >= OPEN_OP_MAX_FILES)
      {
        break;
      }
      result.emplace_back(p);
      wxFileName f(p);
      cfg.LastPkgOpenPath = f.GetPathWithSep().ToStdWstring();
      cfg.AddLastFilePackagePath(result.back().ToStdWstring());
    }
    App::GetSharedApp()->SaveConfig();
    return result;
  }

  wxString SavePackageDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetPackageSavePath();
    }
    wxString result = wxFileSelector(caption, path, filename, wxT("*.gpk"), wxT("Tera Package files (*.gpk;*.gmp;*.upk;*.umap;*.u)|*.gpk;*.gmp;*.upk;*.umap;*.u"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent);
    if (result.Length())
    {
      wxFileName f(result);
      App::GetSharedApp()->SavePackageSavePath(f.GetPathWithSep());
    }
    return result;
  }

  wxString OpenTextureDialog(wxWindow* parent, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetImportPath();
    }
    wxString extensions;
    if (HasAVX2())
    {
      extensions = wxT("Image files (*.png, *.tga, *.dds)|*.png;*.tga;*.dds");
    }
    else
    {
      extensions = wxT("DDS files (*.dds)|*.dds");
    }

    wxString result = wxFileSelector(caption, path, wxEmptyString, extensions, extensions, wxFD_OPEN | wxFD_FILE_MUST_EXIST, parent);
    if (result.Length())
    {
      wxFileName f(result);
      App::GetSharedApp()->SaveImportPath(f.GetPathWithSep());
    }
    return result;
  }

  wxString SaveTextureDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    wxString path = inPath;
    if (path.empty())
    {
      path = cfg.LastExportPath.WString();
    }
    
    wxString result;
    int32 extension = cfg.LastTextureExtension;
    if (HasAVX2())
    {
      static const std::vector<std::pair<std::string, std::string>> extensions = { {".tga", "TGA Image (*.tga)"}, {".png", "PNG Image  (*.png)"}, {".dds", "DDS Texture (*.dds)"} };
      SaveDialog(parent, filename, path, caption, extensions, result, extension);
    }
    else
    {
      static const std::vector<std::pair<std::string, std::string>> extensions = { {".dds", "DDS Texture (*.dds)"} };
      SaveDialog(parent, filename, path, caption, extensions, result, extension);
      extension = 0;
    }

    if (result.Length())
    {
      wxFileName f(result);
      cfg.LastExportPath = f.GetPathWithSep().ToStdWstring();
      cfg.LastTextureExtension = extension;
      App::GetSharedApp()->SaveConfig();
    }

    return result;
  }

  wxString OpenMeshDialog(wxWindow* parent, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetImportPath();
    }
    wxString result = wxFileSelector(caption, path, wxEmptyString, wxT("*.fbx"), wxT("FBX scene file (*.fbx)|*.fbx"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, parent);
    if (result.Length())
    {
      wxFileName f(result);
      App::GetSharedApp()->SaveImportPath(f.GetPathWithSep());
    }
    return result;
  }

  wxString SaveMeshDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetExportPath();
    }

    int32 extension = cfg.SkelMeshExportConfig.LastFormat;;
    static const std::vector<std::pair<std::string, std::string>> extensions = { {".fbx", "Autodesk Filmbox (*.fbx)"}, {".psk", "ActorX PSK (*.psk)"} };
    wxString result;
    SaveDialog(parent, filename, path, caption, extensions, result, extension);
    if (result.Length())
    {
      wxFileName f(result);
      cfg.LastExportPath = f.GetPathWithSep().ToStdWstring();
      cfg.StaticMeshExportConfig.LastFormat = extension;
      cfg.SkelMeshExportConfig.LastFormat = extension;
      App::GetSharedApp()->SaveConfig();
    }
    return result;
  }

  wxString SaveSkelMeshDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetExportPath();
    }

    int32 extension = cfg.SkelMeshExportConfig.LastFormat;;
    static const std::vector<std::pair<std::string, std::string>> extensions = { {".fbx", "Autodesk Filmbox (*.fbx)"}, {".psk", "ActorX PSK (*.psk)"} };
    wxString result;
    SaveDialog(parent, filename, path, caption, extensions, result, extension);
    if (result.Length())
    {
      wxFileName f(result);
      cfg.LastExportPath = f.GetPathWithSep().ToStdWstring();
      cfg.SkelMeshExportConfig.LastFormat = extension;
      App::GetSharedApp()->SaveConfig();
    }
    return result;
  }

  wxString SaveStaticMeshDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetExportPath();
    }

    int32 extension = cfg.StaticMeshExportConfig.LastFormat;;
    static const std::vector<std::pair<std::string, std::string>> extensions = { {".fbx", "Autodesk Filmbox (*.fbx)"}, {".psk", "ActorX PSK (*.psk)"} };
    wxString result;
    SaveDialog(parent, filename, path, caption, extensions, result, extension);
    if (result.Length())
    {
      wxFileName f(result);
      cfg.LastExportPath = f.GetPathWithSep().ToStdWstring();
      cfg.StaticMeshExportConfig.LastFormat = extension;
      App::GetSharedApp()->SaveConfig();
    }
    return result;
  }

  wxString SaveAnimDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetExportPath();
    }

    int32 extension = cfg.AnimationExportConfig.LastFormat;
    static const std::vector<std::pair<std::string, std::string>> extensions = { {".fbx", "Autodesk Filmbox (*.fbx)"}, {".psa", "ActorX PSA (*.psa)"} };
    wxString result;
    SaveDialog(parent, filename, path, caption, extensions, result, extension);
    if (result.Length())
    {
      wxFileName f(result);
      cfg.LastExportPath = f.GetPathWithSep().ToStdWstring();
      cfg.AnimationExportConfig.LastFormat = extension;
      App::GetSharedApp()->SaveConfig();
    }
    return result;
  }

  wxString SaveAnimDirDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetExportPath();
    }
    wxString result = wxDirSelector(caption, path, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST, wxDefaultPosition, parent);
    if (result.Length())
    {
      App::GetSharedApp()->SaveExportPath(result);
    }
    return result;
  }

  wxString SaveSoundCueDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetExportPath();
    }
    wxString result = wxDirSelector(caption, path, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST, wxDefaultPosition, parent);
    if (result.Length())
    {
      App::GetSharedApp()->SaveExportPath(result);
    }
    return result;
  }
}

namespace REDialog
{
  int Message(wxWindow* parent, const wxString& title, const wxString& message, int style)
  {
    if (!message.Contains("\n\n"))
    {
      return wxMessageBox(title + L"\n\n" + message, wxTheApp->GetAppDisplayName(), style);
    }
    return wxMessageBox(message, wxTheApp->GetAppDisplayName(), style);
  }

  bool Auth(wxWindow* parent, const wxString& desc)
  {
    AuthDialog dlg(parent, desc);
    return dlg.ShowModal() == wxID_OK;
  }
}

wxBEGIN_EVENT_TABLE(AuthDialog, WXDialog)
EVT_MENU(wxID_OK, AuthDialog::OnOkClicked)
EVT_MENU(wxID_CANCEL, AuthDialog::OnCancelClicked)
wxEND_EVENT_TABLE()