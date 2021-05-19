#include "PackageWindow.h"
#include "ProgressWindow.h"
#include "CompositePackagePicker.h"
#include "CompositePatcherWindow.h"
#include "CookingOptions.h"
#include "CreateModWindow.h"
#include "ObjectPicker.h"
#include "TextureImporter.h"
#include "IODialogs.h"
#include "../Misc/ArchiveInfo.h"
#include "../Misc/ObjectProperties.h"
#include "../App.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>

#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/evtloop.h>
#include <wx/clipbrd.h>
#include <wx/richmsgdlg.h>

#include <Utils/ALog.h>
#include <Tera/Cast.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>
#include <Tera/UClass.h>
#include <Tera/ULevel.h>
#include <Utils/TextureProcessor.h>

#include <ShlObj.h>

enum ControlElementId {
  New = wxID_HIGHEST + 1,
  CreateMod,
  Open,
  OpenByName,
  OpenComposite,
  ShowInExplorer,
  Save,
  SaveAs,
  Close,
  Exit,
  SettingsWin,
  LogWin,
  Help,
  CompositePatch,
  DecryptMapper,
  EncryptMapper,
  DumpObjectsMap,
  BulkCompositeExtract,
  Import,
  Export,
  DebugTestCookObj,
  DebugSplitMod,
  DebugDup,
  Back,
  Forward,
  ContentSplitter,
  HeartBeat,
  OpenRecentStart /* OpenRecentStart must be the last id. OpenRecentStart + n - recent file at index n */
};

enum ObjTreeMenuId {
  CopyName = wxID_HIGHEST + 1,
  CopyPath,
  BulkImport,
  BulkExport,
  Add,
  AddPackage,
  AddTexture,
  AddMaterial,
  Duplicate,
};

#define MAX_SELECTION_HISTORY 50

wxDEFINE_EVENT(PACKAGE_READY, wxCommandEvent); 
wxDEFINE_EVENT(PACKAGE_ERROR, wxCommandEvent);
wxDEFINE_EVENT(SELECT_OBJECT, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_PROPERTIES, wxCommandEvent);

const wxString HelpUrl = wxS("https://github.com/VenoMKO/RealEditor/wiki");

#include "PackageWindowLayout.h"

