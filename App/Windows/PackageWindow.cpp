#include "PackageWindow.h"
#include "ProgressWindow.h"
#include "CompositePackagePicker.h"
#include "../Misc/ObjectProperties.h"
#include "../App.h"

#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/collpane.h>

#include <Tera/ALog.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>
#include <Tera/UClass.h>

#define FAKE_IMPORT_ROOT MININT
#define FAKE_EXPORT_ROOT MAXINT

enum ControlElementId {
	New = wxID_HIGHEST + 1,
	Open,
	OpenComposite,
	Save,
	SaveAs,
	Close,
	Exit,
	SettingsWin,
	LogWin,
	DebugTestCookObj,
	Back,
	Forward,
	HeartBeat
};

wxDEFINE_EVENT(PACKAGE_READY, wxCommandEvent); 
wxDEFINE_EVENT(PACKAGE_ERROR, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_PROPERTIES, wxCommandEvent);

#include "PackageWindowLayout.h"

PackageWindow::PackageWindow(std::shared_ptr<FPackage>& package, App* application)
  : wxFrame(nullptr, wxID_ANY, application->GetAppDisplayName() + L" - " + A2W(package->GetSourcePath()))
  , Application(application)
  , Package(package)
{
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
	delete ImageList;
}

wxString PackageWindow::GetPackagePath() const
{
  return wxString(Package->GetSourcePath().String());
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

void PackageWindow::SidebarSplitterOnIdle(wxIdleEvent&)
{
	wxSize hint;
	float dx;
	hint = Application->GetLastWindowObjectSash();
	dx = float(hint.x) / float(hint.y);
	SidebarSplitter->SetSashPosition(SidebarSplitter->GetSize().x * dx);

	hint = Application->GetLastWindowPropSash();
	PropertiesPos = hint.y - hint.x;;
	ContentSplitter->SetSashPosition(PropertiesPos);
	ContentSplitter->SetSashGravity(1);
	SidebarSplitter->Disconnect(wxEVT_IDLE, wxIdleEventHandler(PackageWindow::SidebarSplitterOnIdle), NULL, this);
	DisableSizeUpdates = false;
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
	ObjectTitleLabel->SetLabelText(wxString::Format(wxT("%ls (%ls)"), obj->GetObjectName().WString().c_str(), obj->GetClassName().WString().c_str()));
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
	ObjectTitleLabel->SetLabelText(wxString::Format(wxT("%ls (%ls)"), fobj->GetObjectName().WString().c_str(), fobj->GetClassName().WString().c_str()));
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
	PropertyRootCategory->Enable(!Package->IsReadOnly() && object->GetPackage() == Package.get());
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

void PackageWindow::FixOSG()
{
	// Osg refuses to process events
	// Resizing the window for some reasone fixes the issue
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
	wxString path = Application->ShowOpenDialog();
	if (path.size())
	{
		Application->OpenPackage(path);
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

void PackageWindow::OnSaveClicked(wxCommandEvent& e)
{

}

void PackageWindow::OnSaveAsClicked(wxCommandEvent& e)
{
	wxString path = wxSaveFileSelector("package", wxT("GPK package|*.gpk|*.GMP package|*.gmp"), Package->GetPackageName().WString(), this);
	if (path.empty())
	{
		return;
	}

	ProgressWindow progress(this, "Saving");
	progress.SetCurrentProgress(-1);
	progress.SetActionText(wxT("Preparing..."));

	PackageSaveContext context;
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
		return progress.IsCancelled();
	};

	context.ProgressDescriptionCallback = [&progress] (std::string desc) {
		SendEvent(&progress, UPDATE_PROGRESS_DESC, A2W(desc));
	};

	FPackage* package = Package.get();
	std::thread([&progress, &context, package] {
		Sleep(200);
		progress.EndModal(package->Save(context));
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
		Application->SetLastWindowPropertiesSash(ContentSplitter->GetSashPosition(), ContentSplitter->GetSize().x);
		Application->SetLastWindowSize(GetSize());
	}
	e.Skip();
}

void PackageWindow::OnSize(wxSizeEvent& e)
{
	if (!DisableSizeUpdates)
	{
		Application->SetLastWindowObjectSash(SidebarSplitter->GetSashPosition(), SidebarSplitter->GetSize().x);
		Application->SetLastWindowPropertiesSash(ContentSplitter->GetSashPosition(), ContentSplitter->GetSize().x);
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

void PackageWindow::OnPackageReady(wxCommandEvent&)
{
	ObjectTreeCtrl->Freeze();
	LoadObjectTree();
	ObjectTreeCtrl->Thaw();
	SaveMenu->Enable(false); // TODO: track package dirty state
	SaveAsMenu->Enable(!Package->IsReadOnly() && ALLOW_UI_PKG_SAVE);
}

void PackageWindow::OnPackageError(wxCommandEvent& e)
{
	wxMessageBox(e.GetString(), "Failed to load the package!", wxICON_ERROR);
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
EVT_MENU(ControlElementId::New, PackageWindow::OnNewClicked)
EVT_MENU(ControlElementId::Open, PackageWindow::OnOpenClicked)
EVT_MENU(ControlElementId::OpenComposite, PackageWindow::OnOpenCompositeClicked)
EVT_MENU(ControlElementId::Save, PackageWindow::OnSaveClicked)
EVT_MENU(ControlElementId::SaveAs, PackageWindow::OnSaveAsClicked)
EVT_MENU(ControlElementId::Close, PackageWindow::OnCloseClicked)
EVT_MENU(ControlElementId::Exit, PackageWindow::OnExitClicked)
EVT_MENU(ControlElementId::SettingsWin, PackageWindow::OnSettingsClicked)
EVT_MENU(ControlElementId::LogWin, PackageWindow::OnToggleLogClicked)
EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY, PackageWindow::OnObjectTreeStartEdit)
EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, PackageWindow::OnObjectTreeSelectItem)
EVT_MOVE_END(PackageWindow::OnMoveEnd)
EVT_SIZE(PackageWindow::OnSize)
EVT_MAXIMIZE(PackageWindow::OnMaximized)
EVT_CLOSE(PackageWindow::OnCloseWindow)
EVT_COMMAND(wxID_ANY, PACKAGE_READY, PackageWindow::OnPackageReady)
EVT_COMMAND(wxID_ANY, PACKAGE_ERROR, PackageWindow::OnPackageError)
EVT_COMMAND(wxID_ANY, UPDATE_PROPERTIES, PackageWindow::OnUpdateProperties)
EVT_TIMER(wxID_ANY, PackageWindow::OnTick)

EVT_MENU(ControlElementId::DebugTestCookObj, PackageWindow::DebugOnTestCookObject)

wxEND_EVENT_TABLE()