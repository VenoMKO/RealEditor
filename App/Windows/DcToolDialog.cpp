#include <cryptopp/cryptlib.h>
#include <cryptopp/aes.h>
#include <cryptopp/rijndael.h>
#include <cryptopp/modes.h>
#include <cryptopp/files.h>

#include "DcToolDialog.h"
#include <wx/statline.h>
#include <wx/zstream.h>
#include <wx/mstream.h>

#include "../App.h"
#include "ProgressWindow.h"
#include "IODialogs.h"

#include <Tera/Core.h>
#include <Tera/FStream.h>
#include <Tera/FString.h>
#include <Tera/FPackage.h>

#include <Utils/AConfiguration.h>
#include <Utils/ALog.h>
#include <Utils/DCKeyTool.h>

#include <execution>
#include <filesystem>
#include <map>

#include <Tera/DC.h>

DcToolDialog::DcToolDialog(wxWindow* parent)
  : wxDialog(parent, wxID_ANY, wxT("Unpack DataCenter file"), wxDefaultPosition, wxSize(457, 465))
{
  FAppConfig cfg = App::GetSharedApp()->GetConfig();
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer12;
  bSizer12 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText8;
  m_staticText8 = new wxStaticText(this, wxID_ANY, wxT("Client version:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText8->Wrap(-1);
  bSizer12->Add(m_staticText8, 0, wxALL, 5);

  FString ver = GetClientVersionString(cfg.RootDir);
  ClientVersion = new wxStaticText(this, wxID_ANY, ver.Empty() ? wxT("None") : ver.WString(), wxDefaultPosition, wxDefaultSize, 0);
  ClientVersion->Wrap(-1);
  bSizer12->Add(ClientVersion, 1, wxTOP | wxBOTTOM | wxRIGHT, 5);


  bSizer1->Add(bSizer12, 0, wxEXPAND, 5);

  wxStaticLine* m_staticline3;
  m_staticline3 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer1->Add(m_staticline3, 0, wxEXPAND | wxALL, 5);

  wxStaticText* m_staticText6;
  m_staticText6 = new wxStaticText(this, wxID_ANY, wxT("Type your values, or start Tera and press Find. RE must run with Administrator privileges for the Find feature to work correctly."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText6->Wrap(435);
  bSizer1->Add(m_staticText6, 0, wxALL, 5);

  wxBoxSizer* bSizer8;
  bSizer8 = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText2;
  m_staticText2 = new wxStaticText(this, wxID_ANY, wxT("Key:"), wxDefaultPosition, wxSize(38, -1), wxALIGN_RIGHT);
  m_staticText2->Wrap(-1);
  bSizer3->Add(m_staticText2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  KeyField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(KeyField, 1, wxTOP | wxBOTTOM | wxLEFT, 5);


  bSizer9->Add(bSizer3, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer31;
  bSizer31 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText21;
  m_staticText21 = new wxStaticText(this, wxID_ANY, wxT("IV:"), wxDefaultPosition, wxSize(38, -1), wxALIGN_RIGHT);
  m_staticText21->Wrap(-1);
  bSizer31->Add(m_staticText21, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  VecField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  bSizer31->Add(VecField, 1, wxTOP | wxBOTTOM | wxLEFT, 5);


  bSizer9->Add(bSizer31, 0, wxEXPAND, 5);


  bSizer8->Add(bSizer9, 1, wxEXPAND, 5);

  FindButton = new wxButton(this, wxID_ANY, wxT("Find"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer8->Add(FindButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer1->Add(bSizer8, 0, wxEXPAND, 5);

  wxStaticLine* m_staticline2;
  m_staticline2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer1->Add(m_staticline2, 0, wxEXPAND | wxALL, 5);

  wxStaticText* m_staticText5;
  m_staticText5 = new wxStaticText(this, wxID_ANY, wxT("Select a DC file you want to unpack. Can be found in the S1Data folder."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText5->Wrap(-1);
  bSizer1->Add(m_staticText5, 0, wxALL, 5);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("DC file:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText1->Wrap(-1);
  bSizer2->Add(m_staticText1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  DcFilePicker = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, wxT("Select a DC file"), wxT("*.dat"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE);
  bSizer2->Add(DcFilePicker, 1, wxALL, 5);


  bSizer1->Add(bSizer2, 0, wxEXPAND, 5);

  wxStaticText* m_staticText10;
  m_staticText10 = new wxStaticText(this, wxID_ANY, wxT("Key and IV fields must be filled for the Unpack feature."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText10->Wrap(-1);
  bSizer1->Add(m_staticText10, 0, wxALL, 5);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxHORIZONTAL);

  wxString ModeChoices[] = { wxT("Unpack"), wxT("Export as XML"), wxT("Export as JSON") };
  int ModeNChoices = sizeof(ModeChoices) / sizeof(wxString);
  Mode = new wxRadioBox(this, wxID_ANY, wxT("Mode"), wxDefaultPosition, wxDefaultSize, ModeNChoices, ModeChoices, 1, wxRA_SPECIFY_COLS);
  Mode->SetSelection(0);
  bSizer10->Add(Mode, 1, wxTOP | wxBOTTOM | wxLEFT, 5);

  UnpackButton = new wxButton(this, wxID_ANY, wxT("Unpack"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer10->Add(UnpackButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer1->Add(bSizer10, 0, wxEXPAND, 5);

  wxStaticLine* m_staticline1;
  m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer1->Add(m_staticline1, 0, wxEXPAND | wxALL, 5);

  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);

  CloseButton = new wxButton(this, wxID_ANY, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer11->Add(CloseButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  bSizer11->Add(0, 0, 1, wxEXPAND, 5);

  EditButton = new wxButton(this, wxID_ANY, wxT("Edit"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer11->Add(EditButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  EditButton->Enable(false);


  bSizer1->Add(bSizer11, 1, wxEXPAND, 5);


  this->SetSizer(bSizer1);
  this->Layout();

  this->Centre(wxBOTH);

  if (cfg.LastDcKey.Size())
  {
    KeyField->SetValue(cfg.LastDcKey.WString());
  }
  if (cfg.LastDcVec.Size())
  {
    VecField->SetValue(cfg.LastDcVec.WString());
  }
  if (cfg.LastDcPath.Size() && std::filesystem::exists(cfg.LastDcPath.WString()))
  {
    DcFilePicker->SetPath(cfg.LastDcPath.WString());
  }
  else
  {
    DcFilePicker->SetPath(FPackage::GetDcPath(cfg.RootDir).WString());
    DcFilePicker->SetInitialDirectory(std::filesystem::path(FPackage::GetDcPath(cfg.RootDir).WString()).parent_path().wstring());
  }
  Mode->SetSelection(cfg.LastDcMode);

  UpdateButtons();

  // Connect Events
  KeyField->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(DcToolDialog::OnKeyChanged), NULL, this);
  VecField->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(DcToolDialog::OnVecChanged), NULL, this);
  FindButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DcToolDialog::OnFindClicked), NULL, this);
  DcFilePicker->Connect(wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler(DcToolDialog::OnDcFileChanged), NULL, this);
  UnpackButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DcToolDialog::OnUnpackClicked), NULL, this);
  CloseButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DcToolDialog::OnCloseClicked), NULL, this);
  EditButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DcToolDialog::OnEditClicked), NULL, this);

  App::GetSharedApp()->SetDcToolIsOpen(true);
}

DcToolDialog::~DcToolDialog()
{
  App::GetSharedApp()->SetDcToolIsOpen(false);
  KeyField->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(DcToolDialog::OnKeyChanged), NULL, this);
  VecField->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(DcToolDialog::OnVecChanged), NULL, this);
  FindButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DcToolDialog::OnFindClicked), NULL, this);
  DcFilePicker->Disconnect(wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler(DcToolDialog::OnDcFileChanged), NULL, this);
  UnpackButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DcToolDialog::OnUnpackClicked), NULL, this);
  CloseButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DcToolDialog::OnCloseClicked), NULL, this);
  EditButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DcToolDialog::OnEditClicked), NULL, this);
}

void DcToolDialog::OnKeyChanged(wxCommandEvent& event)
{
  UpdateButtons();
}

void DcToolDialog::OnVecChanged(wxCommandEvent& event)
{
  UpdateButtons();
}

void DcToolDialog::OnFindClicked(wxCommandEvent& event)
{
  if (NeedsElevation())
  {
    wxMessageDialog dlg(this, "RE requires Administrator privileges to access TERA process.\n", "Error!", wxICON_AUTH_NEEDED | wxICON_QUESTION | wxYES_NO);
    dlg.SetYesNoLabels(wxS("Restart as Administrator"), wxS("Cancel"));
    if (dlg.ShowModal() == wxID_YES)
    {
      App::GetSharedApp()->RestartElevated();
    }
    return;
  }
  HANDLE proc = DCKeyTool::GetTeraProcess();
  if (!proc)
  {
    wxMessageBox("Start your Tera client and try again.", "Tera is not running!", wxICON_ERROR, this);
    return;
  }
  DCKeyTool tool(proc);
  if (!tool.FindKey())
  {
    wxMessageBox("Wait for the server selection screen to appear and then try again.", "Couldn't find the key!", wxICON_ERROR, this);
    return;
  }
  auto results = tool.GetResults();
  KeyField->SetValue(results.front().first);
  VecField->SetValue(results.front().second);
  App::GetSharedApp()->GetConfig().LastDcKey = results.front().first;
  App::GetSharedApp()->GetConfig().LastDcVec = results.front().second;
  App::GetSharedApp()->SaveConfig();
  UpdateButtons();

  wxString msg = wxString::Format("Key: %s\nVector: %s", results.front().first.c_str(), results.front().second.c_str());
  wxMessageBox(msg, "Success", wxICON_INFORMATION, this);
}

void DcToolDialog::OnDcFileChanged(wxFileDirPickerEvent& event)
{
  auto wstr = DcFilePicker->GetPath().ToStdWstring();
  if (std::filesystem::exists(wstr))
  {
    App::GetSharedApp()->GetConfig().LastDcPath = wstr;
    App::GetSharedApp()->SaveConfig();
  }
  UpdateButtons();
}

void DcToolDialog::OnUnpackClicked(wxCommandEvent& event)
{
  auto wstr = DcFilePicker->GetPath().ToStdWstring();
  if (!std::filesystem::exists(wstr))
  {
    DcFilePicker->SetPath(wxEmptyString);
    DcFilePicker->SetInitialDirectory(std::filesystem::path(FPackage::GetDcPath().WString()).parent_path().wstring());
    wxMessageBox("The DC file does not exist or can't be opened!", "Error!", wxICON_ERROR, this);
    DcFilePicker->SetFocus();
    return;
  }
  App::GetSharedApp()->GetConfig().LastDcMode = Mode->GetSelection();
  App::GetSharedApp()->SaveConfig();

  std::filesystem::path dst;
  if (!Mode->GetSelection())
  {
    wxString dir = std::filesystem::path(FPackage::GetDcPath().WString()).parent_path().wstring();
    if (App::GetSharedApp()->GetConfig().LastDcSavePath.Size())
    {
      dir = App::GetSharedApp()->GetConfig().LastDcSavePath.WString();
      if (std::filesystem::path(dir.ToStdWstring()).has_extension())
      {
        dir = std::filesystem::path(dir.ToStdWstring()).parent_path().wstring();
      }
    }
    dir = IODialog::SaveDatacenter(0, this, dir, std::filesystem::path(wstr).filename().replace_extension().wstring());
    if (dir.IsEmpty())
    {
      return;
    }
    dst = dir.ToStdWstring();
  }
  else
  {
    wxString dir = std::filesystem::path(FPackage::GetDcPath().WString()).parent_path().wstring();
    if (App::GetSharedApp()->GetConfig().LastDcSavePath.Size())
    {
      dir = App::GetSharedApp()->GetConfig().LastDcSavePath.WString();
      if (std::filesystem::path(dir.ToStdWstring()).has_extension())
      {
        dir = std::filesystem::path(dir.ToStdWstring()).parent_path().wstring();
      }
    }
    dir = IODialog::SaveDatacenter(1, this, dir, std::filesystem::path(wstr).parent_path().filename().wstring());
    if (dir.IsEmpty())
    {
      return;
    }
    dst = dir.ToStdWstring();
  }

  App::GetSharedApp()->GetConfig().LastDcSavePath = dst.wstring();
  App::GetSharedApp()->SaveConfig();
  
  ProgressWindow progress(this, wxEmptyString);
  progress.SetActionText(wxT("Unpacking..."));
  progress.SetCanCancel(false);
  progress.SetCurrentProgress(-1);

  wxString err;
  std::thread([&]() {
    std::ifstream in(wstr, std::ios::in | std::ios::binary | std::ios::ate);
    size_t inputLen = (size_t)in.tellg();
    std::vector<byte> inputData(inputLen);
    in.seekg(0, 0);
    in.read((char*)inputData.data(), inputLen);

    std::vector<unsigned char> rawkey;
    std::vector<unsigned char> rawvec;
    {
      std::string tmp;
      wxString wxtmp;

      wxtmp = KeyField->GetValue();
      wxtmp.Replace(" ", "", true);
      wxtmp.Replace("-", "", true);
      tmp = wxtmp;
      rawkey.resize(tmp.size() / 2);
      FString::StringToBytes(tmp.data(), tmp.size(), rawkey.data());

      wxtmp = VecField->GetValue();
      wxtmp.Replace(" ", "", true);
      wxtmp.Replace("-", "", true);
      tmp = wxtmp;
      rawvec.resize(tmp.size() / 2);
      FString::StringToBytes(tmp.data(), tmp.size(), rawvec.data());
    }

    std::vector<byte> outData;
    outData.reserve(inputLen);

    try
    {
      CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption tmp;
      tmp.SetKeyWithIV(rawkey.data(), rawkey.size(), rawvec.data());
      PERF_START(DecryptDC);
      CryptoPP::VectorSource ss(inputData, true, new CryptoPP::StreamTransformationFilter(tmp, new CryptoPP::VectorSink(outData)));
      PERF_END(DecryptDC);
      inputData.clear();
      if (outData[4] != 0x78 && outData[5] != 0x9C)
      {
        UThrow("Decrypted data has no zlib magic!");
      }
    }
    catch (const CryptoPP::Exception& e)
    {
      LogE("Failed to decrypt: %s", e.what());
      inputData.clear();
      outData.clear();
      err = "Failed to decrypt the DC. The Key or IV might be incorrect!\nTry to start your Tera and press Find button above.";
      SendEvent(&progress, UPDATE_PROGRESS_FINISH, false);
      return;
    }
    catch (const std::exception& se)
    {
      LogE("Failed to decrypt: %s", se.what());
      inputData.clear();
      outData.clear();
      err = "Failed to decrypt the DC. The Key or IV might be incorrect!\nTry to start your Tera and press Find button above.";
      SendEvent(&progress, UPDATE_PROGRESS_FINISH, false);
      return;
    }

    uint32 uncompressedSize = *(uint32*)outData.data();
    std::vector<unsigned char> inflatedDc(uncompressedSize);
    {
      wxMemoryInputStream wxis(outData.data() + sizeof(int), outData.size());
      try
      {
        wxZlibInputStream zis(wxis);
        PERF_START(UncompressDC);
        zis.ReadAll(inflatedDc.data(), uncompressedSize);
        PERF_END(UncompressDC);
        outData.clear();
      }
      catch (...)
      {
        inflatedDc.clear();
        err = "Failed to uncompress the DC. The Key or IV might be incorrect!";
        SendEvent(&progress, UPDATE_PROGRESS_FINISH, false);
        return;
      }
    }
    
    if (inflatedDc.empty())
    {
      SendEvent(&progress, UPDATE_PROGRESS_DESC, wxS("Saving..."));
      std::ofstream out(dst, std::ios::out | std::ios::binary);
      out.write((const char*)outData.data(), outData.size());
      SendEvent(&progress, UPDATE_PROGRESS_FINISH, true);
      return;
    }
    if (!Mode->GetSelection())
    {
      SendEvent(&progress, UPDATE_PROGRESS_DESC, wxS("Saving..."));
      std::ofstream out(dst, std::ios::out | std::ios::binary);
      out.write((const char*)inflatedDc.data(), uncompressedSize);
      SendEvent(&progress, UPDATE_PROGRESS_FINISH, true);
      return;
    }

    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxS("Serializing..."));

    MReadStream s(inflatedDc.data(), false, inflatedDc.size());
    PERF_START(SerializeDC);
#ifdef USE_STATIC_DC_4_EXPORT
    S1Data::StaticDataCenter dc;
    dc.Serialize(s);
#else
    S1Data::DataCenter dc;
    dc.Serialize(s);
    inflatedDc.clear();
#endif
    PERF_END(SerializeDC);

    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxS("Saving..."));

    dst /= (std::filesystem::path(wstr).filename().replace_extension().wstring() + L'_' + std::to_wstring(dc.GetHeader()->Version));

    std::unordered_map<S1Data::DCName, std::vector<S1Data::DCElement*>> items;
    S1Data::DCElement* rootElement = dc.GetRootElement();
    int32 total = 0;
    std::vector<S1Data::DCElement*> folders;
    for (int32 rootIndex = 0; rootIndex < rootElement->ChildrenCount; ++rootIndex)
    {
      if (S1Data::DCElement* element = dc.GetElement(rootElement->ChildrenIndices, rootIndex))
      {
        if (items[element->Name].size())
        {
          if (items[element->Name].size() == 1)
          {
            folders.emplace_back(element);
          }
          items[element->Name].emplace_back(element);
        }
        else
        {
          items[element->Name].emplace_back(element);
        }
        total++;
      }
    }
    std::for_each(std::execution::par_unseq, folders.begin(), folders.end(), [&](const S1Data::DCElement* element) {
      std::filesystem::create_directories(dst / std::wstring(dc.GetName(element->Name)));
    });
    SendEvent(&progress, UPDATE_MAX_PROGRESS, total);
    SendEvent(&progress, UPDATE_PROGRESS, 0);
    
    PERF_START(ExportDC);
    S1Data::DCExporter* exporter = nullptr;
    if (Mode->GetSelection() == 1)
    {
      exporter = new S1Data::DCXmlExporter(&dc);
    }
    else
    {
      exporter = new S1Data::DCJsonExporter(&dc);
    }

    std::for_each(std::execution::par_unseq, items.begin(), items.end(), [&](const auto& p) {
      const std::vector<S1Data::DCElement*>& elements = p.second;
      if (elements.size() == 1)
      {
        S1Data::DCElement* element = elements.front();
        exporter->ExportElement(element, dst / std::wstring(dc.GetName(element->Name)));
        SendEvent(&progress, UPDATE_PROGRESS_ADV);
      }
      else
      {
        std::for_each(std::execution::par_unseq, elements.begin(), elements.end(), [&](auto* element) {
          std::wstring name(dc.GetName(element->Name));
          size_t idx = distance(elements.begin(), find(elements.begin(), elements.end(), element));
          if (idx < elements.size() && element->Name.Index)
          {
            exporter->ExportElement(element, dst / name / (name + L"-" + std::to_wstring(idx + 1)));
            SendEvent(&progress, UPDATE_PROGRESS_ADV);
          }
        });
      }
    });
    PERF_END(ExportDC);
    delete exporter;
    items.clear();
    folders.clear();
    SendEvent(&progress, UPDATE_PROGRESS_FINISH, true);
  }).detach();
  if (progress.ShowModal())
  {
    wxMessageBox("Unpacked the DataCenter file successfully.", "Done!", wxICON_INFORMATION, this);
  }
  else if (err.size())
  {
    wxMessageBox(err, "Error!", wxICON_ERROR, this);
    FindButton->SetFocus();
  }
}

void DcToolDialog::OnCloseClicked(wxCommandEvent& event)
{
  EndModal(wxID_CLOSE);
}

void DcToolDialog::OnEditClicked(wxCommandEvent& event)
{
  // TODO
}

void DcToolDialog::UpdateButtons()
{
  UnpackButton->Enable(KeyField->GetValue().size() && VecField->GetValue().size() && DcFilePicker->GetPath().size());
}