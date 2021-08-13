#include "ObjectTreeModel.h"
#include <wx/artprov.h>
#include <wx/mimetype.h>

#include <Tera/Core.h>
#include <Tera/FObjectResource.h>

inline int GetSuitableExportsCount(FObjectExport* exp, const std::vector<FString>& filter, bool recursivly)
{
  int result = 0;
  if (std::find_if(filter.begin(), filter.end(), [&](const FString& cls) { return cls.ToUpper() == exp->GetClassNameString().ToUpper(); }) != filter.end())
  {
    result++;
  }
  else
  {
    bool contains = false;
    for (FObjectExport* child : exp->Inner)
    {
      int tmp = GetSuitableExportsCount(child, filter, true);
      if (recursivly)
      {
        result += tmp;
      }
      if (tmp)
      {
        contains = true;
      }
    }
    if (contains)
    {
      result++;
    }
  }
  return result;
}

inline int GetSuitableExportsCount(FObjectExport* exp, const FString& filter, bool recursivly)
{
  int result = 0;
  if (exp->GetObjectNameString().ToUpper().Contains(filter.ToUpper()) && (!exp->Outer || exp->Outer->GetClassName() == NAME_Package || exp->Outer->GetClassName() == "Level"))
  {
    result++;
  }
  else
  {
    bool contains = false;
    for (FObjectExport* child : exp->Inner)
    {
      int tmp = GetSuitableExportsCount(child, filter, true);
      if (recursivly)
      {
        result += tmp;
      }
      if (tmp)
      {
        contains = true;
      }
    }
    if (contains)
    {
      result++;
    }
  }
  return result;
}

enum ClassIco : int {
  IcoPackage = 0,
  IcoField,
  IcoGeneric,
  IcoClass,
  IcoTexture,
  IcoMesh,
  IcoSound,
  IcoRedirector,
  IcoPackageImp,
  IcoMat,
  IcoMi,
  IcoLevel,
  IcoSLevel,
  IcoAnimSet,
  IcoAnimSeq,
};

wxIcon GetSysIconFromDll(const wxString& path, int id, wxIcon& def)
{
  auto result = wxIcon(wxIconLocation(path, -id));
  if (result.IsOk())
  {
    return result;
  }
  return def;
}

ClassIco ObjectClassToClassIco(const wxString& className)
{
  if (className == wxT("Package"))
  {
    return IcoPackage;
  }
  if (className == wxT("Texture2D"))
  {
    return IcoTexture;
  }
  if (className == wxT("TextureFlipBook"))
  {
    return IcoTexture;
  }
  if (className == wxT("TextureCube"))
  {
    return IcoTexture;
  }
  if (className == wxT("ShadowMapTexture2D"))
  {
    return IcoTexture;
  }
  if (className == wxT("LightMapTexture2D"))
  {
    return IcoTexture;
  }
  if (className == wxT("ObjectRedirector"))
  {
    return IcoRedirector;
  }
  if (className == wxT("SkeletalMesh"))
  {
    return IcoMesh;
  }
  if (className == wxT("StaticMesh"))
  {
    return IcoMesh;
  }
  if (className == wxT("Prefab"))
  {
    return IcoMesh;
  }
  if (className == wxT("Class"))
  {
    return IcoClass;
  }
  if (className == wxT("SoundNodeWave"))
  {
    return IcoSound;
  }
  if (className == wxT("Field") || className == wxT("TextBuffer"))
  {
    return IcoField;
  }
  if (className == wxT("Material"))
  {
    return IcoMat;
  }
  if (className == wxT("AnimSet"))
  {
    return IcoAnimSet;
  }
  if (className == wxT("AnimSequence"))
  {
    return IcoAnimSeq;
  }
  if (className == wxT("MaterialInstanceConstant") || className == wxT("MaterialInstance"))
  {
    return IcoMi;
  }
  if (className == wxT("Level"))
  {
    return IcoLevel;
  }
  if (className == wxT("LevelStreaming") || className == wxT("LevelStreamingAlwaysLoaded") || className == wxT("LevelStreamingDistance") ||
      className == wxT("LevelStreamingKismet") || className == wxT("LevelStreamingPersistent") || className == wxT("S1LevelStreamingDistance") ||
      className == wxT("S1LevelStreamingBaseLevel") || className == wxT("S1LevelStreamingSound") || className == wxT("S1LevelStreamingSuperLow") ||
      className == wxT("S1LevelStreamingVOID"))
  {
    return IcoSLevel;
  }
  return IcoGeneric;
}

