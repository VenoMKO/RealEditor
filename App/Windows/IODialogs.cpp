#include <wx/filedlg.h>
#include "../App.h"

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
}

namespace IODialog
{
  wxString OpenPackageDialog(wxWindow* parent, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      wxString tmp = App::GetSharedApp()->GetPackageOpenPath();
      if (tmp.Length())
      {
        wxFileName f(tmp);
        path = f.GetPath(true);
      }
    }
    wxString result = wxFileSelector(caption, path, wxEmptyString, wxEmptyString, wxS("Package files (*.gpk; *.gmp; *.u; *.umap; *.upk)|*.gpk;*.gmp;*.u;*.umap;*.upk"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, parent);
    if (result.Length())
    {
      App::GetSharedApp()->SavePackageOpenPath(result);
    }
    return result;
  }

  wxString SavePackageDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetPackageSavePath();
    }
    wxString result = wxFileSelector(caption, path, filename, wxT("*.gpk"), wxT("Tera Package files (*.gpk;*.gmp;*.upk)|*.gpk;*.gmp;*.upk"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent);
    if (result.Length())
    {
      wxFileName f(result);
      App::GetSharedApp()->SavePackageSavePath(f.GetPath(true));
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
      App::GetSharedApp()->SaveImportPath(f.GetPath(true));
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
      cfg.LastExportPath = f.GetPath(true).ToStdWstring();
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
      wxString tmp = App::GetSharedApp()->GetImportPath();
      if (tmp.Length())
      {
        wxFileName f(tmp);
        path = f.GetPath(true);
      }
    }
    wxString result = wxFileSelector(caption, path, wxEmptyString, wxT("*.fbx"), wxT("FBX scene file (*.fbx)|*.fbx"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, parent);
    if (result.Length())
    {
      wxFileName f(result);
      App::GetSharedApp()->SaveImportPath(f.GetPath(true));
    }
    return result;
  }

  wxString SaveMeshDialog(wxWindow* parent, const wxString& filename, const wxString& inPath, const wxString& caption)
  {
    wxString path = inPath;
    if (path.empty())
    {
      path = App::GetSharedApp()->GetExportPath();
    }
    wxString result = wxFileSelector(caption, path, filename, wxT("*.fbx"), wxT("FBX scene file (*.fbx)|*.fbx"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent);
    if (result.Length())
    {
      wxFileName f(result);
      App::GetSharedApp()->SaveExportPath(f.GetPath(true));
    }
    return result;
  }
}
