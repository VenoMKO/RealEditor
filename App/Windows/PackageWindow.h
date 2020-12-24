#pragma once
#include <wx/frame.h>
#include <wx/splitter.h>
#include <wx/toolbar.h>
#include <wx/propgrid/manager.h>

#include "../Editors/GenericEditor.h"
#include "../Misc/ObjectTreeModel.h"

#include <map>
#include <vector>

wxDECLARE_EVENT(PACKAGE_READY, wxCommandEvent);
wxDECLARE_EVENT(SELECT_OBJECT, wxCommandEvent);
wxDECLARE_EVENT(PACKAGE_ERROR, wxCommandEvent);
wxDECLARE_EVENT(UPDATE_PROPERTIES, wxCommandEvent);

class App;
class ArchiveInfoView;
class FPackage;
class UObject;

class PackageWindow : public wxFrame {
public:
  PackageWindow(std::shared_ptr<FPackage>& package, App* application);
  ~PackageWindow();
  std::shared_ptr<FPackage> GetPackage() const
  {
    return Package;
  }
  wxString GetPackagePath() const;

  void SelectObject(const wxString& objectPath);
  void SelectObject(UObject* object);

  bool OnObjectLoaded(const std::string& id);
  void OnUpdateProperties(wxCommandEvent&);
  void FixOSG();

private:

  void InitLayout();
  void LoadObjectTree();
  void OnTick(wxTimerEvent& e);

  // Menu
  void OnNewClicked(wxCommandEvent&);
  void OnCreateModClicked(wxCommandEvent&);
  void OnOpenClicked(wxCommandEvent&);
  void OnOpenByNameClicked(wxCommandEvent&);
  void OnOpenCompositeClicked(wxCommandEvent&);
  void OnShowInExplorerClicked(wxCommandEvent&);
  void OnSaveClicked(wxCommandEvent&);
  void OnSaveAsClicked(wxCommandEvent&);
  void OnCloseClicked(wxCommandEvent&);
  void OnExitClicked(wxCommandEvent&);
  void OnSettingsClicked(wxCommandEvent&);
  void OnToggleLogClicked(wxCommandEvent&);
  void OnCloseWindow(wxCloseEvent& e);
  void OnMoveEnd(wxMoveEvent& e);
  void OnSize(wxSizeEvent& e);
  void OnMaximized(wxMaximizeEvent& e);
  void OnPatchCompositeMapClicked(wxCommandEvent&);
  void OnDecryptClicked(wxCommandEvent&);
  void OnEncryptClicked(wxCommandEvent&);
  void OnDumpCompositeObjectsClicked(wxCommandEvent&);
  void OnBulkCompositeExtract(wxCommandEvent&);
  void OnBulkPackageExport(PACKAGE_INDEX objIndex);
  void OnHelpClicked(wxCommandEvent&);

  void OnPackageReady(wxCommandEvent&);
  void OnPackageError(wxCommandEvent& e);
  void OnSelectObject(wxCommandEvent& e);

  void SidebarSplitterOnIdle(wxIdleEvent&);
  void OnObjectTreeStartEdit(wxDataViewEvent& e);
  void OnObjectTreeSelectItem(wxDataViewEvent& e);
  void OnObjectTreeContextMenu(wxDataViewEvent& e);

  void OnImportObjectSelected(INT index);
  void OnExportObjectSelected(INT index);
  void OnNoneObjectSelected();

  void SetPropertiesHidden(bool hidden);
  void SetContentHidden(bool hidden);

  void ShowEditor(GenericEditor* editor);
  void UpdateProperties(UObject* object, std::vector<FPropertyTag*> properties);

  void OnPropertiesSplitter(wxSplitterEvent& e);

  void DebugOnTestCookObject(wxCommandEvent&);

  wxDECLARE_EVENT_TABLE();

private:
  App* Application = nullptr;
  std::shared_ptr<FPackage> Package = nullptr;
  ObjectTreeDataViewCtrl* ObjectTreeCtrl = nullptr;
  wxMenuItem* SettingsWindowMenu = nullptr;
  wxMenuItem* LogWindowMenu = nullptr;
  wxMenuItem* SaveMenu = nullptr;
  wxMenuItem* SaveAsMenu = nullptr;
  wxTextCtrl* ObjectFlagsTextfield = nullptr;
  wxStaticText* ObjectOffsetLabel = nullptr;
  wxStaticText* ObjectSizeLabel = nullptr;
  wxStaticText* ObjectPropertiesSizeLabel = nullptr;
  wxStaticText* ObjectDataSizeLabel = nullptr;
  wxTextCtrl* ExportFlagsTextfield = nullptr;
  wxToolBar* Toolbar = nullptr;
  wxPropertyGridManager* PropertiesCtrl = nullptr;
  wxPropertyCategory* PropertyRootCategory = nullptr;
  wxStaticText* ObjectTitleLabel = nullptr;
  wxSplitterWindow* ContentSplitter = nullptr;
  wxSplitterWindow* SidebarSplitter = nullptr;
  wxButton* BackButton = nullptr;
  wxButton* ForwardButton = nullptr;
  wxPanel* MainPanel = nullptr;
  wxPanel* EditorContainer = nullptr;
  wxPanel* PropertiesPanel = nullptr;
  wxPanel* ObjectInfoPanel = nullptr;
  wxImageList* ImageList = nullptr;
  wxStatusBar* StatusBar = nullptr;
  ArchiveInfoView* PackageInfoView = nullptr;

  wxMenuItem* _DebugTestCookObject = nullptr;

  std::map<PACKAGE_INDEX, GenericEditor*> Editors;
  GenericEditor* ActiveEditor = nullptr;
  wxTimer HeartBeat;

  wxObjectDataPtr<ObjectTreeModel> DataModel;
  bool ContentHidden = false;
  bool PropertiesHidden = false;
  bool DisableSizeUpdates = true;
  size_t ProcessedItemCnt = 0;
  wxDataViewItem RootExport;
  wxDataViewItemArray Exports;
  int PropertiesPos = 0;
  bool FixedOSG = false;
};