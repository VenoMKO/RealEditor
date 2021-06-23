#pragma once
#include <wx/dataview.h>
#include <vector>
#include <string>

#include <Tera/FString.h>

class FObjectResource;
class FObjectExport;
class FObjectImport;
class ObjectTreeNode;
typedef signed int PACKAGE_INDEX;

#define FAKE_EXPORT_ROOT 0x7FFFFFFF
#define FAKE_IMPORT_ROOT 0x80000000

WX_DEFINE_ARRAY_PTR(ObjectTreeNode*, ObjectTreeNodePtrArray);

class ObjectTreeNode {
public:

  ObjectTreeNode(const std::string& name, std::vector<FObjectExport*> exps, const std::vector<FString>& allowedClasses);
  ObjectTreeNode(const std::string& name, std::vector<FObjectExport*> exps, const FString& search);
  ObjectTreeNode(std::vector<FObjectImport*> imps);

  ObjectTreeNode(ObjectTreeNode* parent, FObjectExport* exp, const std::vector<FString>& allowedClasses);
  ObjectTreeNode(ObjectTreeNode* parent, FObjectExport* exp, const FString& search);

  ObjectTreeNode(ObjectTreeNode* parent, FObjectImport* imp);

  ~ObjectTreeNode()
  {
    size_t count = Children.GetCount();
    for (size_t i = 0; i < count; i++)
    {
      ObjectTreeNode* child = Children[i];
      delete child;
    }
  }

  void SetCustomObjectIndex(PACKAGE_INDEX index)
  {
    CustomObjectIndex = index;
  }

  wxString GetObjectName() const;
  wxString GetClassName() const;
  PACKAGE_INDEX GetObjectIndex() const;

  ObjectTreeNode* GetParent()
  {
    return Parent;
  }

  ObjectTreeNodePtrArray& GetChildren()
  {
    return Children;
  }

  void Append(ObjectTreeNode* child);

  ObjectTreeNode* FindItemByObjectIndex(PACKAGE_INDEX index);
  ObjectTreeNode* FindItemByName(const FString& name);
  ObjectTreeNode* FindFirstItemMatch(const FString& name);

  FObjectExport* GetExport()
  {
    return Export;
  }

private:
  FObjectExport* Export = nullptr;
  FObjectImport* Import = nullptr;
  FObjectResource* Resource = nullptr;
  PACKAGE_INDEX CustomObjectIndex = 0;

  ObjectTreeNode* Parent = nullptr;
  ObjectTreeNodePtrArray Children;
  wxString Name;
};

class ObjectTreeModel : public wxDataViewModel {
public:
  ObjectTreeModel(const std::string& packageName, std::vector<FObjectExport*>& rootExports, std::vector<FObjectImport*>& rootImports, const std::vector<FString>& allowedClasses);
  
  ObjectTreeModel(const std::string& packageName, std::vector<FObjectExport*>& rootExports, std::vector<FObjectImport*>& rootImports, const FString& search);
  
  ~ObjectTreeModel()
  {
    delete RootExport;
    delete RootImport;
    delete IconList;
  }

  unsigned int GetColumnCount() const override
  {
    return 1;
  }
  
  wxString GetColumnType(unsigned int col) const override
  {
    return "wxDataViewIconText";
  }
  
  void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
  
  bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override
  {
    return false;
  }

  bool HasValue(const wxDataViewItem& item, unsigned col) const override
  {
    return true;
  }

  bool IsEnabled(const wxDataViewItem& item, unsigned int col) const override;

  wxDataViewItem GetParent(const wxDataViewItem& item) const override;

  bool IsContainer(const wxDataViewItem& item) const override;

  unsigned int GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const override;

  ObjectTreeNode* FindItemByObjectIndex(PACKAGE_INDEX index);

  ObjectTreeNode* GetRootExport() const
  {
    return RootExport;
  }

  ObjectTreeNode* GetRootImport() const
  {
    return RootImport;
  }

  FString GetSearchValue() const
  {
    return SearchName;
  }

protected:
  void BuildIcons();

private:
  std::vector<FString> Filter;
  FString SearchName;
  ObjectTreeNode* RootExport = nullptr;
  ObjectTreeNode* RootImport = nullptr;
  wxImageList* IconList = nullptr;
};

class ObjectTreeDataViewCtrl : public wxDataViewCtrl {
public:
  using wxDataViewCtrl::wxDataViewCtrl;
  void AddExportObject(FObjectExport* exp, bool select = true);
  void AddImportObject(FObjectImport* imp);
  void RemoveExp(PACKAGE_INDEX idx);
  void ExpandAll();
  void SaveTreeState();
  void RestoreTreeState();
  void SelectObject(PACKAGE_INDEX idx);
private:
  void OnSize(wxSizeEvent& e);
  wxDECLARE_EVENT_TABLE();
private:
  std::map<PACKAGE_INDEX, bool> Expanded;
};