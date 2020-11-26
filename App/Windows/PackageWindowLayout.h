#include "PackageWindow.h"
#pragma once

// Moved this to a separate file to make PackageWindow.cpp more clean

void PackageWindow::InitLayout()
{
	wxMenuBar* menuBar;
	menuBar = new wxMenuBar(0);
	menuBar->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	wxMenu* fileMenu;
	fileMenu = new wxMenu();
	/*
	wxMenuItem* m_menuItem3;
	m_menuItem3 = new wxMenuItem(fileMenu, ControlElementId::New, wxString(wxT("New...")) + wxT('\t') + wxT("Ctrl+N"), wxEmptyString, wxITEM_NORMAL);
	m_menuItem3->Enable(false);
	fileMenu->Append(m_menuItem3);*/

	wxMenuItem* m_menuItem68 = new wxMenuItem(fileMenu, ControlElementId::CreateMod, wxS("Create a mod..."), wxS("Create a mod from existing modded GPKs"));
	fileMenu->Append(m_menuItem68);

	fileMenu->AppendSeparator();

	wxMenuItem* m_menuItem4;
	m_menuItem4 = new wxMenuItem(fileMenu, ControlElementId::Open, wxString(wxT("Open...")) + wxT('\t') + wxT("Ctrl+O"), wxEmptyString, wxITEM_NORMAL);
	fileMenu->Append(m_menuItem4);

	wxMenuItem* m_menuItem5;
	m_menuItem5 = new wxMenuItem(fileMenu, ControlElementId::OpenComposite, wxString(wxT("Open composite...")), wxS("Open a composite package by its name"), wxITEM_NORMAL);
	fileMenu->Append(m_menuItem5);

	fileMenu->AppendSeparator();

	SaveMenu = new wxMenuItem(fileMenu, ControlElementId::Save, wxString(wxT("Save")) + wxT('\t') + wxT("Ctrl+S"), wxEmptyString, wxITEM_NORMAL);
	SaveMenu->Enable(false);
	fileMenu->Append(SaveMenu);

	SaveAsMenu = new wxMenuItem(fileMenu, ControlElementId::SaveAs, wxString(wxT("Save As...")) + wxT('\t') + wxT("Ctrl+Shift+S"), wxEmptyString, wxITEM_NORMAL);
	SaveAsMenu->Enable(false);
	fileMenu->Append(SaveAsMenu);

	fileMenu->AppendSeparator();

	wxMenuItem* m_menuItem7;
	m_menuItem7 = new wxMenuItem(fileMenu, ControlElementId::Close, wxString(wxT("Close")) + wxT('\t') + wxT("Ctrl+W"), wxEmptyString, wxITEM_NORMAL);
	fileMenu->Append(m_menuItem7);

	wxMenuItem* m_menuItem61;
	m_menuItem61 = new wxMenuItem(fileMenu, ControlElementId::Exit, wxString(wxT("Exit")) + wxT('\t') + wxT("Alt+F4"), wxEmptyString, wxITEM_NORMAL);
	fileMenu->Append(m_menuItem61);

	menuBar->Append(fileMenu, wxT("File"));

	wxMenu* m_menu4;
	m_menu4 = new wxMenu();
	/*
	wxMenuItem* m_menuItem63 = new wxMenuItem(m_menu4, ControlElementId::Import, wxString(wxT("Import")), wxEmptyString, wxITEM_NORMAL);
	m_menuItem63->Enable(false);
	m_menu4->Append(m_menuItem63);
	wxMenuItem* m_menuItem64 = new wxMenuItem(m_menu4, ControlElementId::Export, wxString(wxT("Export")), wxEmptyString, wxITEM_NORMAL);
	m_menuItem64->Enable(false);
	m_menu4->Append(m_menuItem64);

	m_menu4->AppendSeparator();*/

	wxMenuItem* m_menuItem65 = new wxMenuItem(m_menu4, ControlElementId::CompositePatch, wxString(wxT("Patch the composite map...")), wxS("Patch an entry in the CompositePackageMapper.dat file at your S1Game folder"), wxITEM_NORMAL);
	m_menu4->Append(m_menuItem65);

	m_menu4->AppendSeparator();

	wxMenuItem* m_menuItem66 = new wxMenuItem(m_menu4, ControlElementId::DecryptMapper, wxString(wxT("Decrypt a mapper file...")), wxS("Decrypt a *Mapper.dat file and save it as a text file at a location of your choice."), wxITEM_NORMAL);
	m_menu4->Append(m_menuItem66);
	wxMenuItem* m_menuItem67 = new wxMenuItem(m_menu4, ControlElementId::EncryptMapper, wxString(wxT("Encrypt a mapper file...")), wxS("Encrypt a text file and save it at a location of your choise"), wxITEM_NORMAL);
	m_menu4->Append(m_menuItem67);

	m_menu4->AppendSeparator();

	wxMenuItem* m_menuItem69 = new wxMenuItem(m_menu4, ControlElementId::DumpObjectsMap, wxString(wxT("Dump all composite objects")), wxS("Build a list of all objects stored in all composite packages"), wxITEM_NORMAL);
	m_menu4->Append(m_menuItem69);

	wxMenuItem* m_menuItem70 = new wxMenuItem(m_menu4, ControlElementId::BulkCompositeExtract, wxString(wxT("Bulk import...")), wxS("Bulk import to composite packages"), wxITEM_NORMAL);
	m_menu4->Append(m_menuItem70);

	menuBar->Append(m_menu4, wxT("Edit"));

	wxMenu* m_menu2;
	m_menu2 = new wxMenu();
	SettingsWindowMenu = new wxMenuItem(m_menu2, ControlElementId::SettingsWin, wxString(wxT("Settings")), wxS("Show Real Editors settings window"), wxITEM_NORMAL);
	m_menu2->Append(SettingsWindowMenu);
	LogWindowMenu = new wxMenuItem(m_menu2, ControlElementId::LogWin, wxString(wxT("Log")), wxS("Toggle the Log window"), wxITEM_NORMAL);
	m_menu2->Append(LogWindowMenu);

	menuBar->Append(m_menu2, wxT("Window"));

	wxMenu* m_menu5;
	m_menu5 = new wxMenu();
	wxMenuItem* m_menuItem71 = new wxMenuItem(m_menu5, ControlElementId::Help, wxString(wxT("Guides, Tutorials, Help")), wxS("Open a Wiki page containing various helpful information"), wxITEM_NORMAL);
	m_menu5->Append(m_menuItem71);

	menuBar->Append(m_menu5, wxT("Help"));
#if _DEBUG
	wxMenu* m_menu3;
	m_menu3 = new wxMenu();
	_DebugTestCookObject = new wxMenuItem(m_menu2, ControlElementId::DebugTestCookObj, wxString(wxT("Cook an object...")), wxEmptyString, wxITEM_NORMAL);
	m_menu3->Append(_DebugTestCookObject);

	menuBar->Append(m_menu3, wxT("Debug"));
#endif

	this->SetMenuBar(menuBar);

	wxBoxSizer* topSizer;
	topSizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* topPanel;
	topPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	topPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer(wxVERTICAL);

	SidebarSplitter = new wxSplitterWindow(topPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
	SidebarSplitter->Connect(wxEVT_IDLE, wxIdleEventHandler(PackageWindow::SidebarSplitterOnIdle), NULL, this);
	SidebarSplitter->SetMinimumPaneSize(230);

	SidebarSplitter->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

	wxPanel* sidebarPanel;
	sidebarPanel = new wxPanel(SidebarSplitter, wxID_ANY, wxDefaultPosition, wxSize(150, -1), wxTAB_TRAVERSAL);
	sidebarPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

	wxBoxSizer* treeSizer;
	treeSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* stObjects;
	stObjects = new wxStaticText(sidebarPanel, wxID_ANY, wxT("Objects:"), wxDefaultPosition, wxSize(-1, -1), 0);
	stObjects->Wrap(-1);
	treeSizer->Add(stObjects, 0, wxALL, 3);

	ObjectTreeCtrl = new ObjectTreeDataViewCtrl(sidebarPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER | wxDV_SINGLE);
	ObjectTreeCtrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	ObjectTreeCtrl->SetMinSize(wxSize(230, 600));

	treeSizer->Add(ObjectTreeCtrl, 1, wxALL | wxEXPAND, 4);


	sidebarPanel->SetSizer(treeSizer);
	sidebarPanel->Layout();
	wxPanel* contentSplitterPanel;
	contentSplitterPanel = new wxPanel(SidebarSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	contentSplitterPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
	contentSplitterPanel->SetMinSize(wxSize(740, -1));

	wxBoxSizer* contentSizer;
	contentSizer = new wxBoxSizer(wxHORIZONTAL);

	ContentSplitter = new wxSplitterWindow(contentSplitterPanel, ControlElementId::ContentSplitter, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_NOBORDER | wxSP_THIN_SASH);
	ContentSplitter->SetSashGravity(1);
	ContentSplitter->SetMinimumPaneSize(230);

	MainPanel = new wxPanel(ContentSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	MainPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
	MainPanel->SetMinSize(wxSize(510, -1));

	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer(wxVERTICAL);

	wxPanel* m_panel8;
	m_panel8 = new wxPanel(MainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer(wxHORIZONTAL);

	/*
	* TODO: Create selection history stack
	BackButton = new wxButton(m_panel8, ControlElementId::Back, wxT("<"), wxDefaultPosition, wxSize(30, -1), 0);
	BackButton->Enable(false);

	bSizer16->Add(BackButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

	ForwardButton = new wxButton(m_panel8, ControlElementId::Forward, wxT(">"), wxDefaultPosition, wxSize(30, -1), 0);
	ForwardButton->Enable(false);

	bSizer16->Add(ForwardButton, 0, wxALL, 0);*/

	ObjectTitleLabel = new wxStaticText(m_panel8, wxID_ANY, wxT("[0] Object(Class)"), wxDefaultPosition, wxSize(500, -1), 0);
	ObjectTitleLabel->Wrap(-1);
	ObjectTitleLabel->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer16->Add(ObjectTitleLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	m_panel8->SetSizer(bSizer16);
	m_panel8->Layout();
	bSizer16->Fit(m_panel8);
	bSizer27->Add(m_panel8, 0, wxEXPAND | wxALL, 0);

	ObjectInfoPanel = new wxPanel(MainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer(wxVERTICAL);

	wxStaticLine* m_staticline3;
	m_staticline3 = new wxStaticLine(ObjectInfoPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer13->Add(m_staticline3, 0, wxEXPAND | wxALL, 1);

	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* stObjFlags;
	stObjFlags = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Object flags:"), wxDefaultPosition, wxSize(67, -1), 0);
	stObjFlags->Wrap(-1);
	bSizer31->Add(stObjFlags, 0, wxALIGN_CENTER | wxALL, 5);

	ObjectFlagsTextfield = new wxTextCtrl(ObjectInfoPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	ObjectFlagsTextfield->Enable(false);

	bSizer31->Add(ObjectFlagsTextfield, 1, wxALL, 1);


	bSizer13->Add(bSizer31, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer47;
	bSizer47 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* stOffset;
	stOffset = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Offset:"), wxDefaultPosition, wxDefaultSize, 0);
	stOffset->Wrap(-1);
	bSizer38->Add(stOffset, 0, wxALIGN_RIGHT | wxALL, 3);

	wxStaticText* stSize;
	stSize = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Size:"), wxDefaultPosition, wxDefaultSize, 0);
	stSize->Wrap(-1);
	bSizer38->Add(stSize, 0, wxALIGN_RIGHT | wxALL, 3);


	bSizer47->Add(bSizer38, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer381;
	bSizer381 = new wxBoxSizer(wxVERTICAL);

	ObjectOffsetLabel = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("0xFFFFFFFF"), wxDefaultPosition, wxSize(70, -1), 0);
	ObjectOffsetLabel->Wrap(-1);
	bSizer381->Add(ObjectOffsetLabel, 0, wxALIGN_LEFT | wxALL, 3);

	ObjectSizeLabel = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("0xFFFFFFFF"), wxDefaultPosition, wxSize(70, -1), 0);
	ObjectSizeLabel->Wrap(-1);
	bSizer381->Add(ObjectSizeLabel, 0, wxALIGN_LEFT | wxALL, 3);


	bSizer47->Add(bSizer381, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer382;
	bSizer382 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* stPropSize;
	stPropSize = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Prop. size:"), wxDefaultPosition, wxDefaultSize, 0);
	stPropSize->Wrap(-1);
	bSizer382->Add(stPropSize, 0, wxALIGN_RIGHT | wxALL, 3);

	wxStaticText* stDataSize;
	stDataSize = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Data size:"), wxDefaultPosition, wxDefaultSize, 0);
	stDataSize->Wrap(-1);
	bSizer382->Add(stDataSize, 0, wxALIGN_RIGHT | wxALL, 3);


	bSizer47->Add(bSizer382, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer3811;
	bSizer3811 = new wxBoxSizer(wxVERTICAL);

	ObjectPropertiesSizeLabel = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("0xFFFFFFFF"), wxDefaultPosition, wxSize(70, -1), 0);
	ObjectPropertiesSizeLabel->Wrap(-1);
	bSizer3811->Add(ObjectPropertiesSizeLabel, 0, wxALIGN_LEFT | wxALL, 3);

	ObjectDataSizeLabel = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("0xFFFFFFFF"), wxDefaultPosition, wxSize(70, -1), 0);
	ObjectDataSizeLabel->Wrap(-1);
	bSizer3811->Add(ObjectDataSizeLabel, 0, wxALIGN_LEFT | wxALL, 3);


	bSizer47->Add(bSizer3811, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer60;
	bSizer60 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* stExpFlags;
	stExpFlags = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Exp. flags:"), wxDefaultPosition, wxSize(55, -1), 0);
	stExpFlags->Wrap(-1);
	bSizer60->Add(stExpFlags, 0, wxALIGN_CENTER_VERTICAL | wxALL, 3);

	ExportFlagsTextfield = new wxTextCtrl(ObjectInfoPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	ExportFlagsTextfield->Enable(false);

	bSizer60->Add(ExportFlagsTextfield, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);


	bSizer47->Add(bSizer60, 1, 0, 5);


	bSizer13->Add(bSizer47, 0, wxEXPAND, 5);

	wxStaticLine* m_staticline4;
	m_staticline4 = new wxStaticLine(ObjectInfoPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer13->Add(m_staticline4, 0, wxEXPAND | wxALL, 1);


	ObjectInfoPanel->SetSizer(bSizer13);
	ObjectInfoPanel->Layout();
	bSizer13->Fit(ObjectInfoPanel);
	bSizer27->Add(ObjectInfoPanel, 0, wxALL | wxEXPAND, 0);

	Toolbar = new wxToolBar(MainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORZ_TEXT | wxBORDER_NONE | wxTB_NODIVIDER | wxTB_TEXT);
	Toolbar->SetMinSize(wxSize(-1, 32));
	Toolbar->Realize();

	bSizer27->Add(Toolbar, 0, wxEXPAND, 5);

	EditorContainer = new wxPanel(MainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	EditorContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer(wxVERTICAL);


	EditorContainer->SetSizer(bSizer17);
	EditorContainer->Layout();
	bSizer17->Fit(EditorContainer);
	bSizer27->Add(EditorContainer, 1, wxEXPAND | wxALL, 0);


	MainPanel->SetSizer(bSizer27);
	MainPanel->Layout();
	bSizer27->Fit(MainPanel);
	PropertiesPanel = new wxPanel(ContentSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* propertiesSizer;
	propertiesSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* stProperties;
	stProperties = new wxStaticText(PropertiesPanel, wxID_ANY, wxT("Properties:"), wxDefaultPosition, wxDefaultSize, 0);
	stProperties->Wrap(-1);
	propertiesSizer->Add(stProperties, 0, wxALL, 3);

	PropertiesCtrl = new wxPropertyGridManager(PropertiesPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPGMAN_DEFAULT_STYLE | wxPG_DESCRIPTION | wxPG_SPLITTER_AUTO_CENTER | wxPG_TOOLTIPS);
	PropertiesCtrl->SetExtraStyle(wxPG_EX_ENABLE_TLP_TRACKING | wxPG_EX_HELP_AS_TOOLTIPS | wxPG_EX_MODE_BUTTONS | wxPG_EX_NATIVE_DOUBLE_BUFFERING);
	PropertiesCtrl->SetMinSize(wxSize(230, 600));

	propertiesSizer->Add(PropertiesCtrl, 1, wxALL | wxEXPAND, 4);


	PropertiesPanel->SetSizer(propertiesSizer);
	PropertiesPanel->Layout();
	propertiesSizer->Fit(PropertiesPanel);
	ContentSplitter->SplitVertically(MainPanel, PropertiesPanel, -1);
	contentSizer->Add(ContentSplitter, 1, wxEXPAND, 0);


	contentSplitterPanel->SetSizer(contentSizer);
	contentSplitterPanel->Layout();
	contentSizer->Fit(contentSplitterPanel);
	SidebarSplitter->SplitVertically(sidebarPanel, contentSplitterPanel, 240);
	bSizer14->Add(SidebarSplitter, 1, wxEXPAND, 0);


	topPanel->SetSizer(bSizer14);
	topPanel->Layout();
	bSizer14->Fit(topPanel);
	topSizer->Add(topPanel, 1, wxEXPAND | wxALL, 0);
	SetSizer(topSizer);

	// Reduce flickering on resizing
	std::vector<wxWindow*> flickeringWindows = { menuBar, stExpFlags , stObjFlags, stOffset, stSize, stPropSize, stDataSize, stProperties, EditorContainer, contentSplitterPanel, sidebarPanel,
		PropertiesPanel, MainPanel, Toolbar, ObjectOffsetLabel, ObjectSizeLabel, ObjectTitleLabel, ObjectPropertiesSizeLabel, ObjectDataSizeLabel, SidebarSplitter, ContentSplitter, PropertiesCtrl };
	for (wxWindow* window : flickeringWindows)
	{
		window->SetDoubleBuffered(true);
	}
	SetDoubleBuffered(true);
	Centre(wxBOTH);
	Layout();

	// TODO: add progress UI
	//StatusBar = CreateStatusBar(1, wxSTB_SIZEGRIP, wxID_ANY);
}

void PackageWindow::SetPropertiesHidden(bool hidden)
{
	if (PropertiesHidden == hidden)
	{
		return;
	}
	PropertiesHidden = hidden;
	if (PropertiesHidden)
	{
		PropertiesPos = ContentSplitter->GetSize().x - ContentSplitter->GetSashPosition();
		ContentSplitter->Unsplit(PropertiesPanel);
	}
	else
	{
		MainPanel->Show(true);
		PropertiesPanel->Show(true);
		ContentSplitter->SplitVertically(MainPanel, PropertiesPanel, ContentSplitter->GetSize().x - PropertiesPos);
	}
}

void PackageWindow::SetContentHidden(bool hidden)
{
	if (ContentHidden == hidden)
	{
		return;
	}
	ContentHidden = hidden;
	ObjectInfoPanel->Show(!hidden);
	Toolbar->Show(!hidden);
	MainPanel->Layout();
}

void PackageWindow::ShowEditor(GenericEditor* editor)
{
	if (editor)
	{
		EditorContainer->Show();
		if (PackageInfoView)
    {
      PackageInfoView->Show(false);
		}
	}
	if (ActiveEditor && ActiveEditor != editor)
	{
		Toolbar->Unbind(wxEVT_TOOL, &GenericEditor::OnToolBarEvent, ActiveEditor);
	}
	bool shown = false;
	for (std::pair<const INT, GenericEditor*>& item : Editors)
	{
		if (editor == item.second)
		{
			item.second->Show(true);
			shown = true;
		}
		else
		{
			item.second->Show(false);
		}
	}
	if (!shown && editor && editor->GetObject())
	{
		Editors[editor->GetObject()->GetExportObject()->ObjectIndex] = editor;
		wxSizer* sizer = EditorContainer->GetSizer();
		sizer->Add(editor, 1, wxEXPAND | wxALL, 0);
		MainPanel->GetSizer()->Layout();
		editor->Show();
	}
	if ((ActiveEditor = editor))
	{
		Toolbar->Bind(wxEVT_TOOL, &GenericEditor::OnToolBarEvent, ActiveEditor);
	}
}