PackageWindow::PackageWindow(std::shared_ptr<FPackage>& package, App* application)
  : wxFrame(nullptr, wxID_ANY, wxEmptyString)
  , Application(application)
  , Package(package)
{
  wxString title = application->GetAppDisplayName() + wxT(" ") + GetAppVersion() + wxT(" - ");
  if (package->IsComposite())
  {
    std::wstring sub = package->GetCompositePath();
    if (sub.empty())
    {
      title += package->GetSourcePath().WString();
    }
    else
    {
      std::replace(sub.begin(), sub.end(), '.', '\\');
      title += wxS("Composite: ") + sub;
    }
  }
  else
  {
    title += package->GetSourcePath().WString();
  }
  SelectionHistory.reserve(MAX_SELECTION_HISTORY);
  SetTitle(title);
  SetIcon(wxICON(#114));
  SetSizeHints(wxSize(1024, 700), wxDefaultSize);
  InitLayout();
  
  ObjectTreeCtrl->SetFocus();
  SetPropertiesHidden(true);
  SetContentHidden(true);

  wxPoint pos = Application->GetLastWindowPosition();
  if (pos.x == WIN_POS_FULLSCREEN)
  {
    Maximize();
  }
  else if (pos.x == WIN_POS_CENTER)
  {
    CenterOnScreen();
    Application->SetLastWindowPosition(GetPosition());
  }
  else
  {
    pos.x += 25; pos.y += 25;
    SetPosition(pos);
    Application->SetLastWindowPosition(pos);
  }
  if (pos.x != WIN_POS_FULLSCREEN)
  {
    SetSize(Application->GetLastWindowSize());
  }
  OnNoneObjectSelected();
  {
    wxSize hint;
    float dx;
    hint = Application->GetLastWindowObjectSash();
    dx = float(hint.x) / float(hint.y);
    SidebarSplitter->SetSashPosition(SidebarSplitter->GetSize().x * dx);

    hint = Application->GetLastWindowPropSash();
    PropertiesPos = hint.y - hint.x;
    ContentSplitter->SetSashPosition(PropertiesPos);
    ContentSplitter->SetSashGravity(1);
  }
  
  ObjectTreeCtrl->Bind(wxEVT_SIZE, &PackageWindow::OnSize, this);
  PropertiesCtrl->Bind(wxEVT_SIZE, &PackageWindow::OnSize, this);
  HeartBeat.Bind(wxEVT_TIMER, &PackageWindow::OnTick, this);
  HeartBeat.Start(1);
}

void PackageWindow::OnCloseWindow(wxCloseEvent& event)
{
  if (!Package->IsReady())
  {
    Package->CancelOperation();
  }
  Application->PackageWindowWillClose(this);
  wxFrame::OnCloseWindow(event);
}

PackageWindow::~PackageWindow()
{
  FPackage::UnloadPackage(Package);
  delete FileHistory;
  delete ImageList;
}

wxString PackageWindow::GetPackagePath() const
{
  return wxString(Package->GetSourcePath().String());
}

void PackageWindow::SelectObject(const wxString& objectPath)
{
  std::vector<std::string> components;
  std::istringstream f(objectPath.ToStdString());
  std::string tmp;
  while (std::getline(f, tmp, '.'))
  {
    if (tmp.size())
    {
      components.push_back(tmp);
    }
  }
  if (components.empty())
  {
    return;
  }
  FObjectResource* found = nullptr;

  std::vector<FObjectExport*> searchItems = Package->GetRootExports();
  for (int32 idx = 1; idx < components.size(); ++idx)
  {
    std::string upper = components[idx];
    std::transform(upper.begin(), upper.end(), upper.begin(),
      [](unsigned char c) { return std::toupper(c); }
    );
    for (FObjectExport* exp : searchItems)
    {
      if (exp->GetObjectName().ToUpper() == upper)
      {
        found = exp;
        searchItems = exp->Inner;
        break;
      }
    }
  }
  if (!found)
  {
    std::vector<FObjectImport*> searchItems = Package->GetRootImports();
    for (int32 idx = 1; idx < components.size(); ++idx)
    {
      std::string upper = components[idx];
      std::transform(upper.begin(), upper.end(), upper.begin(),
        [](unsigned char c) { return std::toupper(c); }
      );
      for (FObjectImport* imp : searchItems)
      {
        if (imp->GetObjectName().ToUpper() == upper)
        {
          found = imp;
          searchItems = imp->Inner;
          break;
        }
      }
    }
  }
  if (found && found->ObjectIndex)
  {
    if (ObjectTreeNode* node = ((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->FindItemByObjectIndex(found->ObjectIndex))
    {
      auto item = wxDataViewItem(node);
      ObjectTreeCtrl->Select(item);
      ObjectTreeCtrl->EnsureVisible(item);
      OnExportObjectSelected(found->ObjectIndex);
    }
  }
}

void PackageWindow::SelectObject(UObject* object)
{
  if (!object)
  {
    return;
  }
  PACKAGE_INDEX idx = GetPackage()->GetObjectIndex(object);
  if (idx == INDEX_NONE || idx == 0)
  {
    return;
  }
  if (ObjectTreeNode* node = ((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->FindItemByObjectIndex(idx))
  {
    auto item = wxDataViewItem(node);
    ObjectTreeCtrl->Select(item);
    ObjectTreeCtrl->EnsureVisible(item);
    OnExportObjectSelected(idx);
  }
}

bool PackageWindow::OnObjectLoaded(const std::string& id)
{
  auto editors = Editors;
  auto active = ActiveEditor;
  for (const auto p : editors)
  {
    if (p.second->GetEditorId() == id)
    {
      p.second->OnObjectLoaded();
      if (active == p.second)
      {
        UObject* obj = active->GetObject();
        ObjectSizeLabel->SetLabelText(wxString::Format("0x%08X", obj->GetSerialSize()));
        ObjectOffsetLabel->SetLabelText(wxString::Format("0x%08X", obj->GetSerialOffset()));
        ObjectPropertiesSizeLabel->SetLabelText(wxString::Format("0x%08X", obj->GetPropertiesSize()));
        ObjectDataSizeLabel->SetLabelText(wxString::Format("0x%08X", obj->GetDataSize()));
        UpdateProperties(obj, active->GetObjectProperties());
        active->PopulateToolBar(Toolbar);
        Toolbar->Realize();
      }
      return true;
    }
  }
  return false;
}

void PackageWindow::OnUpdateProperties(wxCommandEvent&)
{
  if (ActiveEditor)
  {
    UObject* obj = ActiveEditor->GetObject();
    UpdateProperties(obj, ActiveEditor->GetObjectProperties());
  }
}

void PackageWindow::LoadObjectTree()
{
  DataModel = new ObjectTreeModel(Package->GetPackageName(), Package->GetRootExports(), Package->GetRootImports());
  DataModel->GetRootExport()->SetCustomObjectIndex(FAKE_EXPORT_ROOT);
  DataModel->GetRootImport()->SetCustomObjectIndex(FAKE_IMPORT_ROOT);
  ObjectTreeCtrl->AssociateModel(DataModel.get());
  wxDataViewColumn* col = new wxDataViewColumn("title", new wxDataViewIconTextRenderer, 1, wxDVC_DEFAULT_WIDTH, wxALIGN_LEFT);
  ObjectTreeCtrl->AppendColumn(col);
  col->SetWidth(ObjectTreeCtrl->GetSize().x - 4);
}

void PackageWindow::OnTick(wxTimerEvent& e)
{
  if (ActiveEditor && !IsIconized() && GetForegroundWindow() == GetHWND())
  {
    ActiveEditor->OnTick();
  }
}

void PackageWindow::OnRecentClicked(wxCommandEvent& e)
{
  wxString path(FileHistory->GetHistoryFile(e.GetId() - ControlElementId::OpenRecentStart));
  if (path.StartsWith("composite\\"))
  {
    if (App::GetSharedApp()->OpenNamedPackage(path.Mid(10, path.Find('.') - 10)))
    {
      App::SavePackageOpenPath(path);
    }
    return;
  }
  if (App::GetSharedApp()->OpenPackage(path))
  {
    App::SavePackageOpenPath(path);
  }
}

void PackageWindow::SidebarSplitterOnIdle(wxIdleEvent&)
{
  SidebarSplitter->Disconnect(wxEVT_IDLE, wxIdleEventHandler(PackageWindow::SidebarSplitterOnIdle), NULL, this);
  DisableSizeUpdates = false;
}

void PackageWindow::OnObjectTreeSelectItem(wxDataViewEvent& e)
{
  ObjectTreeNode* node = (ObjectTreeNode*)ObjectTreeCtrl->GetCurrentItem().GetID();
  if (!node)
  {
    EditorContainer->Show(true);
    if (PackageInfoView)
    {
      PackageInfoView->Show(false);
    }
    OnNoneObjectSelected();
    return;
  }
  PACKAGE_INDEX index = node->GetObjectIndex();
  if (index < 0)
  {
    OnImportObjectSelected(index);
  }
  else
  {
    OnExportObjectSelected(index);
  }
}
                                           
void PackageWindow::OnObjectTreeContextMenu(wxDataViewEvent& e)
{
  if (!e.GetItem().IsOk())
  {
    return;
  }
  ObjectTreeNode* node = (ObjectTreeNode*)ObjectTreeCtrl->GetCurrentItem().GetID();
  if (!node)
  {
    return;
  }

  wxMenu menu;
  menu.SetClientData((void*)node);
  bool allowEdit = !Package->GetPackageFlag(PKG_ContainsMap) && !Package->GetPackageFlag(PKG_ContainsScript);
  if (node->GetObjectIndex() >= 0)
  {
    if (node->GetClassName() == NAME_Package || node->GetObjectIndex() == FAKE_EXPORT_ROOT)
    {
      if (allowEdit)
      {
        wxMenu* addMenu = new wxMenu;
        addMenu->Append(ObjTreeMenuId::AddPackage, wxT("Package..."));
        // TODO: implement material wizard
        // addMenu->Append(ObjTreeMenuId::AddMaterial, wxT("Material Instance..."));
        addMenu->Append(ObjTreeMenuId::AddTexture, wxT("Texture..."));
        menu.AppendSubMenu(addMenu, wxT("Add"));
      }
      menu.Append(ObjTreeMenuId::BulkExport, wxT("Export package..."));
    }
    else if (allowEdit)
    {
      menu.Append(ObjTreeMenuId::BulkImport, wxT("Bulk import..."));
    }
  }
  if (node->GetParent())
  {
    if (node->GetClassName() != NAME_Package)
    {
      menu.Append(ObjTreeMenuId::Duplicate, wxT("Duplicate..."));
    }
    menu.Append(ObjTreeMenuId::CopyName, wxT("Copy object name"));
    menu.Append(ObjTreeMenuId::CopyPath, wxT("Copy object path"));
  }
  
  if (!menu.GetMenuItemCount())
  {
    return;
  }

  switch (GetPopupMenuSelectionFromUser(menu))
  {
  case CopyName:
    if (wxTheClipboard->Open())
    {
      wxTheClipboard->Clear();
      wxTheClipboard->SetData(new wxTextDataObject(node->GetObjectName()));
      wxTheClipboard->Flush();
      wxTheClipboard->Close();
    }
    break;
  case CopyPath:
    if (wxTheClipboard->Open())
    {
      wxTheClipboard->Clear();
      wxTheClipboard->SetData(new wxTextDataObject(Package->GetObject(node->GetObjectIndex())->GetObjectPath().WString()));
      wxTheClipboard->Flush();
      wxTheClipboard->Close();
    }
    break;
  case BulkImport:
    App::GetSharedApp()->ShowBulkImport(this, node->GetClassName(), node->GetObjectName());
    break;
  case BulkExport:
    OnBulkPackageExport(node->GetObjectIndex());
    break;
  case AddPackage:
    OnAddPackageClicked(node->GetObjectIndex());
    break;
  case AddTexture:
    OnAddTextureClicked(node->GetObjectIndex());
    break;
  case AddMaterial:
    OnAddMaterialClicked(node->GetObjectIndex());
    break;
  case Duplicate:
    OnDuplicateClicked(node->GetObjectIndex());
    break;
  default:
    break;
  }
}

void PackageWindow::OnImportObjectSelected(INT index)
{
  ShowEditor(nullptr);
  if (PackageInfoView)
  {
    PackageInfoView->Show(false);
    EditorContainer->Show(true);
  }
  if (!PerformingHistoryNavigation)
  {
    if (SelectionHistoryPos && SelectionHistoryPos < SelectionHistory.size())
    {
      SelectionHistory.erase(SelectionHistory.begin(), SelectionHistory.begin() + SelectionHistoryPos);
    }
    auto it = std::find(SelectionHistory.begin(), SelectionHistory.end(), index);
    if (it != SelectionHistory.end())
    {
      SelectionHistory.erase(it);
    }
    SelectionHistory.insert(SelectionHistory.begin(), index);
    SelectionHistoryPos = 0;
  }
  BackButton->Enable(SelectionHistory.size() && SelectionHistory.size() - 1 > SelectionHistoryPos);
  ForwardButton->Enable(SelectionHistory.size() && SelectionHistoryPos);
  if (index == FAKE_IMPORT_ROOT)
  {
    ObjectTitleLabel->SetLabelText(GetPackage()->GetPackageName(true).WString() + L" imports");
    ObjectTitleLabel->SetToolTip(wxEmptyString);
    SetPropertiesHidden(true);
    SetContentHidden(true);
    return;
  }
  FObjectImport* obj = Package->GetImportObject(index);
  ObjectTitleLabel->SetLabelText(wxString::Format(wxT("Import: %ls (%ls)"), obj->GetObjectName().WString().c_str(), obj->GetClassName().WString().c_str()));
  ObjectTitleLabel->SetToolTip(wxEmptyString);
  SetPropertiesHidden(true);
  SetContentHidden(true);
}

void PackageWindow::OnExportObjectSelected(INT index)
{
  if (!PerformingHistoryNavigation)
  {
    if (SelectionHistoryPos && SelectionHistoryPos < SelectionHistory.size())
    {
      SelectionHistory.erase(SelectionHistory.begin(), SelectionHistory.begin() + SelectionHistoryPos);
    }
    if (SelectionHistory.empty() || SelectionHistory.front() != index)
    {
      SelectionHistory.insert(SelectionHistory.begin(), index);
    }
    if (SelectionHistory.size() > MAX_SELECTION_HISTORY)
    {
      SelectionHistory.pop_back();
    }
    SelectionHistoryPos = 0;
  }
  BackButton->Enable(SelectionHistory.size() && SelectionHistory.size() - 1 > SelectionHistoryPos);
  ForwardButton->Enable(SelectionHistory.size() && SelectionHistoryPos);

  if (index == FAKE_EXPORT_ROOT || !index)
  {
    ShowEditor(nullptr);
    ObjectTitleLabel->SetLabelText(GetPackage()->GetPackageName(true).WString());
    ObjectTitleLabel->SetToolTip(wxEmptyString);
    SetPropertiesHidden(true);
    SetContentHidden(true);
    if (!PackageInfoView)
    {
      PackageInfoView = new ArchiveInfoView(MainPanel, this, Package.get());
      MainPanel->GetSizer()->Add(PackageInfoView, 1, wxEXPAND | wxALL, 0);
    }
    EditorContainer->Show(false);
    PackageInfoView->Show(true);
    MainPanel->GetSizer()->Layout();
    return;
  }
  
  FObjectExport* fobj = Package->GetExportObject(index);
  ObjectTitleLabel->SetLabelText(wxString::Format(wxT("%ls (%ls)"), fobj->GetObjectName().WString().c_str(), fobj->GetClassName().WString().c_str()));
  ObjectTitleLabel->SetToolTip(wxString::Format(wxT("Index: %d"), index));
  ObjectSizeLabel->SetLabelText(wxString::Format("0x%08X", -1));
  ObjectOffsetLabel->SetLabelText(wxString::Format("0x%08X", -1));
  ObjectPropertiesSizeLabel->SetLabelText(wxString::Format("0x%08X", -1));
  ObjectDataSizeLabel->SetLabelText(wxString::Format("0x%08X", -1));
  std::string flags = ObjectFlagsToString(fobj->ObjectFlags);
  ObjectFlagsTextfield->SetLabelText(flags);
  flags = ExportFlagsToString(fobj->ExportFlags);
  ExportFlagsTextfield->SetLabelText(flags);
  SetPropertiesHidden(false);
  SetContentHidden(false);

  {
    auto it = Editors.find(index);
    if (it != Editors.end())
    {
      if (it->second == ActiveEditor)
      {
        return;
      }
      Toolbar->ClearTools();
      ShowEditor(it->second);
      it->second->LoadObject();
    }
    else
    {
      Toolbar->ClearTools();
      GenericEditor* editor = GenericEditor::CreateEditor(EditorContainer, this, Package->GetObject(index, false));
      ShowEditor(editor);
      editor->LoadObject();
    }
  }
}

void PackageWindow::UpdateProperties(UObject* object, std::vector<FPropertyTag*> properties)
{
  PropertiesCtrl->Freeze();
  if (!PropertyRootCategory)
  {
    PropertyRootCategory = new wxPropertyCategory(object->GetObjectName().WString());
    PropertyRootCategory->SetValue(object->GetClassName().String());
    PropertyRootCategory->SetHelpString(L"Object: " + object->GetObjectPath().WString()  + L"\nClass: " + object->GetClassName().WString());
    PropertiesCtrl->Append(PropertyRootCategory);
  }
  else
  {
    PropertyRootCategory->DeleteChildren();
    PropertyRootCategory->SetLabel(object->GetObjectName().WString());
    PropertyRootCategory->SetValue(object->GetClassName().String());
  }

  // TODO: enable if the object is fexp && !Package->IsReadOnly()
  // PropertyRootCategory->Enable(!Package->IsReadOnly() && object->GetPackage() == Package.get());
  CreateProperty(PropertiesCtrl, PropertyRootCategory, properties);
  PropertiesCtrl->Thaw();

  PropertiesCtrl->RefreshGrid();
  PropertyRootCategory->RefreshChildren();
}

void PackageWindow::DebugOnTestCookObject(wxCommandEvent&)
{
  if (!ActiveEditor || !ActiveEditor->GetObject() || ActiveEditor->GetObject()->GetPackage() != Package.get())
  {
    return;
  }
  wxString path = wxSaveFileSelector("object", wxT("BIN file|*.bin"), ActiveEditor->GetObject()->GetObjectName().WString(), this);
  if (path.empty())
  {
    return;
  }
  FWriteStream s(path.ToStdWstring());
  s.SetPackage(Package.get());
  ActiveEditor->GetObject()->Serialize(s);
}

void PackageWindow::DebugOnSplitMod(wxCommandEvent&)
{
  wxString path = wxLoadFileSelector("Mod", wxT("GPK file|*.gpk"), wxEmptyString, this);
  if (path.empty())
  {
    return;
  }
  FReadStream rs(path.ToStdWstring());
  rs.SetPosition(rs.GetSize() - 4);
  int32 magic = 0;
  rs << magic;
  if (magic != PACKAGE_MAGIC)
  {
    return;
  }

  int32 containerOffset = 0;
  int32 containerOffsets = 0;
  int32 containerOffsetsCount = 0;
  int32 metaSize = 0;

  rs.SetPosition(rs.GetSize() - (5 * sizeof(int32)));
  rs << containerOffset;
  rs << containerOffsets;
  rs << containerOffsetsCount;
  rs << metaSize;

  std::vector<int32> offsets;
  rs.SetPosition(containerOffsets);
  for (int32 idx = 0; idx < containerOffsetsCount; ++idx)
  {
    int32 tmp = 0;
    rs << tmp;
    offsets.push_back(tmp);
  }

  if (offsets.empty())
  {
    return;
  }

  wxDirDialog dlg(NULL, "Select a directory to extract packages to...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (dlg.ShowModal() != wxID_OK || dlg.GetPath().empty())
  {
    return;
  }

  for (int32 idx = 0; idx < offsets.size(); ++idx)
  {
    rs.SetPosition(offsets[idx]);
    int32 len = 0;
    if (idx + 1 < offsets.size())
    {
      len = offsets[idx + 1] - offsets[idx];
    }
    else
    {
      len = rs.GetSize() - metaSize;
    }

    if (len <= 0)
    {
      continue;
    }

    void* data = malloc(len);
    FWriteStream ws(dlg.GetPath().ToStdWstring() + L"\\Package_" + std::to_wstring(idx) + L".gpk");
    rs.SerializeBytes(data, len);
    ws.SerializeBytes(data, len);
    free(data);
  }

  offsets.clear();
}

void PackageWindow::DebugOnDupSelection(wxCommandEvent&)
{
  if (!ActiveEditor)
  {
    return;
  }
  UObject* mi = ActiveEditor->GetObject();
  FObjectExport* parent = mi->GetExportObject()->Outer;
  if (!mi)
  {
    return;
  }
  std::weak_ptr packageWeak = Package;
  ObjectNameDialog::Validator validatorFunc = [&](const wxString& name) {
    std::shared_ptr<FPackage> package = packageWeak.lock();
    if (!package)
    {
      return false;
    }
    if (name.Find(" ") != wxString::npos)
    {
      return false;
    }
    FString tmpName = name.ToStdWstring();
    if (!tmpName.IsAnsi())
    {
      return false;
    }
    std::vector<FObjectExport*> exps;
    if (parent)
    {
      exps = parent->Inner;
    }
    else
    {
      exps = package->GetRootExports();
    }
    for (FObjectExport* exp : exps)
    {
      if (exp->GetObjectName() == tmpName)
      {
        return false;
      }
    }
    return true;
  };
  wxString name = mi->GetObjectName().WString();
  ObjectNameDialog nameDialog = ObjectNameDialog(this, name);
  nameDialog.SetValidator(validatorFunc);

  if (nameDialog.ShowModal() != wxID_OK)
  {
    return;
  }

  name = nameDialog.GetObjectName();
  if (name.empty())
  {
    return;
  }

  if (FObjectExport* exp = Package->DuplicateExport(mi->GetExportObject(), mi->GetExportObject()->Outer, name.ToStdWstring()))
  {
    ObjectTreeCtrl->AddExportObject(exp);
    OnExportObjectSelected(exp->ObjectIndex);
  }
}

void PackageWindow::FixOSG()
{
  // Osg refuses to process events
  // Resizing the window for some reasons fixes the issue
  // TODO: fix the issue and get rid of the shitty hack below
  if (FixedOSG)
  {
    return;
  }
  Freeze();
  if (IsMaximized())
  {
    Maximize(false);
    Maximize(true);
  }
  else
  {
    wxSize s = GetSize();
    s.x += 1;
    SetSize(s);
    s.x -= 1;
    SetSize(s);
    
  }
  FixedOSG = true;
  Thaw();
}

int PackageWindow::GetOpenRecentId()
{
  return ControlElementId::OpenRecentStart;
}

bool PackageWindow::Show(bool show /*= true*/)
{
  bool result = wxFrame::Show(show);
  if (show && !MonitorFromWindow(GetHandle(), MONITOR_DEFAULTTONULL))
  {
    Center();
  }
  return result;
}

void PackageWindow::OnNoneObjectSelected()
{
  ObjectTitleLabel->SetLabelText("No selection");
  SetPropertiesHidden(true);
  SetContentHidden(true);
  ShowEditor(nullptr);
}

void PackageWindow::OnNewClicked(wxCommandEvent&)
{

}

void PackageWindow::OnCreateModClicked(wxCommandEvent&)
{
  wxFileDialog fileDialog(this, wxT("Select modded packages"), wxEmptyString,
    wxEmptyString, wxT("Tera Game Package (*.gpk, *.tfc)|*.gpk;*.tfc"),
    wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

  if (fileDialog.ShowModal() != wxID_OK)
  {
    return;
  }
  wxArrayString result;
  fileDialog.GetPaths(result);
  if (result.empty())
  {
    return;
  }
  std::vector<FString> paths;
  for (const wxString& str : result)
  {
    paths.push_back(str.ToStdWstring());
  }

  CreateModWindow modInfo(this);
  if (modInfo.ShowModal() != wxID_OK)
  {
    return;
  }

  wxString dest = wxSaveFileSelector("mod", "gpk", modInfo.GetName(), this);
  if (dest.empty())
  {
    return;
  }

  App::GetSharedApp()->GetConfig().LastModAuthor = modInfo.GetAuthor().ToStdWstring();
  App::GetSharedApp()->SaveConfig();

  try
  {
    FPackage::CreateCompositeMod(paths, dest.ToStdWstring(), modInfo.GetName().ToStdString(), modInfo.GetAuthor().ToStdString());
  }
  catch (const std::exception& e)
  {
    wxMessageBox(e.what(), "Error!", wxICON_ERROR, this);
  }
}

void PackageWindow::OnOpenClicked(wxCommandEvent&)
{
  wxString path = IODialog::OpenPackageDialog(this);
  if (path.size())
  {
    Application->OpenPackage(path);
  }
}

void PackageWindow::OnOpenByNameClicked(wxCommandEvent&)
{
  wxString name = Application->ShowOpenByNameDialog(this);
  if (name.size())
  {
    Application->OpenNamedPackage(name);
  }
}

void PackageWindow::OnOpenCompositeClicked(wxCommandEvent&)
{
  wxString name = Application->ShowOpenCompositeDialog(this);
  if (name.size())
  {
    Application->OpenNamedPackage(name);
  }
}

void PackageWindow::OnShowInExplorerClicked(wxCommandEvent&)
{
  FString path = Package->GetSourcePath();
  if (Package->GetCompositeSourcePath().Size())
  {
    path = Package->GetCompositeSourcePath();
  }
  if (ITEMIDLIST* pidl = ILCreateFromPathW(path.WString().c_str())) {
    SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
    ILFree(pidl);
  }
}

void PackageWindow::OnSaveClicked(wxCommandEvent& e)
{
  if (Package->IsComposite())
  {
    wxMessageBox(wxT("This package is composite. To save this package use \"Save as\"."), wxT("Error!"), wxICON_ERROR, this);
    return;
  }

  FAppConfig& cfg = App::GetSharedApp()->GetConfig();
  if (!cfg.SavePackageDontShowAgain)
  {
    wxRichMessageDialog dlg(this, wxT("You are about to overwrite the original package. This action can not be undone.\nPress \"Yes\" to save the package."), wxT("Are you sure?"), wxICON_INFORMATION | wxYES_NO);
    dlg.ShowCheckBox(wxT("Don't show again"));
    if (dlg.ShowModal() != wxID_YES)
    {
      return;
    }

    if (dlg.IsCheckBoxChecked())
    {
      cfg.SavePackageDontShowAgain = true;
      App::GetSharedApp()->SaveConfig();
    }
  }
  
  

  CookingOptionsWindow optionsWindow(this, Package.get());
  if (optionsWindow.ShowModal() != wxID_OK)
  {
    return;
  }

  wxString path = GetTempFilePath().WString();
  ProgressWindow progress(this, "Saving");
  progress.SetCurrentProgress(-1);
  progress.SetActionText(wxT("Preparing..."));

  PackageSaveContext context;
  optionsWindow.ConfigureSaveContext(context);
  context.Path = W2A(path.ToStdWstring());

  context.ProgressCallback = [&progress](int value) {
    wxCommandEvent* e = new wxCommandEvent;
    e->SetId(UPDATE_PROGRESS);
    e->SetInt(value);
    wxQueueEvent(&progress, e);
  };

  context.MaxProgressCallback = [&progress](int value) {
    wxCommandEvent* e = new wxCommandEvent;
    e->SetId(UPDATE_MAX_PROGRESS);
    e->SetInt(value);
    wxQueueEvent(&progress, e);
  };

  context.IsCancelledCallback = [&progress] {
    return progress.IsCanceled();
  };

  context.ProgressDescriptionCallback = [&progress](std::string desc) {
    SendEvent(&progress, UPDATE_PROGRESS_DESC, A2W(desc));
  };

  FPackage* package = Package.get();
  std::thread([&progress, &context, package] {
    bool result = package->Save(context);
    SendEvent(&progress, UPDATE_PROGRESS_FINISH, result);
  }).detach();

  if (!progress.ShowModal())
  {
    wxString err = wxT("Failed to save the package! Error: ");
    err += A2W(context.Error);
    wxMessageBox(err, "Error!", wxICON_ERROR);
    return;
  }

  App::GetSharedApp()->SaveAndReopenPackage(Package, path.ToStdWstring(), Package->GetSourcePath());
}

void PackageWindow::OnSaveAsClicked(wxCommandEvent& e)
{
  wxString path = IODialog::SavePackageDialog(this, Package->GetPackageName(true).WString());
  if (path.empty())
  {
    return;
  }

  CookingOptionsWindow optionsWindow(this, Package.get());

  if (optionsWindow.ShowModal() != wxID_OK)
  {
    return;
  }


  ProgressWindow progress(this, "Saving");
  progress.SetCurrentProgress(-1);
  progress.SetActionText(wxT("Preparing..."));

  PackageSaveContext context;
  optionsWindow.ConfigureSaveContext(context);
  context.Path = W2A(path.ToStdWstring());

  context.ProgressCallback = [&progress](int value) {
    wxCommandEvent* e = new wxCommandEvent;
    e->SetId(UPDATE_PROGRESS);
    e->SetInt(value);
    wxQueueEvent(&progress, e);
  };

  context.MaxProgressCallback = [&progress](int value) {
    wxCommandEvent* e = new wxCommandEvent;
    e->SetId(UPDATE_MAX_PROGRESS);
    e->SetInt(value);
    wxQueueEvent(&progress, e);
  };

  context.IsCancelledCallback = [&progress] {
    return progress.IsCanceled();
  };

  context.ProgressDescriptionCallback = [&progress] (std::string desc) {
    SendEvent(&progress, UPDATE_PROGRESS_DESC, A2W(desc));
  };

  FPackage* package = Package.get();
  std::thread([&progress, &context, package] {
    bool result = package->Save(context);
    SendEvent(&progress, UPDATE_PROGRESS_FINISH, result);
  }).detach();

  if (!progress.ShowModal())
  {
    wxString err = wxT("Failed to save the package! Error: ");
    err += A2W(context.Error);
    wxMessageBox(err,"Error!", wxICON_ERROR, this);
  }
}

void PackageWindow::OnCloseClicked(wxCommandEvent& e)
{
  Close();
}

void PackageWindow::OnExitClicked(wxCommandEvent&)
{
  Application->ExitMainLoop();
}

void PackageWindow::OnSettingsClicked(wxCommandEvent&  e)
{
  Application->OnShowSettings(e);
}

void PackageWindow::OnToggleLogClicked(wxCommandEvent&)
{
  bool isShown = ALog::IsShown();
  ALog::Show(!isShown);
  Application->GetConfig().LogConfig.ShowLog = !isShown;
}

void PackageWindow::OnMoveEnd(wxMoveEvent& e)
{
  if (IsMaximized())
  {
    Application->SetLastWindowPosition(wxPoint(WIN_POS_FULLSCREEN, 0));
  }
  else
  {
    Application->SetLastWindowPosition(GetPosition());

    Application->SetLastWindowObjectSash(SidebarSplitter->GetSashPosition(), SidebarSplitter->GetSize().x);
    if (ContentSplitter->IsSplit())
    {
      Application->SetLastWindowPropertiesSash(ContentSplitter->GetSashPosition(), ContentSplitter->GetSize().x);
    }
    Application->SetLastWindowSize(GetSize());
  }
  if (ActiveEditor)
  {
    ActiveEditor->SetNeedsUpdate();
  }
  e.Skip();
}

void PackageWindow::OnSize(wxSizeEvent& e)
{
  if (!DisableSizeUpdates)
  {
    Application->SetLastWindowObjectSash(SidebarSplitter->GetSashPosition(), SidebarSplitter->GetSize().x);
    if (ContentSplitter->IsSplit())
    {
      Application->SetLastWindowPropertiesSash(ContentSplitter->GetSashPosition(), ContentSplitter->GetSize().x);
    }
    if (!IsMaximized())
    {
      Application->SetLastWindowSize(GetSize());
    }
  }
  e.Skip();
}

void PackageWindow::OnMaximized(wxMaximizeEvent& e)
{
  Application->SetLastWindowPosition(wxPoint(WIN_POS_FULLSCREEN, 0));
  e.Skip();
}

void PackageWindow::OnPatchCompositeMapClicked(wxCommandEvent&)
{
  CompositePatcherWindow patcher(this, Package->GetPackageName().WString());
  patcher.ShowModal();
}

void PackageWindow::OnDecryptClicked(wxCommandEvent&)
{
  wxMessageBox(_("Select a source file you want to decrypt(e.g. CompositePackageMapper.dat). You can find these files in your S1Game\\CookedPC folder."), _("Select source..."), wxICON_INFORMATION);
  wxString source = IODialog::OpenMapperForDecryption(this);
  if (source.empty())
  {
    return;
  }
  wxString destFileName = std::filesystem::path(source.ToStdWstring()).filename().replace_extension(".txt").wstring();
  wxMessageBox(_("Select where you want to save the decrypted file."), _("Select destination..."), wxICON_INFORMATION);
  wxString destination = IODialog::SaveDecryptedMapperFile(this, destFileName);
  if (destination.empty())
  {
    return;
  }

  ProgressWindow progress(this, "Decrypting...");
  progress.SetCanCancel(false);
  progress.SetCurrentProgress(-1);
  progress.SetActionText("Decrypting the file...");

  std::thread([&progress, source, destination] {
    std::string output;
    try
    {
      GDecrytMapperFile(source.ToStdWstring(), output);
    }
    catch (...)
    {
      wxMessageBox(_("Failed to decrypt the file!"), _("Error!"), wxICON_ERROR);
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    try
    {
      std::ofstream os(destination.ToStdWstring(), std::ios::out | std::ios::binary);
      os.write(&output[0], output.size());
    }
    catch (...)
    {
      wxMessageBox(_("Failed to save the file!"), _("Error!"), wxICON_ERROR);
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();
}

void PackageWindow::OnEncryptClicked(wxCommandEvent&)
{
  wxMessageBox(_("Select a source file you want to encrypt(e.g. CompositePackageMapper.txt)"), _("Select source..."), wxICON_INFORMATION);
  wxString source = IODialog::OpenMapperForEncryption(this);
  if (source.empty())
  {
    return;
  }
  wxString destFileName = std::filesystem::path(source.ToStdWstring()).filename().replace_extension(".dat").wstring();
  wxMessageBox(_("Select where you want to save the encrypted file."), _("Select destination..."), wxICON_INFORMATION);
  wxString destination = IODialog::SaveEncryptedMapperFile(this, destFileName);
  if (destination.empty())
  {
    return;
  }

  ProgressWindow progress(this, "Encrypting...");
  progress.SetCanCancel(false);
  progress.SetCurrentProgress(-1);
  progress.SetActionText("Encrypting the file...");

  std::thread([&progress, source, destination] {
    std::string decrypted;
    try
    {
      std::ifstream s(source.ToStdWstring(), std::ios::in | std::ios::binary);
      s.seekg(0, std::ios::end);
      size_t size = s.tellg();
      decrypted.resize(size);
      s.seekg(0);
      s.read(&decrypted[0], size);
    }
    catch (...)
    {
      wxMessageBox(_("Failed to read the source file!"), _("Error!"), wxICON_ERROR);
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    try
    {
      GEncrytMapperFile(destination.ToStdWstring(), decrypted);
    }
    catch (...)
    {
      wxMessageBox(_("Failed to encrypt the file!"), _("Error!"), wxICON_ERROR);
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();
}

void PackageWindow::OnDumpCompositeObjectsClicked(wxCommandEvent&)
{
  wxString dest = wxSaveFileSelector("composite objects map", "txt", "ObjectDump", this);
  if (dest.empty())
  {
    return;
  }

  if (wxMessageBox(_("Please, make sure you've turned off all composite mods!\nThis operation takes 20 to 30 minutes and requires at least 1.7 GB of free disk space.\nAre you ready to continue?"), _("Dump a list of objects from composite packages..."), wxICON_INFORMATION | wxYES_NO) != wxYES)
  {
    return;
  }

  ProgressWindow progress(this, "Dumping all objects");
  progress.SetCurrentProgress(-1);
  std::vector<std::pair<std::string, std::string>> failed;
  std::thread([dest, &progress, &failed] {

    // Update the mappers
    
    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating package mapper..."));
    Sleep(200);
    try
    {
      FPackage::LoadPkgMapper(true);
    }
    catch (const std::exception& e)
    {
      wxMessageBox(e.what(), wxS("Error!"), wxICON_ERROR);
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    if (progress.IsCanceled())
    {
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating composite mapper..."));

    try
    {
      FPackage::LoadCompositePackageMapper(true);
    }
    catch (const std::exception& e)
    {
      wxMessageBox(e.what(), wxS("Error!"), wxICON_ERROR);
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    if (progress.IsCanceled())
    {
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }

    // Run dumping

    std::ofstream s(dest.ToStdWstring(), std::ios::out | std::ios::binary);
    auto compositeMap = FPackage::GetCompositePackageMap();
    int idx = 0;
    const int total = (int)compositeMap.size();
    SendEvent(&progress, UPDATE_MAX_PROGRESS, total);
    for (const auto& pair : compositeMap)
    {
      if (pair.first == "tmm_marker")
      {
        // Skip the TMM marker
        continue;
      }
      std::shared_ptr<FPackage> pkg = nullptr;
      try
      {
        if ((pkg = FPackage::GetPackageNamed(pair.first)))
        {
          pkg->Load();
          s << "// Object path: " << pair.second.ObjectPath.UTF8() << '\n';
          auto allExports = pkg->GetAllExports();
          for (FObjectExport* exp : allExports)
          {
            s << exp->GetClassName().UTF8() << '\t' << exp->ObjectIndex << '\t' << exp->GetObjectPath().UTF8() << '\n';
          }
          s << std::endl;
          FPackage::UnloadPackage(pkg);
          pkg = nullptr;
        }
        else
        {
          failed.emplace_back(std::make_pair(pair.first, "Failed to open the package!"));
        }
      }
      catch (const std::exception& e)
      {
        failed.emplace_back(std::make_pair(pair.first, e.what()));
        FPackage::UnloadPackage(pkg);
        pkg = nullptr;
      }
      if (idx % 3 == 0)
      {
        if (progress.IsCanceled())
        {
          SendEvent(&progress, UPDATE_PROGRESS_FINISH);
          return;
        }
        SendEvent(&progress, UPDATE_PROGRESS, idx);
        SendEvent(&progress, UPDATE_PROGRESS_DESC, wxString::Format("Saving %d/%d...", idx, total));
      }
      idx++;
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();

  if (failed.size())
  {
    std::filesystem::path errDst = dest.ToStdWstring();
    errDst.replace_extension("errors.txt");
    std::ofstream s(errDst, std::ios::out | std::ios::binary);

    for (const auto& pair : failed)
    {
      s << pair.first << ": " << pair.second << '\n';
    }

    wxMessageBox(_("Failed to iterate some of the packages!\nSee ") + errDst.filename().wstring() + _(" file for details"), _(""), wxICON_WARNING);
  }
}

void PackageWindow::OnPackageReady(wxCommandEvent&)
{
  ObjectTreeCtrl->Freeze();
  LoadObjectTree();
  ObjectTreeCtrl->Thaw();
  SaveMenu->Enable(!Package->IsComposite() && ALLOW_UI_PKG_SAVE); // TODO: track package dirty state
  SaveAsMenu->Enable(!(Package->IsReadOnly() && !Package->IsComposite()) && ALLOW_UI_PKG_SAVE);
  bool selected = false;

  if (Package->GetSummary().PackageFlags & PKG_ContainsMap)
  {
    std::vector<FObjectExport*> exports = Package->GetAllExports();
    PACKAGE_INDEX levelIndex = INDEX_NONE;
    for (const FObjectExport* exp : exports)
    {
      if (exp->GetClassName() == ULevel::StaticClassName() && exp->GetObjectName() == "PersistentLevel")
      {
        levelIndex = exp->ObjectIndex;
      }
    }
    if (levelIndex != INDEX_NONE)
    {
      if (ObjectTreeNode* item = ((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->FindItemByObjectIndex(levelIndex))
      {
        ObjectTreeCtrl->Select(wxDataViewItem(item));
        OnExportObjectSelected(levelIndex);
        ObjectTreeCtrl->EnsureVisible(wxDataViewItem(item));
        selected = true;
      }
    }
  }

  if (!selected)
  {
    ObjectTreeCtrl->Select(wxDataViewItem(((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->GetRootExport()));
    OnExportObjectSelected(FAKE_EXPORT_ROOT);
    ObjectTreeCtrl->Expand(wxDataViewItem(((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->GetRootExport()));
  }
}

void PackageWindow::OnPackageError(wxCommandEvent& e)
{
  wxMessageBox(e.GetString(), "Failed to load the package!", wxICON_ERROR);
  FPackage::UnloadPackage(Package);
  Close();
}

void PackageWindow::OnSelectObject(wxCommandEvent& e)
{
  SelectObject(e.GetString());
}

void PackageWindow::OnObjectTreeStartEdit(wxDataViewEvent& e)
{
  e.Veto();
}

void PackageWindow::OnBulkCompositeExtract(wxCommandEvent&)
{
  App::GetSharedApp()->ShowBulkImport(this);
}

void PackageWindow::OnHelpClicked(wxCommandEvent&)
{
  wxLaunchDefaultBrowser(HelpUrl);
}

void PackageWindow::OnAddPackageClicked(int parentIdx)
{
  FObjectExport* parent = parentIdx != FAKE_EXPORT_ROOT ? Package->GetExportObject(parentIdx) : nullptr;
  std::weak_ptr packageWeak = Package;
  ObjectNameDialog::Validator validatorFunc = [&](const wxString& name) {
    std::shared_ptr<FPackage> package = packageWeak.lock();
    if (!package)
    {
      return false;
    }
    if (name.Find(" ") != wxString::npos)
    {
      return false;
    }
    FString tmpName = name.ToStdWstring();
    if (!tmpName.IsAnsi())
    {
      return false;
    }
    std::vector<FObjectExport*> exps;
    if (parent)
    {
      exps = parent->Inner;
    }
    else
    {
      exps = package->GetRootExports();
    }
    for (FObjectExport* exp : exps)
    {
      if (exp->GetObjectName() == tmpName)
      {
        return false;
      }
    }
    return true;
  };

  ObjectNameDialog nameDialog = ObjectNameDialog(this, wxT("Untitled"));
  nameDialog.SetValidator(validatorFunc);

  if (nameDialog.ShowModal() != wxID_OK)
  {
    return;
  }

  wxString name = nameDialog.GetObjectName();
  if (name.empty())
  {
    return;
  }

  if (FObjectExport* newExp = Package->AddExport(FString(name.ToStdWstring()), NAME_Package, parent))
  {
    ObjectTreeCtrl->AddExportObject(newExp);
    OnExportObjectSelected(newExp->ObjectIndex);
  }
}

void PackageWindow::OnAddTextureClicked(int parentIdx)
{
  FObjectExport* parent = parentIdx != FAKE_EXPORT_ROOT ? Package->GetExportObject(parentIdx) : nullptr;
  wxString path = TextureImporterOptions::LoadImageDialog(this);
  if (path.empty())
  {
    return;
  }

  wxString name;
  wxString extension;
  wxFileName::SplitPath(path, nullptr, nullptr, &name, &extension);
  extension.MakeLower();

  std::weak_ptr packageWeak = Package;
  ObjectNameDialog::Validator validatorFunc = [&](const wxString& name) {
    std::shared_ptr<FPackage> package = packageWeak.lock();
    if (!package)
    {
      return false;
    }
    if (name.Find(" ") != wxString::npos)
    {
      return false;
    }
    FString tmpName = name.ToStdWstring();
    if (!tmpName.IsAnsi())
    {
      return false;
    }
    std::vector<FObjectExport*> exps;
    if (parent)
    {
      exps = parent->Inner;
    }
    else
    {
      exps = package->GetRootExports();
    }
    for (FObjectExport* exp : exps)
    {
      if (exp->GetObjectName() == tmpName)
      {
        return false;
      }
    }
    return true;
  };
  ObjectNameDialog nameDialog = ObjectNameDialog(this, name);
  nameDialog.SetValidator(validatorFunc);

  if (nameDialog.ShowModal() != wxID_OK)
  {
    return;
  }

  name = nameDialog.GetObjectName();
  if (name.empty())
  {
    return;
  }

  FObjectExport* exp = Package->AddExport(FString(name.ToStdWstring()), UTexture2D::StaticClassName(), parent);
  UTexture2D* texture = Cast<UTexture2D>(Package->GetObject(exp));

  if (!texture)
  {
    Package->RemoveExport(exp);
    return;
  }

  TextureImporter importer(this, texture, true);
  importer.SetCustomPath(path);

  if (!importer.Run())
  {
    Package->RemoveExport(exp);
    return;
  }

  ObjectTreeCtrl->AddExportObject(exp);
  OnExportObjectSelected(exp->ObjectIndex);
}

void PackageWindow::OnAddMaterialClicked(int parentIdx)
{
  FObjectExport* parent = parentIdx != FAKE_EXPORT_ROOT ? Package->GetExportObject(parentIdx) : nullptr;
}

void PackageWindow::OnDuplicateClicked(PACKAGE_INDEX objIndex)
{
  ObjectPicker picker(this, wxS("Select destination folder..."), false, Package, 0, { NAME_Package });
  picker.SetAllowRootExport(true);
  UObject* dest = nullptr;
  while (!dest)
  {
    if (picker.ShowModal() != wxID_OK)
    {
      return;
    }
    dest = picker.GetSelectedObject();
    if (!dest)
    {
      // ROOT_EXPORT
      break;
    }
    if (dest->GetClassName() != NAME_Package)
    {
      wxMessageBox(wxT("The selected object is not a package/folder."), "Error!", wxICON_ERROR, this);
      dest = nullptr;
    }
  }

  FObjectExport* source = Package->GetExportObject(objIndex);
  FObjectExport* parent = dest ? dest->GetExportObject() : nullptr;
  wxString name = source->GetObjectName().WString();
  std::weak_ptr packageWeak = Package;
  ObjectNameDialog::Validator validatorFunc = [&](const wxString& name) {
    std::shared_ptr<FPackage> package = packageWeak.lock();
    if (!package)
    {
      return false;
    }
    if (name.Find(" ") != wxString::npos)
    {
      return false;
    }
    FString tmpName = name.ToStdWstring();
    if (!tmpName.IsAnsi())
    {
      return false;
    }
    std::vector<FObjectExport*> exps;
    if (parent)
    {
      exps = parent->Inner;
    }
    else
    {
      exps = package->GetRootExports();
    }
    for (FObjectExport* exp : exps)
    {
      if (exp->GetObjectName() == tmpName)
      {
        return false;
      }
    }
    return true;
  };
  ObjectNameDialog nameDialog = ObjectNameDialog(this, name);
  nameDialog.SetValidator(validatorFunc);

  if (nameDialog.ShowModal() != wxID_OK)
  {
    return;
  }

  name = nameDialog.GetObjectName();
  if (name.empty())
  {
    return;
  }

  std::function<void(FObjectExport*)> addExpRec;
  addExpRec = [&](FObjectExport* exp) {
    ObjectTreeCtrl->AddExportObject(exp);
    for (FObjectExport* inner : exp->Inner)
    {
      addExpRec(inner);
    }
  };

  if (FObjectExport* result = Package->DuplicateExport(source, parent, name.ToStdWstring()))
  {
    ObjectTreeCtrl->Freeze();
    addExpRec(result);
    wxDataViewItem item(((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->FindItemByObjectIndex(result->ObjectIndex));
    ObjectTreeCtrl->Select(item);
    ObjectTreeCtrl->Collapse(item);
    ObjectTreeCtrl->EnsureVisible(item);
    ObjectTreeCtrl->Thaw();
    OnExportObjectSelected(result->ObjectIndex);
  }
}

void PackageWindow::OnPropertiesSplitter(wxSplitterEvent& e)
{
  if (!ContentSplitter->IsSashInvisible())
  {
    Application->SetLastWindowPropertiesSash(ContentSplitter->GetSashPosition(), ContentSplitter->GetSize().x);
  }
}

void PackageWindow::OnBackClicked(wxCommandEvent&)
{
  SelectionHistoryPos++;
  NavigateHistory();
}

void PackageWindow::OnForwardClicked(wxCommandEvent&)
{
  SelectionHistoryPos--;
  NavigateHistory();
}

void PackageWindow::NavigateHistory()
{
  PerformingHistoryNavigation = true;
  if (SelectionHistoryPos < 0)
  {
    SelectionHistoryPos = 0;
  }
  else if (SelectionHistoryPos >= SelectionHistory.size() && SelectionHistory.size())
  {
    SelectionHistoryPos = SelectionHistory.size() - 1;
  }

  PACKAGE_INDEX idx = SelectionHistory[SelectionHistoryPos];
  if (ObjectTreeNode* node = ((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->FindItemByObjectIndex(idx))
  {
    wxDataViewItem item = wxDataViewItem(node);
    ObjectTreeCtrl->Select(item);
    if (idx >= 0)
    {
      OnExportObjectSelected(idx);
    }
    else
    {
      OnImportObjectSelected(idx);
    }
    ObjectTreeCtrl->EnsureVisible(item);
  }
  else if (idx == FAKE_EXPORT_ROOT)
  {
    wxDataViewItem item = wxDataViewItem(((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->GetRootExport());
    ObjectTreeCtrl->Select(item);
    OnExportObjectSelected(idx);
    ObjectTreeCtrl->EnsureVisible(item);
  }
  else if (idx == FAKE_IMPORT_ROOT)
  {
    wxDataViewItem item = wxDataViewItem(((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->GetRootImport());
    ObjectTreeCtrl->Select(item);
    OnImportObjectSelected(idx);
    ObjectTreeCtrl->EnsureVisible(item);
  }
  PerformingHistoryNavigation = false;
}


wxBEGIN_EVENT_TABLE(PackageWindow, wxFrame)
EVT_MENU(ControlElementId::New, PackageWindow::OnNewClicked)
EVT_MENU(ControlElementId::CreateMod, PackageWindow::OnCreateModClicked)
EVT_MENU(ControlElementId::Open, PackageWindow::OnOpenClicked)
EVT_MENU(ControlElementId::OpenByName, PackageWindow::OnOpenByNameClicked)
EVT_MENU(ControlElementId::OpenComposite, PackageWindow::OnOpenCompositeClicked)
EVT_MENU(ControlElementId::ShowInExplorer, PackageWindow::OnShowInExplorerClicked)
EVT_MENU(ControlElementId::Save, PackageWindow::OnSaveClicked)
EVT_MENU(ControlElementId::SaveAs, PackageWindow::OnSaveAsClicked)
EVT_MENU(ControlElementId::Close, PackageWindow::OnCloseClicked)
EVT_MENU(ControlElementId::Exit, PackageWindow::OnExitClicked)
EVT_MENU(ControlElementId::CompositePatch, PackageWindow::OnPatchCompositeMapClicked)
EVT_MENU(ControlElementId::DecryptMapper, PackageWindow::OnDecryptClicked)
EVT_MENU(ControlElementId::EncryptMapper, PackageWindow::OnEncryptClicked)
EVT_MENU(ControlElementId::SettingsWin, PackageWindow::OnSettingsClicked)
EVT_MENU(ControlElementId::LogWin, PackageWindow::OnToggleLogClicked)
EVT_MENU(ControlElementId::Help, PackageWindow::OnHelpClicked)
EVT_MENU(ControlElementId::DumpObjectsMap, PackageWindow::OnDumpCompositeObjectsClicked)
EVT_MENU(ControlElementId::BulkCompositeExtract, PackageWindow::OnBulkCompositeExtract)
EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY, PackageWindow::OnObjectTreeStartEdit)
EVT_SPLITTER_SASH_POS_CHANGED(ControlElementId::ContentSplitter, PackageWindow::OnPropertiesSplitter)
EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, PackageWindow::OnObjectTreeSelectItem)
EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, PackageWindow::OnObjectTreeContextMenu)
EVT_MOVE_END(PackageWindow::OnMoveEnd)
EVT_SIZE(PackageWindow::OnSize)
EVT_MAXIMIZE(PackageWindow::OnMaximized)
EVT_CLOSE(PackageWindow::OnCloseWindow)
EVT_COMMAND(wxID_ANY, PACKAGE_READY, PackageWindow::OnPackageReady)
EVT_COMMAND(wxID_ANY, PACKAGE_ERROR, PackageWindow::OnPackageError)
EVT_COMMAND(wxID_ANY, SELECT_OBJECT, PackageWindow::OnSelectObject)
EVT_COMMAND(wxID_ANY, UPDATE_PROPERTIES, PackageWindow::OnUpdateProperties)
EVT_BUTTON(ControlElementId::Back, PackageWindow::OnBackClicked)
EVT_BUTTON(ControlElementId::Forward, PackageWindow::OnForwardClicked)

EVT_TIMER(wxID_ANY, PackageWindow::OnTick)

EVT_MENU(ControlElementId::DebugTestCookObj, PackageWindow::DebugOnTestCookObject)
EVT_MENU(ControlElementId::DebugSplitMod, PackageWindow::DebugOnSplitMod)
EVT_MENU(ControlElementId::DebugDup, PackageWindow::DebugOnDupSelection)
wxEND_EVENT_TABLE()