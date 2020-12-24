#include "ObjectTreeModel.h"
#include <wx/artprov.h>
#include <wx/mimetype.h>

#include <Tera/Core.h>
#include <Tera/FObjectResource.h>

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
  if (className == wxT("MaterialInstanceConstant") || className == wxT("MaterialInstance"))
  {
    return IcoMi;
  }
  else if (className == wxT("Level"))
  {
    return IcoLevel;
  }
  else if (className == wxT("LevelStreaming") || className == wxT("LevelStreamingAlwaysLoaded") || className == wxT("LevelStreamingDistance") ||
           className == wxT("LevelStreamingKismet") || className == wxT("LevelStreamingPersistent") || className == wxT("S1LevelStreamingDistance") ||
           className == wxT("S1LevelStreamingBaseLevel") || className == wxT("S1LevelStreamingSound") || className == wxT("S1LevelStreamingSuperLow") ||
           className == wxT("S1LevelStreamingVOID"))
  {
    return IcoSLevel;
  }
  return IcoGeneric;
}

ObjectTreeNode::ObjectTreeNode(const std::string& name, std::vector<FObjectExport*> exps)
{
  Name = A2W(name);
  for (FObjectExport* exp : exps)
  {
    ObjectTreeNode* child = new ObjectTreeNode(this, exp);
    Children.Add(child);
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

ObjectTreeNode::ObjectTreeNode(ObjectTreeNode* parent, FObjectExport* exp)
  : Export(exp)
  , Parent(parent)
{
  Resource = (FObjectResource*)exp;
  for (FObjectExport* expChild : exp->Inner)
  {
    ObjectTreeNode* child = new ObjectTreeNode(this, expChild);
    Children.Add(child);
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
  return Resource ? Resource->GetObjectName().WString() : Name;
}

wxString ObjectTreeNode::GetClassName() const
{
  return Resource ? Resource->GetClassName().WString() : Name;
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

ObjectTreeModel::ObjectTreeModel(const std::string& packageName, std::vector<FObjectExport*>& rootExports, std::vector<FObjectImport*>& rootImports)
{
  RootExport = new ObjectTreeNode(packageName, rootExports);
  if (rootImports.size())
  {
    RootImport = new ObjectTreeNode(rootImports);
  }
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

void ObjectTreeDataViewCtrl::OnSize(wxSizeEvent& e)
{
  if (GetColumnCount() && GetParent())
  {
    GetColumn(0)->SetWidth(GetParent()->GetSize().x - 30);
  }
  e.Skip();
}

wxBEGIN_EVENT_TABLE(ObjectTreeDataViewCtrl, wxDataViewCtrl)
EVT_SIZE(ObjectTreeDataViewCtrl::OnSize)
wxEND_EVENT_TABLE()