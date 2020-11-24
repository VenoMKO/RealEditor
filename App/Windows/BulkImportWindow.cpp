#include "../App.h"
#include "BulkImportWindow.h"
#include "ProgressWindow.h"
#include "TextureImporter.h"
#include "ObjectPicker.h"

#include <wx/notebook.h>
#include <wx/clipbrd.h>
#include <filesystem>

#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UClass.h>
#include <Tera/USoundNode.h>
#include <Tera/UTexture.h>
#include <Tera/Cast.h>

enum ObjTreeMenuId {
	ObjectList = wxID_HIGHEST + 1,
	ShowObject,
	CopyPackage,
	CopyPath,
	Toggle
};

class BulkImportOperationEntryModel : public wxDataViewVirtualListModel {
public:
	enum
	{
		Col_Check = 0,
		Col_Package,
		Col_Path,
		Col_Max
	};

	BulkImportOperationEntryModel(std::vector<BulkImportAction::Entry> entries)
		: Rows(entries)
	{}

	unsigned int GetColumnCount() const override
	{
		return Col_Max;
	}

	wxString GetColumnType(unsigned int col) const override
	{
		if (col == Col_Check)
		{
			return "bool";
		}
		return wxDataViewCheckIconTextRenderer::GetDefaultType();
	}

	unsigned int GetCount() const override
	{
		return Rows.size();
	}

	void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override
	{
		switch (col)
		{
		case Col_Check:
			variant = Rows[row].Enabled;
			break;
		case Col_Package:
			variant = Rows[row].PackageName;
			break;
		case Col_Path:
			variant = Rows[row].ObjectPath;
			break;
		}
	}

	bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override
	{
		return false;
	}

	bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override
	{
		if (col == Col_Check)
		{
			Rows[row].Enabled = variant.GetBool();
			return true;
		}
		return false;
	}

	std::vector<BulkImportAction::Entry> GetRows() const
	{
		return Rows;
	}

private:
	std::vector<BulkImportAction::Entry> Rows;
};

class BulkImportModel : public wxDataViewVirtualListModel {
public:
	enum
	{
		Col_Type = 0,
		Col_Object,
		Col_Mod,
		Col_Max
	};

	BulkImportModel(std::vector<BulkImportAction> ops)
		: Rows(ops)
	{}

	unsigned int GetColumnCount() const override
	{
		return Col_Max;
	}

	wxString GetColumnType(unsigned int col) const override
	{
		return wxDataViewCheckIconTextRenderer::GetDefaultType();
	}

	unsigned int GetCount() const override
	{
		return Rows.size();
	}

	void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override
	{
		switch (col)
		{
		case Col_Type:
			variant = Rows[row].ImportPath.size() ? wxT("Import") : Rows[row].RedirectPath.size() ? wxT("Redirect") : wxT("None");
			break;
		case Col_Object:
			variant = Rows[row].ObjectName;
			break;
		case Col_Mod:
			variant = Rows[row].GetChangeString();
			break;
		}
	}

	bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override
	{
		return false;
	}

	bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override
	{
		return false;
	}

	std::vector<BulkImportAction> GetRows() const
	{
		return Rows;
	}

private:
	std::vector<BulkImportAction> Rows;
};


class AddImportOperationDialog : public wxDialog {
public:

