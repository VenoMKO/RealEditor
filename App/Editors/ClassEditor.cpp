#include "ClassEditor.h"

#include <Tera/UProperty.h>

#include <wx/notebook.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>

ClassEditor::ClassEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  Notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
  wxPanel* panel1;
  panel1 = new wxPanel(Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  panel1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));

  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxVERTICAL);

  wxPanel* m_panel6;
  m_panel6 = new wxPanel(panel1, wxID_ANY, wxDefaultPosition, wxSize(-1, 1), wxTAB_TRAVERSAL);
  bSizer11->Add(m_panel6, 0, wxALL | wxEXPAND, FromDIP(5));

  wxFlexGridSizer* fgSizer1;
  fgSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer1->SetFlexibleDirection(wxHORIZONTAL);
  fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  wxStaticText* m_staticText4;
  m_staticText4 = new wxStaticText(panel1, wxID_ANY, wxT("Super:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText4->Wrap(-1);
  m_staticText4->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  fgSizer1->Add(m_staticText4, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, FromDIP(5));

  SuperField = new wxStaticText(panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  SuperField->Wrap(-1);
  fgSizer1->Add(SuperField, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));

  wxStaticText* m_staticText5;
  m_staticText5 = new wxStaticText(panel1, wxID_ANY, wxT("Flags:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText5->Wrap(-1);
  m_staticText5->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  fgSizer1->Add(m_staticText5, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, FromDIP(5));

  FlagsField = new wxStaticText(panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
  FlagsField->Wrap(-1);
  fgSizer1->Add(FlagsField, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));

  wxStaticText* m_staticText6;
  m_staticText6 = new wxStaticText(panel1, wxID_ANY, wxT("Header:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText6->Wrap(-1);
  m_staticText6->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  fgSizer1->Add(m_staticText6, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  HeaderField = new wxStaticText(panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  HeaderField->Wrap(-1);
  fgSizer1->Add(HeaderField, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));

  wxStaticText* m_staticText61;
  m_staticText61 = new wxStaticText(panel1, wxID_ANY, wxT("DLL:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText61->Wrap(-1);
  m_staticText61->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  fgSizer1->Add(m_staticText61, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  DLLField = new wxStaticText(panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  DLLField->Wrap(-1);
  fgSizer1->Add(DLLField, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));


  bSizer11->Add(fgSizer1, 0, wxEXPAND, FromDIP(5));

  wxPanel* m_panel61;
  m_panel61 = new wxPanel(panel1, wxID_ANY, wxDefaultPosition, wxSize(-1, 1), wxTAB_TRAVERSAL);
  bSizer11->Add(m_panel61, 0, wxALL | wxEXPAND, FromDIP(5));

  PropertiesList = new wxPropertyGridManager(panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPGMAN_DEFAULT_STYLE);
  PropertiesList->SetExtraStyle(wxPG_EX_MODE_BUTTONS | wxPG_EX_ENABLE_TLP_TRACKING | wxPG_EX_NATIVE_DOUBLE_BUFFERING);
  bSizer11->Add(PropertiesList, 1, wxALL | wxEXPAND, FromDIP(5));


  panel1->SetSizer(bSizer11);
  panel1->Layout();
  bSizer11->Fit(panel1);
  Notebook->AddPage(panel1, wxT("Information"), true);
  wxPanel* panel2;
  panel2 = new wxPanel(Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  panel2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxVERTICAL);

  ScriptView = new wxStyledTextCtrl(panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString);
  ScriptView->SetUseTabs(true);
  ScriptView->SetTabWidth(4);
  ScriptView->SetIndent(4);
  ScriptView->SetTabIndents(true);
  ScriptView->SetBackSpaceUnIndents(true);
  ScriptView->SetViewEOL(false);
  ScriptView->SetViewWhiteSpace(false);
  ScriptView->SetMarginWidth(2, 0);
  ScriptView->SetIndentationGuides(true);
  ScriptView->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
  ScriptView->SetMarginMask(1, wxSTC_MASK_FOLDERS);
  ScriptView->SetMarginWidth(1, 16);
  ScriptView->SetMarginSensitive(1, true);
  ScriptView->SetProperty(wxT("fold"), wxT("1"));
  ScriptView->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
  ScriptView->SetMarginType(0, wxSTC_MARGIN_NUMBER);
  ScriptView->SetMarginWidth(0, ScriptView->TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
  ScriptView->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS);
  ScriptView->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, wxColour(wxT("BLACK")));
  ScriptView->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, wxColour(wxT("WHITE")));
  ScriptView->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS);
  ScriptView->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("BLACK")));
  ScriptView->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("WHITE")));
  ScriptView->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
  ScriptView->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS);
  ScriptView->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("BLACK")));
  ScriptView->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("WHITE")));
  ScriptView->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS);
  ScriptView->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("BLACK")));
  ScriptView->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("WHITE")));
  ScriptView->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
  ScriptView->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
  ScriptView->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
  ScriptView->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
  bSizer10->Add(ScriptView, 1, wxEXPAND | wxALL, FromDIP(5));


  panel2->SetSizer(bSizer10);
  panel2->Layout();
  bSizer10->Fit(panel2);
  Notebook->AddPage(panel2, wxT("Script"), false);
  wxPanel* panel3;
  panel3 = new wxPanel(Notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  panel3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxVERTICAL);

  CppView = new wxStyledTextCtrl(panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString);
  CppView->SetUseTabs(true);
  CppView->SetTabWidth(4);
  CppView->SetIndent(4);
  CppView->SetTabIndents(true);
  CppView->SetBackSpaceUnIndents(true);
  CppView->SetViewEOL(false);
  CppView->SetViewWhiteSpace(false);
  CppView->SetMarginWidth(2, 0);
  CppView->SetIndentationGuides(true);
  CppView->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
  CppView->SetMarginMask(1, wxSTC_MASK_FOLDERS);
  CppView->SetMarginWidth(1, 16);
  CppView->SetMarginSensitive(1, true);
  CppView->SetProperty(wxT("fold"), wxT("1"));
  CppView->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
  CppView->SetMarginType(0, wxSTC_MARGIN_NUMBER);
  CppView->SetMarginWidth(0, CppView->TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
  CppView->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS);
  CppView->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, wxColour(wxT("BLACK")));
  CppView->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, wxColour(wxT("WHITE")));
  CppView->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS);
  CppView->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("BLACK")));
  CppView->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("WHITE")));
  CppView->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
  CppView->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS);
  CppView->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("BLACK")));
  CppView->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("WHITE")));
  CppView->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS);
  CppView->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("BLACK")));
  CppView->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("WHITE")));
  CppView->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
  CppView->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
  CppView->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
  CppView->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
  bSizer9->Add(CppView, 1, wxEXPAND | wxALL, FromDIP(5));


  panel3->SetSizer(bSizer9);
  panel3->Layout();
  bSizer9->Fit(panel3);
  Notebook->AddPage(panel3, wxT("C++"), false);

  bSizer1->Add(Notebook, 1, wxEXPAND | wxALL, FromDIP(5));


  this->SetSizer(bSizer1);
  this->Layout();

  CppView->SetReadOnly(true);
  CppView->SetLexer(wxSTC_LEX_CPP);
  ScriptView->SetReadOnly(true);
  ScriptView->SetLexer(wxSTC_LEX_CPP);
}

