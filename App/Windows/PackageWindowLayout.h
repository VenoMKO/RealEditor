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

#if IS_TERA_BUILD
  bool IsModernClient = FPackage::GetCoreVersion() == VER_TERA_MODERN;
  wxMenuItem* m_menuItem68 = new wxMenuItem(fileMenu, ControlElementId::CreateMod, wxS("Create mod..."), wxS("Create a mod from existing modded GPKs"));
  m_menuItem68->Enable(IsModernClient);
  fileMenu->Append(m_menuItem68);

  fileMenu->AppendSeparator();

  wxMenuItem* m_menuItem3 = new wxMenuItem(fileMenu, ControlElementId::New, wxString(wxT("New GPK file...")) + wxT('\t') + wxT("Ctrl+N"), wxEmptyString, wxITEM_NORMAL);
  fileMenu->Append(m_menuItem3);
#endif
  wxMenu* openMenu = new wxMenu;
  wxMenuItem* m_menuItem4 = new wxMenuItem(openMenu, ControlElementId::Open, wxString(IS_TERA_BUILD ? wxT("GPK file...") : wxT("UPK file...")) + wxT('\t') + wxT("Ctrl+O"), wxEmptyString, wxITEM_NORMAL);
  openMenu->Append(m_menuItem4);
  wxMenuItem* m_menuItem41;
  m_menuItem41 = new wxMenuItem(openMenu, ControlElementId::OpenByName, wxString(wxT("By name...")) + wxT('\t') + wxT("Ctrl+Shift+O"), wxEmptyString, wxITEM_NORMAL);
  openMenu->Append(m_menuItem41);
#if IS_TERA_BUILD
  wxMenuItem* m_menuItem5;
  m_menuItem5 = new wxMenuItem(openMenu, ControlElementId::OpenComposite, wxString(wxT("Composite package...")), wxS("Open a composite package by its name"), wxITEM_NORMAL);
  m_menuItem5->Enable(IsModernClient);
  openMenu->Append(m_menuItem5);
#endif
  fileMenu->AppendSubMenu(openMenu, "Open");

  wxMenu* menuRecent = new wxMenu;
  fileMenu->AppendSubMenu(menuRecent, "Open recent");

  fileMenu->AppendSeparator();

  SaveMenu = new wxMenuItem(fileMenu, ControlElementId::Save, wxString(wxT("Save")) + wxT('\t') + wxT("Ctrl+S"), wxEmptyString, wxITEM_NORMAL);
  SaveMenu->Enable(false);
  fileMenu->Append(SaveMenu);

  SaveAsMenu = new wxMenuItem(fileMenu, ControlElementId::SaveAs, wxString(wxT("Save As...")) + wxT('\t') + wxT("Ctrl+Shift+S"), wxEmptyString, wxITEM_NORMAL);
  SaveAsMenu->Enable(false);
  fileMenu->Append(SaveAsMenu);

  fileMenu->AppendSeparator();

  wxMenuItem* m_menuItem51;
  m_menuItem51 = new wxMenuItem(fileMenu, ControlElementId::ShowInExplorer, wxString(wxT("Show in Explorer")), wxEmptyString, wxITEM_NORMAL);
  fileMenu->Append(m_menuItem51);

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

#if IS_TERA_BUILD
  wxMenuItem* dcMenu = new wxMenuItem(m_menu4, ControlElementId::DcTool, wxString(wxT("DataCenter tool...")), wxS("Unpack and export datacenter files"), wxITEM_NORMAL);
  m_menu4->Append(dcMenu);

  wxMenu* mapperTools = new wxMenu();

  wxMenuItem* m_menuItem66 = new wxMenuItem(mapperTools, ControlElementId::DecryptMapper, wxString(wxT("Decrypt a mapper file...")), wxS("Decrypt a *Mapper.dat file and save it as a text file at a location of your choice."), wxITEM_NORMAL);
  m_menuItem66->Enable(IsModernClient);
  mapperTools->Append(m_menuItem66);
  wxMenuItem* m_menuItem67 = new wxMenuItem(mapperTools, ControlElementId::EncryptMapper, wxString(wxT("Encrypt a mapper file...")), wxS("Encrypt a text file and save it at a location of your choise"), wxITEM_NORMAL);
  m_menuItem67->Enable(IsModernClient);
  mapperTools->Append(m_menuItem67);

  m_menu4->AppendSubMenu(mapperTools, wxT("Mapper tools"));
  m_menu4->AppendSeparator();

  wxMenuItem* m_menuItem69 = new wxMenuItem(m_menu4, ControlElementId::DumpObjectsMap, wxString(wxT("Dump all composite objects")), wxS("Build a list of all objects stored in all composite packages"), wxITEM_NORMAL);
  m_menuItem69->Enable(IsModernClient);
  m_menu4->Append(m_menuItem69);

  wxMenuItem* m_menuItem70 = new wxMenuItem(m_menu4, ControlElementId::BulkCompositeExtract, wxString(wxT("Bulk import...")), wxS("Bulk import to composite packages"), wxITEM_NORMAL);
  m_menuItem70->Enable(IsModernClient);
  m_menu4->Append(m_menuItem70);

  m_menu4->AppendSeparator();