	AddImportOperationDialog(wxWindow* parent, std::stringstream& objectDumpBuffer, const wxString& confirmTitle = wxT("Add"), const wxString& objectClass = wxT("Texture2D"), const wxString& objectName = wxEmptyString)
		: wxDialog(parent, wxID_ANY, wxT("Add bulk action"), wxDefaultPosition, wxSize(605, 619))
		, ObjectDumpBuffer(objectDumpBuffer)
	{
		SetSizeHints(wxDefaultSize, wxDefaultSize);
		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

		wxBoxSizer* bSizer20;
		bSizer20 = new wxBoxSizer(wxVERTICAL);

		wxStaticText* m_staticText17;
		m_staticText17 = new wxStaticText(this, wxID_ANY, wxT("Object info"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText17->Wrap(-1);
		m_staticText17->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

		bSizer20->Add(m_staticText17, 0, wxTOP | wxRIGHT | wxLEFT, 5);

		wxPanel* m_panel8;
		m_panel8 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
		wxBoxSizer* bSizer23;
		bSizer23 = new wxBoxSizer(wxVERTICAL);

		wxStaticText* m_staticText20;
		m_staticText20 = new wxStaticText(m_panel8, wxID_ANY, wxT("Enter class and name of the object and you want to change and press Search. RE will look for all objects matching the class with similar names."), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText20->Wrap(580);
		bSizer23->Add(m_staticText20, 0, wxALL, 5);

		wxBoxSizer* bSizer22;
		bSizer22 = new wxBoxSizer(wxHORIZONTAL);

		wxStaticText* m_staticText18;
		m_staticText18 = new wxStaticText(m_panel8, wxID_ANY, wxT("Class:"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText18->Wrap(-1);
		bSizer22->Add(m_staticText18, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		ObjectClassTextField = new wxTextCtrl(m_panel8, wxID_ANY, objectClass, wxDefaultPosition, wxSize(170, -1), 0);
		bSizer22->Add(ObjectClassTextField, 0, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

		wxStaticText* m_staticText19;
		m_staticText19 = new wxStaticText(m_panel8, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText19->Wrap(-1);
		bSizer22->Add(m_staticText19, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		ObjectNameTextField = new wxTextCtrl(m_panel8, wxID_ANY, objectName, wxDefaultPosition, wxSize(-1, -1), 0);
		bSizer22->Add(ObjectNameTextField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		SearchButton = new wxButton(m_panel8, wxID_ANY, wxT("Search"), wxDefaultPosition, wxDefaultSize, 0);
		bSizer22->Add(SearchButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


		bSizer23->Add(bSizer22, 0, wxEXPAND, 5);


		m_panel8->SetSizer(bSizer23);
		m_panel8->Layout();
		bSizer23->Fit(m_panel8);
		bSizer20->Add(m_panel8, 0, wxEXPAND | wxALL, 5);

		wxStaticText* m_staticText171;
		m_staticText171 = new wxStaticText(this, wxID_ANY, wxT("Search results"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText171->Wrap(-1);
		m_staticText171->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

		bSizer20->Add(m_staticText171, 0, wxTOP | wxRIGHT | wxLEFT, 5);

		wxPanel* m_panel9;
		m_panel9 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
		wxBoxSizer* bSizer26;
		bSizer26 = new wxBoxSizer(wxVERTICAL);

		wxStaticText* m_staticText24;
		m_staticText24 = new wxStaticText(m_panel9, wxID_ANY, wxT("List of packages containing a copy of the object. Right-click an entry and press \"Show Object\" to view the duplicate object."), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText24->Wrap(575);
		bSizer26->Add(m_staticText24, 0, wxALL | wxEXPAND, 5);

		List = new wxDataViewCtrl(m_panel9, wxID_ANY, wxDefaultPosition, wxSize(-1, 200), wxDV_MULTIPLE);
		bSizer26->Add(List, 0, wxALL | wxEXPAND, 5);


		m_panel9->SetSizer(bSizer26);
		m_panel9->Layout();
		bSizer26->Fit(m_panel9);
		bSizer20->Add(m_panel9, 0, wxEXPAND | wxALL, 5);

		wxStaticText* m_staticText172;
		m_staticText172 = new wxStaticText(this, wxID_ANY, wxT("Action"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText172->Wrap(-1);
		m_staticText172->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

		bSizer20->Add(m_staticText172, 0, wxTOP | wxRIGHT | wxLEFT, 5);

		wxBoxSizer* bSizer38;
		bSizer38 = new wxBoxSizer(wxVERTICAL);

		wxPanel* m_panel13;
		m_panel13 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
		wxBoxSizer* bSizer39;
		bSizer39 = new wxBoxSizer(wxVERTICAL);

		OperationView = new wxNotebook(m_panel13, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
		wxPanel* m_panel11;
		m_panel11 = new wxPanel(OperationView, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		m_panel11->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

		wxBoxSizer* bSizer29;
		bSizer29 = new wxBoxSizer(wxVERTICAL);

		wxStaticText* m_staticText28;
		m_staticText28 = new wxStaticText(m_panel11, wxID_ANY, wxT("Import an external file"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText28->Wrap(-1);
		bSizer29->Add(m_staticText28, 0, wxTOP | wxRIGHT | wxLEFT, 5);

		wxBoxSizer* bSizer28;
		bSizer28 = new wxBoxSizer(wxHORIZONTAL);

		wxStaticText* m_staticText27;
		m_staticText27 = new wxStaticText(m_panel11, wxID_ANY, wxT("Path:"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText27->Wrap(-1);
		bSizer28->Add(m_staticText27, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		ImportTextField = new wxTextCtrl(m_panel11, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
		ImportTextField->Enable(false);

		bSizer28->Add(ImportTextField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		ImportBrowseButton = new wxButton(m_panel11, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0);
		ImportBrowseButton->Enable(false);

		bSizer28->Add(ImportBrowseButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		ImportClearButton = new wxButton(m_panel11, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
		ImportClearButton->Enable(false);

		bSizer28->Add(ImportClearButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


		bSizer29->Add(bSizer28, 1, wxEXPAND, 5);


		m_panel11->SetSizer(bSizer29);
		m_panel11->Layout();
		bSizer29->Fit(m_panel11);
		OperationView->AddPage(m_panel11, wxT("Import"), true);
		wxPanel* m_panel12;
		m_panel12 = new wxPanel(OperationView, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		m_panel12->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

		wxBoxSizer* bSizer291;
		bSizer291 = new wxBoxSizer(wxVERTICAL);

		wxStaticText* m_staticText281;
		m_staticText281 = new wxStaticText(m_panel12, wxID_ANY, wxT("Redirect to an existing object"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText281->Wrap(-1);
		bSizer291->Add(m_staticText281, 0, wxTOP | wxRIGHT | wxLEFT, 5);

		wxBoxSizer* bSizer281;
		bSizer281 = new wxBoxSizer(wxHORIZONTAL);

		wxStaticText* m_staticText271;
		m_staticText271 = new wxStaticText(m_panel12, wxID_ANY, wxT("Target:"), wxDefaultPosition, wxDefaultSize, 0);
		m_staticText271->Wrap(-1);
		bSizer281->Add(m_staticText271, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		RedirectTextField = new wxTextCtrl(m_panel12, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
		RedirectTextField->Enable(false);

		bSizer281->Add(RedirectTextField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		RedirectBrowseButton = new wxButton(m_panel12, wxID_ANY, wxT("Browse"), wxDefaultPosition, wxDefaultSize, 0);
		RedirectBrowseButton->Enable(false);

		bSizer281->Add(RedirectBrowseButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		RedirectClearButton = new wxButton(m_panel12, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
		RedirectClearButton->Enable(false);

		bSizer281->Add(RedirectClearButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


		bSizer291->Add(bSizer281, 1, wxEXPAND, 5);


		m_panel12->SetSizer(bSizer291);
		m_panel12->Layout();
		bSizer291->Fit(m_panel12);
		OperationView->AddPage(m_panel12, wxT("Redirect"), false);

		bSizer39->Add(OperationView, 0, wxALL | wxEXPAND, 5);


		m_panel13->SetSizer(bSizer39);
		m_panel13->Layout();
		bSizer39->Fit(m_panel13);
		bSizer38->Add(m_panel13, 1, wxEXPAND | wxALL, 5);


		bSizer20->Add(bSizer38, 1, wxEXPAND, 5);

		wxBoxSizer* bSizer37;
		bSizer37 = new wxBoxSizer(wxHORIZONTAL);

		SearchResultLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
		SearchResultLabel->Wrap(-1);
		bSizer37->Add(SearchResultLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


		bSizer37->Add(0, 0, 1, wxEXPAND, 5);

		AddButton = new wxButton(this, wxID_ANY, confirmTitle, wxDefaultPosition, wxDefaultSize, 0);
		AddButton->Enable(false);

		bSizer37->Add(AddButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

		CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
		bSizer37->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


		bSizer20->Add(bSizer37, 1, wxEXPAND, 5);


		this->SetSizer(bSizer20);
		this->Layout();

		this->Centre(wxBOTH);

		// Connect Events
		ObjectClassTextField->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(AddImportOperationDialog::OnClassText), NULL, this);
		ObjectClassTextField->Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(AddImportOperationDialog::OnClassEnter), NULL, this);
		ObjectNameTextField->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(AddImportOperationDialog::OnNameText), NULL, this);
		ObjectNameTextField->Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(AddImportOperationDialog::OnNameEnter), NULL, this);
		SearchButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnSearchClicked), NULL, this);
		ImportBrowseButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnImportClicked), NULL, this);
		ImportClearButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnImportClearClicked), NULL, this);
		RedirectBrowseButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnRedirectBrowseClicked), NULL, this);
		RedirectClearButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnRedirectClearClicked), NULL, this);
		AddButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnAddClicked), NULL, this);
		CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnCancelClicked), NULL, this);
		List->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(AddImportOperationDialog::OnListContextMenu), NULL, this);
		List->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEventHandler(AddImportOperationDialog::OnListValueChanged), NULL, this);
		
		Connect(wxEVT_IDLE, wxIdleEventHandler(AddImportOperationDialog::OnFirstIdle), NULL, this);

		OperationView->SetSelection(0);

		wxArrayString classList;
		std::vector<UClass*> classes = FPackage::GetClasses();
		for (UClass* cls : classes)
		{
			classList.push_back(cls->GetObjectName().WString());
		}
		ObjectClassTextField->AutoComplete(classList);

		List->AppendToggleColumn(_(""), BulkImportOperationEntryModel::Col_Check, wxDATAVIEW_CELL_ACTIVATABLE, 25);
		List->AppendTextColumn(_("Package"), BulkImportOperationEntryModel::Col_Package, wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
		List->AppendTextColumn(_("Object"), BulkImportOperationEntryModel::Col_Path, wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);

		UpdateControls();
	}

	AddImportOperationDialog(wxWindow* parent, std::stringstream& objectDumpBuffer, const BulkImportAction& op, const wxString& confirmTitle = wxT("Apply"))
		: AddImportOperationDialog(parent, objectDumpBuffer, confirmTitle, op.ClassName, op.ObjectName)
	{
		List->AssociateModel(new BulkImportOperationEntryModel(op.Entries));
		ImportTextField->SetValue(op.ImportPath);
		RedirectTextField->SetValue(op.RedirectPath);
		RedirectIndex = op.RedirectIndex;
		if (op.RedirectPath.size())
		{
			OperationView->SetSelection(1);
		}
		UpdateControls();
	}

	~AddImportOperationDialog()
	{
		ObjectClassTextField->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(AddImportOperationDialog::OnClassText), NULL, this);
		ObjectClassTextField->Disconnect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(AddImportOperationDialog::OnClassEnter), NULL, this);
		ObjectNameTextField->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(AddImportOperationDialog::OnNameText), NULL, this);
		ObjectNameTextField->Disconnect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(AddImportOperationDialog::OnNameEnter), NULL, this);
		SearchButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnSearchClicked), NULL, this);
		ImportBrowseButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnImportClicked), NULL, this);
		ImportClearButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnImportClearClicked), NULL, this);
		RedirectBrowseButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnRedirectBrowseClicked), NULL, this);
		RedirectClearButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnRedirectClearClicked), NULL, this);
		AddButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnAddClicked), NULL, this);
		CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AddImportOperationDialog::OnCancelClicked), NULL, this);
		List->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(AddImportOperationDialog::OnListContextMenu), NULL, this);
		List->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEventHandler(AddImportOperationDialog::OnListValueChanged), NULL, this);
	}

