#include "GenericEditor.h"
#include "../Windows/PackageWindow.h"
#include "../Windows/REDialogs.h"
#include "../App.h"

#include <Utils/ALog.h>
#include <Tera/UObject.h>
#include <Tera/FStream.h>
#include <Tera/FPackage.h>

#include "AnimationEditor.h"
#include "ObjectRedirectorEditor.h"
#include "TextureEditor.h"
#include "SkelMeshEditor.h"
#include "StaticMeshEditor.h"
#include "StaticMeshActorEditor.h"
#include "SoundWaveEditor.h"
#include "ClassEditor.h"
#include "LevelEditor.h"
#include "SpeedTreeEditor.h"
#include "MaterialEditor.h"
#include "MaterialInstanceEditor.h"
#include "PrefabEditor.h"
#include "SoundCueEditor.h"

#include "../resource.h"

enum ExportMode {
  ExportProperties = wxID_HIGHEST + 1,
  ExportObject,
  ExportAll
};

GenericEditor* GenericEditor::CreateEditor(wxPanel* parent, PackageWindow* window, UObject* object)
{
  GenericEditor* editor = nullptr;
  const FString c = object->GetClassNameString();
  if (c == UObjectRedirector::StaticClassName())
  {
    editor = new ObjectRedirectorEditor(parent, window);
  }
  else if (c == UAnimSet::StaticClassName())
  {
    editor = new AnimSetEditor(parent, window);
  }
  else if (c == UAnimSequence::StaticClassName())
  {
    editor = new AnimSequenceEditor(parent, window);
  }
  else if (c == UTexture2D::StaticClassName() || 
           c == UTerrainWeightMapTexture::StaticClassName() ||
           c == UTextureFlipBook::StaticClassName() ||
           c == UShadowMapTexture2D::StaticClassName() ||
           c == ULightMapTexture2D::StaticClassName())
  {
    editor = new TextureEditor(parent, window);
  }
  else if (c == USkeletalMesh::StaticClassName())
  {
    editor = new SkelMeshEditor(parent, window);
  }
  else if (c == UStaticMesh::StaticClassName())
  {
    editor = new StaticMeshEditor(parent, window);
  }
  else if (c == USoundNodeWave::StaticClassName())
  {
    editor = new SoundWaveEditor(parent, window);
  }
  else if (c == USoundCue::StaticClassName())
  {
    editor = new SoundCueEditor(parent, window);
  }
  else if (c == NAME_Class)
  {
    editor = new ClassEditor(parent, window);
  }
  else if (c == ULevel::StaticClassName())
  {
    editor = new LevelEditor(parent, window);
  }
  else if (c == UStaticMeshActor::StaticClassName())
  {
    editor = new StaticMeshActorEditor(parent, window);
  }
  else if (c == USpeedTree::StaticClassName())
  {
    editor = new SpeedTreeEditor(parent, window);
  }
  else if (c == UMaterial::StaticClassName())
  {
    editor = new MaterialEditor(parent, window);
  }
  else if (c == UMaterialInstance::StaticClassName() || c == UMaterialInstanceConstant::StaticClassName())
  {
    editor = new MaterialInstanceEditor(parent, window);
  }
  else if (c == UTextureCube::StaticClassName())
  {
    editor = new TextureCubeEditor(parent, window);
  }
  else if (c == UPrefab::StaticClassName())
  {
    editor = new PrefabEditor(parent, window);
  }
  else if (c == ULevelStreaming::StaticClassName() || c == ULevelStreamingAlwaysLoaded::StaticClassName() || c == ULevelStreamingDistance::StaticClassName() ||
    c == ULevelStreamingKismet::StaticClassName() || c == ULevelStreamingPersistent::StaticClassName() || c == US1LevelStreamingDistance::StaticClassName() ||
    c == US1LevelStreamingBaseLevel::StaticClassName() || c == US1LevelStreamingSound::StaticClassName() || c == US1LevelStreamingSuperLow::StaticClassName() ||
    c == US1LevelStreamingVOID::StaticClassName())
  {
    editor = new StreamingLevelEditor(parent, window);
  }
  else
  {
    editor = new GenericEditor(parent, window);
  }
  editor->SetObject(object);
  return editor;
}

GenericEditor::GenericEditor(wxPanel* parent, PackageWindow* window)
  : wxPanel(parent, wxID_ANY)
  , Window(window)
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
}

void GenericEditor::LoadObject()
{
  if (IsLoading())
  {
    return;
  }
  std::string id = GetEditorId();
  if (!Object->IsLoaded())
  {
    Loading = true;
    UObject* obj = Object;
    std::weak_ptr<FPackage> wpackage = Window->GetPackage();
    std::thread([obj, id, wpackage] {
      if (auto l = wpackage.lock())
      {
        obj->Load();
        SendEvent(wxTheApp, OBJECT_LOADED, id);
      }
    }).detach();
    // TODO: some UI progress?
  }
  else
  {
    SendEvent(wxTheApp, OBJECT_LOADED, id);
  }
}

void GenericEditor::OnObjectLoaded()
{
  Loading = false;
}

std::string GenericEditor::GetEditorId() const
{
  return std::to_string((uint64)std::addressof(*this)) + "." + std::to_string((uint64)std::addressof(Object));
}