void ClassEditor::OnObjectLoaded()
{
  if (NeedsSetup)
  {
    NeedsSetup = false;
    UClass* cls = (UClass*)Object;
    SuperField->SetLabelText(cls->GetSuperClass() ? cls->GetSuperClass()->GetObjectPath().WString() : L"None");
    FlagsField->SetLabelText(ClassFlagsToString(cls->GetClassFlags()).WString());
    FlagsField->SetToolTip(FlagsField->GetLabelText());
    HeaderField->SetLabelText(cls->GetClassHeaderFilename().Size() ? cls->GetClassHeaderFilename().WString() : L"None");
    DLLField->SetLabelText(cls->GetDLLBindName().WString());

    auto applyStyle = [](wxStyledTextCtrl* ctrl) {
      ctrl->StyleSetForeground(wxSTC_C_STRING, wxColour(150, 0, 0));
      ctrl->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(165, 105, 0));
      ctrl->StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(40, 0, 60));
      ctrl->StyleSetForeground(wxSTC_C_NUMBER, wxColour(0, 0, 0));
      ctrl->StyleSetForeground(wxSTC_C_CHARACTER, wxColour(0, 0, 0));
      ctrl->StyleSetForeground(wxSTC_C_WORD, wxColour(0, 150, 0));
      ctrl->StyleSetForeground(wxSTC_C_WORD2, wxColour(0, 0, 150));
      ctrl->StyleSetForeground(wxSTC_C_COMMENT, wxColour(0, 150, 0));
      ctrl->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(0, 150, 0));
      ctrl->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(0, 150, 0));
      ctrl->StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORD, wxColour(0, 0, 200));
      ctrl->StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORDERROR, wxColour(0, 0, 200));
      ctrl->StyleSetBold(wxSTC_C_WORD, true);
      ctrl->StyleSetBold(wxSTC_C_WORD2, true);
      ctrl->StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, true);
    };

    if (UTextBuffer* buffer = cls->GetScriptText())
    {
      ScriptView->SetReadOnly(false);
      ScriptView->SetText(buffer->GetText().WString());
      applyStyle(ScriptView);
      ScriptView->SetReadOnly(true);
    }
    else
    {
      Notebook->RemovePage(Notebook->GetPageCount() - 2);
    }

    if (UTextBuffer* buffer = cls->GetCppText())
    {
      CppView->SetReadOnly(false);
      CppView->SetText(buffer->GetText().WString());
      applyStyle(CppView);
      CppView->SetReadOnly(true);
    }
    else
    {
      Notebook->RemovePage(Notebook->GetPageCount() - 1);
    }
    LoadClassProperties();
  }

  GenericEditor::OnObjectLoaded();
}