	inline void SetAutoSearch(bool flag)
	{
		AutoSearch = flag;
	}

	void OnFirstIdle(wxIdleEvent&)
	{
		Disconnect(wxEVT_IDLE, wxIdleEventHandler(AddImportOperationDialog::OnFirstIdle), NULL, this);
		if (ObjectClassTextField->GetValue().size() && ObjectNameTextField->GetValue().size() && AutoSearch)
		{
			UpdateControls();
			MSWClickButtonIfPossible(SearchButton);
		}
	}

	void ConfigureOperation(BulkImportAction& op)
	{
		op.ClassName = ObjectClassTextField->GetValue();
		op.ObjectName = ObjectNameTextField->GetValue();
		op.Entries = List->GetModel() ? ((BulkImportOperationEntryModel*)List->GetModel())->GetRows() : std::vector<BulkImportAction::Entry>();
		if (OperationView->GetSelection())
		{
			op.RedirectPath = RedirectTextField->GetValue();
			op.RedirectIndex = RedirectIndex;
			op.ImportPath.clear();
		}
		else
		{
			op.ImportPath = ImportTextField->GetValue();
			op.RedirectPath.clear();
			op.RedirectIndex = 0;
		}
	}

protected:
	void OnClassText(wxCommandEvent& event)
	{
		UpdateControls();
	}

