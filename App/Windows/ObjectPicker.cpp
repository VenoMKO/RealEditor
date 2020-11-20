#include "ObjectPicker.h"
#include "../App.h"
#include "../Misc/ObjectTreeModel.h"

#include <Tera/FPackage.h>
#include <Tera/UObject.h>

ObjectPicker::ObjectPicker(wxWindow* parent, const wxString& title, bool allowDifferentPackage, const wxString& packageName, PACKAGE_INDEX selection)
	: wxDialog(parent, wxID_ANY, wxT("Select the object"), wxDefaultPosition, wxSize(441, 438))
	, AllowDifferentPackage(allowDifferentPackage)
	, TableTitle(title)
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxVERTICAL);

	ObjectTreeCtrl = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(250, -1), 0);
	bSizer2->Add(ObjectTreeCtrl, 1, wxRIGHT | wxEXPAND, 1);

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* m_panel6;
	m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 60), wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer(wxHORIZONTAL);

	bSizer11->SetMinSize(wxSize(-1, 60));
	PackageButton = new wxButton(m_panel6, wxID_ANY, wxT("Package..."), wxDefaultPosition, wxDefaultSize, 0);
	PackageButton->Enable(AllowDifferentPackage);

	bSizer11->Add(PackageButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer11->Add(0, 0, 1, wxEXPAND, 5);

	OkButton = new wxButton(m_panel6, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
	OkButton->Enable(false);

	bSizer11->Add(OkButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	CancelButton = new wxButton(m_panel6, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer11->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	m_panel6->SetSizer(bSizer11);
	m_panel6->Layout();
	bSizer4->Add(m_panel6, 1, wxALL, 0);


	bSizer2->Add(bSizer4, 0, wxEXPAND, 5);


	SetSizer(bSizer2);
	Layout();

	Centre(wxBOTH);

	// Connect Events
	ObjectTreeCtrl->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(ObjectPicker::OnObjectSelected), NULL, this);
	PackageButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnPackageClicked), NULL, this);
	OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnOkClicked), NULL, this);
	CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnCancelClicked), NULL, this);

	try
	{
		if ((Package = FPackage::GetPackageNamed(packageName.ToStdWstring())))
		{
			Package->Load();
		}
	}
	catch (const std::exception& e)
	{
		wxMessageBox(e.what(), "Failed to open the package!", wxICON_ERROR);
		Package = nullptr;
		return;
	}

	wxDataViewColumn* col = new wxDataViewColumn(TableTitle + wxT(":"), new wxDataViewIconTextRenderer, 1, wxDVC_DEFAULT_WIDTH, wxALIGN_LEFT);
	ObjectTreeCtrl->AppendColumn(col);

	LoadObjectTree();

	if (selection > 0)
	{
		if (ObjectTreeNode* node = ((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->FindItemByObjectIndex(selection))
		{
			auto item = wxDataViewItem(node);
			ObjectTreeCtrl->Select(item);
			ObjectTreeCtrl->EnsureVisible(item);
			Selection = Package->GetObject(selection);
			OkButton->Enable(Selection);
			UpdateTableTitle();
		}
	}
}

ObjectPicker::~ObjectPicker()
{
	ObjectTreeCtrl->Disconnect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(ObjectPicker::OnObjectSelected), NULL, this);
	PackageButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnPackageClicked), NULL, this);
	OkButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnOkClicked), NULL, this);
	CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnCancelClicked), NULL, this);
	FPackage::UnloadPackage(Package);
}

void ObjectPicker::SetCanChangePackage(bool flag)
{
	PackageButton->Enable(flag);
}

void ObjectPicker::OnObjectSelected(wxDataViewEvent& event)
{
	Selection = nullptr;
	ObjectTreeNode* node = (ObjectTreeNode*)ObjectTreeCtrl->GetCurrentItem().GetID();
	OkButton->Enable(node);
	if (!node)
	{
		UpdateTableTitle();
		return;
	}
	PACKAGE_INDEX index = node->GetObjectIndex();
	if (index == FAKE_EXPORT_ROOT || index == FAKE_IMPORT_ROOT)
	{
		OkButton->Enable(false);
		UpdateTableTitle();
		return;
	}
	Selection = Package->GetObject(index);
	UpdateTableTitle();
}

void ObjectPicker::OnPackageClicked(wxCommandEvent& event)
{
	wxString path = App::GetSharedApp()->ShowOpenDialog();
	if (path.empty())
	{
		return;
	}

	try
	{
		if (auto pkg = FPackage::GetPackage(path.ToStdWstring()))
		{
			pkg->Load();
			Package = pkg;
		}
	}
	catch (const std::exception& e)
	{
		wxMessageBox(e.what(), "Failed to open the package!", wxICON_ERROR);
		return;
	}
	Selection = nullptr;
	LoadObjectTree();
}

void ObjectPicker::OnOkClicked(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}

void ObjectPicker::OnCancelClicked(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

void ObjectPicker::LoadObjectTree()
{
	if (!Package)
	{
		return;
	}
	ObjectTreeModel* model = new ObjectTreeModel(Package->GetPackageName(), Package->GetRootExports(), std::vector<FObjectImport*>());
	model->GetRootExport()->SetCustomObjectIndex(FAKE_EXPORT_ROOT);
	ObjectTreeCtrl->AssociateModel(model);
	ObjectTreeCtrl->GetColumn(0)->SetWidth(ObjectTreeCtrl->GetSize().x - 4);
}

void ObjectPicker::UpdateTableTitle()
{
	if (Selection)
	{
		wxString title = TableTitle.empty() ? wxT("Selected object: ") : TableTitle;
		ObjectTreeCtrl->GetColumn(0)->SetTitle(title + wxT(":") + Selection->GetObjectName().WString());
	}
	else
	{
		ObjectTreeCtrl->GetColumn(0)->SetTitle(TableTitle.empty() ? wxT("Selected object: None") : (TableTitle + wxT(": None")));
	}
}
