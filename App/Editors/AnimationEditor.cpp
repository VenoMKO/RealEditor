#include "AnimationEditor.h"
#include "../App.h"
#include "../Misc/WXDialog.h"
#include "../Windows/ObjectPicker.h"
#include "../Windows/REDialogs.h"
#include "../Windows/ProgressWindow.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osg/Depth>

#include <osg/LineWidth>
#include <osgAnimation/Bone>
#include <osgAnimation/Skeleton>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/StackedTranslateElement>
#include <osgAnimation/StackedQuaternionElement>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/RigGeometry>

#include <Tera/Cast.h>
#include <Tera/FName.h>
#include <Tera/FObjectResource.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/UAnimation.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/UTexture.h>
#include <Tera/FPackage.h>

#include <Utils/AConfiguration.h>
#include <Utils/FbxUtils.h>

#include <wx/valnum.h>
#include <wx/statline.h>

#include <filesystem>

static const osg::Vec3d YawAxis(0.0, 0.0, -1.0);
static const osg::Vec3d PitchAxis(0.0, -1.0, 0.0);
static const osg::Vec3d RollAxis(1.0, 0.0, 0.0);

static const osg::Vec4 BoneColor(1., 1., 1., 1.);
static const float BoneWidth = 2.f;

namespace
{
  osg::Geode* CreateBoneShape(const FVector& pos)
  {
    osg::Vec3 trans(pos.X, -pos.Y, pos.Z);
    osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array;
    va->push_back(osg::Vec3());
    va->push_back(trans);
    osg::ref_ptr<osg::Vec4Array> ca = new osg::Vec4Array;
    ca->push_back(BoneColor);
    osg::ref_ptr<osg::Geometry> line = new osg::Geometry;
    line->setVertexArray(va.get());
    line->setColorArray(ca.get());
    line->setColorBinding(osg::Geometry::BIND_OVERALL);
    line->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(line.get());
    geode->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(BoneWidth));
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    return geode.release();
  }

  osgAnimation::Bone* CreateBone(const FMeshBone& refBone, osg::Group* parent)
  {
    osg::Vec3 trans(refBone.BonePos.Position.X, -refBone.BonePos.Position.Y, refBone.BonePos.Position.Z);
    osg::Quat orient(refBone.BonePos.Orientation.X, -refBone.BonePos.Orientation.Y, refBone.BonePos.Orientation.Z, -refBone.BonePos.Orientation.W);
    osg::ref_ptr<osgAnimation::Bone> bone = new osgAnimation::Bone;
    parent->insertChild(0, bone.get());
    parent->addChild(CreateBoneShape(refBone.BonePos.Position));
    osg::ref_ptr<osgAnimation::UpdateBone> updater = new osgAnimation::UpdateBone(refBone.Name.String().C_str());
    updater->getStackedTransforms().push_back(new osgAnimation::StackedTranslateElement("translate", trans));
    updater->getStackedTransforms().push_back(new osgAnimation::StackedQuaternionElement("quaternion", orient));
    bone->setUpdateCallback(updater.get());
    bone->setMatrixInSkeletonSpace(osg::Matrix::translate(trans) * bone->getMatrixInSkeletonSpace());
    bone->setName(refBone.Name.String().C_str());
    return bone.get();
  }

  osgAnimation::Bone* CreateEndBone(const FMeshBone& refBone, osg::Group* parent)
  {
    osgAnimation::Bone* bone = CreateBone(refBone, parent);
    bone->addChild(CreateBoneShape(FVector(.1)));
    return bone;
  }

  osgAnimation::Channel* CreateChannel(const char* name, const osg::Vec3& axis, float rad)
  {
    osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> ch = new osgAnimation::QuatSphericalLinearChannel;
    ch->setName("quaternion");
    ch->setTargetName(name);
    osgAnimation::QuatKeyframeContainer* kfs = ch->getOrCreateSampler()->getOrCreateKeyframeContainer();
    kfs->push_back(osgAnimation::QuatKeyframe(0.0, osg::Quat(0.0, axis)));
    kfs->push_back(osgAnimation::QuatKeyframe(8.0, osg::Quat(rad, axis)));
    return ch.release();
  }

  osg::ref_ptr<osgAnimation::Skeleton> CreateSkeleton(const std::vector<FMeshBone>& refBones)
  {
    osg::ref_ptr<osgAnimation::Skeleton> skeleton = new osgAnimation::Skeleton;
    std::vector<osgAnimation::Bone*> bones;
    bones.emplace_back(CreateBone(refBones.front(), skeleton));
    for (int32 idx = 1; idx < refBones.size(); ++idx)
    {
      osgAnimation::Bone* bone = nullptr;
      const FMeshBone& refBone = refBones[idx];
      if (!refBone.NumChildren)
      {
        bone = CreateEndBone(refBone, bones[refBone.ParentIndex]);
      }
      else
      {
        bone = CreateBone(refBone, bones[refBone.ParentIndex]);
      }
      bones.emplace_back(bone);
    }
    return skeleton;
  }

  osg::ref_ptr<osgAnimation::Animation> CreateAnimation(UAnimSequence* sequence, USkeletalMesh* mesh)
  {
    osg::ref_ptr<osgAnimation::Animation> anim = new osgAnimation::Animation;

    UAnimSet* set = sequence->GetTypedOuter<UAnimSet>();
    int32 numRawFrames = sequence->GetRawFramesCount();

    std::vector<FTranslationTrack> ttracks;
    std::vector<FRotationTrack> rtracks;
    sequence->GetTracks(ttracks, rtracks);

    for (const FRotationTrack& track : rtracks)
    {
      osg::ref_ptr<osgAnimation::QuatSphericalLinearChannel> rotChannel = new osgAnimation::QuatSphericalLinearChannel;
      rotChannel->setName("quaternion");
      rotChannel->setTargetName(track.Name.String().C_str());
      osgAnimation::QuatKeyframeContainer* kfs = rotChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();

      float step = sequence->SequenceLength;
      if (track.RotKeys.size() > 1)
      {
        step /= track.RotKeys.size() - 1;
      }
      for (int32 keyIdx = 0; keyIdx < track.RotKeys.size(); ++keyIdx)
      {
        FQuat q = track.RotKeys[keyIdx];
        kfs->push_back(osgAnimation::QuatKeyframe(step * float(keyIdx), osg::Quat(q.X, -q.Y, q.Z, q.W)));
      }
      anim->addChannel(rotChannel.release());
    }

    for (const FTranslationTrack& track : ttracks)
    {
      osg::ref_ptr<osgAnimation::Vec3LinearChannel> posChannel = new osgAnimation::Vec3LinearChannel;
      posChannel->setName("translate");
      posChannel->setTargetName(track.Name.String().C_str());
      osgAnimation::Vec3KeyframeContainer* kfs = posChannel->getOrCreateSampler()->getOrCreateKeyframeContainer();

      float step = sequence->SequenceLength;
      if (track.PosKeys.size() > 1)
      {
        step /= track.PosKeys.size() - 1;
      }
      for (int32 keyIdx = 0; keyIdx < track.PosKeys.size(); ++keyIdx)
      {
        FVector p = track.PosKeys[keyIdx];
        kfs->push_back(osgAnimation::Vec3Keyframe(step * float(keyIdx), osg::Vec3(p.X, -p.Y, p.Z)));
      }
      anim->addChannel(posChannel.release());
    }

    return anim;
  }
}


