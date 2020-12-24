#include "LevelExportOptions.h"
#include "../App.h"

#include <wx/notebook.h>
#include <wx/valnum.h>

struct ActorExportEntry {
  ActorExportEntry() = default;
  ActorExportEntry(const char* name, FMapExportConfig::ActorClass actor, bool state)
    : Name(name)
    , Actor(actor)
    , Enabled(state)
  {}

  wxString Name;
  FMapExportConfig::ActorClass Actor = FMapExportConfig::ActorClass::None;
  bool Enabled = false;
};

class ActorExportEntryModel : public wxDataViewVirtualListModel {
public:
  enum
  {
    Col_Check = 0,
    Col_Name,
    Col_Max
  };

  ActorExportEntryModel(const std::vector<ActorExportEntry>& entries)
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
    case Col_Name:
      variant = Rows[row].Name;
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

  std::vector<ActorExportEntry> GetRows() const
  {
    return Rows;
  }

  uint32 GetActorsMask() const
  {
    uint32 result = 0;
    for (const auto& item : Rows)
    {
      if (item.Enabled)
      {
        result |= (uint32)item.Actor;
      }
    }
    return result;
  }

private:
  std::vector<ActorExportEntry> Rows;
};

LevelExportOptionsWindow::LevelExportOptionsWindow(wxWindow* parent, const LevelExportContext& ctx)
  : wxDialog(parent, wxID_ANY, wxT("Export options"), wxDefaultPosition, wxSize(597, 576), wxDEFAULT_DIALOG_STYLE)
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

  wxPanel* m_panel111;
  m_panel111 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer17;
  bSizer17 = new wxBoxSizer(wxHORIZONTAL);

  ActorTable = new wxDataViewCtrl(m_panel111, wxID_ANY, wxDefaultPosition, wxSize(-1, 200), wxDV_NO_HEADER);
  bSizer17->Add(ActorTable, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizer16;
  bSizer16 = new wxBoxSizer(wxVERTICAL);

  TurnOnAllButton = new wxButton(m_panel111, wxID_ANY, wxT("All"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer16->Add(TurnOnAllButton, 0, wxALL, 5);

  TurnOffAllButton = new wxButton(m_panel111, wxID_ANY, wxT("None"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer16->Add(TurnOffAllButton, 0, wxALL, 5);


  bSizer17->Add(bSizer16, 0, wxEXPAND, 5);


  m_panel111->SetSizer(bSizer17);
  m_panel111->Layout();
  bSizer17->Fit(m_panel111);
  bSizer10->Add(m_panel111, 1, wxEXPAND | wxALL, 5);

  wxStaticText* m_staticText511;
  m_staticText511 = new wxStaticText(this, wxID_ANY, wxT("Options"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText511->Wrap(-1);
  m_staticText511->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer10->Add(m_staticText511, 0, wxTOP | wxRIGHT | wxLEFT, 5);

  wxNotebook* m_notebook1;
  m_notebook1 = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
  m_notebook1->SetMinSize(wxSize(-1, 100));

  wxPanel* m_panel10;
  m_panel10 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer141;
  bSizer141 = new wxBoxSizer(wxHORIZONTAL);

  OverrideFiles = new wxCheckBox(m_panel10, wxID_ANY, wxT("Override files"), wxDefaultPosition, wxDefaultSize, 0);
  OverrideFiles->SetToolTip(wxT("Override existing data files(e.g., fbx)"));

  bSizer141->Add(OverrideFiles, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  IgnoreHidden = new wxCheckBox(m_panel10, wxID_ANY, wxT("Make all visible"), wxDefaultPosition, wxDefaultSize, 0);
  IgnoreHidden->SetToolTip(wxT("Make hidden objects visible. If this is disabled, hidden actors will be shown in the World Outliner, but UE4 won't show them in the scene view."));

  bSizer141->Add(IgnoreHidden, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  SplitT3d = new wxCheckBox(m_panel10, wxID_ANY, wxT("Split T3D"), wxDefaultPosition, wxDefaultSize, 0);
  SplitT3d->SetToolTip(wxT("Save each streamed level to it's own T3D file."));

  bSizer141->Add(SplitT3d, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  m_panel10->SetSizer(bSizer141);
  m_panel10->Layout();
  bSizer141->Fit(m_panel10);
  m_notebook1->AddPage(m_panel10, wxT("General"), true);
  wxPanel* m_panel11;
  m_panel11 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer26;
  bSizer26 = new wxBoxSizer(wxHORIZONTAL);

  Materials = new wxCheckBox(m_panel11, wxID_ANY, wxT("Export Materials"), wxDefaultPosition, wxDefaultSize, 0);
  Materials->SetToolTip(wxT("Export parameters of materials used by actors. Will export materials that are related to current export operation."));

  bSizer26->Add(Materials, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  Textures = new wxCheckBox(m_panel11, wxID_ANY, wxT("Export textures:"), wxDefaultPosition, wxDefaultSize, 0);
  Textures->SetToolTip(wxT("Export textures, used by exported actors. Will export textures that are related to the current export operation."));

  bSizer26->Add(Textures, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  wxString TextureFormatSelectorChoices[] = { wxT("TGA"), wxT("PNG"), wxT("DDS") };
  int TextureFormatSelectorNChoices = sizeof(TextureFormatSelectorChoices) / sizeof(wxString);
  TextureFormatSelector = new wxChoice(m_panel11, wxID_ANY, wxDefaultPosition, wxSize(100, -1), TextureFormatSelectorNChoices, TextureFormatSelectorChoices, 0);
  TextureFormatSelector->SetSelection(0);
  TextureFormatSelector->SetToolTip(wxT("Format used to save textures."));

  bSizer26->Add(TextureFormatSelector, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


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

  PointLightMultiplier = new wxTextCtrl(m_panel8, wxID_ANY, wxT("1.0"), wxDefaultPosition, wxSize(30, -1), 0);
  PointLightMultiplier->SetToolTip(wxT("Multiply point light intensity by this value."));

  bSizer18->Add(PointLightMultiplier, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  m_panel8->SetSizer(bSizer18);
  m_panel8->Layout();
  bSizer18->Fit(m_panel8);
  bSizer24->Add(m_panel8, 0, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 0);

  wxPanel* m_panel9;
  m_panel9 = new wxPanel(m_panel12, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer19;
  bSizer19 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText151;
  m_staticText151 = new wxStaticText(m_panel9, wxID_ANY, wxT("Spot lights scale:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText151->Wrap(-1);
  bSizer19->Add(m_staticText151, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);

  SpotLightMultiplier = new wxTextCtrl(m_panel9, wxID_ANY, wxT("1.0"), wxDefaultPosition, wxSize(30, -1), 0);
  SpotLightMultiplier->SetToolTip(wxT("Multiply spot light intensity by this value."));

  bSizer19->Add(SpotLightMultiplier, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  m_panel9->SetSizer(bSizer19);
  m_panel9->Layout();
  bSizer19->Fit(m_panel9);
  bSizer24->Add(m_panel9, 0, wxEXPAND | wxALL | wxALIGN_CENTER_VERTICAL, 0);


  bSizer23->Add(bSizer24, 1, wxEXPAND | wxALIGN_CENTER_HORIZONTAL, 5);

  LightInvSqrt = new wxCheckBox(m_panel12, wxID_ANY, wxT("Use inverse squared falloff"), wxDefaultPosition, wxDefaultSize, 0);
  LightInvSqrt->SetToolTip(wxT("Use physically based inverse squared distance falloff."));

  bSizer23->Add(LightInvSqrt, 0, wxALL, 5);


  m_panel12->SetSizer(bSizer23);
  m_panel12->Layout();
  bSizer23->Fit(m_panel12);
  m_notebook1->AddPage(m_panel12, wxT("Lights"), false);
  wxPanel* m_panel14;
  m_panel14 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer27;
  bSizer27 = new wxBoxSizer(wxHORIZONTAL);

  ResampleTerrain = new wxCheckBox(m_panel14, wxID_ANY, wxT("Resample Terrain"), wxDefaultPosition, wxDefaultSize, 0);
  ResampleTerrain->SetToolTip(wxT("Resize terrain heightmap to match weightmap resolution. Allows to use original full resolution weightmaps, but reduces accuracy of the terrain geometry."));

  bSizer27->Add(ResampleTerrain, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  SplitTerrainWeightMaps = new wxCheckBox(m_panel14, wxID_ANY, wxT("Split weightmaps"), wxDefaultPosition, wxDefaultSize, 0);
  SplitTerrainWeightMaps->SetToolTip(wxT("Save weightmap layers to individual files."));

  bSizer27->Add(SplitTerrainWeightMaps, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  m_panel14->SetSizer(bSizer27);
  m_panel14->Layout();
  bSizer27->Fit(m_panel14);
  m_notebook1->AddPage(m_panel14, wxT("Terrain"), false);
  wxPanel* m_panel121;
  m_panel121 = new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer181;
  bSizer181 = new wxBoxSizer(wxHORIZONTAL);

  ExportLods = new wxCheckBox(m_panel121, wxID_ANY, wxT("Export LODs"), wxDefaultPosition, wxDefaultSize, 0);
  ExportLods->SetToolTip(wxT("Embed model LODs into fbx"));

  bSizer181->Add(ExportLods, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  ConvexCollisions = new wxCheckBox(m_panel121, wxID_ANY, wxT("Export collisions"), wxDefaultPosition, wxDefaultSize, 0);
  ConvexCollisions->SetToolTip(wxT("Export Rigid Body collisions."));

  bSizer181->Add(ConvexCollisions, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  m_panel121->SetSizer(bSizer181);
  m_panel121->Layout();
  bSizer181->Fit(m_panel121);
  m_notebook1->AddPage(m_panel121, wxT("Models"), false);

  bSizer10->Add(m_notebook1, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer* bSizer14;
  bSizer14 = new wxBoxSizer(wxHORIZONTAL);

  DefaultsButton = new wxButton(this, wxID_ANY, wxT("Defaults"), wxDefaultPosition, wxDefaultSize, 0);
  DefaultsButton->SetToolTip(wxT("Restore default settings."));

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
  SpotLightMultiplier->SetValidator(wxFloatingPointValidator<float>(1, &SpotLightMultiplierValue, wxNUM_VAL_DEFAULT));
  PointLightMultiplierValue = ctx.Config.PointLightMul;
  PointLightMultiplier->SetValidator(wxFloatingPointValidator<float>(1, &PointLightMultiplierValue, wxNUM_VAL_DEFAULT));

  SetExportContext(ctx);
  m_notebook1->SetSelection(0);

  DelayedTextureFormat = HasAVX2() ? ctx.Config.TextureFormat : 2;
  TextureFormatSelector->Enable(HasAVX2());

  ActorTable->AppendToggleColumn(_(""), ActorExportEntryModel::Col_Check, wxDATAVIEW_CELL_ACTIVATABLE, 25);
  ActorTable->AppendTextColumn(_("Actor Type"), ActorExportEntryModel::Col_Name, wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
  ActorTable->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEventHandler(LevelExportOptionsWindow::OnActorTableValueChanged), NULL, this);

  TurnOnAllButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnAllClicked), NULL, this);
  TurnOffAllButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnNoneClicked), NULL, this);

  PathPicker->Connect(wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler(LevelExportOptionsWindow::OnDirChanged), NULL, this);
  DefaultsButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnDefaultsClicked), NULL, this);
  ExportButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnExportClicked), NULL, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnCancelClicked), NULL, this);
  Connect(wxEVT_IDLE, wxIdleEventHandler(LevelExportOptionsWindow::OnFirstIdle), NULL, this);
}

LevelExportOptionsWindow::~LevelExportOptionsWindow()
{
  ActorTable->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEventHandler(LevelExportOptionsWindow::OnActorTableValueChanged), NULL, this);
  TurnOnAllButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnAllClicked), NULL, this);
  TurnOffAllButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnNoneClicked), NULL, this);
  PathPicker->Disconnect(wxEVT_COMMAND_DIRPICKER_CHANGED, wxFileDirPickerEventHandler(LevelExportOptionsWindow::OnDirChanged), NULL, this);
  DefaultsButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnDefaultsClicked), NULL, this);
  ExportButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnExportClicked), NULL, this);
  CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LevelExportOptionsWindow::OnCancelClicked), NULL, this);
}

void LevelExportOptionsWindow::SetExportContext(const LevelExportContext& ctx)
{
  PathPicker->GetTextCtrl()->SetValue(ctx.Config.RootDir.WString());
  FillActorsTable(ctx.Config);

  SplitT3d->SetValue(ctx.Config.SplitT3D);
  Materials->SetValue(ctx.Config.Materials);
  LightInvSqrt->SetValue(ctx.Config.InvSqrtFalloff);
  ResampleTerrain->SetValue(ctx.Config.ResampleTerrain);
  SplitTerrainWeightMaps->SetValue(HasAVX2() && ctx.Config.SplitTerrainWeights);
  SplitTerrainWeightMaps->Enable(HasAVX2());
  Textures->SetValue(ctx.Config.Textures);
  if (DelayedTextureFormat == -1)
  {
    TextureFormatSelector->Select(HasAVX2() ? ctx.Config.TextureFormat : 2);
  }
  else
  {
    DelayedTextureFormat = HasAVX2() ? ctx.Config.TextureFormat : 2;
  }
  TextureFormatSelector->Enable(HasAVX2());
  OverrideFiles->SetValue(ctx.Config.OverrideData);
  ExportLods->SetValue(ctx.Config.ExportLods);
  ConvexCollisions->SetValue(ctx.Config.ConvexCollisions);
  IgnoreHidden->SetValue(ctx.Config.IgnoreHidden);

  SpotLightMultiplierValue = ctx.Config.SpotLightMul;
  PointLightMultiplierValue = ctx.Config.PointLightMul;
  SpotLightMultiplier->GetValidator()->TransferToWindow();
  PointLightMultiplier->GetValidator()->TransferToWindow();

  wxFileDirPickerEvent e;
  OnDirChanged(e);
}

LevelExportContext LevelExportOptionsWindow::GetExportContext() const
{
  LevelExportContext ctx;
  ctx.Config.RootDir = PathPicker->GetTextCtrl()->GetValue().ToStdWstring();
  ctx.Config.ActorClasses = GetActorsTableMask();

  ctx.Config.SplitT3D = SplitT3d->GetValue();
  ctx.Config.Materials = Materials->GetValue();
  ctx.Config.InvSqrtFalloff = LightInvSqrt->GetValue();
  ctx.Config.ResampleTerrain = ResampleTerrain->GetValue();
  ctx.Config.SplitTerrainWeights = SplitTerrainWeightMaps->GetValue();
  ctx.Config.Textures = Textures->GetValue();
  ctx.Config.TextureFormat = TextureFormatSelector->GetSelection();
  ctx.Config.OverrideData = OverrideFiles->GetValue();
  ctx.Config.ExportLods = ExportLods->GetValue();
  ctx.Config.ConvexCollisions = ConvexCollisions->GetValue();
  ctx.Config.IgnoreHidden = IgnoreHidden->GetValue();

  PointLightMultiplier->GetValidator()->TransferFromWindow();
  SpotLightMultiplier->GetValidator()->TransferFromWindow();
  ctx.Config.PointLightMul = PointLightMultiplierValue;
  ctx.Config.SpotLightMul = SpotLightMultiplierValue;

  return ctx;
}

void LevelExportOptionsWindow::FillActorsTable(const FMapExportConfig& cfg)
{
  std::vector<ActorExportEntry> list;
  list.emplace_back("Static Meshes", FMapExportConfig::ActorClass::StaticMeshes, cfg.GetClassEnabled(FMapExportConfig::ActorClass::StaticMeshes));
  list.emplace_back("Skeletal Meshes", FMapExportConfig::ActorClass::SkeletalMeshes, cfg.GetClassEnabled(FMapExportConfig::ActorClass::SkeletalMeshes));
  list.emplace_back("Interp Actors", FMapExportConfig::ActorClass::Interps, cfg.GetClassEnabled(FMapExportConfig::ActorClass::Interps));
  list.emplace_back("SpeedTrees", FMapExportConfig::ActorClass::SpeedTrees, cfg.GetClassEnabled(FMapExportConfig::ActorClass::SpeedTrees));
  list.emplace_back("Prefabs", FMapExportConfig::ActorClass::Prefabs, cfg.GetClassEnabled(FMapExportConfig::ActorClass::Prefabs));
  list.emplace_back("Terrains", FMapExportConfig::ActorClass::Terrains, cfg.GetClassEnabled(FMapExportConfig::ActorClass::Terrains));
  list.emplace_back("Point Lights", FMapExportConfig::ActorClass::PointLights, cfg.GetClassEnabled(FMapExportConfig::ActorClass::PointLights));
  list.emplace_back("Spot Lights", FMapExportConfig::ActorClass::SpotLights, cfg.GetClassEnabled(FMapExportConfig::ActorClass::SpotLights));
  list.emplace_back("Directional Lights", FMapExportConfig::ActorClass::DirectionalLights, cfg.GetClassEnabled(FMapExportConfig::ActorClass::DirectionalLights));
  list.emplace_back("Sky Lights", FMapExportConfig::ActorClass::SkyLights, cfg.GetClassEnabled(FMapExportConfig::ActorClass::SkyLights));
  list.emplace_back("Emitter Nodes", FMapExportConfig::ActorClass::Emitters, cfg.GetClassEnabled(FMapExportConfig::ActorClass::Emitters));
  list.emplace_back("Height Fog", FMapExportConfig::ActorClass::HeightFog, cfg.GetClassEnabled(FMapExportConfig::ActorClass::HeightFog));
  // TODO: implement
  // list.emplace_back("Sound Nodes", FMapExportConfig::ActorClass::Sounds, cfg.GetClassEnabled(FMapExportConfig::ActorClass::Sounds));

  ActorTable->AssociateModel(new ActorExportEntryModel(list));
}

int32 LevelExportOptionsWindow::GetActorsTableMask() const
{
  return ((const ActorExportEntryModel*)ActorTable->GetModel())->GetActorsMask();
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

void LevelExportOptionsWindow::OnAllClicked(wxCommandEvent&)
{
  FMapExportConfig tmp;
  tmp.ActorClasses = (uint32)FMapExportConfig::ActorClass::All;
  FillActorsTable(tmp);
}

void LevelExportOptionsWindow::OnNoneClicked(wxCommandEvent&)
{
  FMapExportConfig tmp;
  tmp.ActorClasses = (uint32)FMapExportConfig::ActorClass::None;
  FillActorsTable(tmp);
}

void LevelExportOptionsWindow::OnActorTableValueChanged(wxDataViewEvent&)
{
  LevelExportContext::SaveToAppConfig(GetExportContext());
}

void LevelExportOptionsWindow::OnFirstIdle(wxIdleEvent&)
{
  Disconnect(wxEVT_IDLE, wxIdleEventHandler(LevelExportOptionsWindow::OnFirstIdle), NULL, this);
  TextureFormatSelector->SetSelection(DelayedTextureFormat);
  DelayedTextureFormat = -1;
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
