#include "ObjectTreeDataViewCtrl.h"
#include <wx/artprov.h>

#include <Tera/FObjectResource.h>

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
	return Resource ? A2W(Resource->GetObjectName()) : Name;
}

wxString ObjectTreeNode::GetClassName() const
{
	return Resource ? A2W(Resource->GetClassName()) : Name;
}

PACKAGE_INDEX ObjectTreeNode::GetObjectIndex() const
{
	return Resource->ObjectIndex;
}

void ObjectTreeNode::Append(ObjectTreeNode* child)
{
	Children.Add(child);
}

ObjectTreeModel::ObjectTreeModel(const std::string& packageName, std::vector<FObjectExport*>& rootExports, std::vector<FObjectImport*>& rootImports)
{
	RootExport = new ObjectTreeNode(packageName, rootExports);
	RootImport = new ObjectTreeNode(rootImports);
	IconList = new wxImageList(16, 16, true, 2);
	auto client = wxART_MAKE_CLIENT_ID("OBJECT_TREE_ICONS");
	wxBitmap bitmap = wxArtProvider::GetBitmap(wxART_FOLDER, client, wxSize(16, 16));
	IconList->Add(bitmap);
	bitmap = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, client, wxSize(16, 16));
	IconList->Add(bitmap);
}

void ObjectTreeModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
	ObjectTreeNode* node = (ObjectTreeNode*)item.GetID();
	wxDataViewIconText value = wxDataViewIconText(node->GetObjectName());
	if (!node->GetParent() || node->GetClassName() == "Package")
	{
		value.SetIcon(IconList->GetIcon(0));
	}
	else
	{
		value.SetIcon(IconList->GetIcon(1));
	}
	variant << value;
}

bool ObjectTreeModel::SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col)
{
	return false;
}

bool ObjectTreeModel::IsEnabled(const wxDataViewItem& item, unsigned int col) const
{
	return false;
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
		array.Add(wxDataViewItem((void*)RootImport));
		return 2;
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

void ObjectTreeDataViewCtrl::OnSize(wxSizeEvent& e)
{
	if (GetColumnCount())
	{
		GetColumn(0)->SetWidth(e.GetSize().x - 4);
	}
	e.Skip();
}

wxBEGIN_EVENT_TABLE(ObjectTreeDataViewCtrl, wxDataViewCtrl)
EVT_SIZE(OnSize)
wxEND_EVENT_TABLE()