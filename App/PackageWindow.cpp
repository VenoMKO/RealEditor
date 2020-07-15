#include "PackageWindow.h"
#include "App.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/artprov.h>

#include <Tera/ALog.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>

#define FAKE_IMPORT_ROOT MININT
#define FAKE_EXPORT_ROOT MAXINT

#define PACKAGE_ICON 0
#define PACKAGE_O_ICON 1
#define FILE_ICON 2
#define FILE_O_ICON FILE_ICON


enum ControlElementId {
	New = wxID_HIGHEST + 1,
	Open,
	Save,
	SaveAs,
	Close,
	Exit,
	LogWin,
	Back,
	Forward
};

#include "PackageWindowLayout.h"

void LoadImportToTree(wxDataViewTreeCtrl* tree, const wxDataViewItem& parent, FObjectImport* imp)
{
	wxDataViewItem item;
	int icon = FILE_ICON;
	int icon_o = FILE_O_ICON;
	wxStringClientData* data = new wxStringClientData(std::to_string(imp->ObjectIndex));

	if (imp->GetClassName() == "Package")
	{
		icon = PACKAGE_ICON;
		icon_o = PACKAGE_O_ICON;
	}
	if (imp->Inner.size())
	{
		item = tree->AppendContainer(parent, imp->GetObjectName(), icon, icon_o, data);
		for (FObjectImport* inner : imp->Inner)
		{
			LoadImportToTree(tree, item, inner);
		}
	}
	else
	{
		item = tree->AppendItem(parent, imp->GetObjectName(), icon, data);
	}
}

void LoadExportToTree(wxDataViewTreeCtrl* tree, const wxDataViewItem& parent, FObjectExport* exp)
{
	wxDataViewItem item;
	int icon = FILE_ICON;
	int icon_o = FILE_O_ICON;
	wxStringClientData* data = new wxStringClientData(std::to_string(exp->ObjectIndex));

	if (exp->GetClassName() == "Package")
	{
		icon = PACKAGE_ICON;
		icon_o = PACKAGE_O_ICON;
	}
	if (exp->Inner.size())
	{
		item = tree->AppendContainer(parent, exp->GetObjectName(), icon, icon_o, data);
		for (FObjectExport* inner : exp->Inner)
		{
			LoadExportToTree(tree, item, inner);
		}
	}
	else
	{
		item = tree->AppendItem(parent, exp->GetObjectName(), icon, data);
	}
}

PackageWindow::PackageWindow(std::shared_ptr<FPackage>& package, App* application)
  : wxFrame(nullptr, wxID_ANY, application->GetAppDisplayName() + L" - " + A2W(package->GetSourcePath()))
  , Application(application)
  , Package(package)
{
	SetSizeHints(wxSize(1024, 700), wxDefaultSize);
	InitLayout();
	
	ObjectTreeCtrl->SetFocus();

	LoadTreeIcons();
	LoadObjectTree();

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
}

void PackageWindow::OnCloseWindow(wxCloseEvent& event)
{
	Application->PackageWindowWillClose(this);
	wxFrame::OnCloseWindow(event);
}

PackageWindow::~PackageWindow()
{
	FPackage::UnloadPackage(Package.get());
	delete ImageList;
}

wxString PackageWindow::GetPackagePath() const
{
  return wxString(Package->GetSourcePath());
}

void PackageWindow::LoadTreeIcons()
{
	ImageList = new wxImageList(16, 16, true, 1);
	ObjectTreeCtrl->SetImageList(ImageList);
	auto clientId = wxART_MAKE_CLIENT_ID("OBJECT_TREE_ICONS");
	wxBitmap icon = wxArtProvider::GetBitmap(wxART_FOLDER, clientId, wxSize(16, 16));
	ImageList->Add(icon);
	icon = wxArtProvider::GetBitmap(wxART_FOLDER_OPEN, clientId, wxSize(16, 16));
	ImageList->Add(icon);
	icon = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, clientId, wxSize(16, 16));
	ImageList->Add(icon);
}

void PackageWindow::LoadObjectTree()
{
	ObjectTreeCtrl->Freeze();
	ObjectTreeCtrl->DeleteAllItems();

	wxDataViewItem expRoot = ObjectTreeCtrl->AppendContainer(wxDataViewItem(0), Package->GetPackageName(), PACKAGE_ICON, PACKAGE_O_ICON, new wxStringClientData(std::to_string(FAKE_EXPORT_ROOT)));
	std::vector<FObjectExport*> exports = Package->GetRootExports();
	for (FObjectExport* exp : exports)
	{
		LoadExportToTree(ObjectTreeCtrl, expRoot, exp);
	}

	wxDataViewItem impRoot = ObjectTreeCtrl->AppendContainer(wxDataViewItem(0), "Imports", PACKAGE_ICON, PACKAGE_O_ICON, new wxStringClientData(std::to_string(FAKE_IMPORT_ROOT)));
	std::vector<FObjectImport*> imports = Package->GetRootImports();
	for (FObjectImport* imp : imports)
	{
		LoadImportToTree(ObjectTreeCtrl, impRoot, imp);
	}
	
	ObjectTreeCtrl->Thaw();
}