ObjectTreeNode::ObjectTreeNode(const std::string& name, std::vector<FObjectExport*> exps, const std::vector<FString>& allowedClasses)
{
  Name = A2W(name);
  for (FObjectExport* exp : exps)
  {
    if (allowedClasses.empty() || GetSuitableExportsCount(exp, allowedClasses, true))
    {
      ObjectTreeNode* child = new ObjectTreeNode(this, exp, allowedClasses);
      Children.Add(child);
    }
  }
}

ObjectTreeNode::ObjectTreeNode(const std::string& name, std::vector<FObjectExport*> exps, const FString& search)
{
  Name = A2W(name);
  for (FObjectExport* exp : exps)
  {
    if (search.Empty() || GetSuitableExportsCount(exp, search, true))
    {
      ObjectTreeNode* child = new ObjectTreeNode(this, exp, search);
      Children.Add(child);
    }
  }
}

ObjectTreeNode::ObjectTreeNode(std::vector<FObjectImport*> imps)
{
  Name = wxT("Imports");
  for (FObjectImport* imp : imps)
  {
    ObjectTreeNode* child = new ObjectTreeNode(this, imp);
    Children.Add(child);
  }
}

ObjectTreeNode::ObjectTreeNode(ObjectTreeNode* parent, FObjectExport* exp, const std::vector<FString>& allowedClasses)
  : Export(exp)
  , Parent(parent)
{
  Resource = (FObjectResource*)exp;
  for (FObjectExport* expChild : exp->Inner)
  {
    if (allowedClasses.empty() || GetSuitableExportsCount(expChild, allowedClasses, true))
    {
      ObjectTreeNode* child = new ObjectTreeNode(this, expChild, allowedClasses);
      Children.Add(child);
    }
  }
}

ObjectTreeNode::ObjectTreeNode(ObjectTreeNode* parent, FObjectExport* exp, const FString& search)
  : Export(exp)
  , Parent(parent)
{
  Resource = (FObjectResource*)exp;
  for (FObjectExport* expChild : exp->Inner)
  {
    if (search.Empty() || GetSuitableExportsCount(expChild, search, true))
    {
      ObjectTreeNode* child = new ObjectTreeNode(this, expChild, search);
      Children.Add(child);
    }
  }
}

ObjectTreeNode::ObjectTreeNode(ObjectTreeNode* parent, FObjectImport* imp)
  : Import(imp)
  , Parent(parent)
{
  Resource = (FObjectResource*)imp;
  for (FObjectImport* impChild : imp->Inner)
  {
    ObjectTreeNode* child = new ObjectTreeNode(this, impChild);
    Children.Add(child);
  }
}

wxString ObjectTreeNode::GetObjectName() const
{
  if (Resource)
  {
    if (Resource->ObjectIndex > 0)
    {
      // TODO: Need to notify PackageWindow about new changes
      bool isDirty = (((FObjectExport*)Resource)->ObjectFlags & RF_Marked);
      return isDirty ? L"*" + Resource->GetObjectNameString().WString() : Resource->GetObjectNameString().WString();
    }
    return Resource->GetObjectNameString().WString();
  }
  return Name;
}

wxString ObjectTreeNode::GetClassName() const
{
  return Resource ? Resource->GetClassNameString().WString() : Name;
}

PACKAGE_INDEX ObjectTreeNode::GetObjectIndex() const
{
  return Resource ? Resource->ObjectIndex : CustomObjectIndex;
}

void ObjectTreeNode::Append(ObjectTreeNode* child)
{
  Children.Add(child);
}

ObjectTreeNode* ObjectTreeNode::FindItemByObjectIndex(PACKAGE_INDEX index)
{
  if (index > 0)
  {
    if (Export && Export->ObjectIndex == index)
    {
      return this;
    }
  }
  else if (index < 0)
  {
    if (Import && Import->ObjectIndex == index)
    {
      return this;
    }
  }
  for (ObjectTreeNode* child : Children)
  {
    if (ObjectTreeNode* result = child->FindItemByObjectIndex(index))
    {
      return result;
    }
  }
  return nullptr;
}

