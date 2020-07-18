#pragma once
#include <wx/dataview.h>
#include <vector>
#include <string>

class FObjectResource;
class FObjectExport;
class FObjectImport;
class ObjectTreeNode;
typedef signed int PACKAGE_INDEX;

WX_DEFINE_ARRAY_PTR(ObjectTreeNode*, ObjectTreeNodePtrArray);

class ObjectTreeNode {
public:

	ObjectTreeNode(const std::string& name, std::vector<FObjectExport*> exps);
	ObjectTreeNode(std::vector<FObjectImport*> imps);

	ObjectTreeNode(ObjectTreeNode* parent, FObjectExport* exp);

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

private:
	FObjectExport* Export = nullptr;
	FObjectImport* Import = nullptr;
	FObjectResource* Resource = nullptr;

	ObjectTreeNode* Parent = nullptr;
	ObjectTreeNodePtrArray Children;
	wxString Name;
};

class ObjectTreeModel : public wxDataViewModel {
public:
	ObjectTreeModel(const std::string& packageName, std::vector<FObjectExport*>& rootExports, std::vector<FObjectImport*>& rootImports);

	~ObjectTreeModel()
	{
		delete RootExport;
		delete RootImport;
		delete IconList;
	}

	virtual unsigned int GetColumnCount() const override
	{
		return 1;
	}
	virtual wxString GetColumnType(unsigned int col) const override
	{
		return "wxDataViewIconText";
	}
	virtual void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
	virtual bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override;
	virtual bool IsEnabled(const wxDataViewItem& item, unsigned int col) const override;
	virtual wxDataViewItem GetParent(const wxDataViewItem& item) const override;
	virtual bool IsContainer(const wxDataViewItem& item) const override;
	virtual unsigned int GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const override;

private:
	ObjectTreeNode* RootExport = nullptr;
	ObjectTreeNode* RootImport = nullptr;
	wxImageList* IconList = nullptr;
};

class ObjectTreeDataViewCtrl : public wxDataViewCtrl {
public:
	using wxDataViewCtrl::wxDataViewCtrl;
private:
	void OnSize(wxSizeEvent& e);
	wxDECLARE_EVENT_TABLE();
};