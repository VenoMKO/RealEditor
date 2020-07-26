#include "PackageWindow.h"
#include "App.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/collpane.h>

#include <Tera/ALog.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>

#define FAKE_IMPORT_ROOT MININT
#define FAKE_EXPORT_ROOT MAXINT

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

wxDEFINE_EVENT(PACKAGE_READY, wxCommandEvent); 
wxDEFINE_EVENT(PACKAGE_ERROR, wxCommandEvent);

#include "PackageWindowLayout.h"

PackageWindow::PackageWindow(std::shared_ptr<FPackage>& package, App* application)
  : wxFrame(nullptr, wxID_ANY, application->GetAppDisplayName() + L" - " + A2W(package->GetSourcePath()))
  , Application(application)
  , Package(package)
{
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
	delete ImageList;
}

wxString PackageWindow::GetPackagePath() const
{
  return wxString(Package->GetSourcePath());
}


void PackageWindow::LoadObjectTree()
{
	DataModel = new ObjectTreeModel(Package->GetPackageName(), Package->GetRootExports(), Package->GetRootImports());
	ObjectTreeCtrl->AssociateModel(DataModel.get());
	wxDataViewColumn* col = new wxDataViewColumn("title", new wxDataViewIconTextRenderer, 1, wxDVC_DEFAULT_WIDTH, wxALIGN_LEFT);
	ObjectTreeCtrl->AppendColumn(col);
	col->SetWidth(ObjectTreeCtrl->GetSize().x - 4);
}

void PackageWindow::OnIdle(wxIdleEvent& e)
{
}

void PackageWindow::SidebarSplitterOnIdle(wxIdleEvent&)
{
	SidebarSplitter->SetSashPosition(230);
	SidebarSplitter->Disconnect(wxEVT_IDLE, wxIdleEventHandler(PackageWindow::SidebarSplitterOnIdle), NULL, this);
}

void PackageWindow::OnObjectTreeSelectItem(wxDataViewEvent& e)
{
	ObjectTreeNode* node = (ObjectTreeNode*)ObjectTreeCtrl->GetCurrentItem().GetID();
	if (!node || !node->GetParent())
	{
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
	Application->ShowOpenDialog();
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

void PackageWindow::OnPackageReady(wxCommandEvent&)
{
	ObjectTreeCtrl->Freeze();
	LoadObjectTree();
	ObjectTreeCtrl->Thaw();
}

void PackageWindow::OnPackageError(wxCommandEvent& e)
{
	wxMessageBox("Failed to load the package!", e.GetString(), wxICON_ERROR);
	FPackage::UnloadPackage(Package);
	Close();
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
EVT_COMMAND(wxID_ANY, PACKAGE_READY, OnPackageReady)
EVT_COMMAND(wxID_ANY, PACKAGE_ERROR, OnPackageError)
EVT_IDLE(OnIdle)
wxEND_EVENT_TABLE()