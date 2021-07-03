#include "AnimationEditor.h"
#include "../App.h"
#include "../Windows/ObjectPicker.h"
#include "../Windows/REDialogs.h"
#include "../Windows/ProgressWindow.h"

#include <Tera/Cast.h>
#include <Tera/FName.h>
#include <Tera/FObjectResource.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/UAnimation.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/FPackage.h>

#include <Utils/AConfiguration.h>
#include <Utils/FbxUtils.h>

#include <wx/valnum.h>

#include <filesystem>

class AnimExportOptions : public wxDialog {
public:
  AnimExportOptions(wxWindow* parent, UAnimSet* anim, USkeletalMesh* mesh)
    : wxDialog(parent, wxID_ANY, wxT("Export options"), wxDefaultPosition, wxSize(562, 198))
    , AnimationSet(anim)
    , Mesh(mesh)
    , DefaultMesh(anim->GetPreviewSkeletalMesh())
  {
    SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxPanel* m_panel1;
    m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    bSizer1->Add(m_panel1, 0, wxEXPAND | wxALL, 5);

    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* m_staticText2;
    m_staticText2 = new wxStaticText(this, wxID_ANY, wxT("Skeleton:"), wxDefaultPosition, wxSize(60, -1), wxALIGN_RIGHT);
    m_staticText2->Wrap(-1);
    bSizer2->Add(m_staticText2, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);

    SkeletonField = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    bSizer2->Add(SkeletonField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    SelectButton = new wxButton(this, wxID_ANY, wxT("Select..."), wxDefaultPosition, wxDefaultSize, 0);
    bSizer2->Add(SelectButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


    bSizer1->Add(bSizer2, 0, wxEXPAND, 5);

    wxBoxSizer* bSizer4;
    bSizer4 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* m_staticText4;
    m_staticText4 = new wxStaticText(this, wxID_ANY, wxT("Scale:"), wxDefaultPosition, wxSize(60, -1), wxALIGN_RIGHT);
    m_staticText4->Wrap(-1);
    bSizer4->Add(m_staticText4, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);

    Scale = new wxTextCtrl(this, wxID_ANY, wxT("1.0"), wxDefaultPosition, wxSize(40, -1), 0);
    Scale->SetToolTip(wxT("Uniform skeleton scale"));

    bSizer4->Add(Scale, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    wxStaticText* m_staticText3;
    m_staticText3 = new wxStaticText(this, wxID_ANY, wxT("Rate:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText3->Wrap(-1);
    bSizer4->Add(m_staticText3, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);

    Rate = new wxTextCtrl(this, wxID_ANY, wxT("1.0"), wxDefaultPosition, wxSize(40, -1), 0);
    Rate->SetToolTip(wxT("Inverse animation duration modifier. Bigger value makes the animation last longer"));

    bSizer4->Add(Rate, 0, wxALL, 5);


    bSizer1->Add(bSizer4, 0, wxEXPAND, 5);

    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer(wxHORIZONTAL);

    wxPanel* m_panel2;
    m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(56, -1), wxTAB_TRAVERSAL);
    bSizer5->Add(m_panel2, 0, wxEXPAND | wxALL, 5);

    ExportGeometry = new wxCheckBox(this, wxID_ANY, wxT("Export geometry"), wxDefaultPosition, wxDefaultSize, 0);
    ExportGeometry->SetValue(true);
    ExportGeometry->SetToolTip(wxT("Export 3D model with the skeleton"));

    bSizer5->Add(ExportGeometry, 0, wxRIGHT | wxLEFT, 5);

    Split = new wxCheckBox(this, wxID_ANY, wxT("Split takes"), wxDefaultPosition, wxDefaultSize, 0);
    Split->SetValue(true);
    Split->SetToolTip(wxT("Split animations to separate FBX files"));

    bSizer5->Add(Split, 0, wxRIGHT | wxLEFT, 5);

    Compress = new wxCheckBox(this, wxID_ANY, wxT("Compress"), wxDefaultPosition, wxDefaultSize, 0);
    Compress->SetValue(true);
    Compress->SetToolTip(wxT("Compress animation tracks by removing trivial keys"));

    bSizer5->Add(Compress, 0, wxRIGHT | wxLEFT, 5);

    Resample = new wxCheckBox(this, wxID_ANY, wxT("Resample"), wxDefaultPosition, wxDefaultSize, 0);
    Resample->SetToolTip(wxT("Resample animations to 60 FPS"));

    bSizer5->Add(Resample, 0, wxRIGHT | wxLEFT, 5);

    InvqW = new wxCheckBox(this, wxID_ANY, wxT("Inverse qW"), wxDefaultPosition, wxDefaultSize, 0);
    InvqW->SetToolTip(wxT("Inverse quat W when exporting. Enable this if your skeleton has orientation issues."));

    bSizer5->Add(InvqW, 0, wxRIGHT | wxLEFT, 5);


    bSizer1->Add(bSizer5, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);

    wxBoxSizer* bSizer3;
    bSizer3 = new wxBoxSizer(wxHORIZONTAL);

    DefaultButton = new wxButton(this, wxID_ANY, wxT("Default"), wxDefaultPosition, wxDefaultSize, 0);
    DefaultButton->Enable(false);

    bSizer3->Add(DefaultButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


    bSizer3->Add(0, 0, 1, wxEXPAND, 5);

    OkButton = new wxButton(this, wxID_ANY, wxT("Export..."), wxDefaultPosition, wxDefaultSize, 0);
    bSizer3->Add(OkButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer3->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


    bSizer1->Add(bSizer3, 1, wxEXPAND, 15);


    SetSizer(bSizer1);
    Layout();

    Centre(wxBOTH);

    // Connect Events
    SelectButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimExportOptions::OnSelectClicked), NULL, this);
    DefaultButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimExportOptions::OnDefaultClicked), NULL, this);
    OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimExportOptions::OnOkClicked), NULL, this);
    CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimExportOptions::OnCancelClicked), NULL, this);

    Scale->SetValidator(wxFloatingPointValidator<float>(1, &ScaleValue, wxNUM_VAL_DEFAULT));
    Rate->SetValidator(wxFloatingPointValidator<float>(1, &RateValue, wxNUM_VAL_DEFAULT));

    DefaultButton->Enable(DefaultMesh);
    if (DefaultMesh && !Mesh)
    {
      Mesh = DefaultMesh;
    }
    if (Mesh)
    {
      SkeletonField->SetValue(Mesh->GetObjectPath().WString());
    }
    OkButton->Enable(Mesh);
  }

  ~AnimExportOptions()
  {
    SelectButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimExportOptions::OnSelectClicked), NULL, this);
    DefaultButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimExportOptions::OnDefaultClicked), NULL, this);
    OkButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimExportOptions::OnOkClicked), NULL, this);
    CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimExportOptions::OnCancelClicked), NULL, this);
  }

  USkeletalMesh* GetMesh() const
  {
    return Mesh;
  }

  void GetConfig(FbxExportContext& ctx)
  {
    Scale->GetValidator()->TransferFromWindow();
    Rate->GetValidator()->TransferFromWindow();
    ctx.ExportMesh = ExportGeometry->GetValue();
    ctx.Scale3D = FVector(ScaleValue);
    ctx.TrackRateScale = RateValue;
    ctx.CompressTracks = Compress->GetValue();
    ctx.SplitTakes = Split->GetValue();
    ctx.ResampleTracks = Resample->GetValue();
    ctx.InverseAnimQuatW = InvqW->GetValue();
  }

  void ApplyConfig(const FAnimationExportConfig& cfg)
  {
    ScaleValue = cfg.ScaleFactor;
    RateValue = cfg.RateFactor;
    ExportGeometry->SetValue(cfg.ExportMesh);
    Compress->SetValue(cfg.Compress);
    Split->SetValue(cfg.Split);
    Resample->SetValue(cfg.Resample);
    Scale->GetValidator()->TransferToWindow();
    Rate->GetValidator()->TransferToWindow();
    InvqW->SetValue(cfg.InverseQuatW);
  }

  void SaveConfig(FAnimationExportConfig& cfg)
  {
    Scale->GetValidator()->TransferFromWindow();
    Rate->GetValidator()->TransferFromWindow();
    cfg.ScaleFactor = ScaleValue;
    cfg.RateFactor = RateValue;
    cfg.ExportMesh = ExportGeometry->GetValue();
    cfg.Compress = Compress->GetValue();
    cfg.Split = Split->GetValue();
    cfg.Resample = Resample->GetValue();
    cfg.InverseQuatW = InvqW->GetValue();
  }

  void AllowSplit(bool flag)
  {
    Split->Enable(flag);
  }

