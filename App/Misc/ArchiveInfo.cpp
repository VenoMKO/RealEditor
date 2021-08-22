#include "ArchiveInfo.h"
#include "../Windows/PackageWindow.h"

#include <wx/statline.h>
#include <wx/clipbrd.h>

#include <Tera/FPackage.h>

enum NameTableMenuId {
  CopyName = wxID_HIGHEST + 1
};

class NameDataModel : public wxDataViewVirtualListModel {
public:

  enum
  {
    Col_Idx = 0,
    Col_Name,
    Col_MAX
  };

  NameDataModel(const std::vector<FString>& names)
    : Data(names)
  {}

  unsigned int GetColumnCount() const override
  {
    return Col_MAX;
  }

  unsigned int GetCount() const override
  {
    return Data.size();
  }

  wxString GetColumnType(unsigned int col) const override
  {
    return wxDataViewCheckIconTextRenderer::GetDefaultType();
  }

  bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override
  {
    return false;
  }

  void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override
  {
    switch (col)
    {
    case Col_Idx:
      variant = wxString::Format(wxT("%d"), row);
      break;
    case Col_Name:
      variant = Data[row].WString();
      break;
    }
  }

  bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override
  {
    return false;
  }

private:
  std::vector<FString> Data;
};

class GenerationDataModel : public wxDataViewVirtualListModel {
public:

  enum
  {
    Col_Idx = 0,
    Col_Names,
    Col_Exports,
    Col_Network,
    Col_MAX
  };

  GenerationDataModel(const std::vector<FGeneration>& generations)
    : Data(generations)
  {}

  unsigned int GetColumnCount() const override
  {
    return Col_MAX;
  }

  unsigned int GetCount() const override
  {
    return Data.size();
  }

  wxString GetColumnType(unsigned int col) const override
  {
    return wxDataViewCheckIconTextRenderer::GetDefaultType();
  }

  bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override
  {
    return false;
  }

  void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override
  {
    switch (col)
    {
    case Col_Idx:
      variant = wxString::Format(wxT("%d"), row + 1);
      break;
    case Col_Names:
      variant = wxString::Format(wxT("%d"), Data[row].Names);
      break;
    case Col_Exports:
      variant = wxString::Format(wxT("%d"), Data[row].Exports);
      break;
    case Col_Network:
      variant = wxString::Format(wxT("%d"), Data[row].NetObjects);
      break;
    }
  }

  bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override
  {
    return false;
  }

private:
  std::vector<FGeneration> Data;
};