void ClassEditor::LoadClassProperties()
{
  PropertiesList->Freeze();
  wxPropertyCategory* root = new wxPropertyCategory(Object->GetObjectNameString().WString(), wxT("Root"));
  root->SetValue(wxT("Properties"));
  PropertiesList->Append(root);
  std::function<void(UProperty*, wxPropertyGridManager* ,wxPropertyCategory*, int32& idx)> iterator;
  iterator = [&iterator](UProperty* start, wxPropertyGridManager* mgr, wxPropertyCategory* root, int32& idx) {
    UProperty* property = start;
    for (; property; property = property->PropertyLinkNext, idx++)
    {
      if (property->GetID() == NAME_StructProperty)
      {
        UStructProperty* p = (UStructProperty*)property;
        if (p->Struct)
        {
          wxPropertyCategory* nroot = new wxPropertyCategory(p->GetObjectNameString().WString(), wxString::Format("%016llx%i", (uint64)std::addressof(*property), idx));
          nroot->SetValue(property->GetID().WString());
          nroot->SetExpanded(false);
          mgr->AppendIn(root, nroot);
          iterator(p->Struct->GetPropertyLink(), mgr, nroot, idx);
        }
      }
      else if (property->GetID() == NAME_ArrayProperty)
      {
        UArrayProperty* p = (UArrayProperty*)property;
        if (p->Inner)
        {
          wxPropertyCategory* nroot = new wxPropertyCategory(p->GetObjectNameString().WString(), wxString::Format("%016llx%i", (uint64)std::addressof(*property), idx));
          nroot->SetValue(property->GetID().WString());
          nroot->SetExpanded(false);
          mgr->AppendIn(root, nroot);
          iterator(p->Inner, mgr, nroot, idx);
        }
      }
      else
      {
        auto pgp = new wxStringProperty(property->GetObjectNameString().WString(), wxString::Format("%016llx%i", (uint64)std::addressof(*property), idx), property->GetID().WString());
        pgp->Enable(false);
        
        mgr->AppendIn(root, pgp);
      }
    }
  };

  int32 idx = 0;
  iterator(((UClass*)Object)->GetPropertyLink(), PropertiesList, root, idx);
  PropertiesList->Thaw();
}