protected:
  void OnSelectClicked(wxCommandEvent& event)
  {
    ObjectPicker* picker = nullptr;
    if (Mesh)
    {
      picker = new ObjectPicker(this, wxT("Select a skeletal mesh..."), true, Mesh->GetPackage()->Ref(), Mesh->GetPackage()->GetObjectIndex(Mesh), { USkeletalMesh::StaticClassName() });
    }
    else
    {
      picker = new ObjectPicker(this, wxT("Select a skeletal mesh..."), true, AnimationSet->GetPackage()->Ref(), INDEX_NONE, { USkeletalMesh::StaticClassName() });
    }
    picker->SetCanChangePackage(true);
    USkeletalMesh* mesh = nullptr;
    while (!mesh && picker->ShowModal() == wxID_OK)
    {
      if (mesh = Cast<USkeletalMesh>(picker->GetSelectedObject()))
      {
        if (AnimationSet->GetSkeletalMeshMatchRatio(mesh) == 0.)
        {
          REDialog::Error(wxS("Provided skeleton does not share bones with the animation. Select a different skeleton."), wxS("Can't use ") + mesh->GetObjectNameString().WString() + wxS("!"));
          mesh = nullptr;
          continue;
        }
        SkeletonField->SetValue(mesh->GetObjectPath().WString());
      }
    }
    if (mesh)
    {
      Mesh = mesh;
    }
    OkButton->Enable(Mesh);
  }

  void OnDefaultClicked(wxCommandEvent& event)
  {
    if (DefaultMesh)
    {
      Mesh = DefaultMesh;
      OkButton->Enable(Mesh);
      SkeletonField->SetValue(Mesh->GetObjectPath().WString());
    }
  }

  void OnOkClicked(wxCommandEvent& event)
  {
    EndModal(wxID_OK);
  }

  void OnCancelClicked(wxCommandEvent& event)
  {
    EndModal(wxID_CANCEL);
  }

