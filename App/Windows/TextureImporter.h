#pragma once
#include <wx/wx.h>
#include <Tera/UTexture.h>

class TextureImporter : public wxDialog {
public:

  static wxString LoadImageDialog(wxWindow* parent);
  static wxString SaveImageDialog(wxWindow* parent, const wxString& defaultFileName = wxEmptyString);

  TextureImporter(wxWindow* parent, EPixelFormat fmt, bool bNormal, bool bSRGB, TextureAddress addressX, TextureAddress addressY);

  EPixelFormat GetPixelFormat() const;
  TextureAddress GetAddressX() const;
  TextureAddress GetAddressY() const;
  MipFilterType GetMipFilter() const;
  bool IsNormal() const;
  bool IsSRGB() const;
  bool GetGenerateMips() const;

protected:
  void OnNormalClick(wxCommandEvent&);
  void OnGenMipsClicked(wxCommandEvent&);
  void OnSRGBClick(wxCommandEvent&);

protected:
  wxChoice* PixelFormat = nullptr;
  wxCheckBox* Normal = nullptr;
  wxCheckBox* SRGB = nullptr;
  wxCheckBox* GenMips = nullptr;
  wxChoice* MipFilter = nullptr;
  wxChoice* AddressX = nullptr;
  wxChoice* AddressY = nullptr;
  wxButton* CancelButton = nullptr;
  wxButton* ImportButton = nullptr;

  DECLARE_EVENT_TABLE();
};