#include "CompositePatcherWindow.h"
#include "PackageWindow.h"
#include "../App.h"

#include <wx/statline.h>

#include <Tera/FStructs.h>
#include <Tera/FPackage.h>

enum ControlElementId {
	Select = wxID_HIGHEST + 1,
	Source,
	Size,
	Patch
};

CompositePatcherWindow::CompositePatcherWindow(wxWindow* parent, const wxString& sourceName)
  : wxDialog(parent, wxID_ANY, wxT("Patch composite package map"), wxDefaultPosition, wxSize(540, 300))
	, Patcher(FPackage::GetCompositePackageMapPath().WString())
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText29;
	m_staticText29 = new wxStaticText(this, wxID_ANY, wxT("To patch a specific entry, enter the target composite package name and press Select. Modify the fields you want and click Patch. Make sure you backup your CompositePackageMapper.dat"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText29->Wrap(520);
	bSizer12->Add(m_staticText29, 0, wxALL | wxEXPAND, 5);

	wxStaticLine* m_staticline3;
	m_staticline3 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer12->Add(m_staticline3, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText14;
	m_staticText14 = new wxStaticText(this, wxID_ANY, wxT("Composite Package Name:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText14->Wrap(-1);
	bSizer13->Add(m_staticText14, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	SourceField = new wxTextCtrl(this, ControlElementId::Source, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	SourceField->AutoComplete(((App*)wxTheApp)->GetCompositePackageNames());
	bSizer13->Add(SourceField, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	SelectButton = new wxButton(this, ControlElementId::Select, wxT("Select"), wxDefaultPosition, wxDefaultSize, 0);
	SelectButton->Enable(false);

	bSizer13->Add(SelectButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	bSizer12->Add(bSizer13, 0, wxEXPAND, 5);

	wxStaticLine* m_staticline2;
	m_staticline2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer12->Add(m_staticline2, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText17;
	m_staticText17 = new wxStaticText(this, wxID_ANY, wxT("Container:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText17->Wrap(-1);
	bSizer23->Add(m_staticText17, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ContainerField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer23->Add(ContainerField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	wxStaticText* m_staticText27;
	m_staticText27 = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText27->Wrap(-1);
	bSizer23->Add(m_staticText27, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	CompositeNameField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer23->Add(CompositeNameField, 1, wxALL, 5);


	bSizer12->Add(bSizer23, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText18;
	m_staticText18 = new wxStaticText(this, wxID_ANY, wxT("Object:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText18->Wrap(-1);
	bSizer27->Add(m_staticText18, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ObjectField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer27->Add(ObjectField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer12->Add(bSizer27, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer231;
	bSizer231 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText171;
	m_staticText171 = new wxStaticText(this, wxID_ANY, wxT("Offset:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText171->Wrap(-1);
	bSizer231->Add(m_staticText171, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	OffsetField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer231->Add(OffsetField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	wxStaticText* m_staticText181;
	m_staticText181 = new wxStaticText(this, wxID_ANY, wxT("Size:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText181->Wrap(-1);
	bSizer231->Add(m_staticText181, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	SizeField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer231->Add(SizeField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	SizeButton = new wxButton(this, ControlElementId::Size, wxT("Get Size"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer231->Add(SizeButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	bSizer12->Add(bSizer231, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* m_panel9;
	m_panel9 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	bSizer28->Add(m_panel9, 1, wxEXPAND | wxALL, 5);

	PatchButton = new wxButton(this, ControlElementId::Patch, wxT("Patch"), wxDefaultPosition, wxDefaultSize, 0);
	PatchButton->Enable(false);

	bSizer28->Add(PatchButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	CancelButton = new wxButton(this, wxID_CANCEL, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer28->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer12->Add(bSizer28, 1, wxEXPAND, 5);


	SetSizer(bSizer12);
	Layout();

	Centre(wxBOTH);

	PatchButton->Enable(false);
	ObjectField->Enable(false);
	ContainerField->Enable(false);
	CompositeNameField->Enable(false);
	OffsetField->Enable(false);
	SizeField->Enable(false);
	SizeButton->Enable(false);
	SourceField->SetValue(sourceName);
}

void CompositePatcherWindow::OnSourceFieldText(wxCommandEvent&)
{
	bool enabled = ((App*)wxTheApp)->GetCompositePackageNames().Index(SourceField->GetValue(), false) != wxNOT_FOUND;
	SelectButton->Enable(enabled);
	if (!enabled)
	{
		PatchButton->Enable(enabled);
		ObjectField->Enable(enabled);
		ContainerField->Enable(enabled);
		CompositeNameField->Enable(enabled);
		OffsetField->Enable(enabled);
		SizeField->Enable(enabled);
		SizeButton->Enable(enabled);
	}
}

void CompositePatcherWindow::OnSourceFieldEnter(wxCommandEvent& e)
{
	if (SelectButton->IsEnabled())
	{
		OnSelectClicked(e);
	}
}

void CompositePatcherWindow::OnSelectClicked(wxCommandEvent&)
{
	auto& map = FPackage::GetCompositePackageMap();
	FString name = SourceField->GetValue().ToStdWstring();

	if (!map.count(name))
	{
		wxMessageBox("Failed to find package!", wxT("Error!"), wxICON_ERROR);
		return;
	}

	PatchButton->Enable(true);
	ObjectField->Enable(true);
	ContainerField->Enable(true);
	CompositeNameField->Enable(true);
	OffsetField->Enable(true);
	SizeField->Enable(true);
	SizeButton->Enable(true);

	FCompositePackageMapEntry info = map.at(name);
	ObjectField->SetValue(info.ObjectPath.String());
	ContainerField->SetValue(info.FileName.String());
	CompositeNameField->SetValue(SourceField->GetValue());
	OffsetField->SetValue(wxString::Format("%d", info.Offset));
	SizeField->SetValue(wxString::Format("%d", info.Size));
}

void CompositePatcherWindow::OnPatchClicked(wxCommandEvent&)
{
	CompositeEntry entry;
	entry.Object = ObjectField->GetValue();
	entry.Filename = ContainerField->GetValue();
	entry.CompositeName = CompositeNameField->GetValue();
	if (entry.Object.empty())
	{
		wxMessageBox(wxT("Object is can't be empty."), wxT("Error!"));
		ObjectField->SetFocus();
		return;
	}
	if (entry.Filename.empty())
	{
		wxMessageBox(wxT("Container is can't be empty."), wxT("Error!"));
		ContainerField->SetFocus();
		return;
	}
	long offset = 0;
	if (!OffsetField->GetValue().ToLong(&offset) || offset < 0)
	{
		wxMessageBox(wxT("Offset is invalid."), wxT("Error!"));
		OffsetField->SetFocus();
		return;
	}
	entry.Offset = (int)offset;
	long size = 0;
	if (!SizeField->GetValue().ToLong(&size) || size < 0)
	{
		wxMessageBox(wxT("Size is invalid."), wxT("Error!"));
		SizeField->SetFocus();
		return;
	}
	entry.Size = (int)size;
	
	CompositePatcher& patcher = Patcher;
	const std::string source = SourceField->GetValue().ToStdString();

	try
	{
		if (!patcher.IsLoaded())
		{
			patcher.Load();
		}
		patcher.Patch(source, entry);
		patcher.Apply();
	}
	catch (const std::exception& e)
	{
		wxMessageBox(e.what(), wxT("Error!"), wxICON_ERROR);
		EndModal(wxID_CANCEL);
		return;
	}
	wxMessageBox(wxT("Composite package map has been patched."), wxT("Done!"), wxICON_INFORMATION);
	EndModal(wxID_OK);
}

void CompositePatcherWindow::OnSizeClicked(wxCommandEvent&)
{
	FPackage* package = ((PackageWindow*)GetParent())->GetPackage().get();
	SizeField->SetValue(wxString::Format("%d", (int)package->GetSourceSize()));
}

wxBEGIN_EVENT_TABLE(CompositePatcherWindow, wxDialog)
EVT_BUTTON(ControlElementId::Select, CompositePatcherWindow::OnSelectClicked)
EVT_TEXT(ControlElementId::Source, CompositePatcherWindow::OnSourceFieldText)
EVT_TEXT_ENTER(ControlElementId::Source, CompositePatcherWindow::OnSourceFieldEnter)
EVT_BUTTON(ControlElementId::Patch, CompositePatcherWindow::OnPatchClicked)
EVT_BUTTON(ControlElementId::Size, CompositePatcherWindow::OnSizeClicked)
wxEND_EVENT_TABLE()