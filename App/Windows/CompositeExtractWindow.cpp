#include "CompositeExtractWindow.h"
#include "TextureImporter.h"
#include "ProgressWindow.h"
#include "CookingOptions.h"
#include "../App.h"

#include <wx/statline.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>

#include <Utils/TextureTravaller.h>
#include <Utils/TextureProcessor.h>
#include <Tera/FPackage.h>
#include <Tera/Cast.h>

CompositeExtractWindow::CompositeExtractWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxDialog(parent, id, title, pos, size, style)
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
	ObjectClassTextField->Enable(false);

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

	wxStaticLine* m_staticline41;
	m_staticline41 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer10->Add(m_staticline41, 0, wxEXPAND | wxALL, 5);

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
	}
}

void CompositeExtractWindow::OnSearchClicked(wxCommandEvent& event)
{
	std::ifstream s(DumpTextField->GetValue().ToStdWstring());
	Found.clear();
	const std::string className = ObjectClassTextField->GetValue().ToStdString();
	const std::string objectName = ObjectTextField->GetValue().ToStdString();
	const std::string dupObjectName = objectName + "_dup";
	ProgressWindow progress(this, wxT("Searching..."));
	wxString desc = "Looking for all " + className + " objects with " + objectName + " name";
	progress.SetActionText(desc);
	progress.SetCanCancel(false);
	progress.SetCurrentProgress(-1);
	std::thread([&] {
		std::stringstream buffer;
		buffer << s.rdbuf();
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
			if (lineObjectName == objectName || lineObjectName == dupObjectName)
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
				Found.emplace_back(std::make_pair(std::string(&line[pos + 1], end - pos - 1), objIndex));
			}
		}
		SendEvent(&progress, UPDATE_PROGRESS_FINISH);
	}).detach();
	progress.ShowModal();
	ResultLabel->SetLabelText(wxString::Format(wxT("Found: %Iu item(s)"), Found.size()));
	ResultLabel->Show();
	ImportButton->Enable(Found.size());
	ExtractButton->Enable(Found.size());
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
		ImportButton->Enable(Found.size());
		ClearButton->Enable(ImportTextField->GetValue().size());
	}
	Found.clear();
}

void CompositeExtractWindow::OnObjectNameEnter(wxCommandEvent& event)
{
	SearchButton->Enable(ObjectTextField->GetValue().size() && ObjectClassTextField->GetValue().size());
	ResultLabel->SetLabelText(wxEmptyString);
	Found.clear();
}

void CompositeExtractWindow::OnImportClicked(wxCommandEvent& event)
{
	wxString path = TextureImporter::LoadImageDialog(this);
	if (path.empty())
	{
		return;
	}
	ImportTextField->SetValue(path);
	ClearButton->Enable(true);
}

void CompositeExtractWindow::OnImportClearClicked(wxCommandEvent& event)
{
	ImportTextField->SetValue(wxEmptyString);
	ClearButton->Enable(false);
}

void CompositeExtractWindow::OnExtractClicked(wxCommandEvent& event)
{
	if (Found.empty())
	{
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
	
	TextureProcessor processor(inputFormat, TextureProcessor::TCFormat::None);
	EPixelFormat processorFormat = PF_Unknown;
	ProgressWindow progress(this, wxT("Extracting packages..."));
	progress.SetActionText(wxT("Preparing..."));
	progress.SetCanCancel(true);
	progress.SetMaxProgress((int)Found.size());
	progress.SetCurrentProgress(0);
	if (doImport)
	{
		processor.SetInputPath(W2A(ImportTextField->GetValue().ToStdWstring()));
	}

	bool canceled = false;
	std::vector<std::pair<std::string, std::string>> failed;
	std::thread([&] {
#define RetIfCancel if (progress.IsCanceled()) {SendEvent(&progress, UPDATE_PROGRESS_FINISH); canceled = true; return; } //
		
		for (size_t idx = 0; idx < Found.size(); ++idx)
		{
			RetIfCancel;
			wxString desc = "Extracting " + Found[idx].first + ".gpk";
			SendEvent(&progress, UPDATE_PROGRESS_DESC, desc);
			SendEvent(&progress, UPDATE_PROGRESS, (int)idx);
			std::shared_ptr<FPackage> pkg = nullptr;
			try
			{
				if ((pkg = FPackage::GetPackageNamed(Found[idx].first)))
				{
					pkg->Load();

					if (doImport)
					{
						// TODO: allow to import anything
						if (UTexture2D* tex = Cast<UTexture2D>(pkg->GetObject(Found[idx].second)))
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

					ctx.Path = W2A((destDir / (Found[idx].first + ".gpk")).wstring());
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
				failed.emplace_back(std::make_pair(Found[idx].first, e.what()));
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