class AnimExportOptions : public WXDialog {
public:
  AnimExportOptions(wxWindow* parent, UAnimSet* anim, USkeletalMesh* mesh)
    : WXDialog(parent, wxID_ANY, wxT("Export options"), wxDefaultPosition, wxSize(562, 198))
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

AnimSetEditor::AnimSetEditor(wxPanel* parent, PackageWindow* window)
  : GenericAnimEditor(parent, window)
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxVERTICAL);

  wxStaticLine* m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer2->Add(m_staticline1, 0, wxEXPAND | wxBOTTOM, 5);

  wxPanel* m_panel1;
  m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 32), wxTAB_TRAVERSAL);
  m_panel1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText2;
  m_staticText2 = new wxStaticText(m_panel1, wxID_ANY, wxT("Take:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText2->Wrap(-1);
  bSizer3->Add(m_staticText2, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, 5);

  TakePicker = new wxChoice(m_panel1, wxID_ANY, wxDefaultPosition, wxSize(175, -1));
  bSizer3->Add(TakePicker, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxRIGHT, 5);

  MeshButton = new wxButton(m_panel1, wxID_ANY, wxT("Mesh..."), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(MeshButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  ErrorLabel = new wxStaticText(m_panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  ErrorLabel->Wrap(-1);
  ErrorLabel->SetForegroundColour(wxColour(255, 13, 13));

  bSizer3->Add(ErrorLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  m_panel1->SetSizer(bSizer3);
  m_panel1->Layout();
  bSizer2->Add(m_panel1, 0, wxALL | wxEXPAND, 5);

  ContainerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  ContainerPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

  bSizer2->Add(ContainerPanel, 1, wxEXPAND | wxTOP, 5);


  this->SetSizer(bSizer2);
  this->Layout();

  CreateRenderer(ContainerPanel);
  window->FixOSG();
  Manager = new osgAnimation::BasicAnimationManager;

  TakePicker->Connect(wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(AnimSetEditor::OnTakeChanged), NULL, this);
  MeshButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AnimSetEditor::OnMeshClicked), NULL, this);
  MeshButton->Enable(false);
}

void AnimSetEditor::PopulateToolBar(wxToolBar* toolbar)
{
  if (wxToolBarToolBase* tool = toolbar->AddTool(eID_Export, "Export", wxBitmap("#112", wxBITMAP_TYPE_PNG_RESOURCE), "Export object data..."))
  {
    if (UAnimSet* set = Cast<UAnimSet>(Object))
    {
      tool->Enable(set->GetSequences().size());
      if (set->GetSequences().empty())
      {
        tool->SetShortHelp(wxT("The animation has no takes! Nothing to show or export."));
      }
    }
    else
    {
      tool->Enable(false);
    }
  }
  GenericEditor::PopulateToolBar(toolbar);
}

void AnimSetEditor::OnExportClicked(wxCommandEvent& e)
{
  UAnimSet* set = Cast<UAnimSet>(Object);
  USkeletalMesh* source = Mesh;
  if (!source)
  {
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

void AnimSetEditor::OnObjectLoaded()
{
  if (!Mesh)
  {
    if (UAnimSet* set = Cast<UAnimSet>(Object))
    {
      TakePicker->Freeze();
      std::vector<UAnimSequence*> sequences = set->GetSequences();
      MeshButton->Enable(sequences.size());
      TakePicker->Clear();
      for (UAnimSequence* sequence : sequences)
      {
        TakePicker->Append(sequence->SequenceName.String().WString());
      }
      TakePicker->SetSelection(0);
      TakePicker->Thaw();
    }
  }
  GenericAnimEditor::OnObjectLoaded();
  ShowMissingMesh(!Mesh);
}

USkeletalMesh* AnimSetEditor::GetMesh()
{
  if (Mesh)
  {
    return Mesh;
  }
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
  if (!source)
  {
    source = set->GetPreviewSkeletalMesh();
  }
  Mesh = source;
  return Mesh;
}

UAnimSequence* AnimSetEditor::GetActiveSequence()
{
  int sel = TakePicker->GetSelection();
  return sel >= 0 ? Cast<UAnimSet>(Object)->GetSequences()[sel] : nullptr;
}

void AnimSetEditor::ShowMissingMesh(bool show)
{
  wxString text = show ? Cast<UAnimSet>(Object)->GetSequences().size() ? wxT("Preview is unavailable! Missing skeletal mesh.") : wxT("The animation set has no takes! Nothing to show or export.") : wxEmptyString;
  ErrorLabel->SetLabelText(text);
  ErrorLabel->SetToolTip(text);
  TakePicker->Enable(!show);
}

void AnimSetEditor::OnTakeChanged(wxCommandEvent&)
{
  if (ActiveAnimation)
  {
    Manager->stopAll();
    Manager->unregisterAnimation(ActiveAnimation);
    ActiveAnimation = nullptr;
  }
  if ((ActiveAnimation = CreateAnimation(GetActiveSequence(), Mesh)))
  {
    Manager->registerAnimation(ActiveAnimation);
    Manager->playAnimation(ActiveAnimation);
  }
}

void AnimSetEditor::OnMeshClicked(wxCommandEvent&)
{
  ObjectPicker picker(this, wxT("Select a skeletal mesh..."), true, Mesh ? Mesh->GetPackage()->Ref() : Object->GetPackage()->Ref(), Mesh ? Mesh->GetPackage()->GetObjectIndex(Mesh) : 0, { USkeletalMesh::StaticClassName() });
  picker.SetCanChangePackage(true);
  if (picker.ShowModal() != wxID_OK)
  {
    return;
  }
  Mesh = Cast<USkeletalMesh>(picker.GetSelectedObject());
  ShowMissingMesh(!Mesh);
  CreateRenderModel();
}

AnimSequenceEditor::AnimSequenceEditor(wxPanel* parent, PackageWindow* window)
  : GenericAnimEditor(parent, window)
{
  CreateRenderer(this);
  window->FixOSG();
  Manager = new osgAnimation::BasicAnimationManager;
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

USkeletalMesh* AnimSequenceEditor::GetMesh()
{
  if (Mesh)
  {
    return Mesh;
  }
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
  if (!source)
  {
    source = set->GetPreviewSkeletalMesh();
  }
  Mesh = source;
  return Mesh;
}

UAnimSequence* AnimSequenceEditor::GetActiveSequence()
{
  return Cast<UAnimSequence>(Object);
}

GenericAnimEditor::~GenericAnimEditor()
{
  if (Renderer)
  {
    delete Renderer;
  }
}

void GenericAnimEditor::OnTick()
{
  if (Renderer && Renderer->isRealized() && Renderer->checkNeedToDoFrame())
  {
    Renderer->frame();
  }
}

void GenericAnimEditor::OnObjectLoaded()
{
  if (!Mesh)
  {
    Mesh = GetMesh();
    CreateRenderModel();
  }
  else if (Mesh && ActiveAnimation)
  {
    Manager->playAnimation(ActiveAnimation);
  }
  GenericEditor::OnObjectLoaded();
}

void GenericAnimEditor::ClearToolbar()
{
  Manager->stopAll();
  GenericEditor::ClearToolbar();
}

void GenericAnimEditor::CreateRenderer(wxPanel* parent)
{
  int attrs[] = { int(WX_GL_DOUBLEBUFFER), WX_GL_RGBA, WX_GL_DEPTH_SIZE, 8, WX_GL_STENCIL_SIZE, 8, 0 };

  Canvas = new OSGCanvas(Window, parent, wxID_ANY, wxDefaultPosition, GetSize(), wxNO_BORDER, wxT("OSGCanvas"), attrs);

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(Canvas, 1, wxALL | wxEXPAND);
  parent->SetSizer(sizer);
  sizer->Fit(parent);
  sizer->SetSizeHints(parent);

  OSGProxy = new OSGWindow(Canvas);
  Canvas->SetGraphicsWindow(OSGProxy);
  Renderer = new osgViewer::Viewer;
  Renderer->setRunFrameScheme(osgViewer::ViewerBase::FrameScheme::ON_DEMAND);
  Renderer->getCamera()->setClearColor({ .3, .3, .3, 1 });
  Renderer->getCamera()->setGraphicsContext(OSGProxy);
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->getCamera()->setProjectionMatrixAsPerspective(60, GetSize().x / GetSize().y, 0.1, 500);
  Renderer->getCamera()->setDrawBuffer(GL_BACK);
  Renderer->getCamera()->setReadBuffer(GL_BACK);

#if _DEBUG
  Renderer->addEventHandler(new osgViewer::StatsHandler);
#endif

  osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
  manipulator->setAllowThrow(false);
  manipulator->setVerticalAxisFixed(true);
  Renderer->setCameraManipulator(manipulator);
  Renderer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
}

void GenericAnimEditor::CreateRenderModel()
{
  if (!Mesh || !Renderer)
  {
    return;
  }
  osg::ref_ptr<osg::Geode> root = new osg::Geode;
  osg::ref_ptr<osgAnimation::Skeleton> skeleton = CreateSkeleton(Mesh->GetReferenceSkeleton());

  FVector customRotation;
  FString objName = Mesh->GetObjectNameString().ToUpper();
  if (objName.Find("_FACE") != std::string::npos)
  {
    customRotation.X = 90;
    customRotation.Z = 180 + 90;
  }
  else if (objName.Find("_TAIL") != std::string::npos)
  {
    customRotation.Y = 90;
  }
  else if (objName.Find("_HAIR") != std::string::npos)
  {
    customRotation.X = 90;
    customRotation.Z = 180 + 90;
  }
  else if (objName.StartWith("ATTACH_"))
  {
    customRotation.Z = -90;
  }
  else if (objName.StartWith("SWITCH_"))
  {
    customRotation.Z = 180;
    customRotation.X = 90;
  }

  if (!customRotation.IsZero())
  {
    osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
    osg::Matrix m;
    m.makeIdentity();
    osg::Quat quat;
    quat.makeRotate(
      customRotation.X * M_PI / 180., RollAxis,
      customRotation.Y * M_PI / 180., PitchAxis,
      customRotation.Z * M_PI / 180., YawAxis
    );
    m.preMultRotate(quat);
    mt->setMatrix(m);
    mt->addChild(root);
    Root = new osg::Geode;
    Root->addChild(mt);
  }
  else
  {
    Root = new osg::Geode;
    Root->addChild(root);
  }
  root->addChild(skeleton);
  root->setUpdateCallback(Manager.get());

  Renderer->setSceneData(Root.get());
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);

  if (customRotation.IsZero())
  {
    const FBoxSphereBounds& bounds = Mesh->GetBounds();
    float distance = bounds.SphereRadius * 1.25;
    if (!distance)
    {
      distance = 10.;
    }
    osg::Vec3d center(bounds.Origin.X, bounds.Origin.Y, bounds.Origin.Z);
    osg::Vec3d eye = { center[0] + distance, center[1] + distance, center[2] * 1.75 };
    osg::Vec3d up = { -.18, -.14, .97 };
    osgGA::TrackballManipulator* manipulator = (osgGA::TrackballManipulator*)Renderer->getCameraManipulator();
    manipulator->setTransformation(eye, center, up);
  }

  if (!ActiveAnimation)
  {
    ActiveAnimation = CreateAnimation(GetActiveSequence(), Mesh);
  }
  if (ActiveAnimation)
  {
    Manager->registerAnimation(ActiveAnimation.get());
    Manager->playAnimation(ActiveAnimation.get());
  }
}
