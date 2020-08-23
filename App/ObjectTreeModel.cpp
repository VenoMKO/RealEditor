#include "ObjectTreeModel.h"
#include <wx/artprov.h>
#include <wx/mimetype.h>

#include <Tera/FObjectResource.h>

enum ClassIco : int {
	IcoPackage = 0,
	IcoField,
	IcoGeneric,
	IcoClass,
	IcoTexture,
	IcoMesh,
	IcoSound
};

wxIcon GetSysIconForExtension(const wxString& ext, wxIcon& def)
{
	wxMimeTypesManager manager;
	try
	{
		wxFileType* type = manager.GetFileTypeFromExtension(ext);
		wxIconLocation location;
		if (type->GetIcon(&location))
		{
			auto icon = wxIcon(location);
			if (icon.IsOk())
			{
				return icon;
			}
			return def;
		}
	}
	catch(...)
	{}
	return def;
}

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
	if (className == wxT("SkeletalMesh"))
	{
		return IcoMesh;
	}
	if (className == wxT("StaticMesh"))
	{
		return IcoMesh;
	}
	if (className == wxT("Class"))
	{
		return IcoClass;
	}
	if (className == wxT("Field") || className == wxT("TextBuffer"))
	{
		return IcoField;
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
	
	// ORDER MATTERS!!!
	// Keep in sync with the ClassIco enum and ObjectClassToClassIco

	// Package icon
	wxBitmap bitmap = wxArtProvider::GetBitmap(wxART_FOLDER, client, wxSize(16, 16));
	IconList->Add(bitmap);
	
	// Field icon
	bitmap = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, client, wxSize(16, 16));
	IconList->Add(bitmap);
	
	const wxString imageres = R"(C:\Windows\system32\imageres.dll)";

	// Default icon
	auto defIcon = GetSysIconFromDll(imageres, 2, wxIcon());
	IconList->Add(defIcon);

	// Class icon
	IconList->Add(GetSysIconFromDll(imageres, 114, defIcon));

	// Texture icon
	IconList->Add(GetSysIconForExtension("jpg", defIcon));

	// Mesh icon
	IconList->Add(GetSysIconFromDll(imageres, 198, defIcon));

	// Sound icon
	IconList->Add(GetSysIconForExtension("wav", defIcon));
}

void ObjectTreeModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
	ObjectTreeNode* node = (ObjectTreeNode*)item.GetID();
	wxDataViewIconText value = wxDataViewIconText(node->GetObjectName());
	if (!node->GetParent() || node->GetClassName() == wxS("Package"))
	{
		value.SetIcon(IconList->GetIcon(IcoPackage));
	}
	else
	{
		value.SetIcon(IconList->GetIcon(ObjectClassToClassIco(node->GetClassName())));
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
EVT_SIZE(ObjectTreeDataViewCtrl::OnSize)
wxEND_EVENT_TABLE()