#endif

  EditFlagsMenu = new wxMenuItem(m_menu4, ControlElementId::EditPkgFlags, wxString(wxT("Package flags...")), wxEmptyString, wxITEM_NORMAL);
  m_menu4->Append(EditFlagsMenu);

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
  _DebugTestCookObject = new wxMenuItem(m_menu3, ControlElementId::DebugTestCookObj, wxString(wxT("Cook an object...")), wxEmptyString, wxITEM_NORMAL);
  m_menu3->Append(_DebugTestCookObject);

  _DebugSplitMod = new wxMenuItem(m_menu3, ControlElementId::DebugSplitMod, wxString(wxT("Split Mod...")), wxEmptyString, wxITEM_NORMAL);
  m_menu3->Append(_DebugSplitMod);

  wxMenuItem* debugDup = new wxMenuItem(m_menu3, ControlElementId::DebugDup, wxT("Dup selection..."), wxEmptyString, wxITEM_NORMAL);
  m_menu3->Append(debugDup);

  wxMenuItem* debugDirty = new wxMenuItem(m_menu3, ControlElementId::DebugDirty, wxT("Mark Dirty"), wxEmptyString, wxITEM_NORMAL);
  m_menu3->Append(debugDirty);

  wxMenuItem* debugIter = new wxMenuItem(m_menu3, ControlElementId::DebugIter, wxT("Iterate GPKs"), wxEmptyString, wxITEM_NORMAL);
  m_menu3->Append(debugIter);

  wxMenuItem* debugSelect = new wxMenuItem(m_menu3, ControlElementId::DebugSelect, wxT("Select object..."), wxEmptyString, wxITEM_NORMAL);
  m_menu3->Append(debugSelect);

  menuBar->Append(m_menu3, wxT("Debug"));

  