	void OnClassEnter(wxCommandEvent& event)
	{
		UpdateControls();
		ObjectNameTextField->SetFocus();
	}

	void OnNameText(wxCommandEvent& event)
	{
		UpdateControls();
	}

	void OnNameEnter(wxCommandEvent& event)
	{
		UpdateControls();
		this->MSWClickButtonIfPossible(SearchButton);
	}

	void OnSearchClicked(wxCommandEvent& event)
	{
		const std::string className = ObjectClassTextField->GetValue().ToStdString();
		const std::string objectName = ObjectNameTextField->GetValue().ToStdString();
		const std::string dupObjectName = objectName + '_';
		ProgressWindow progress(this, wxT("Searching..."));
		wxString desc = "Looking for all " + className + " objects with " + objectName + " name";
		progress.SetActionText(desc);
		progress.SetCanCancel(false);
		progress.SetCurrentProgress(-1);
		std::stringstream& buffer = ObjectDumpBuffer;
		std::vector<BulkImportAction::Entry> found;
		std::thread([&] {
			buffer.clear();
			buffer.seekg(0);
			std::string line;
			while (std::getline(buffer, line))
			{
				if (line.empty() || line.size() < 2 || !line._Starts_with(className))
				{
					continue;
				}
				auto pos = line.find_last_of('.');
				if (pos == std::string::npos)
				{
					continue;
				}
				PACKAGE_INDEX objIndex = INDEX_NONE;
				std::string_view lineObjectName(&line[pos + 1]);
				if (lineObjectName == objectName || lineObjectName._Starts_with(dupObjectName))
				{
					pos = line.find_first_of('\t');
					if (pos == std::string::npos)
					{
						continue;
					}
					auto end = line.find('\t', pos + 1);
					if (pos == std::string::npos)
					{
						continue;
					}
					try
					{
						objIndex = std::stoi(std::string(&line[pos + 1], end - pos - 1));
					}
					catch (...)
					{
						continue;
					}
					if (objIndex < 0)
					{
						continue;
					}
					pos = end;
					end = line.find('.', pos + 1);
					if (end == std::string::npos)
					{
						continue;
					}
					std::string objectPath(&line[end + 1], line.size() - end - 1);
					std::replace(objectPath.begin(), objectPath.end(), '.', '\\');
					found.push_back({ objectPath, std::string(&line[pos + 1], end - pos - 1), objIndex, true });
				}
			}
			SendEvent(&progress, UPDATE_PROGRESS_FINISH);
		}).detach();
		progress.ShowModal();

		SearchResultLabel->SetLabelText(wxString::Format(wxT("Found: %Iu item(s)"), found.size()));
		List->AssociateModel(new BulkImportOperationEntryModel(found));
		UpdateControls();
	}

