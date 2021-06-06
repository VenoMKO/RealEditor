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
  wxString SavePackageDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save Tera package..."));

  wxString OpenTextureDialog(wxWindow* parent = nullptr, const wxString& path = wxEmptyString, const wxString& caption = wxT("Import a texture..."));
  wxString SaveTextureDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the texture..."));

  wxString OpenMeshDialog(wxWindow* parent = nullptr, const wxString& path = wxEmptyString, const wxString& caption = wxT("Import a 3D model..."));
  wxString SaveMeshDialog(wxWindow* parent = nullptr, const wxString& filename = wxEmptyString, const wxString& path = wxEmptyString, const wxString& caption = wxT("Save the 3D model..."));
}