protected:
  wxTextCtrl* SkeletonField = nullptr;
  wxButton* SelectButton = nullptr;
  wxTextCtrl* Scale = nullptr;
  wxTextCtrl* Rate = nullptr;
  wxCheckBox* ExportGeometry = nullptr;
  wxCheckBox* Split = nullptr;
  wxCheckBox* Compress = nullptr;
  wxCheckBox* Resample = nullptr;
  wxCheckBox* InvqW = nullptr;
  wxButton* DefaultButton = nullptr;
  wxButton* OkButton = nullptr;
  wxButton* CancelButton = nullptr;

  UAnimSet* AnimationSet = nullptr;
  USkeletalMesh* DefaultMesh = nullptr;
  USkeletalMesh* Mesh = nullptr;

  float ScaleValue = 1.;
  float RateValue = 1.;
};

void AnimSetEditor::PopulateToolBar(wxToolBar* toolbar)
{
  toolbar->AddTool(eID_Export, "Export", wxBitmap("#112", wxBITMAP_TYPE_PNG_RESOURCE), "Export object data...");
  GenericEditor::PopulateToolBar(toolbar);
}

void AnimSetEditor::OnExportClicked(wxCommandEvent& e)
{
  UAnimSet* set = Cast<UAnimSet>(Object);
  USkeletalMesh* source = nullptr;
  std::vector<FObjectExport*> allExports = Object->GetPackage()->GetAllExports();
  for (FObjectExport* exp : allExports)
  {
    if (exp->GetClassName() == USkeletalMesh::StaticClassName())
    {
      USkeletalMesh* skelMesh = Cast<USkeletalMesh>(Object->GetPackage()->GetObject(exp));
      if (set->GetSkeletalMeshMatchRatio(skelMesh))
      {
        source = skelMesh;
        break;
      }
    }
  }
  AnimExportOptions opts(this, set, source);
  FAppConfig& cfg = App::GetSharedApp()->GetConfig();
  opts.ApplyConfig(cfg.AnimationExportConfig);
  if (opts.ShowModal() != wxID_OK)
  {
    return;
  }
  opts.SaveConfig(cfg.AnimationExportConfig);
  App::GetSharedApp()->SaveConfig();
  source = opts.GetMesh();
  FbxExportContext ctx;
  opts.GetConfig(ctx);
  if (ctx.SplitTakes)
  {
    ctx.Path = IODialog::SaveAnimDirDialog(this, set->GetObjectNameString().WString()).ToStdWstring();
  }
  else
  {
    ctx.Path = IODialog::SaveAnimDialog(this, set->GetObjectNameString().WString()).ToStdWstring();
  }
  if (ctx.Path.empty())
  {
    return;
  }
  
  ProgressWindow progress(this, "Exporting animations");
  progress.SetCurrentProgress(-1);
  progress.SetActionText(wxT("Preparing..."));
  progress.SetCanCancel(false);

  ctx.ProgressDescFunc = [&](const FString desc) {
    SendEvent(&progress, UPDATE_PROGRESS_DESC, desc.WString());
  };

  ctx.ProgressMaxFunc = [&](int32 max) {
    SendEvent(&progress, UPDATE_MAX_PROGRESS, max);
  };

  ctx.ProgressFunc = [&](int32 val) {
    SendEvent(&progress, UPDATE_PROGRESS, val);
  };
  
  std::thread([&]() {
    bool result;
    {
      if (ctx.SplitTakes)
      {
        std::filesystem::path p(ctx.Path);
        p /= Object->GetObjectNameString().WString();
        ctx.Path = p.wstring();
        try
        {
          std::filesystem::create_directories(p);
        }
        catch (...)
        {}

        UAnimSet* set = Cast<UAnimSet>(Object);
        std::vector<UObject*> inner = set->GetInner();
        ctx.ProgressMaxFunc(inner.size());
        int32 total = (int32)inner.size();
        for (int32 idx = 0; idx < inner.size(); ++idx)
        {
          ctx.ProgressFunc(idx + 1);
          if (UAnimSequence* seq = Cast<UAnimSequence>(inner[idx]))
          {
            if (ctx.ProgressDescFunc)
            {
              ctx.ProgressDescFunc(FString::Sprintf("Exporting: %s(%d/%d)", seq->SequenceName.String().C_str(), idx + 1, total));
            }
            ctx.Path = (p / seq->SequenceName.String().WString()).replace_extension("fbx").wstring();
            FbxUtils fbx;
            if (!(result = fbx.ExportAnimationSequence(source, seq, ctx)))
            {
              break;
            }
          }
        }
      }
      else
      {
        FbxUtils fbx;
        result = fbx.ExportAnimationSet(source, Cast<UAnimSet>(Object), ctx);
      }
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH, result);
  }).detach();

  if (!progress.ShowModal())
  {
    REDialog::Error(ctx.Error, wxS("Failed to export ") + Object->GetObjectNameString().WString(), this);
  }
}