	void OnImportClicked(wxCommandEvent& event)
	{
		const std::string className = ObjectClassTextField->GetValue().ToStdString();
		wxString path;
		if (className == UTexture2D::StaticClassName())
		{
			path = TextureImporter::LoadImageDialog(this);
		}
		else if (className == USoundNodeWave::StaticClassName())
		{
			wxString ext = "OGG files (*.ogg)|*.ogg";
			path = wxFileSelector("Import a sound file", wxEmptyString, wxEmptyString, ext, ext, wxFD_OPEN, this);
		}
		else
		{
			wxString ext = "Any files (*.*)|*.*";
			path = wxFileSelector("Import a file", wxEmptyString, wxEmptyString, ext, ext, wxFD_OPEN, this);
		}
		if (path.size())
		{
			ImportTextField->SetValue(path);
			UpdateControls();
		}
	}

	void OnImportClearClicked(wxCommandEvent& event)
	{
		ImportTextField->SetValue(wxEmptyString);
		UpdateControls();
	}

	void OnRedirectBrowseClicked(wxCommandEvent& event)
	{
		ObjectPicker picker(this, wxT("Redirect to"), true, wxEmptyString);
		if (picker.ShowModal() != wxID_OK || !picker.GetSelectedObject())
		{
			return;
		}
		RedirectTextField->SetValue(picker.GetSelectedObject()->GetObjectPath().WString());
		RedirectIndex = picker.GetSelectedObject()->GetExportObject()->ObjectIndex;
		UpdateControls();
	}

	void OnRedirectClearClicked(wxCommandEvent& event)
	{
		RedirectTextField->SetValue(wxEmptyString);
		UpdateControls();
	}

	void OnAddClicked(wxCommandEvent& event)
	{
		EndModal(wxID_OK);
	}

	void OnCancelClicked(wxCommandEvent& event)
	{
		EndModal(wxID_CANCEL);
	}

	void UpdateControls()
	{
		SearchButton->Enable(ObjectClassTextField->GetValue().size() && ObjectNameTextField->GetValue().size());
		bool hasValidEntries = false;
		if (List->GetModel())
		{
			auto rows = ((BulkImportOperationEntryModel*)List->GetModel())->GetRows();
			for (const auto& entry : rows)
			{
				if (entry.IsValid())
				{
					hasValidEntries = true;
					break;
				}
			}
		}
		AddButton->Enable(hasValidEntries);
		ImportBrowseButton->Enable(hasValidEntries);
		ImportClearButton->Enable(hasValidEntries && ImportTextField->GetValue().size());
		RedirectBrowseButton->Enable(hasValidEntries);
		RedirectClearButton->Enable(hasValidEntries && RedirectTextField->GetValue().size());
	}

	void OnListValueChanged(wxDataViewEvent& event)
	{
		UpdateControls();
	}

	void OnListContextMenu(wxDataViewEvent& event)
	{
		if (!event.GetItem().IsOk())
		{
			return;
		}
		auto rows = ((BulkImportOperationEntryModel*)List->GetModel())->GetRows();
		int idx = int(event.GetItem().GetID()) - 1;
		if (idx < 0 || rows.size() <= idx)
		{
			return;
		}

		wxMenu menu;
		menu.Append(ObjTreeMenuId::ShowObject, wxT("Show the object..."));
		menu.Append(ObjTreeMenuId::Toggle, wxT("Toggle selected items"));
		menu.Append(ObjTreeMenuId::CopyPackage, wxT("Copy package name"));
		menu.Append(ObjTreeMenuId::CopyPath, wxT("Copy object path"));

		const int selectedMenuId = GetPopupMenuSelectionFromUser(menu);
		if (selectedMenuId == ObjTreeMenuId::ShowObject)
		{
			std::string objectPath = rows[idx].ObjectPath.ToStdString();
			std::replace(objectPath.begin(), objectPath.end(), '\\', '.');
			((App*)wxTheApp)->OpenNamedPackage(rows[idx].PackageName, rows[idx].PackageName + '.' + objectPath);
		}
		else if (selectedMenuId == ObjTreeMenuId::CopyPackage)
		{
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->Clear();
				wxTheClipboard->SetData(new wxTextDataObject(rows[idx].PackageName));
				wxTheClipboard->Flush();
				wxTheClipboard->Close();
			}
		}
		else if (selectedMenuId == ObjTreeMenuId::CopyPath)
		{
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->Clear();
				wxTheClipboard->SetData(new wxTextDataObject(rows[idx].ObjectPath));
				wxTheClipboard->Flush();
				wxTheClipboard->Close();
			}
		}
		else if (selectedMenuId == ObjTreeMenuId::Toggle)
		{
			wxDataViewItemArray selection;
			List->GetSelections(selection);
			if (!selection.GetCount())
			{
				return;
			}

			auto results = ((BulkImportOperationEntryModel*)List->GetModel())->GetRows();
			List->Freeze();
			for (auto& item : selection)
			{
				int idx = int(item.GetID()) - 1;
				wxVariant value = !results[idx].Enabled;
				List->GetModel()->ChangeValue(value, item, BulkImportOperationEntryModel::Col_Check);
			}
			List->Thaw();
		}
	}

