#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>

#include <memory>

class FPackage;
class PackageWindow;
class ArchiveInfoView : public wxPanel
{
public:
  ArchiveInfoView(wxPanel* parent, PackageWindow* window, FPackage* package);
  void UpdateInfo();

protected:
  void OnFindObjectByName(wxCommandEvent& e);
  void OnFindObjectByIndex(wxCommandEvent& e);
  void OnNameContextMenu(wxDataViewEvent& e);

protected:
  FPackage* Package = nullptr;
  PackageWindow* Window = nullptr;
  wxDataViewCtrl* NamesTable = nullptr;
  wxStaticText* FileVersion = nullptr;
  wxStaticText* LicenseeVersion = nullptr;
  wxStaticText* HeaderSize = nullptr;
  wxStaticText* EngineVersion = nullptr;
  wxStaticText* ContentVersion = nullptr;
  wxStaticText* PackageSource = nullptr;
  wxStaticText* GUID = nullptr;
  wxStaticText* FolderName = nullptr;
  wxStaticText* Flags = nullptr;
  wxStaticText* Compression = nullptr;
  wxStaticText* AdditionalPackages = nullptr;
  wxDataViewCtrl* GenerationsTable = nullptr;
  wxButton* FindObjByName = nullptr;
  wxButton* FindObjByIndex = nullptr;
};