void AnimSequenceEditor::OnExportClicked(wxCommandEvent& e)
{
  UAnimSet* set = Object->GetTypedOuter<UAnimSet>();
  set->Load();
  USkeletalMesh* source = nullptr;
  std::vector<FObjectExport*> allExports = Object->GetPackage()->GetAllExports();
  for (FObjectExport* exp : allExports)
  {
    if (exp->GetClassName() == USkeletalMesh::StaticClassName())
    {
      USkeletalMesh* skelMesh = Cast<USkeletalMesh>(Object->GetPackage()->GetObject(exp));
      if (set->GetSkeletalMeshMatchRatio(skelMesh))
      {
        source = skelMesh;
        break;
      }
    }
  }
  AnimExportOptions opts(this, set, source);
  FAppConfig& cfg = App::GetSharedApp()->GetConfig();
  opts.AllowSplit(false);
  opts.ApplyConfig(cfg.AnimationExportConfig);
  if (opts.ShowModal() != wxID_OK)
  {
    return;
  }
  opts.SaveConfig(cfg.AnimationExportConfig);
  App::GetSharedApp()->SaveConfig();
  source = opts.GetMesh();
  FbxExportContext ctx;
  opts.GetConfig(ctx);
  ctx.Path = IODialog::SaveAnimDialog(this, Cast<UAnimSequence>(Object)->SequenceName.String().WString()).ToStdWstring();
  if (ctx.Path.empty())
  {
    return;
  }

  ProgressWindow progress(this, "Exporting animations");
  progress.SetCurrentProgress(-1);
  progress.SetActionText(wxT("Savings..."));
  progress.SetCanCancel(false);

  ctx.ProgressDescFunc = [&](const FString desc) {
    SendEvent(&progress, UPDATE_PROGRESS_DESC, desc.WString());
  };

  std::thread([&]() {
    bool result;
    {
      FbxUtils fbx;
      result = fbx.ExportAnimationSequence(source, Cast<UAnimSequence>(Object), ctx);
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH, result);
  }).detach();

  if (!progress.ShowModal())
  {
    REDialog::Error(ctx.Error, wxS("Failed to export ") + Object->GetObjectNameString().WString(), this);
  }
}