void PackageWindow::SidebarSplitterOnIdle(wxIdleEvent&)
{
	SidebarSplitter->SetSashPosition(230);
	SidebarSplitter->Disconnect(wxEVT_IDLE, wxIdleEventHandler(PackageWindow::SidebarSplitterOnIdle), NULL, this);
}

void PackageWindow::OnObjectTreeSelectItem(wxDataViewEvent& e)
{
	wxStringClientData *client = (wxStringClientData*)ObjectTreeCtrl->GetItemData(e.GetItem());
	if (!client)
	{
		OnNoneObjectSelected();
		return;
	}
	INT index = wxAtoi(client->GetData());
	if (index < 0)
	{
		OnImportObjectSelected(index);
	}
	else
	{
		OnExportObjectSelected(index);
	}
}

void PackageWindow::OnImportObjectSelected(INT index)
{
	ShowEditor(nullptr);
	if (index == FAKE_IMPORT_ROOT)
	{
		ObjectTitleLabel->SetLabelText("No selection");
		SetPropertiesHidden(true);
		SetContentHidden(true);
		return;
	}
	FObjectImport* obj = Package->GetImportObject(index);
	ObjectTitleLabel->SetLabelText(wxString::Format("%s (%s)", obj->GetObjectName(), obj->GetClassName()));
	SetPropertiesHidden(true);
	SetContentHidden(true);
}

void PackageWindow::OnExportObjectSelected(INT index)
{
	if (index == FAKE_EXPORT_ROOT)
	{
		ShowEditor(nullptr);
		ObjectTitleLabel->SetLabelText("No selection");
		SetPropertiesHidden(true);
		SetContentHidden(true);
		return;
	}
	
	FObjectExport* fobj = Package->GetExportObject(index);
	ObjectTitleLabel->SetLabelText(wxString::Format("%s (%s)", fobj->GetObjectName(), fobj->GetClassName()));
	ObjectSizeLabel->SetLabelText(wxString::Format("0x%08X", fobj->SerialSize));
	ObjectOffsetLabel->SetLabelText(wxString::Format("0x%08X", fobj->SerialOffset));
	//std::string flags = ObjectFlagsToString(fobj->ObjectFlags);
	//ObjectFlagsTextfield->SetLabelText(flags);
	SetPropertiesHidden(false);
	SetContentHidden(false);

	{
		auto it = Editors.find(index);
		if (it != Editors.end())
		{
			ShowEditor(it->second);
		}
		else
		{
			UObject* object = fobj->Object;
			if (!object)
			{
				Package->GetObject(index);
				object = fobj->Object;
			}
			GenericEditor* e = GenericEditor::CreateEditor(EditorContainer, object);
			ShowEditor(e);
			e->LoadObject();
		}
	}
}

void PackageWindow::OnNoneObjectSelected()
{
	ObjectTitleLabel->SetLabelText("No selection");
	SetPropertiesHidden(true);
	SetContentHidden(true);
	ShowEditor(nullptr);
}

void PackageWindow::OnNewClicked(wxCommandEvent& e)
{

}

void PackageWindow::OnOpenClicked(wxCommandEvent& e)
{
	Application->OpenDialog();
}

void PackageWindow::OnSaveClicked(wxCommandEvent& e)
{

}

void PackageWindow::OnSaveAsClicked(wxCommandEvent& e)
{

}

void PackageWindow::OnCloseClicked(wxCommandEvent& e)
{
	Close();
}

void PackageWindow::OnExitClicked(wxCommandEvent&)
{
	Application->ExitMainLoop();
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
	}
	e.Skip();
}

void PackageWindow::OnMaximized(wxMaximizeEvent& e)
{
	Application->SetLastWindowPosition(wxPoint(WIN_POS_FULLSCREEN, 0));
	e.Skip();
}

void PackageWindow::OnObjectTreeStartEdit(wxDataViewEvent& e)
{
	wxDataViewItem item = e.GetItem();
	if (ObjectTreeCtrl->IsExpanded(item))
	{
		ObjectTreeCtrl->Collapse(item);
	}
	else
	{
		ObjectTreeCtrl->Expand(item);
	}
	e.Veto();
}

wxBEGIN_EVENT_TABLE(PackageWindow, wxFrame)
EVT_MENU(ControlElementId::New, OnNewClicked)
EVT_MENU(ControlElementId::Open, OnOpenClicked)
EVT_MENU(ControlElementId::Save, OnSaveClicked)
EVT_MENU(ControlElementId::SaveAs, OnSaveAsClicked)
EVT_MENU(ControlElementId::Close, OnCloseClicked)
EVT_MENU(ControlElementId::Exit, OnExitClicked)
EVT_MENU(ControlElementId::LogWin, OnToggleLogClicked)
EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY, OnObjectTreeStartEdit)
EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, OnObjectTreeSelectItem)
EVT_MOVE_END(OnMoveEnd)
EVT_MAXIMIZE(OnMaximized)
EVT_CLOSE(OnCloseWindow)
wxEND_EVENT_TABLE()