#include "CompositeExtractWindow.h"
#include "TextureImporter.h"
#include "ProgressWindow.h"
#include "CookingOptions.h"
#include "../App.h"

#include <wx/dataview.h>
#include <wx/statline.h>
#include <wx/clipbrd.h>
#include <filesystem>
#include <fstream>
#include <thread>

#include <Utils/TextureTravaller.h>
#include <Utils/TextureProcessor.h>
#include <Utils/SoundTravaller.h>
#include <Tera/FPackage.h>
#include <Tera/UClass.h>
#include <Tera/USoundNode.h>
#include <Tera/Cast.h>

enum ObjTreeMenuId {
  ObjectList = wxID_HIGHEST + 1,
	ShowObject,
  CopyPackage,
	CopyPath,
  Toggle
};

CompositeExtractWindow::CompositeExtractWindow(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, wxT("Extract composite packages"), wxDefaultPosition, wxSize(682, 537), wxDEFAULT_DIALOG_STYLE)
{
	SetIcon(wxICON(#114));
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText6;
	m_staticText6 = new wxStaticText(this, wxID_ANY, wxT("Select the composite objects dump file. You can create one using Edit->Dump all composite objects. Then enter the object name and press Search. When the search finished press Extract and select a folder where you want to save all packages."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText6->Wrap(650);
	bSizer10->Add(m_staticText6, 0, wxALL, 5);

	wxStaticLine* m_staticline3;
	m_staticline3 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer10->Add(m_staticline3, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText4;
	m_staticText4 = new wxStaticText(this, wxID_ANY, wxT("Composite dump:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText4->Wrap(-1);
	bSizer11->Add(m_staticText4, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	DumpTextField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	bSizer11->Add(DumpTextField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	BrowseButton = new wxButton(this, wxID_ANY, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0);
	bSizer11->Add(BrowseButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer10->Add(bSizer11, 0, wxEXPAND, 5);

	wxStaticLine* m_staticline4;
	m_staticline4 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer10->Add(m_staticline4, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText7;
	m_staticText7 = new wxStaticText(this, wxID_ANY, wxT("Object class:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText7->Wrap(-1);
	bSizer12->Add(m_staticText7, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ObjectClassTextField = new wxTextCtrl(this, wxID_ANY, wxT("Texture2D"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer12->Add(ObjectClassTextField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	ObjectClassTextField->SetMaxSize(wxSize(125, -1));
	ObjectClassTextField->Enable(false);

	wxArrayString classList;
	std::vector<UClass*> classes = FPackage::GetClasses();
	for (UClass* cls : classes)
	{
		classList.push_back(cls->GetObjectName().WString());
	}
	ObjectClassTextField->AutoComplete(classList);

	wxStaticText* m_staticText72;
	m_staticText72 = new wxStaticText(this, wxID_ANY, wxT("Object name:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText72->Wrap(-1);
	bSizer12->Add(m_staticText72, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ObjectTextField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	bSizer12->Add(ObjectTextField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	ObjectTextField->Enable(false);

	SearchButton = new wxButton(this, wxID_ANY, wxT("Search"), wxDefaultPosition, wxDefaultSize, 0);
	SearchButton->Enable(false);

	bSizer12->Add(SearchButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer10->Add(bSizer12, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText71;
	m_staticText71 = new wxStaticText(this, wxID_ANY, wxT("Bulk import:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText71->Wrap(-1);
	bSizer121->Add(m_staticText71, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ImportTextField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	bSizer121->Add(ImportTextField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ImportButton = new wxButton(this, wxID_ANY, wxT("Import..."), wxDefaultPosition, wxDefaultSize, 0);
	ImportButton->Enable(false);

	bSizer121->Add(ImportButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ClearButton = new wxButton(this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
	ClearButton->Enable(false);

	bSizer121->Add(ClearButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer10->Add(bSizer121, 1, wxEXPAND, 5);

  ResultList = new wxDataViewCtrl(this, ObjTreeMenuId::ObjectList, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE);
  ResultList->SetMinSize(wxSize(-1, 250));

  bSizer10->Add(ResultList, 1, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer(wxHORIZONTAL);

	ResultLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	ResultLabel->Wrap(-1);
	bSizer14->Add(ResultLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	wxPanel* m_panel6;
	m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	bSizer14->Add(m_panel6, 1, wxEXPAND | wxALL, 5);

	ExtractButton = new wxButton(this, wxID_ANY, wxT("Extract"), wxDefaultPosition, wxDefaultSize, 0);
	ExtractButton->Enable(false);

	bSizer14->Add(ExtractButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer10->Add(bSizer14, 1, wxEXPAND, 5);


	SetSizer(bSizer10);
	Layout();

	Centre(wxBOTH);

	// Connect Events
	BrowseButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnBrowseClicked), NULL, this);
	SearchButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnSearchClicked), NULL, this);
	ObjectTextField->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CompositeExtractWindow::OnObjectNameText), NULL, this);
	ObjectClassTextField->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CompositeExtractWindow::OnObjectNameText), NULL, this);
	ObjectTextField->Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(CompositeExtractWindow::OnObjectNameEnter), NULL, this);
	ImportButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnImportClicked), NULL, this);
	ClearButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnImportClearClicked), NULL, this);
	ExtractButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnExtractClicked), NULL, this);

	FAppConfig& cfg = ((App*)wxTheApp)->GetConfig();
	if (cfg.CompositeDumpPath.Size())
	{
		DumpTextField->SetValue(cfg.CompositeDumpPath.WString());
		std::error_code err;
		if (std::filesystem::exists(cfg.CompositeDumpPath.WString(), err))
		{
      ObjectClassTextField->Enable(true);
      ObjectTextField->Enable(true);
      ObjectTextField->SetFocus();
		}
	}

  ResultList->AppendToggleColumn(_(""), CompositeExtractModel::Col_Check, wxDATAVIEW_CELL_ACTIVATABLE, 25);
	ResultList->AppendTextColumn(_("Package"), CompositeExtractModel::Col_Package, wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	ResultList->AppendTextColumn(_("Object"), CompositeExtractModel::Col_Path, wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
}

CompositeExtractWindow::CompositeExtractWindow(wxWindow* parent, const wxString& objClass, const wxString objName)
  : CompositeExtractWindow(parent)
{
  ObjectClassTextField->SetValue(objClass);
  ObjectTextField->SetValue(objName);

  if (DumpTextField->GetValue().size() && objClass.size() && objName.size())
  {
    ObjectClassTextField->Enable(true);
    ObjectTextField->Enable(true);
    SearchButton->Enable(true);
  }
}

CompositeExtractWindow::~CompositeExtractWindow()
{
	// Disconnect Events
	BrowseButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnBrowseClicked), NULL, this);
	SearchButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnSearchClicked), NULL, this);
	ObjectTextField->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CompositeExtractWindow::OnObjectNameText), NULL, this);
	ObjectClassTextField->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CompositeExtractWindow::OnObjectNameText), NULL, this);
	ObjectTextField->Disconnect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(CompositeExtractWindow::OnObjectNameEnter), NULL, this);
	ImportButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnImportClicked), NULL, this);
	ClearButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnImportClearClicked), NULL, this);
	ExtractButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CompositeExtractWindow::OnExtractClicked), NULL, this);
}

void CompositeExtractWindow::OnBrowseClicked(wxCommandEvent& event)
{
	wxString extensions = wxS("Objects dump file (*.txt)|*.txt");
	wxString path = wxFileSelector("Select a composite dump...", wxEmptyString, wxT("ObjectDump.txt"), extensions, extensions, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (path.size())
	{
		DumpTextField->SetValue(path);
		ObjectClassTextField->Enable(true);
		ObjectTextField->Enable(true);
		FAppConfig& cfg = ((App*)wxTheApp)->GetConfig();
		cfg.CompositeDumpPath = path.ToStdWstring();
		((App*)wxTheApp)->SaveConfig();
	}
}

void CompositeExtractWindow::OnSearchClicked(wxCommandEvent& event)
{
	const std::string className = ObjectClassTextField->GetValue().ToStdString();
	const std::string objectName = ObjectTextField->GetValue().ToStdString();
	const std::string dupObjectName = objectName + '_';
	ProgressWindow progress(this, wxT("Searching..."));
	wxString desc = "Looking for all " + className + " objects with " + objectName + " name";
	progress.SetActionText(desc);
	progress.SetCanCancel(false);
	progress.SetCurrentProgress(-1);
	std::stringstream& buffer = LoadedBuffer;
	wxString& loadedPath = LoadedPath;
	std::vector<CompositeExtractModelNode> found;
	std::thread([&] {
		if (DumpTextField->GetValue() != LoadedPath)
		{
			loadedPath = DumpTextField->GetValue();
      std::ifstream s(loadedPath.ToStdWstring());
			buffer = std::stringstream();
      buffer << s.rdbuf();
		}
		else
		{
			buffer.clear();
			buffer.seekg(0);
		}
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
				found.push_back({ objectPath, true, std::string(&line[pos + 1], end - pos - 1), objIndex });
			}
		}
		SendEvent(&progress, UPDATE_PROGRESS_FINISH);
	}).detach();
	progress.ShowModal();
	ResultLabel->SetLabelText(wxString::Format(wxT("Found: %Iu item(s)"), found.size()));
	ResultList->AssociateModel(new CompositeExtractModel(found));
	ResultLabel->Show();
	ImportButton->Enable(found.size());
	ExtractButton->Enable(found.size());
	ClearButton->Enable(ImportTextField->GetValue().size());
}

void CompositeExtractWindow::OnObjectNameText(wxCommandEvent& event)
{
	SearchButton->Enable(ObjectTextField->GetValue().size() && ObjectClassTextField->GetValue().size());
}

void CompositeExtractWindow::OnObjectClassText(wxCommandEvent& event)
{
	SearchButton->Enable(ObjectTextField->GetValue().size() && ObjectClassTextField->GetValue().size());
	ResultLabel->SetLabelText(wxEmptyString);
	if (ObjectClassTextField->GetValue() != UTexture2D::StaticClassName())
	{
		ImportTextField->SetValue(wxEmptyString);
		ImportButton->Enable(false);
		ClearButton->Enable(false);
	}
	else
	{
		ImportButton->Enable(GetResultsCount());
		ClearButton->Enable(ImportTextField->GetValue().size());
	}
	ResultList->AssociateModel(new CompositeExtractModel(std::vector<CompositeExtractModelNode>()));
}

void CompositeExtractWindow::OnObjectNameEnter(wxCommandEvent& event)
{
	SearchButton->Enable(ObjectTextField->GetValue().size() && ObjectClassTextField->GetValue().size());
	ResultLabel->SetLabelText(wxEmptyString);
	ResultList->AssociateModel(new CompositeExtractModel(std::vector<CompositeExtractModelNode>()));
}

void CompositeExtractWindow::OnImportClicked(wxCommandEvent& event)
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
    wxMessageBox(_("This class does not support bulk import! But you can extract packages anyway."), _("Can't import!"), wxICON_INFORMATION);
    return;
  }
  if (path.size())
  {
    ImportTextField->SetValue(path);
    ClearButton->Enable(true);
  }
}

void CompositeExtractWindow::OnImportClearClicked(wxCommandEvent& event)
{
	ImportTextField->SetValue(wxEmptyString);
	ClearButton->Enable(false);
}

void CompositeExtractWindow::OnExtractClicked(wxCommandEvent& event)
{
	const std::string className = ObjectClassTextField->GetValue().ToStdString();
	if (className == UTexture2D::StaticClassName())
	{
		ExtractTextures();
	}
	else if (className == USoundNodeWave::StaticClassName())
	{
    ExtractSounds();
	}
  else
  {
    ExtractUntyped();
  }
}

void CompositeExtractWindow::OnContextMenu(wxDataViewEvent& event)
{
  if (!event.GetItem().IsOk())
  {
    return;
  }
	auto rows = GetSearchResult();
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
    ResultList->GetSelections(selection);
    if (!selection.GetCount())
    {
      return;
    }
    
    auto results = GetSearchResult();
    ResultList->Freeze();
    for (auto& item : selection)
    {
      int idx = int(item.GetID()) - 1;
      wxVariant value = !results[idx].Enabled;
      ResultList->GetModel()->ChangeValue(value, item, CompositeExtractModel::Col_Check);
    }
    ResultList->Thaw();
  }
}

void CompositeExtractWindow::ExtractTextures()
{
  int resultCount = GetEnabledResultsCount();
  if (!resultCount)
  {
    wxMessageBox(_("There are no packages selected.\nPlease select packages you want to extract!"), _("Nothing to extract!"), wxICON_INFORMATION);
    return;
  }

  wxDirDialog dlg(NULL, "Select a directory to extract packages to...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (dlg.ShowModal() != wxID_OK || dlg.GetPath().empty())
  {
    return;
  }
  const std::string className = ObjectClassTextField->GetValue().ToStdString();
  bool doImport = className == UTexture2D::StaticClassName() && ImportTextField->GetValue().size();
  const std::filesystem::path destDir = dlg.GetPath().ToStdWstring();

  PackageSaveContext ctx;
  ctx.EmbedObjectPath = true;
  ctx.DisableTextureCaching = true;

  TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
  if (doImport)
  {
    wxString extension;
    wxFileName::SplitPath(ImportTextField->GetValue(), nullptr, nullptr, nullptr, &extension);
    extension.MakeLower();
    if (extension == "tga")
    {
      inputFormat = TextureProcessor::TCFormat::TGA;
    }
    else if (extension == "png")
    {
      inputFormat = TextureProcessor::TCFormat::PNG;
    }
    else if (extension == "dds")
    {
      inputFormat = TextureProcessor::TCFormat::DDS;
    }
    else
    {
      doImport = false;
    }
  }

  auto searchResult = GetSearchResult();

  TextureProcessor processor(inputFormat, TextureProcessor::TCFormat::None);
  EPixelFormat processorFormat = PF_Unknown;
  ProgressWindow progress(this, wxT("Extracting packages..."));
  progress.SetActionText(wxT("Preparing..."));
  progress.SetCanCancel(true);
  progress.SetMaxProgress(resultCount);
  progress.SetCurrentProgress(0);
  if (doImport)
  {
    processor.SetInputPath(W2A(ImportTextField->GetValue().ToStdWstring()));
  }

  bool canceled = false;
  std::vector<std::pair<std::string, std::string>> failed;
  std::thread([&] {
#define RetIfCancel if (progress.IsCanceled()) {SendEvent(&progress, UPDATE_PROGRESS_FINISH); canceled = true; return; } //

    for (size_t idx = 0; idx < searchResult.size(); ++idx)
    {
      RetIfCancel;
      if (!searchResult[idx].Enabled)
      {
        continue;
      }
      wxString desc = "Extracting " + searchResult[idx].PackageName + ".gpk";
      SendEvent(&progress, UPDATE_PROGRESS_DESC, desc);
      SendEvent(&progress, UPDATE_PROGRESS, (int)idx);
      std::shared_ptr<FPackage> pkg = nullptr;
      try
      {
        if ((pkg = FPackage::GetPackageNamed(searchResult[idx].PackageName.ToStdWstring())))
        {
          pkg->Load();

          if (doImport)
          {
            // TODO: allow to import anything
            if (UTexture2D* tex = Cast<UTexture2D>(pkg->GetObject(searchResult[idx].ObjectIndex)))
            {
              bool isNormal = tex->CompressionSettings == TC_Normalmap ||
                tex->CompressionSettings == TC_NormalmapAlpha ||
                tex->CompressionSettings == TC_NormalmapUncompressed ||
                tex->CompressionSettings == TC_NormalmapBC5;

              if (tex->Format != processorFormat)
              {
                switch ((processorFormat = tex->Format))
                {
                case PF_DXT1:
                  processor.SetOutputFormat(TextureProcessor::TCFormat::DXT1);
                  break;
                case PF_DXT3:
                  processor.SetOutputFormat(TextureProcessor::TCFormat::DXT3);
                  break;
                case PF_DXT5:
                  processor.SetOutputFormat(TextureProcessor::TCFormat::DXT5);
                  break;
                default:
                  UThrow("Unsupported texture format!");
                }

                processor.SetSrgb(tex->SRGB);
                processor.SetNormal(isNormal);
                processor.SetGenerateMips(false); //  TODO: change to true when mips don't crash the game
                processor.SetAddressX(tex->AddressX);
                processor.SetAddressY(tex->AddressY);
                processor.ClearOutput();
                if (!processor.Process())
                {
                  UThrow(processor.GetError().c_str());
                }
              }

              TextureTravaller travaller;
              travaller.SetFormat(processorFormat);
              travaller.SetAddressX(tex->AddressX);
              travaller.SetAddressY(tex->AddressY);

              const auto& mips = processor.GetOutputMips();
              for (const auto mip : mips)
              {
                travaller.AddMipMap(mip.SizeX, mip.SizeY, mip.Size, mip.Data);
              }

              if (!travaller.Visit(tex))
              {
                UThrow(travaller.GetError().c_str());
              }
            }
            else
            {
              UThrow("Failed to load the object!");
            }
          }

          ctx.Path = W2A((destDir / (searchResult[idx].PackageName.ToStdWstring() + L".gpk")).wstring());
          pkg->Save(ctx);
          FPackage::UnloadPackage(pkg);
        }
        else
        {
          UThrow("Failed to open a package!");
        }
      }
      catch (const std::exception& e)
      {
        failed.emplace_back(std::make_pair(searchResult[idx].PackageName, e.what()));
        FPackage::UnloadPackage(pkg);
      }
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();
  if (!canceled)
  {
    if (failed.size())
    {
      std::ofstream s(destDir / "error_log.txt", std::ios::out | std::ios::binary);
      for (const auto& pair : failed)
      {
        s << pair.first << ": " << pair.second << '\n';
      }
      wxMessageBox(_("Failed to process some of the packages.\nSee error_log.txt in the output folder for more details."), _("Warning!"), wxICON_WARNING);
    }
    else
    {
      wxString msg = doImport ? wxS("Packages were extracted and patched successfuly!") : wxS("Packages were extracted successfuly!");
      wxMessageBox(msg, _("Done!"), wxICON_INFORMATION);
    }
  }
}

void CompositeExtractWindow::ExtractSounds()
{
  int resultCount = GetEnabledResultsCount();
  if (!resultCount)
  {
    wxMessageBox(_("There are no packages selected.\nPlease select packages you want to extract!"), _("Nothing to extract!"), wxICON_INFORMATION);
    return;
  }

  wxDirDialog dlg(NULL, "Select a directory to extract packages to...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (dlg.ShowModal() != wxID_OK || dlg.GetPath().empty())
  {
    return;
  }
  const std::string className = ObjectClassTextField->GetValue().ToStdString();
  bool doImport = className == USoundNodeWave::StaticClassName() && ImportTextField->GetValue().size();
  const std::filesystem::path destDir = dlg.GetPath().ToStdWstring();

  PackageSaveContext ctx;
  ctx.EmbedObjectPath = true;
  ctx.DisableTextureCaching = true;

  auto searchResult = GetSearchResult();

  ProgressWindow progress(this, wxT("Extracting packages..."));
  progress.SetActionText(wxT("Preparing..."));
  progress.SetCanCancel(true);
  progress.SetMaxProgress(resultCount);
  progress.SetCurrentProgress(0);
  
  SoundTravaller travaller;
  if (doImport)
  {
    void* soundData = nullptr;
    FILE_OFFSET size = 0;

    std::ifstream s(ImportTextField->GetValue().ToStdWstring(), std::ios::in | std::ios::binary);
    size_t tmpPos = s.tellg();
    s.seekg(0, std::ios::end);
    size = (FILE_OFFSET)s.tellg();
    s.seekg(tmpPos);
    if (size > 0)
    {
      soundData = malloc(size);
      s.read((char*)soundData, size);
      travaller.SetData(soundData, size);
    }
  }

  bool canceled = false;
  std::vector<std::pair<std::string, std::string>> failed;
  std::thread([&] {
#define RetIfCancel if (progress.IsCanceled()) {SendEvent(&progress, UPDATE_PROGRESS_FINISH); canceled = true; return; } //

    for (size_t idx = 0; idx < searchResult.size(); ++idx)
    {
      RetIfCancel;
      if (!searchResult[idx].Enabled)
      {
        continue;
      }
      wxString desc = "Extracting " + searchResult[idx].PackageName + ".gpk";
      SendEvent(&progress, UPDATE_PROGRESS_DESC, desc);
      SendEvent(&progress, UPDATE_PROGRESS, (int)idx);
      std::shared_ptr<FPackage> pkg = nullptr;
      try
      {
        if ((pkg = FPackage::GetPackageNamed(searchResult[idx].PackageName.ToStdWstring())))
        {
          pkg->Load();

          if (doImport)
          {
            if (USoundNodeWave* obj = Cast<USoundNodeWave>(pkg->GetObject(searchResult[idx].ObjectIndex)))
            {
              if (!travaller.Visit(obj))
              {
                failed.emplace_back(std::make_pair(searchResult[idx].PackageName, "Failed to import data!"));
              }
            }
            else
            {
              UThrow("Failed to load the object!");
            }
          }

          ctx.Path = W2A((destDir / (searchResult[idx].PackageName.ToStdWstring() + L".gpk")).wstring());
          pkg->Save(ctx);
          FPackage::UnloadPackage(pkg);
        }
        else
        {
          UThrow("Failed to open a package!");
        }
      }
      catch (const std::exception& e)
      {
        failed.emplace_back(std::make_pair(searchResult[idx].PackageName, e.what()));
        FPackage::UnloadPackage(pkg);
      }
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();
  if (!canceled)
  {
    if (failed.size())
    {
      std::ofstream s(destDir / "error_log.txt", std::ios::out | std::ios::binary);
      for (const auto& pair : failed)
      {
        s << pair.first << ": " << pair.second << '\n';
      }
      wxMessageBox(_("Failed to process some of the packages.\nSee error_log.txt in the output folder for more details."), _("Warning!"), wxICON_WARNING);
    }
    else
    {
      wxString msg = doImport ? wxS("Packages were extracted and patched successfuly!") : wxS("Packages were extracted successfuly!");
      wxMessageBox(msg, _("Done!"), wxICON_INFORMATION);
    }
  }
}

void CompositeExtractWindow::ExtractUntyped()
{
  int resultCount = GetEnabledResultsCount();
  if (!resultCount)
  {
    wxMessageBox(_("There are no packages selected.\nPlease select packages you want to extract!"), _("Nothing to extract!"), wxICON_INFORMATION);
    return;
  }

  wxDirDialog dlg(NULL, "Select a directory to extract packages to...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (dlg.ShowModal() != wxID_OK || dlg.GetPath().empty())
  {
    return;
  }
  const std::string className = ObjectClassTextField->GetValue().ToStdString();
  bool doImport = className == USoundNodeWave::StaticClassName() && ImportTextField->GetValue().size();
  const std::filesystem::path destDir = dlg.GetPath().ToStdWstring();

  PackageSaveContext ctx;
  ctx.EmbedObjectPath = true;
  ctx.DisableTextureCaching = true;

  auto searchResult = GetSearchResult();

  ProgressWindow progress(this, wxT("Extracting packages..."));
  progress.SetActionText(wxT("Preparing..."));
  progress.SetCanCancel(true);
  progress.SetMaxProgress(resultCount);
  progress.SetCurrentProgress(0);

  bool canceled = false;
  std::vector<std::pair<std::string, std::string>> failed;
  std::thread([&] {
#define RetIfCancel if (progress.IsCanceled()) {SendEvent(&progress, UPDATE_PROGRESS_FINISH); canceled = true; return; } //

    for (size_t idx = 0; idx < searchResult.size(); ++idx)
    {
      RetIfCancel;
      if (!searchResult[idx].Enabled)
      {
        continue;
      }
      wxString desc = "Extracting " + searchResult[idx].PackageName + ".gpk";
      SendEvent(&progress, UPDATE_PROGRESS_DESC, desc);
      SendEvent(&progress, UPDATE_PROGRESS, (int)idx);
      std::shared_ptr<FPackage> pkg = nullptr;
      try
      {
        if ((pkg = FPackage::GetPackageNamed(searchResult[idx].PackageName.ToStdWstring())))
        {
          pkg->Load();
          ctx.Path = W2A((destDir / (searchResult[idx].PackageName.ToStdWstring() + L".gpk")).wstring());
          pkg->Save(ctx);
          FPackage::UnloadPackage(pkg);
        }
        else
        {
          UThrow("Failed to open a package!");
        }
      }
      catch (const std::exception& e)
      {
        failed.emplace_back(std::make_pair(searchResult[idx].PackageName, e.what()));
        FPackage::UnloadPackage(pkg);
      }
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();
  if (!canceled)
  {
    if (failed.size())
    {
      std::ofstream s(destDir / "error_log.txt", std::ios::out | std::ios::binary);
      for (const auto& pair : failed)
      {
        s << pair.first << ": " << pair.second << '\n';
      }
      wxMessageBox(_("Failed to process some of the packages.\nSee error_log.txt in the output folder for more details."), _("Warning!"), wxICON_WARNING);
    }
    else
    {
      wxString msg = doImport ? wxS("Packages were extracted and patched successfuly!") : wxS("Packages were extracted successfuly!");
      wxMessageBox(msg, _("Done!"), wxICON_INFORMATION);
    }
  }
}

int CompositeExtractWindow::GetResultsCount()
{
	CompositeExtractModel* model = (CompositeExtractModel*)ResultList->GetModel();
	return model ? model->GetCount() : 0;
}

int CompositeExtractWindow::GetEnabledResultsCount()
{
	if (CompositeExtractModel* model = (CompositeExtractModel*)ResultList->GetModel())
	{
		auto rows = model->GetRows();
		int result = 0;
		for (const auto& row : rows)
		{
			if (row.Enabled)
			{
				result++;
			}
		}
		return result;
	}
	return 0;
}

std::vector<CompositeExtractModelNode> CompositeExtractWindow::GetSearchResult()
{
	CompositeExtractModel* model = (CompositeExtractModel*)ResultList->GetModel();
	return model ? model->GetRows() : std::vector<CompositeExtractModelNode>();
}

wxBEGIN_EVENT_TABLE(CompositeExtractWindow, wxDialog)
EVT_DATAVIEW_ITEM_CONTEXT_MENU(ObjTreeMenuId::ObjectList, CompositeExtractWindow::OnContextMenu)
wxEND_EVENT_TABLE()