protected:
	wxTextCtrl* ObjectClassTextField = nullptr;
	wxTextCtrl* ObjectNameTextField = nullptr;
	wxButton* SearchButton = nullptr;
	wxDataViewCtrl* List = nullptr;
	wxNotebook* OperationView = nullptr;
	wxTextCtrl* ImportTextField = nullptr;
	wxButton* ImportBrowseButton = nullptr;
	wxButton* ImportClearButton = nullptr;
	wxTextCtrl* RedirectTextField = nullptr;
	wxButton* RedirectBrowseButton = nullptr;
	wxButton* RedirectClearButton = nullptr;
	wxStaticText* SearchResultLabel = nullptr;
	wxButton* AddButton = nullptr;
	wxButton* CancelButton = nullptr;

	std::stringstream& ObjectDumpBuffer;
	PACKAGE_INDEX RedirectIndex = 0;
	bool AutoSearch = false;
};

BulkImportWindow::BulkImportWindow(wxWindow* parent)
	: wxFrame(parent, wxID_ANY, wxT("Bulk Import"), wxDefaultPosition, wxSize(667, 541))
{
	SetIcon(wxICON(#114));
	SetSizeHints(wxDefaultSize, wxDefaultSize);
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText10;
	m_staticText10 = new wxStaticText(this, wxID_ANY, wxT("Composite package dump"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText10->Wrap(-1);
	m_staticText10->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer10->Add(m_staticText10, 0, wxALL, 5);

	wxPanel* m_panel6;
	m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText11;
	m_staticText11 = new wxStaticText(m_panel6, wxID_ANY, wxT("Select the ObjectDump.txt file. This file is used to search for objects in the composite storage."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText11->Wrap(650);
	bSizer12->Add(m_staticText11, 0, wxALL, 5);

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText9;
	m_staticText9 = new wxStaticText(m_panel6, wxID_ANY, wxT("Path:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText9->Wrap(-1);
	bSizer11->Add(m_staticText9, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	PathPicker = new wxFilePickerCtrl(m_panel6, wxID_ANY, wxEmptyString, wxT("Select the object dump file"), wxT("*.txt"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE | wxFLP_FILE_MUST_EXIST);
	PathPicker->GetTextCtrl()->Enable(false);
	bSizer11->Add(PathPicker, 1, wxALL, 5);


	bSizer12->Add(bSizer11, 1, wxEXPAND, 5);


	m_panel6->SetSizer(bSizer12);
	m_panel6->Layout();
	bSizer12->Fit(m_panel6);
	bSizer10->Add(m_panel6, 0, wxEXPAND | wxALL, 5);

	wxStaticText* m_staticText101;
	m_staticText101 = new wxStaticText(this, wxID_ANY, wxT("Actions"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText101->Wrap(-1);
	m_staticText101->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer10->Add(m_staticText101, 0, wxALL, 5);

	wxPanel* m_panel7;
	m_panel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText15;
	m_staticText15 = new wxStaticText(m_panel7, wxID_ANY, wxT("List of import actions. Each entry represents a single action. Press Add to create an import action."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText15->Wrap(650);
	bSizer15->Add(m_staticText15, 0, wxALL, 5);

	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer(wxHORIZONTAL);

	OperationsList = new wxDataViewCtrl(m_panel7, wxID_ANY, wxDefaultPosition, wxSize(-1, 250), 0);
	bSizer16->Add(OperationsList, 1, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer(wxVERTICAL);

	AddOperationButton = new wxButton(m_panel7, wxID_ANY, wxT("Add"), wxDefaultPosition, wxDefaultSize, 0);
	AddOperationButton->Enable(false);

	bSizer17->Add(AddOperationButton, 0, wxALL, 5);

	EditOperationButton = new wxButton(m_panel7, wxID_ANY, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0);
	EditOperationButton->Enable(false);

	bSizer17->Add(EditOperationButton, 0, wxALL, 5);

	RemoveOperationButton = new wxButton(m_panel7, wxID_ANY, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0);
	RemoveOperationButton->Enable(false);

	bSizer17->Add(RemoveOperationButton, 0, wxALL, 5);

	ClearOperationsButton = new wxButton(m_panel7, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
	ClearOperationsButton->Enable(false);

	bSizer17->Add(ClearOperationsButton, 0, wxALL, 5);


	bSizer16->Add(bSizer17, 0, wxEXPAND, 5);


	bSizer15->Add(bSizer16, 1, wxEXPAND, 5);


	m_panel7->SetSizer(bSizer15);
	m_panel7->Layout();
	bSizer15->Fit(m_panel7);
	bSizer10->Add(m_panel7, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer(wxHORIZONTAL);


	bSizer18->Add(0, 0, 1, wxEXPAND, 5);

	ContinueButton = new wxButton(this, wxID_ANY, wxT("Continue"), wxDefaultPosition, wxDefaultSize, 0);
	ContinueButton->Enable(false);

	bSizer18->Add(ContinueButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer18->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer10->Add(bSizer18, 1, wxEXPAND, 5);


	SetSizer(bSizer10);
	Layout();

	Centre(wxBOTH);

	// Connect Events
	PathPicker->Connect(wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler(BulkImportWindow::OnPathChanged), NULL, this);
	OperationsList->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING, wxDataViewEventHandler(BulkImportWindow::OnStartEditingOperation), NULL, this);
	OperationsList->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(BulkImportWindow::OnOperationSelected), NULL, this);
	AddOperationButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnAddOperationClicked), NULL, this);
	EditOperationButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnEditOperationClicked), NULL, this);
	RemoveOperationButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnRemoveOperationClicked), NULL, this);
	ClearOperationsButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnClearOperationsClicked), NULL, this);
	ContinueButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnContinueClicked), NULL, this);
	CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnCancelClicked), NULL, this);
	OperationsList->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(BulkImportWindow::OnOperationsListContextMenu), NULL, this);
	Connect(wxEVT_IDLE, wxIdleEventHandler(BulkImportWindow::OnFirstIdle), NULL, this);

	OperationsList->AppendTextColumn(_("Object"), BulkImportModel::Col_Object, wxDATAVIEW_CELL_INERT, 200, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	OperationsList->AppendTextColumn(_("Action"), BulkImportModel::Col_Type, wxDATAVIEW_CELL_INERT, 100);
	OperationsList->AppendTextColumn(_("Change"), BulkImportModel::Col_Mod, wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);

	FAppConfig& cfg = App::GetSharedApp()->GetConfig();
	if (cfg.CompositeDumpPath.Size())
	{
		PreviousStreamPath = cfg.CompositeDumpPath.WString();
		PathPicker->SetPath(PreviousStreamPath);
	}

	OperationsList->AssociateModel(new BulkImportModel(Actions));
}

BulkImportWindow::BulkImportWindow(wxWindow* parent, const wxString& className, const wxString& objectName)
	: BulkImportWindow(parent)
{
	FirstStartClass = className;
	FirstStartName = objectName;
}

void BulkImportWindow::AddOperation(const wxString& className, const wxString& objectName)
{
	if (AddOperationButton->IsEnabled() && className.size() && objectName.size())
	{
		if (!BufferLoaded)
		{
			if (!LoadBuffer())
			{
				return;
			}
		}

		AddImportOperationDialog dlg(this, ObjectDumpBuffer, wxT("Add"), className, objectName);
		dlg.SetAutoSearch(true);
		if (dlg.ShowModal() != wxID_OK)
		{
			return;
		}

		BulkImportAction& op = Actions.emplace_back();
		dlg.ConfigureOperation(op);
		OperationsList->AssociateModel(new BulkImportModel(Actions));

		UpdateControls();
	}
}

BulkImportWindow::~BulkImportWindow()
{
	PathPicker->Disconnect(wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler(BulkImportWindow::OnPathChanged), NULL, this);
	OperationsList->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING, wxDataViewEventHandler(BulkImportWindow::OnStartEditingOperation), NULL, this);
	OperationsList->Disconnect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(BulkImportWindow::OnOperationSelected), NULL, this);
	AddOperationButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnAddOperationClicked), NULL, this);
	EditOperationButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnEditOperationClicked), NULL, this);
	RemoveOperationButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnRemoveOperationClicked), NULL, this);
	ClearOperationsButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnClearOperationsClicked), NULL, this);
	ContinueButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnContinueClicked), NULL, this);
	CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(BulkImportWindow::OnCancelClicked), NULL, this);
	OperationsList->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(BulkImportWindow::OnOperationsListContextMenu), NULL, this);
	App::GetSharedApp()->ResetBulkImport();
}