ObjectTreeNode* ObjectTreeNode::FindItemByName(const FString& name)
{
  const FString upper = name.ToUpper();
  if (Export)
  {
    if (Export->GetClassName() != NAME_Package && Export->GetObjectNameString().ToUpper() == upper)
    {
      return this;
    }
  }
  for (ObjectTreeNode* child : Children)
  {
    if (ObjectTreeNode* result = child->FindItemByName(name))
    {
      return result;
    }
  }
  return nullptr;
}

ObjectTreeNode* ObjectTreeNode::FindFirstItemMatch(const FString& name)
{
  const FString upper = name.ToUpper();
  if (Export)
  {
    if (Export->GetClassName() != NAME_Package && Export->GetObjectNameString().ToUpper().Contains(upper))
    {
      return this;
    }
  }
  for (ObjectTreeNode* child : Children)
  {
    if (ObjectTreeNode* result = child->FindFirstItemMatch(name))
    {
      return result;
    }
  }
  return nullptr;
}

ObjectTreeModel::ObjectTreeModel(const std::string& packageName, std::vector<FObjectExport*>& rootExports, std::vector<FObjectImport*>& rootImports, const std::vector<FString>& allowedClasses)
{
  Filter = allowedClasses;
  RootExport = new ObjectTreeNode(packageName, rootExports, allowedClasses);
  if (rootImports.size())
  {
    RootImport = new ObjectTreeNode(rootImports);
  }
  BuildIcons();
}

ObjectTreeModel::ObjectTreeModel(const std::string& packageName, std::vector<FObjectExport*>& rootExports, std::vector<FObjectImport*>& rootImports, const FString& search)
{
  SearchName = search;
  RootExport = new ObjectTreeNode(packageName, rootExports, search);
  BuildIcons();
}

void ObjectTreeModel::BuildIcons()
{
  IconList = new wxImageList(16, 16, true, 2);

  // ORDER MATTERS!!!
  // Keep in sync with the ClassIco enum and ObjectClassToClassIco

  // Package icon
  wxBitmap bitmap = wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16));
  IconList->Add(bitmap);

  // Field icon
  bitmap = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16));
  IconList->Add(bitmap);

  const wxString imageres = R"(C:\Windows\system32\imageres.dll)";

  // Default icon
  auto defIcon = wxIcon();
  defIcon = GetSysIconFromDll(imageres, 2, defIcon);
  IconList->Add(defIcon);

  // Class icon
  IconList->Add(wxBitmap("#117", wxBITMAP_TYPE_PNG_RESOURCE));

  // Texture icon
  IconList->Add(wxBitmap("#121", wxBITMAP_TYPE_PNG_RESOURCE));

  // Mesh icon
  IconList->Add(wxBitmap("#118", wxBITMAP_TYPE_PNG_RESOURCE));

  // Sound icon
  IconList->Add(wxBitmap("#120", wxBITMAP_TYPE_PNG_RESOURCE));

  // Redirector
  IconList->Add(wxBitmap("#119", wxBITMAP_TYPE_PNG_RESOURCE));

  // Imp dir
  IconList->Add(wxBitmap("#123", wxBITMAP_TYPE_PNG_RESOURCE));

  // Material icon
  IconList->Add(wxBitmap("#125", wxBITMAP_TYPE_PNG_RESOURCE));

  // MaterialInstance icon
  IconList->Add(wxBitmap("#126", wxBITMAP_TYPE_PNG_RESOURCE));

  // Level icon
  IconList->Add(wxBitmap("#128", wxBITMAP_TYPE_PNG_RESOURCE));

  // StreamingLevel icon
  IconList->Add(wxBitmap("#129", wxBITMAP_TYPE_PNG_RESOURCE));

  // AnimSet icon
  IconList->Add(wxBitmap("#134", wxBITMAP_TYPE_PNG_RESOURCE));

  // AnimSequence icon
  IconList->Add(wxBitmap("#135", wxBITMAP_TYPE_PNG_RESOURCE));
}

void ObjectTreeModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
  ObjectTreeNode* node = (ObjectTreeNode*)item.GetID();
  wxDataViewIconText value = wxDataViewIconText(node->GetObjectName());
  if (!node->GetParent() || node->GetClassName() == wxS("Package"))
  {
    bool isImp = node->GetObjectIndex() == FAKE_IMPORT_ROOT;
    value.SetIcon(IconList->GetIcon(isImp ? IcoPackageImp : IcoPackage));
  }
  else
  {
    value.SetIcon(IconList->GetIcon(ObjectClassToClassIco(node->GetClassName())));
  }
  variant << value;
}

