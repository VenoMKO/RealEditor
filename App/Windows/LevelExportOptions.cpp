#include "LevelExportOptions.h"
#include "../App.h"

#include <wx/notebook.h>
#include <wx/valnum.h>

TextureProcessor::TCFormat TextureFormatToTcf(int idx)
{
	switch (idx)
	{
	case 0:
		return TextureProcessor::TCFormat::TGA;
	case 1:
		return TextureProcessor::TCFormat::PNG;
	}
	return TextureProcessor::TCFormat::DDS;
}

int TcfToTextureFormat(TextureProcessor::TCFormat tcf)
{
	switch (tcf)
	{
	case TextureProcessor::TCFormat::TGA:
		return 0;
	case TextureProcessor::TCFormat::PNG:
		return 1;
	}
	return 2;
}

LevelExportOptionsWindow::LevelExportOptionsWindow(wxWindow* parent, const LevelExportContext& ctx)
  : wxDialog(parent, wxID_ANY, wxT("Export options"), wxDefaultPosition, wxSize(597, 457), wxDEFAULT_DIALOG_STYLE)
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText5;
	m_staticText5 = new wxStaticText(this, wxID_ANY, wxT("Destination"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText5->Wrap(-1);
	m_staticText5->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer10->Add(m_staticText5, 0, wxTOP | wxRIGHT | wxLEFT, 5);

	wxPanel* m_panel5;
	m_panel5 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText6;
	m_staticText6 = new wxStaticText(m_panel5, wxID_ANY, wxT("Select a folder to save exported data to."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText6->Wrap(-1);
	bSizer11->Add(m_staticText6, 0, wxTOP | wxRIGHT | wxLEFT, 5);

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText7;
	m_staticText7 = new wxStaticText(m_panel5, wxID_ANY, wxT("Path:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText7->Wrap(-1);
	bSizer12->Add(m_staticText7, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	PathPicker = new wxDirPickerCtrl(m_panel5, wxID_ANY, wxEmptyString, wxT("Select a folder"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE);
	bSizer12->Add(PathPicker, 1, wxALL, 5);


	bSizer11->Add(bSizer12, 1, wxEXPAND, 5);


	m_panel5->SetSizer(bSizer11);
	m_panel5->Layout();
	bSizer11->Fit(m_panel5);
	bSizer10->Add(m_panel5, 0, wxEXPAND | wxALL, 5);

	wxStaticText* m_staticText51;
	m_staticText51 = new wxStaticText(this, wxID_ANY, wxT("Actors"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText51->Wrap(-1);
	m_staticText51->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer10->Add(m_staticText51, 0, wxTOP | wxRIGHT | wxLEFT, 5);

	wxPanel* m_panel6;
	m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText11;
	m_staticText11 = new wxStaticText(m_panel6, wxID_ANY, wxT("Select actor types to export:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText11->Wrap(-1);
	bSizer13->Add(m_staticText11, 0, wxTOP | wxRIGHT | wxLEFT, 5);

	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer(0, 4, 0, 0);

	StaticMeshes = new wxCheckBox(m_panel6, wxID_ANY, wxT("Static Meshes"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer1->Add(StaticMeshes, 0, wxALL, 5);

	SkeletalMeshes = new wxCheckBox(m_panel6, wxID_ANY, wxT("Skeletal Meshes"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer1->Add(SkeletalMeshes, 0, wxALL, 5);

	SpeedTrees = new wxCheckBox(m_panel6, wxID_ANY, wxT("SpeedTrees"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer1->Add(SpeedTrees, 0, wxALL, 5);

	Interps = new wxCheckBox(m_panel6, wxID_ANY, wxT("Interp Actors"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer1->Add(Interps, 0, wxALL, 5);

	Terrains = new wxCheckBox(m_panel6, wxID_ANY, wxT("Terrain Actors"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer1->Add(Terrains, 0, wxALL, 5);

	PointLights = new wxCheckBox(m_panel6, wxID_ANY, wxT("Point Lights"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer1->Add(PointLights, 0, wxALL, 5);

	SpotLights = new wxCheckBox(m_panel6, wxID_ANY, wxT("Spot Lights"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer1->Add(SpotLights, 0, wxALL, 5);

	SoundNodes = new wxCheckBox(m_panel6, wxID_ANY, wxT("Sound Nodes"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer1->Add(SoundNodes, 0, wxALL, 5);


	bSizer13->Add(gSizer1, 1, wxEXPAND, 5);


	m_panel6->SetSizer(bSizer13);
	m_panel6->Layout();
	bSizer13->Fit(m_panel6);
	bSizer10->Add(m_panel6, 0, wxALL | wxEXPAND, 5);

	wxStaticText* m_staticText511;
	m_staticText511 = new wxStaticText(this, wxID_ANY, wxT("Options"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText511->Wrap(-1);
	m_staticText511->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer10->Add(m_staticText511, 0, wxTOP | wxRIGHT | wxLEFT, 5);

	wxNotebook* m_notebook1;
	m_notebook1 = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	wxPanel* m_panel11;
	m_panel11 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer(wxVERTICAL);

	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer(0, 4, 0, 0);

	Materials = new wxCheckBox(m_panel11, wxID_ANY, wxT("Export Materials"), wxDefaultPosition, wxDefaultSize, 0);
	gSizer2->Add(Materials, 0, wxALL, 5);


	bSizer26->Add(gSizer2, 0, 0, 5);


	m_panel11->SetSizer(bSizer26);
	m_panel11->Layout();
	bSizer26->Fit(m_panel11);
	m_notebook1->AddPage(m_panel11, wxT("Materials"), false);
	wxPanel* m_panel12;
	m_panel12 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* m_panel8;
	m_panel8 = new wxPanel(m_panel12, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText15;
	m_staticText15 = new wxStaticText(m_panel8, wxID_ANY, wxT("Point lights scale:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText15->Wrap(-1);
	bSizer18->Add(m_staticText15, 0, wxTOP | wxBOTTOM | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);

	PointLightMultiplier = new wxTextCtrl(m_panel8, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(40, -1), wxTE_CENTER);
	bSizer18->Add(PointLightMultiplier, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	m_panel8->SetSizer(bSizer18);
	m_panel8->Layout();
	bSizer18->Fit(m_panel8);
	bSizer24->Add(m_panel8, 0, wxALL | wxEXPAND, 0);

	wxPanel* m_panel9;
	m_panel9 = new wxPanel(m_panel12, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText151;
	m_staticText151 = new wxStaticText(m_panel9, wxID_ANY, wxT("Spot lights scale:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText151->Wrap(-1);
	bSizer19->Add(m_staticText151, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);

	SpotLightMultiplier = new wxTextCtrl(m_panel9, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(40, -1), wxTE_CENTER);
	bSizer19->Add(SpotLightMultiplier, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	m_panel9->SetSizer(bSizer19);
	m_panel9->Layout();
	bSizer19->Fit(m_panel9);
	bSizer24->Add(m_panel9, 0, wxEXPAND | wxALL, 0);


	bSizer23->Add(bSizer24, 1, wxEXPAND, 5);

	LightInvSqrt = new wxCheckBox(m_panel12, wxID_ANY, wxT("Use inverse square falloff"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer23->Add(LightInvSqrt, 0, wxALL, 5);


	bSizer23->Add(0, 0, 1, wxEXPAND, 5);


	m_panel12->SetSizer(bSizer23);
	m_panel12->Layout();
	bSizer23->Fit(m_panel12);
	m_notebook1->AddPage(m_panel12, wxT("Lights"), false);
	wxPanel* m_panel14;
	m_panel14 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer(wxHORIZONTAL);

	ResampleTerrain = new wxCheckBox(m_panel14, wxID_ANY, wxT("Resample Terrain"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer27->Add(ResampleTerrain, 0, wxALL, 5);

	SplitTerrainWeightMaps = new wxCheckBox(m_panel14, wxID_ANY, wxT("Split weightmaps"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer27->Add(SplitTerrainWeightMaps, 0, wxALL, 5);


	m_panel14->SetSizer(bSizer27);
	m_panel14->Layout();
	bSizer27->Fit(m_panel14);
	m_notebook1->AddPage(m_panel14, wxT("Terrain"), false);
	wxPanel* m_panel15;
	m_panel15 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer(wxVERTICAL);

	Textures = new wxCheckBox(m_panel15, wxID_ANY, wxT("Export Textures"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer28->Add(Textures, 0, wxALL, 5);

	wxPanel* m_panel16;
	m_panel16 = new wxPanel(m_panel15, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText26;
	m_staticText26 = new wxStaticText(m_panel16, wxID_ANY, wxT("Format:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText26->Wrap(-1);
	bSizer29->Add(m_staticText26, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	wxString TextureFormatSelectorChoices[] = { wxT("TGA"), wxT("PNG"), wxT("DDS") };
	int TextureFormatSelectorNChoices = sizeof(TextureFormatSelectorChoices) / sizeof(wxString);
	TextureFormatSelector = new wxChoice(m_panel16, wxID_ANY, wxDefaultPosition, wxSize(100, -1), TextureFormatSelectorNChoices, TextureFormatSelectorChoices, 0);
	TextureFormatSelector->SetSelection(HasAVX2() ? 0 : 2);
	TextureFormatSelector->Enable(HasAVX2());
	bSizer29->Add(TextureFormatSelector, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	m_panel16->SetSizer(bSizer29);
	m_panel16->Layout();
	bSizer29->Fit(m_panel16);
	bSizer28->Add(m_panel16, 0, wxALL, 5);


	m_panel15->SetSizer(bSizer28);
	m_panel15->Layout();
	bSizer28->Fit(m_panel15);
	m_notebook1->AddPage(m_panel15, wxT("Textures"), true);
	wxPanel* m_panel17;
	m_panel17 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer(wxVERTICAL);

	Emitters = new wxCheckBox(m_panel17, wxID_ANY, wxT("Export emitter placeholders"), wxDefaultPosition, wxDefaultSize, 0);
	Emitters->SetValue(true);
	bSizer30->Add(Emitters, 0, wxALL, 5);


	m_panel17->SetSizer(bSizer30);
	m_panel17->Layout();
	bSizer30->Fit(m_panel17);
	m_notebook1->AddPage(m_panel17, wxT("Other"), false);

	bSizer10->Add(m_notebook1, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer(wxHORIZONTAL);

	DefaultsButton = new wxButton(this, wxID_ANY, wxT("Defaults"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer14->Add(DefaultsButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer14->Add(0, 0, 1, wxEXPAND, 5);

	ExportButton = new wxButton(this, wxID_ANY, wxT("Export"), wxDefaultPosition, wxDefaultSize, 0);
	ExportButton->Enable(false);

	bSizer14->Add(ExportButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer14->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	bSizer10->Add(bSizer14, 1, wxEXPAND, 5);


	SetSizer(bSizer10);
	Layout();

	Centre(wxBOTH);
	
	SpotLightMultiplierValue = ctx.Config.SpotLightMul;
	SpotLightMultiplier->SetValidator(wxFloatingPointValidator<float>(2, &SpotLightMultiplierValue, wxNUM_VAL_DEFAULT));
	PointLightMultiplierValue = ctx.Config.PointLightMul;
	PointLightMultiplier->SetValidator(wxFloatingPointValidator<float>(2, &PointLightMultiplierValue, wxNUM_VAL_DEFAULT));

	SetExportContext(ctx);
	m_notebook1->SetSelection(0);

	PathPicker->Connect(wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler(LevelExportOptionsWindow::OnDirChanged), NULL, this);
  DefaultsButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnDefaultsClicked), NULL, this);
  ExportButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnExportClicked), NULL, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnCancelClicked), NULL, this);
}

LevelExportOptionsWindow::~LevelExportOptionsWindow()
{
	PathPicker->Disconnect(wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler(LevelExportOptionsWindow::OnDirChanged), NULL, this);
  DefaultsButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnDefaultsClicked), NULL, this);
  ExportButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnExportClicked), NULL, this);
  CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnCancelClicked), NULL, this);
}

void LevelExportOptionsWindow::SetExportContext(const LevelExportContext& ctx)
{
	PathPicker->GetTextCtrl()->SetValue(ctx.Config.RootDir.WString());
	StaticMeshes->SetValue(ctx.Config.Statics);
	SkeletalMeshes->SetValue(ctx.Config.Skeletals);
	SpeedTrees->SetValue(ctx.Config.SpeedTrees);
	Interps->SetValue(ctx.Config.Interps);
	Terrains->SetValue(ctx.Config.Terrains);
	PointLights->SetValue(ctx.Config.PointLights);
	SpotLights->SetValue(ctx.Config.SpotLights);
	SoundNodes->SetValue(ctx.Config.Sounds);
	Emitters->SetValue(ctx.Config.Emitters);
	Materials->SetValue(ctx.Config.Materials);
	SpotLightMultiplierValue = ctx.Config.SpotLightMul;
	PointLightMultiplierValue = ctx.Config.PointLightMul;
	SpotLightMultiplier->GetValidator()->TransferToWindow();
	PointLightMultiplier->GetValidator()->TransferToWindow();
	LightInvSqrt->SetValue(ctx.Config.InvSqrtFalloff);
	ResampleTerrain->SetValue(ctx.Config.ResampleTerrain);
	SplitTerrainWeightMaps->SetValue(HasAVX2() && ctx.Config.SplitTerrainWeights);
	SplitTerrainWeightMaps->Enable(HasAVX2());
	Textures->SetValue(ctx.Config.Textures);
	TextureFormatSelector->SetSelection(HasAVX2() ? ctx.Config.TextureFormat : 2);
	TextureFormatSelector->Enable(HasAVX2());
	wxFileDirPickerEvent e;
	OnDirChanged(e);
}

LevelExportContext LevelExportOptionsWindow::GetExportContext() const
{
	LevelExportContext ctx;
	ctx.Config.RootDir = PathPicker->GetTextCtrl()->GetValue().ToStdWstring();
	ctx.Config.Statics = StaticMeshes->GetValue();
	ctx.Config.Skeletals = StaticMeshes->GetValue();
	ctx.Config.SpeedTrees = SpeedTrees->GetValue();
	ctx.Config.Interps = Interps->GetValue();
	ctx.Config.Terrains = Terrains->GetValue();
	ctx.Config.PointLights = PointLights->GetValue();
	ctx.Config.SpotLights = SpotLights->GetValue();
	ctx.Config.Sounds = SoundNodes->GetValue();
	ctx.Config.Emitters = Emitters->GetValue();
	ctx.Config.Materials = Materials->GetValue();
	PointLightMultiplier->GetValidator()->TransferFromWindow();
	ctx.Config.PointLightMul = PointLightMultiplierValue;
	SpotLightMultiplier->GetValidator()->TransferFromWindow();
	ctx.Config.SpotLightMul = SpotLightMultiplierValue;
	ctx.Config.InvSqrtFalloff = LightInvSqrt->GetValue();
	ctx.Config.ResampleTerrain = ResampleTerrain->GetValue();
	ctx.Config.SplitTerrainWeights = SplitTerrainWeightMaps->GetValue();
	ctx.Config.Textures = Textures->GetValue();
	ctx.Config.TextureFormat = TextureFormatSelector->GetSelection();
	return ctx;
}

void LevelExportOptionsWindow::OnDirChanged(wxFileDirPickerEvent& event)
{
	wxString path = PathPicker->GetPath();
	std::error_code err;
	ExportButton->Enable(path.size() && std::filesystem::exists(path.ToStdWstring(), err));
}

void LevelExportOptionsWindow::OnDefaultsClicked(wxCommandEvent& event)
{
	LevelExportContext ctx;
	ctx.Config.RootDir = App::GetSharedApp()->GetConfig().MapExportConfig.RootDir;
	SetExportContext(ctx);
}

void LevelExportOptionsWindow::OnExportClicked(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}

void LevelExportOptionsWindow::OnCancelClicked(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

LevelExportContext LevelExportContext::LoadFromAppConfig()
{
	LevelExportContext ctx;
	ctx.Config = App::GetSharedApp()->GetConfig().MapExportConfig;
	return ctx;
}

void LevelExportContext::SaveToAppConfig(const LevelExportContext& ctx)
{
	App::GetSharedApp()->GetConfig().MapExportConfig = ctx.Config;
	App::GetSharedApp()->SaveConfig();
}
