#pragma once
namespace IODialog
{
  wxString GetLastTextureExtension();

  wxString SaveDatacenter(int mode = 0, wxWindow* parent = nullptr, const wxString& path = wxEmptyString, const wxString& filename = wxEmptyString);

  wxString OpenMapperForEncryption(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString);
  wxString OpenMapperForDecryption(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString);
  wxString SaveEncryptedMapperFile(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString);
  wxString SaveDecryptedMapperFile(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString);

  wxString OpenPackageDialog(wxWindow* parent = nullptr, const wxString& path = wxEmptyString, const wxString& caption = wxT("Open Tera package..."));
  std::vector<wxString> OpenMultiPackageDialog(wxWindow* parent = nullptr, const wxString& path = wxEmptyString, const wxString& caption = wxT("Open Tera packages..."));
  wxString SavePackageDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save Tera package..."));

  wxString OpenTextureDialog(wxWindow* parent = nullptr, const wxString& path = wxEmptyString, const wxString& caption = wxT("Import a texture..."));
  wxString SaveTextureDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the texture..."));

  wxString OpenMeshDialog(wxWindow* parent = nullptr, const wxString& path = wxEmptyString, const wxString& caption = wxT("Import a 3D model..."));
  wxString SaveMeshDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the 3D model..."));
  wxString SaveSkelMeshDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the 3D model..."));
  wxString SaveStaticMeshDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the 3D model..."));


  wxString SaveAnimDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the animation..."));
  wxString SaveAnimDirDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the animations..."));

  wxString SaveSoundCueDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the sound cue..."));
}

namespace REDialog
{
  int Message(wxWindow* parent, const wxString& title, const wxString& message, int style = wxOK | wxCENTRE);
  
  inline int Info(const wxString& message, const wxString& title = wxT("Done!"), wxWindow* parent = nullptr, int style = wxOK | wxCENTRE)
  {
    return Message(parent, title, message, style | wxICON_INFORMATION);
  }

  inline int Error(const wxString& message, const wxString& title = wxT("Error!"), wxWindow* parent = nullptr, int style = wxOK | wxCENTRE)
  {
    return Message(parent, title, message, style | wxICON_ERROR);
  }

  inline int Warning(const wxString& message, const wxString& title = wxT("Warning!"), wxWindow* parent = nullptr, int style = wxOK | wxCENTRE)
  {
    return Message(parent, title, message, style | wxICON_WARNING);
  }

  bool Auth(wxWindow* parent = nullptr, const wxString& desc = wxEmptyString);
}