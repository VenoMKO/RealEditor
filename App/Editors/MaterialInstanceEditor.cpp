#include "MaterialInstanceEditor.h"
#include "../CustomViews/MaterialView.h"

#include <Tera/Cast.h>

MaterialInstanceEditor::MaterialInstanceEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel1;
  m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(m_panel1, wxID_ANY, wxT("Static Parameter Overrides:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText1->Wrap(-1);
  bSizer2->Add(m_staticText1, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));

  StaticParameterOverrides = new wxPropertyGridManager(m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPGMAN_DEFAULT_STYLE);
  StaticParameterOverrides->SetExtraStyle(wxPG_EX_MODE_BUTTONS | wxPG_EX_NATIVE_DOUBLE_BUFFERING);
  bSizer2->Add(StaticParameterOverrides, 1, wxEXPAND | wxALL, FromDIP(1));


  m_panel1->SetSizer(bSizer2);
  m_panel1->Layout();
  bSizer2->Fit(m_panel1);
  bSizer1->Add(m_panel1, 1, wxEXPAND | wxALL, FromDIP(5));

  SetSizer(bSizer1);
  Layout();
}

void MaterialInstanceEditor::OnObjectLoaded()
{
  if (Loading || NeedsGraph)
  {
    NeedsGraph = false;
    if (UMaterialInstance* mi = Cast<UMaterialInstance>(Object))
    {
      StaticParameterOverrides->Freeze();
      wxPropertyCategory* root = new wxPropertyCategory(Object->GetObjectNameString().WString(), wxT("Root"));
      StaticParameterOverrides->Append(root);

      wxPropertyCategory* cat = new wxPropertyCategory("Switches", wxT("SSwitches"));
      StaticParameterOverrides->AppendIn(root, cat);
      for (const FStaticSwitchParameter& param : mi->StaticParameters.StaticSwitchParameters)
      {
        wxBoolProperty* prop = new wxBoolProperty(param.ParameterName.String().WString(), wxString::Format("%016llx", (uint64)std::addressof(param)), param.Value);
        prop->Enable(false);
        StaticParameterOverrides->AppendIn(cat, prop);
      }

      cat = new wxPropertyCategory("Tex. Compression", wxT("SNormals"));
      StaticParameterOverrides->AppendIn(root, cat);
      for (const FNormalParameter& param : mi->StaticParameters.NormalParameters)
      {
        wxStringProperty* prop = new wxStringProperty(param.ParameterName.String().WString(), wxString::Format("%016llx", (uint64)std::addressof(param)), TextureCompressionSettingsToString(param.CompressionSettings).WString());
        prop->Enable(false);
        StaticParameterOverrides->AppendIn(cat, prop);
      }

      cat = new wxPropertyCategory("Layers", wxT("STWLayers"));
      StaticParameterOverrides->AppendIn(root, cat);
      for (const FStaticTerrainLayerWeightParameter& param : mi->StaticParameters.TerrainLayerWeightParameters)
      {
        wxIntProperty* prop = new wxIntProperty(param.ParameterName.String().WString(), wxString::Format("%016llx", (uint64)std::addressof(param)), param.WeightmapIndex);
        prop->Enable(false);
        StaticParameterOverrides->AppendIn(cat, prop);
      }

      cat = new wxPropertyCategory("Masks", wxT("SCMasks"));
      StaticParameterOverrides->AppendIn(root, cat);
      for (const FStaticComponentMaskParameter& param : mi->StaticParameters.StaticComponentMaskParameters)
      {
        wxString value;
        if (param.R)
        {
          value += "R,";
        }
        if (param.G)
        {
          value += "G,";
        }
        if (param.B)
        {
          value += "B,";
        }
        if (param.A)
        {
          value += "A,";
        }
        if (value.size())
        {
          value = value.substr(0, value.size() - 1);
        }
        wxStringProperty* prop = new wxStringProperty(param.ParameterName.String().WString(), wxString::Format("%016llx", (uint64)std::addressof(param)), value);
        prop->Enable(false);
        StaticParameterOverrides->AppendIn(cat, prop);
      }
      
      StaticParameterOverrides->Thaw();
    }
  }
  GenericEditor::OnObjectLoaded();
}