ArchiveInfoView::ArchiveInfoView(wxPanel* parent, PackageWindow* window, FPackage* package)
  : wxPanel(parent, wxID_ANY)
  , Package(package)
  , Window(window)
{
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);
  this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  wxBoxSizer* bSizer7;
  bSizer7 = new wxBoxSizer(wxVERTICAL);

  wxStaticLine* m_staticline5;
  m_staticline5 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer7->Add(m_staticline5, 0, wxEXPAND | wxALL, 1);

  wxBoxSizer* bSizer35;
  bSizer35 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel2;
  m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer13;
  bSizer13 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer15;
  bSizer15 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText40;
  m_staticText40 = new wxStaticText(m_panel2, wxID_ANY, wxT("File Ver.:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText40->Wrap(-1);
  m_staticText40->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer15->Add(m_staticText40, 0, wxALL, FromDIP(5));

  FileVersion = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, 0);
  FileVersion->Wrap(-1);
  bSizer15->Add(FileVersion, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));

  wxStaticText* m_staticText401;
  m_staticText401 = new wxStaticText(m_panel2, wxID_ANY, wxT("Licensee Ver.:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText401->Wrap(-1);
  m_staticText401->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer15->Add(m_staticText401, 0, wxALL, FromDIP(5));

  LicenseeVersion = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, 0);
  LicenseeVersion->Wrap(-1);
  bSizer15->Add(LicenseeVersion, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));

  wxStaticText* m_staticText42;
  m_staticText42 = new wxStaticText(m_panel2, wxID_ANY, wxT("Header:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText42->Wrap(-1);
  m_staticText42->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer15->Add(m_staticText42, 0, wxALL, FromDIP(5));

  HeaderSize = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, 0);
  HeaderSize->Wrap(-1);
  bSizer15->Add(HeaderSize, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));


  bSizer13->Add(bSizer15, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer151;
  bSizer151 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText4012;
  m_staticText4012 = new wxStaticText(m_panel2, wxID_ANY, wxT("Engine Ver.:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText4012->Wrap(-1);
  m_staticText4012->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer151->Add(m_staticText4012, 0, wxALL, FromDIP(5));

  EngineVersion = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, 0);
  EngineVersion->Wrap(-1);
  bSizer151->Add(EngineVersion, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));

  wxStaticText* m_staticText40111;
  m_staticText40111 = new wxStaticText(m_panel2, wxID_ANY, wxT("Cooker Ver.:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText40111->Wrap(-1);
  m_staticText40111->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer151->Add(m_staticText40111, 0, wxALL, FromDIP(5));

  ContentVersion = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, 0);
  ContentVersion->Wrap(-1);
  bSizer151->Add(ContentVersion, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));

  wxStaticText* m_staticText401111;
  m_staticText401111 = new wxStaticText(m_panel2, wxID_ANY, wxT("CRC:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText401111->Wrap(-1);
  m_staticText401111->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer151->Add(m_staticText401111, 0, wxALL, FromDIP(5));

  PackageSource = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, 0);
  PackageSource->Wrap(-1);
  bSizer151->Add(PackageSource, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));


  bSizer13->Add(bSizer151, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer20;
  bSizer20 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText80;
  m_staticText80 = new wxStaticText(m_panel2, wxID_ANY, wxT("GUID:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText80->Wrap(-1);
  m_staticText80->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer20->Add(m_staticText80, 0, wxALL, FromDIP(5));

  GUID = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, 0);
  GUID->Wrap(-1);
  bSizer20->Add(GUID, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));


  bSizer13->Add(bSizer20, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer19;
  bSizer19 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText78;
  m_staticText78 = new wxStaticText(m_panel2, wxID_ANY, wxT("Group:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText78->Wrap(-1);
  m_staticText78->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer19->Add(m_staticText78, 0, wxALL, FromDIP(5));

  FolderName = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
  FolderName->Wrap(-1);
  bSizer19->Add(FolderName, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));


  bSizer13->Add(bSizer19, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer191;
  bSizer191 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText781;
  m_staticText781 = new wxStaticText(m_panel2, wxID_ANY, wxT("Flags:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText781->Wrap(-1);
  m_staticText781->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer191->Add(m_staticText781, 0, wxALL, FromDIP(5));

  Flags = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
  Flags->Wrap(-1);
  bSizer191->Add(Flags, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));


  bSizer13->Add(bSizer191, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer201;
  bSizer201 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText801;
  m_staticText801 = new wxStaticText(m_panel2, wxID_ANY, wxT("Compression:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText801->Wrap(-1);
  m_staticText801->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer201->Add(m_staticText801, 0, wxALL, FromDIP(5));

  Compression = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, 0);
  Compression->Wrap(-1);
  bSizer201->Add(Compression, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));


  bSizer13->Add(bSizer201, 0, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer1911;
  bSizer1911 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText7811;
  m_staticText7811 = new wxStaticText(m_panel2, wxID_ANY, wxT("Additional Packages:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText7811->Wrap(-1);
  m_staticText7811->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer1911->Add(m_staticText7811, 0, wxALL, FromDIP(5));

  AdditionalPackages = new wxStaticText(m_panel2, wxID_ANY, wxT("-"), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
  AdditionalPackages->Wrap(-1);
  bSizer1911->Add(AdditionalPackages, 0, wxTOP | wxBOTTOM | wxRIGHT, FromDIP(5));


  bSizer13->Add(bSizer1911, 0, wxEXPAND, FromDIP(5));

  wxStaticText* m_staticText78111;
  m_staticText78111 = new wxStaticText(m_panel2, wxID_ANY, wxT("Generations:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText78111->Wrap(-1);
  m_staticText78111->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer13->Add(m_staticText78111, 0, wxALL, FromDIP(5));

  GenerationsTable = new wxDataViewCtrl(m_panel2, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(350, 100)), 0);
  bSizer13->Add(GenerationsTable, 0, wxALL, FromDIP(5));

  /*
  wxBoxSizer* bSizer131;
  bSizer131 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText27 = new wxStaticText(m_panel2, wxID_ANY, wxT("Find object by:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText27->Wrap(-1);
  m_staticText27->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer131->Add(m_staticText27, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  FindObjByName = new wxButton(m_panel2, wxID_ANY, wxT("Name"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer131->Add(FindObjByName, 0, wxALL, FromDIP(5));

  FindObjByIndex = new wxButton(m_panel2, wxID_ANY, wxT("Index"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer131->Add(FindObjByIndex, 0, wxALL, FromDIP(5));


  bSizer13->Add(bSizer131, 0, wxEXPAND, FromDIP(5));*/


  m_panel2->SetSizer(bSizer13);
  m_panel2->Layout();
  bSizer13->Fit(m_panel2);
  bSizer35->Add(m_panel2, 1, wxEXPAND | wxALL, FromDIP(5));

  wxBoxSizer* bSizer8;
  bSizer8 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText7;
  m_staticText7 = new wxStaticText(this, wxID_ANY, wxT("Names:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText7->Wrap(-1);
  m_staticText7->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer8->Add(m_staticText7, 0, wxALL, FromDIP(5));

  NamesTable = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(270, -1)), 0);
  bSizer8->Add(NamesTable, 1, wxALL, FromDIP(5));


  bSizer35->Add(bSizer8, 0, wxEXPAND, FromDIP(5));


  bSizer7->Add(bSizer35, 1, wxEXPAND, FromDIP(5));


  this->SetSizer(bSizer7);
  this->Layout();

  this->Centre(wxBOTH);

  // TODO: implement object search
  // FindObjByName->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ArchiveInfoView::OnFindObjectByName), NULL, this);
  // FindObjByIndex->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ArchiveInfoView::OnFindObjectByIndex), NULL, this);
  NamesTable->Connect(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(ArchiveInfoView::OnNameContextMenu), nullptr, this);

  NamesTable->AppendTextColumn(wxEmptyString, NameDataModel::Col_Idx, wxDATAVIEW_CELL_INERT, FromDIP(40), wxALIGN_CENTER_HORIZONTAL);
  NamesTable->AppendTextColumn(_("Name"), NameDataModel::Col_Name);

  GenerationsTable->AppendTextColumn(wxEmptyString, GenerationDataModel::Col_Idx, wxDATAVIEW_CELL_INERT, FromDIP(25), wxALIGN_CENTER_HORIZONTAL);
  GenerationsTable->AppendTextColumn(_("Names"), GenerationDataModel::Col_Names, wxDATAVIEW_CELL_INERT, FromDIP(95), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
  GenerationsTable->AppendTextColumn(_("Exports"), GenerationDataModel::Col_Exports, wxDATAVIEW_CELL_INERT, FromDIP(95), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
  GenerationsTable->AppendTextColumn(_("Net. Objects"), GenerationDataModel::Col_Network, wxDATAVIEW_CELL_INERT, FromDIP(95), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);

  UpdateInfo();
}

void ArchiveInfoView::UpdateInfo()
{
  if (!Package)
  {
    return;
  }
  FPackageSummary s = Package->GetSummary();
  FileVersion->SetLabelText(wxString::Format("%d", Package->GetFileVersion()));
  LicenseeVersion->SetLabelText(wxString::Format("%d", Package->GetLicenseeVersion()));
  HeaderSize->SetLabelText(wxString::Format("%d bytes", s.HeaderSize));
  EngineVersion->SetLabelText(wxString::Format("%d", s.EngineVersion));
  ContentVersion->SetLabelText(wxString::Format("%d", s.ContentVersion));
  PackageSource->SetLabelText(wxString::Format("%.08X", s.PackageSource));
  GUID->SetLabelText(s.Guid.FormattedString().UTF8());
  FolderName->SetLabelText(s.FolderName.WString());
  Flags->SetLabelText(PackageFlagsToString(s.OriginalPackageFlags).WString());
  Flags->SetToolTip(Flags->GetLabelText());
  if (s.OriginalCompressionFlags & COMPRESS_LZO)
  {
    Compression->SetLabelText(wxT("LZO"));
  }
  else if (s.OriginalCompressionFlags & COMPRESS_ZLIB)
  {
    Compression->SetLabelText(wxT("ZLIB"));
  }
  else if (s.OriginalCompressionFlags & COMPRESS_LZX)
  {
    Compression->SetLabelText(wxT("LZX"));
  }
  else
  {
    Compression->SetLabelText(wxT("None"));
  }
  if (s.AdditionalPackagesToCook.empty())
  {
    AdditionalPackages->SetLabelText("None");
  }
  else
  {
    FString result;
    for (const auto& pkg : s.AdditionalPackagesToCook)
    {
      result += pkg + ", ";
    }
    if (result.Size())
    {
      result = result.Substr(0, result.Size() - 2);
    }
    AdditionalPackages->SetLabelText(result.WString());
    AdditionalPackages->SetToolTip(result.WString());
  }
  wxDataViewModel* model = new GenerationDataModel(s.Generations);
  GenerationsTable->AssociateModel(model);
  model->DecRef();
  model = new NameDataModel(Package->GetNames());
  NamesTable->AssociateModel(model);
  model->DecRef();
}

void ArchiveInfoView::OnFindObjectByName(wxCommandEvent& event)
{

}

void ArchiveInfoView::OnFindObjectByIndex(wxCommandEvent& event)
{

}

void ArchiveInfoView::OnNameContextMenu(wxDataViewEvent& e)
{
  if (!e.GetItem().IsOk())
  {
    return;
  }

  int idx = (int)e.GetItem().GetID() - 1;
  if (idx < 0)
  {
    return;
  }

  FString name;
  try
  {
    Package->GetIndexedNameString(idx, name);
  }
  catch (...)
  {
    return;
  }

  wxMenu menu;
  menu.Append(NameTableMenuId::CopyName, wxT("Copy"));

  switch (GetPopupMenuSelectionFromUser(menu))
  {
  case NameTableMenuId::CopyName:
  {
    if (wxTheClipboard->Open())
    {
      wxTheClipboard->Clear();
      wxTheClipboard->SetData(new wxTextDataObject(name.WString()));
      wxTheClipboard->Flush();
      wxTheClipboard->Close();
    }
    break;
  }
  default:
    break;
  }
}