void BulkImportWindow::OnFirstIdle(wxIdleEvent&)
{
	this->Disconnect(wxEVT_IDLE, wxIdleEventHandler(BulkImportWindow::OnFirstIdle));
	if (PathPicker->GetPath().size())
	{
		LoadBuffer();
		UpdateControls();
	}
	else
	{
	}
	if (FirstStartName.size() && FirstStartClass.size() && AddOperationButton->IsEnabled())
	{
		AddImportOperationDialog dlg(this, ObjectDumpBuffer, wxT("Add"), FirstStartClass, FirstStartName);
		dlg.SetAutoSearch(true);
		if (dlg.ShowModal() != wxID_OK)
		{
			return;
		}

		BulkImportAction& op = Actions.emplace_back();
		dlg.ConfigureOperation(op);
		OperationsList->AssociateModel(new BulkImportModel(Actions));

		UpdateControls();
	}
}

void BulkImportWindow::OnPathChanged(wxFileDirPickerEvent& event)
{
	if (PreviousStreamPath != PathPicker->GetPath())
	{
		PreviousStreamPath = PathPicker->GetPath();
		LoadBuffer();
		UpdateControls();
	}
}

void BulkImportWindow::OnStartEditingOperation(wxDataViewEvent& event)
{
}

void BulkImportWindow::OnOperationSelected(wxDataViewEvent& event)
{
	UpdateControls();
}