std::vector<FPropertyTag*> GenericEditor::GetObjectProperties()
{
  return Object->GetProperties();
}

void GenericEditor::PopulateToolBar(wxToolBar* toolbar)
{
  Toolbar = toolbar;
  if (Object->GetDataSize() > 0)
  {
    toolbar->AddTool(eID_Export, "Export", wxBitmap(MAKE_IDB(IDB_EXPORT), wxBITMAP_TYPE_PNG_RESOURCE), "Export object data...");
    toolbar->AddTool(eID_Import, "Import", wxBitmap(MAKE_IDB(IDB_IMPORT), wxBITMAP_TYPE_PNG_RESOURCE), "Import object data...");
    toolbar->FindById(eID_Import)->Enable(false);
  }

  if (Object)
  {
    if (Object->GetPackage()->GetFileVersion() == VER_TERA_MODERN)
    {
      CompositeObjectPath = FPackage::GetObjectCompositePath(Object->GetObjectPath()).WString();
      if (CompositeObjectPath.size())
      {
        toolbar->AddTool(eID_Composite, "Source", wxBitmap(MAKE_IDB(IDB_FORWARD), wxBITMAP_TYPE_PNG_RESOURCE), "Open composite package containing this object...");
      }
    }
    if (Object->GetClass() && !Object->GetClass()->IsBuiltIn())
    {
      toolbar->AddTool(eID_Class, "Class", wxBitmap(MAKE_IDB(IDB_ICO_BCLASS), wxBITMAP_TYPE_PNG_RESOURCE), "Show class object");
    }
  }
}

void GenericEditor::ClearToolbar()
{
  Toolbar = nullptr;
}

void GenericEditor::OnToolBarEvent(wxCommandEvent& e)
{
  if (e.GetId() == eID_Export)
  {
    OnExportClicked(e);
  }
  else if (e.GetId() == eID_Import)
  {
    OnImportClicked(e);
  }
  else if (e.GetId() == eID_Composite)
  {
    OnSourceClicked(e);
  }
  else if (e.GetId() == eID_Class)
  {
    OnClassClicked(e);
  }
}

void GenericEditor::OnExportClicked(wxCommandEvent& e)
{
  wxMenu menu;
  wxMenuItem* mitem = nullptr;
  bool hasData = Object->GetDataSize() > 0 && !Object->IsDirty();
  mitem = menu.Append(ExportMode::ExportProperties, wxT("Properties"));
  mitem = menu.Append(ExportMode::ExportObject, wxT("Object data"));
  mitem->Enable(hasData);
  mitem = menu.Append(ExportMode::ExportAll, wxT("All"));
  mitem->Enable(hasData);
  FILE_OFFSET start = 0;
  FILE_OFFSET size = 0;
  switch (GetPopupMenuSelectionFromUser(menu))
  {
  case ExportMode::ExportProperties:
    start = Object->GetSerialOffset();
    size = Object->GetPropertiesSize();
    break;
  case ExportMode::ExportObject:
    start = Object->GetSerialOffset() + Object->GetPropertiesSize();
    size = Object->GetDataSize();
    break;
  case ExportMode::ExportAll:
    start = Object->GetSerialOffset();
    size = Object->GetSerialSize();
    break;
  default:
    return;
  }
  wxString path = wxSaveFileSelector("raw object data", ".*", Object->GetObjectNameString().WString(), Window);
  if (path.empty())
  {
    return;
  }
  DBreakIf(!Object->GetDataSize());
  FWriteStream s(path.ToStdWstring());
  if (!s.IsGood())
  {
    REDialog::Error("Failed to create/open \"" + path + "\"");
    return;
  }
  if (size <= 0)
  {
    return;
  }

  FReadStream rs = FReadStream(A2W(Object->GetPackage()->GetDataPath()));
  rs.SetPackage(Object->GetPackage());
  rs.SetLoadSerializedObjects(Object->GetPackage()->GetStream().GetLoadSerializedObjects());
  rs.SetPosition(start);

  void* data = malloc(size);
  rs.SerializeBytes(data, size);
  s.SerializeBytes(data, size);

  if (!s.IsGood())
  {
    REDialog::Error("Failed to save data to \"" + path + "\"");
    return;
  }
}

void GenericEditor::OnImportClicked(wxCommandEvent& e)
{
  wxString path = wxLoadFileSelector("raw object data", ".*", wxEmptyString, Window);
  if (path.empty())
  {
    return;
  }
  // TODO: import data
  // Check if called by a redirector. In this case with might want to pull the object in this package and remove the redirector
}

void GenericEditor::OnSourceClicked(wxCommandEvent& e)
{
  wxString pkg = CompositeObjectPath.substr(0, CompositeObjectPath.find('.'));
  App::GetSharedApp()->OpenNamedPackage(pkg, CompositeObjectPath);
}

void GenericEditor::OnClassClicked(wxCommandEvent& e)
{
  if (UClass* cls = Object->GetClass())
  {
    App::GetSharedApp()->OpenNamedPackage(cls->GetPackage()->GetPackageName().WString(), cls->GetObjectPath().WString());
  }
}