#endif

  this->SetMenuBar(menuBar);

  wxBoxSizer* topSizer;
  topSizer = new wxBoxSizer(wxVERTICAL);

  TopPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  TopPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  wxBoxSizer* bSizer14;
  bSizer14 = new wxBoxSizer(wxVERTICAL);

  SidebarSplitter = new wxSplitterWindow(TopPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
  SidebarSplitter->Connect(wxEVT_IDLE, wxIdleEventHandler(PackageWindow::SidebarSplitterOnIdle), NULL, this);
  SidebarSplitter->SetMinimumPaneSize(230);

  SidebarSplitter->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

  wxPanel* sidebarPanel;
  sidebarPanel = new wxPanel(SidebarSplitter, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(150, -1)), wxTAB_TRAVERSAL);
  sidebarPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  wxBoxSizer* treeSizer;
  treeSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticText* stObjects;
  stObjects = new wxStaticText(sidebarPanel, wxID_ANY, wxT("Objects:"), wxDefaultPosition, wxSize(-1, -1), 0);
  stObjects->Wrap(-1);
  treeSizer->Add(stObjects, 0, wxALL, 3);

  ObjectTreeCtrl = new ObjectTreeDataViewCtrl(sidebarPanel, ControlElementId::ObjTreeCtrl, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER | wxDV_SINGLE);
  ObjectTreeCtrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
  ObjectTreeCtrl->SetMinSize(FromDIP(wxSize(230, 600)));

  treeSizer->Add(ObjectTreeCtrl, 1, wxALL | wxEXPAND, 4);

  SearchField = new wxSearchCtrl(sidebarPanel, ControlElementId::Search, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  SearchField->SetHint(wxT("Search(Ctrl+F)..."));
  SearchField->Enable(false);
  SearchField->ShowCancelButton(true);
  treeSizer->Add(SearchField, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));

  sidebarPanel->SetSizer(treeSizer);
  sidebarPanel->Layout();
  wxPanel* contentSplitterPanel;
  contentSplitterPanel = new wxPanel(SidebarSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  contentSplitterPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  contentSplitterPanel->SetMinSize(FromDIP(wxSize(740, -1)));

  wxBoxSizer* contentSizer;
  contentSizer = new wxBoxSizer(wxHORIZONTAL);

  ContentSplitter = new wxSplitterWindow(contentSplitterPanel, ControlElementId::ContentSplitter, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_NOBORDER | wxSP_THIN_SASH);
  ContentSplitter->SetSashGravity(1);
  ContentSplitter->SetMinimumPaneSize(230);

  MainPanel = new wxPanel(ContentSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  MainPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  MainPanel->SetMinSize(FromDIP(wxSize(510, -1)));

  wxBoxSizer* bSizer27;
  bSizer27 = new wxBoxSizer(wxVERTICAL);

  wxPanel* m_panel8;
  m_panel8 = new wxPanel(MainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer16;
  bSizer16 = new wxBoxSizer(wxHORIZONTAL);

  BackButton = new wxButton(m_panel8, ControlElementId::Back, wxT("<"), wxDefaultPosition, FromDIP(wxSize(25, -1)), 0);
  BackButton->SetToolTip(wxT("Navigate backward"));
  BackButton->Enable(false);

  bSizer16->Add(BackButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

  ForwardButton = new wxButton(m_panel8, ControlElementId::Forward, wxT(">"), wxDefaultPosition, FromDIP(wxSize(25, -1)), 0);
  BackButton->SetToolTip(wxT("Navigate forward"));
  ForwardButton->Enable(false);

  bSizer16->Add(ForwardButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

  ObjectTitleLabel = new wxStaticText(m_panel8, wxID_ANY, wxT("[0] Object(Class)"), wxDefaultPosition, FromDIP(wxSize(500, -1)), 0);
  ObjectTitleLabel->Wrap(-1);
  ObjectTitleLabel->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer16->Add(ObjectTitleLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, FromDIP(5));


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
  stObjFlags = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Object flags:"), wxDefaultPosition, FromDIP(wxSize(67, -1)), 0);
  stObjFlags->Wrap(-1);
  bSizer31->Add(stObjFlags, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  ObjectFlagsTextfield = new wxTextCtrl(ObjectInfoPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  ObjectFlagsTextfield->Enable(false);

  bSizer31->Add(ObjectFlagsTextfield, 1, wxALL, 1);

  EditObjectFlagsButton = new wxButton(ObjectInfoPanel, ControlElementId::EditObjFlags, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0);
  EditObjectFlagsButton->SetToolTip(wxT("Edit object flags"));
  bSizer31->Add(EditObjectFlagsButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);


  bSizer13->Add(bSizer31, 0, wxEXPAND, FromDIP(5));

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


  bSizer47->Add(bSizer38, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer381;
  bSizer381 = new wxBoxSizer(wxVERTICAL);

  ObjectOffsetLabel = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("0xFFFFFFFF"), wxDefaultPosition, FromDIP(wxSize(70, -1)), 0);
  ObjectOffsetLabel->Wrap(-1);
  bSizer381->Add(ObjectOffsetLabel, 0, wxALIGN_LEFT | wxALL, 3);

  ObjectSizeLabel = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("0xFFFFFFFF"), wxDefaultPosition, FromDIP(wxSize(70, -1)), 0);
  ObjectSizeLabel->Wrap(-1);
  bSizer381->Add(ObjectSizeLabel, 0, wxALIGN_LEFT | wxALL, 3);


  bSizer47->Add(bSizer381, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer382;
  bSizer382 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* stPropSize;
  stPropSize = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Prop. size:"), wxDefaultPosition, wxDefaultSize, 0);
  stPropSize->SetToolTip(wxT("Properties size"));
  stPropSize->Wrap(-1);
  bSizer382->Add(stPropSize, 0, wxALIGN_RIGHT | wxALL, 3);

  wxStaticText* stDataSize;
  stDataSize = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Data size:"), wxDefaultPosition, wxDefaultSize, 0);
  stDataSize->Wrap(-1);
  bSizer382->Add(stDataSize, 0, wxALIGN_RIGHT | wxALL, 3);


  bSizer47->Add(bSizer382, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer3811;
  bSizer3811 = new wxBoxSizer(wxVERTICAL);

  ObjectPropertiesSizeLabel = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("0xFFFFFFFF"), wxDefaultPosition, FromDIP(wxSize(70, -1)), 0);
  ObjectPropertiesSizeLabel->Wrap(-1);
  bSizer3811->Add(ObjectPropertiesSizeLabel, 0, wxALIGN_LEFT | wxALL, 3);

  ObjectDataSizeLabel = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("0xFFFFFFFF"), wxDefaultPosition, FromDIP(wxSize(70, -1)), 0);
  ObjectDataSizeLabel->Wrap(-1);
  bSizer3811->Add(ObjectDataSizeLabel, 0, wxALIGN_LEFT | wxALL, 3);


  bSizer47->Add(bSizer3811, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer60;
  bSizer60 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* stExpFlags;
  stExpFlags = new wxStaticText(ObjectInfoPanel, wxID_ANY, wxT("Exp. flags:"), wxDefaultPosition, FromDIP(wxSize(55, -1)), 0);
  stExpFlags->SetToolTip(wxT("Export flags"));
  stExpFlags->Wrap(-1);
  bSizer60->Add(stExpFlags, 0, wxALIGN_CENTER_VERTICAL | wxALL, 3);

  ExportFlagsTextfield = new wxTextCtrl(ObjectInfoPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  ExportFlagsTextfield->Enable(false);

  bSizer60->Add(ExportFlagsTextfield, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);

  EditExportFlagsButton = new wxButton(ObjectInfoPanel, ControlElementId::EditExpFlags, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0);
  EditExportFlagsButton->SetToolTip(wxT("Edit export flags"));
  bSizer60->Add(EditExportFlagsButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

  bSizer47->Add(bSizer60, 1, 0, FromDIP(5));


  bSizer13->Add(bSizer47, 0, wxEXPAND, FromDIP(5));


  ObjectInfoPanel->SetSizer(bSizer13);
  ObjectInfoPanel->Layout();
  bSizer13->Fit(ObjectInfoPanel);
  bSizer27->Add(ObjectInfoPanel, 0, wxALL | wxEXPAND, 0);

  Toolbar = new wxToolBar(MainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORZ_TEXT);
  Toolbar->Realize();

  bSizer27->Add(Toolbar, 0, wxEXPAND, FromDIP(5));

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
  PropertiesCtrl->SetMinSize(FromDIP(wxSize(280, 600)));

  propertiesSizer->Add(PropertiesCtrl, 1, wxALL | wxEXPAND, 4);


  PropertiesPanel->SetSizer(propertiesSizer);
  PropertiesPanel->Layout();
  propertiesSizer->Fit(PropertiesPanel);
  ContentSplitter->SplitVertically(MainPanel, PropertiesPanel, -1);
  contentSizer->Add(ContentSplitter, 1, wxEXPAND, 0);


  contentSplitterPanel->SetSizer(contentSizer);
  contentSplitterPanel->Layout();
  contentSizer->Fit(contentSplitterPanel);
  SidebarSplitter->SplitVertically(sidebarPanel, contentSplitterPanel, FromDIP(240));
  bSizer14->Add(SidebarSplitter, 1, wxEXPAND, 0);


  TopPanel->SetSizer(bSizer14);
  TopPanel->Layout();
  bSizer14->Fit(TopPanel);
  topSizer->Add(TopPanel, 1, wxEXPAND | wxALL, 0);
  SetSizer(topSizer);

  SearchField->Connect(wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler(PackageWindow::OnSearchCancelClicked), nullptr, this);

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

  auto& config = App::GetSharedApp()->GetConfig();
  FileHistory = new wxFileHistory(config.MaxLastFilePackages, PackageWindow::GetOpenRecentId());
  FileHistory->UseMenu(menuRecent);
  auto it = config.LastFilePackages.rbegin();
  while (it != config.LastFilePackages.rend())
  {
    FileHistory->AddFileToHistory((*it).WString());
    it++;
  }
  menuRecent->Bind(wxEVT_COMMAND_MENU_SELECTED, &PackageWindow::OnRecentClicked, this);
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
    ActiveEditor->ClearToolbar();
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