bool ObjectTreeModel::IsEnabled(const wxDataViewItem& item, unsigned int col) const
{
  ObjectTreeNode* node = (ObjectTreeNode*)item.GetID();
  return Filter.empty() || std::find_if(Filter.begin(), Filter.end(), [&](const FString& cls) { return cls.ToUpper() == node->GetClassName().Upper().ToStdWstring(); }) != Filter.end();
}

wxDataViewItem ObjectTreeModel::GetParent(const wxDataViewItem& item) const
{
  ObjectTreeNode* node = (ObjectTreeNode*)item.GetID();

  if (node == RootExport || node == RootImport)
  {
    return wxDataViewItem(0);
  }
  return wxDataViewItem((void*)node->GetParent());
}

bool ObjectTreeModel::IsContainer(const wxDataViewItem& item) const
{
  ObjectTreeNode* node = (ObjectTreeNode*)item.GetID();
  if (!node)
  {
    return true;
  }
  return node->GetChildren().GetCount();
}

unsigned int ObjectTreeModel::GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const
{
  ObjectTreeNode* node = (ObjectTreeNode*)parent.GetID();
  if (!node)
  {
    array.Add(wxDataViewItem((void*)RootExport));
    if (RootImport)
    {
      array.Add(wxDataViewItem((void*)RootImport));
    }
    return array.size();
  }
  if (!node->GetChildren().GetCount())
  {
    return 0;
  }
  unsigned int count = node->GetChildren().GetCount();
  for (unsigned int pos = 0; pos < count; pos++)
  {
    ObjectTreeNode* child = node->GetChildren().Item(pos);
    array.Add(wxDataViewItem((void*)child));
  }
  return count;
}

ObjectTreeNode* ObjectTreeModel::FindItemByObjectIndex(PACKAGE_INDEX index)
{
  if (index > 0)
  {
    return RootExport->FindItemByObjectIndex(index);
  }
  else if (index < 0 && RootImport)
  {
    return RootImport->FindItemByObjectIndex(index);
  }
  return nullptr;
}

ObjectTreeDataViewCtrl::ObjectTreeDataViewCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name)
  : wxDataViewCtrl(parent, id, pos, size, style, validator, name)
{
  Connect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(ObjectTreeDataViewCtrl::OnItemActivated), nullptr, this);
}

void ObjectTreeDataViewCtrl::AddExportObject(FObjectExport* exp, bool select)
{
  ObjectTreeModel* model = (ObjectTreeModel*)GetModel();
  if (!exp || !model || model->FindItemByObjectIndex(exp->ObjectIndex))
  {
    return;
  }
  if (ObjectTreeNode* parentNode = exp->GetOuter() ? model->FindItemByObjectIndex(exp->OuterIndex) : model->GetRootExport())
  {
    ObjectTreeNode* itemNode = new ObjectTreeNode(parentNode, exp, FString());
    parentNode->GetChildren().Add(itemNode);
    wxDataViewItem parent = wxDataViewItem((void*)parentNode);
    wxDataViewItem item = wxDataViewItem((void*)itemNode);
    model->ItemAdded(parent, item);
    if (select)
    {
      Select(item);
      EnsureVisible(item);
    }
  }
}

void ObjectTreeDataViewCtrl::AddImportObject(FObjectImport* imp)
{
  ObjectTreeModel* model = (ObjectTreeModel*)GetModel();
  if (!imp || !model || model->FindItemByObjectIndex(imp->ObjectIndex))
  {
    return;
  }
  if (ObjectTreeNode* parentNode = imp->GetOuter() ? model->FindItemByObjectIndex(imp->OuterIndex) : model->GetRootImport())
  {
    ObjectTreeNode* itemNode = new ObjectTreeNode(parentNode, imp);
    parentNode->GetChildren().Add(itemNode);
    wxDataViewItem parent = wxDataViewItem((void*)parentNode);
    wxDataViewItem item = wxDataViewItem((void*)itemNode);
    model->ItemAdded(parent, item);
  }
}