void BulkImportWindow::OnAddOperationClicked(wxCommandEvent& event)
{
	if (!BufferLoaded)
	{
		if (!LoadBuffer())
		{
			return;
		}
	}

	AddImportOperationDialog dlg(this, ObjectDumpBuffer);
	if (dlg.ShowModal() != wxID_OK)
	{
		return;
	}

	BulkImportAction& op = Actions.emplace_back();
	dlg.ConfigureOperation(op);
	OperationsList->AssociateModel(new BulkImportModel(Actions));

	UpdateControls();
}

void BulkImportWindow::OnEditOperationClicked(wxCommandEvent& event)
{
	if (!OperationsList->GetSelectedItemsCount())
	{
		return;
	}
	int idx = int(OperationsList->GetCurrentItem().GetID()) - 1;
	BulkImportAction& op = Actions[idx];
	AddImportOperationDialog dlg(this, ObjectDumpBuffer, op);
	if (dlg.ShowModal() != wxID_OK)
	{
		return;
	}
	dlg.ConfigureOperation(op);
	OperationsList->AssociateModel(new BulkImportModel(Actions));

	UpdateControls();
}

void BulkImportWindow::OnRemoveOperationClicked(wxCommandEvent& event)
{
	if (!OperationsList->GetSelectedItemsCount())
	{
		return;
	}
	int idx = int(OperationsList->GetCurrentItem().GetID()) - 1;
	Actions.erase(Actions.begin() + idx);
	OperationsList->AssociateModel(new BulkImportModel(Actions));
	UpdateControls();
}

void BulkImportWindow::OnClearOperationsClicked(wxCommandEvent& event)
{
	Actions.clear();
	OperationsList->AssociateModel(new BulkImportModel(Actions));
	UpdateControls();
}

void BulkImportWindow::OnContinueClicked(wxCommandEvent& event)
{
	wxDirDialog dlg(NULL, "Select a directory to extract packages to...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dlg.ShowModal() != wxID_OK || dlg.GetPath().empty())
	{
		return;
	}
	BulkImportOperation operation(Actions, dlg.GetPath());
	ProgressWindow progress(this, wxT("Please wait..."));
	progress.SetCanCancel(false);
	progress.SetCurrentProgress(-1);
	progress.SetActionText(wxT("Preparing packages"));

	bool result = false;
	std::thread([&progress, &operation, &result] {
		result = operation.Execute(progress);
		SendEvent(&progress, UPDATE_PROGRESS_FINISH);
	}).detach();

	progress.ShowModal();

	if (result)
	{
		auto icon = wxICON_INFORMATION;
		wxString msg = wxT("Done! Packages extracted sucessfully!");
		if (operation.HasErrors())
		{
			msg += wxT("\nSome operations have failed. See Errors.txt in the otput folder.");
			icon = wxICON_WARNING;

			auto failed = operation.GetErrors();
			std::ofstream s(std::filesystem::path(dlg.GetPath().ToStdWstring()) / "Errors.txt", std::ios::out | std::ios::binary);
			for (const auto& pair : failed)
			{
				s << pair.first << " - " << pair.second << '\n';
			}
		}
		wxMessageBox(msg, wxT("Finished"), icon);
	}
	else
	{
		wxMessageBox(wxT("Failed to execute operations. See Errors.txt in the otput folder."), wxT("Error"), wxICON_ERROR);
	}
}

void BulkImportWindow::OnCancelClicked(wxCommandEvent& event)
{
	Close(true);
}

void BulkImportWindow::OnOperationsListContextMenu(wxDataViewEvent& event)
{
	// Do nothing
}

void BulkImportWindow::UpdateControls()
{
	AddOperationButton->Enable(BufferLoaded);
	ClearOperationsButton->Enable(Actions.size());
	ContinueButton->Enable(Actions.size());
	if (Actions.size() && OperationsList->GetSelectedItemsCount())
	{
		RemoveOperationButton->Enable(true);
		EditOperationButton->Enable(true);
	}
	else
	{
		RemoveOperationButton->Enable(false);
		EditOperationButton->Enable(false);
	}
}

bool BulkImportWindow::LoadBuffer()
{
	ProgressWindow progress(this, wxT("Please, wait..."));
	progress.SetActionText(wxT("Loading object dump"));
	progress.SetCanCancel(false);
	progress.SetCurrentProgress(-1);

	std::ifstream s(PathPicker->GetPath().ToStdWstring());

	std::stringstream& buffer = ObjectDumpBuffer;
	bool err = false;
	std::thread([&buffer, &s, &progress, &err] {
		try
		{
			buffer = std::stringstream();
			buffer << s.rdbuf();
		}
		catch (...)
		{
			err = true;
		}
		SendEvent(&progress, UPDATE_PROGRESS_FINISH);
	}).detach();
	progress.ShowModal();

	if (err)
	{
		PathPicker->SetPath(wxEmptyString);
		UpdateControls();
		Actions.clear();
		BufferLoaded = false;
		return false;
	}
	UpdateControls();
	Actions.clear();
	BufferLoaded = true;
	FAppConfig& cfg = App::GetSharedApp()->GetConfig();
	cfg.CompositeDumpPath = PathPicker->GetPath().ToStdWstring();
	App::GetSharedApp()->SaveConfig();
	return true;
}

