#pragma once
#include <wx/wx.h>
#include <Tera/UTexture.h>

class TextureImporter : public wxDialog {
public:
  TextureImporter(wxWindow* parent, EPixelFormat fmt, bool bNormal, bool bSRGB, TextureAddress addressX, TextureAddress addressY);

  EPixelFormat GetPixelFormat() const;
  TextureAddress GetAddressX() const;
  TextureAddress GetAddressY() const;
  bool GetIsNormal() const;
  bool GetIsSRGB() const;

protected:
  void OnNormalClick(wxCommandEvent&);
  void OnSRGBClick(wxCommandEvent&);

protected:
  wxChoice* PixelFormat = nullptr;
  wxCheckBox* Normal = nullptr;
  wxCheckBox* SRGB = nullptr;
  wxChoice* AddressX = nullptr;
  wxChoice* AddressY = nullptr;
  wxButton* CancelButton = nullptr;
  wxButton* ImportButton = nullptr;

  DECLARE_EVENT_TABLE();
};