void ObjectTreeDataViewCtrl::RemoveExp(PACKAGE_INDEX idx)
{
  ObjectTreeModel* model = (ObjectTreeModel*)GetModel();
  if (idx < 0 || !model)
  {
    return;
  }
  if (ObjectTreeNode* node = model->FindItemByObjectIndex(idx))
  {
    if (ObjectTreeNode* parent = node->GetParent())
    {
      parent->GetChildren().Remove(node);
    }
    model->ItemDeleted(wxDataViewItem(node->GetParent()), wxDataViewItem(node));
  }
}

void ObjectTreeDataViewCtrl::ExpandAll()
{
  std::function<void(ObjectTreeNode*)> expFunc;
  expFunc = [&](ObjectTreeNode* item) {
    if (!item)
    {
      return;
    }
    ObjectTreeNodePtrArray& children = item->GetChildren();
    if (children.size())
    {
      Expand(wxDataViewItem(item));
    }
    for (ObjectTreeNode* child : children)
    {
      expFunc(child);
    }
  };
  if (ObjectTreeModel* model = (ObjectTreeModel*)GetModel())
  {
    expFunc(model->GetRootExport());
  }
}

void ObjectTreeDataViewCtrl::SaveTreeState()
{
  std::function<void(ObjectTreeNode*)> expFunc;
  expFunc = [&](ObjectTreeNode* item) {
    if (!item)
    {
      return;
    }
    ObjectTreeNodePtrArray& children = item->GetChildren();
    Expanded[item->GetObjectIndex()] = IsExpanded(wxDataViewItem(item));
    for (ObjectTreeNode* child : children)
    {
      expFunc(child);
    }
  };
  if (ObjectTreeModel* model = (ObjectTreeModel*)GetModel())
  {
    Expanded.clear();
    expFunc(model->GetRootExport());
  }
}

void ObjectTreeDataViewCtrl::RestoreTreeState()
{
  std::function<void(ObjectTreeNode*)> expFunc;
  expFunc = [&](ObjectTreeNode* item) {
    if (!item)
    {
      return;
    }
    ObjectTreeNodePtrArray& children = item->GetChildren();
    if (Expanded[item->GetObjectIndex()])
    {
      Expand(wxDataViewItem(item));
    }
    for (ObjectTreeNode* child : children)
    {
      expFunc(child);
    }
  };
  if (ObjectTreeModel* model = (ObjectTreeModel*)GetModel())
  {
    expFunc(model->GetRootExport());
  }
}

void ObjectTreeDataViewCtrl::SelectObject(PACKAGE_INDEX idx)
{
  ObjectTreeModel* model = (ObjectTreeModel*)GetModel();
  if (ObjectTreeNode* node = model->FindItemByObjectIndex(idx))
  {
    wxDataViewItem item(node);
    Select(item);
    EnsureVisible(item);
  }
}

int32 ObjectTreeDataViewCtrl::SuitableObjectsCount()
{
  ObjectTreeModel* model = (ObjectTreeModel*)GetModel();
  ObjectTreeNode* root = model->GetRootExport();
  std::function<int32(ObjectTreeNode*)> GetChildrenCntRecursive;
  GetChildrenCntRecursive = [&](ObjectTreeNode* node) {
    int32 result = 0;
    if (node)
    {
      if (model->IsEnabled(wxDataViewItem(node), 0) && node->GetClassName() != NAME_Package)
      {
        result++;
      }
      ObjectTreeNodePtrArray& children = node->GetChildren();
      for (ObjectTreeNode* child : children)
      {
        result += GetChildrenCntRecursive(child);
      }
    }
    return result;
  };
  return GetChildrenCntRecursive(root);
}

void ObjectTreeDataViewCtrl::OnSize(wxSizeEvent& e)
{
  if (GetColumnCount() && GetParent())
  {
    GetColumn(0)->SetWidth(GetParent()->GetSize().x - 30);
  }
  e.Skip();
}

void ObjectTreeDataViewCtrl::OnItemActivated(wxDataViewEvent& e)
{
  wxDataViewItem item = GetCurrentItem();
  if (item.GetID())
  {
    if (IsExpanded(item))
    {
      Collapse(item);
    }
    else
    {
      Expand(item);
    }
  }
  e.Skip();
}

wxBEGIN_EVENT_TABLE(ObjectTreeDataViewCtrl, wxDataViewCtrl)
EVT_SIZE(ObjectTreeDataViewCtrl::OnSize)
wxEND_EVENT_TABLE()
