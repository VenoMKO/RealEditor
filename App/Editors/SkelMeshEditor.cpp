#include "SkelMeshEditor.h"
#include "../Windows/REDialogs.h"
#include "../Windows/PackageWindow.h"
#include "../Windows/ProgressWindow.h"
#include "../Windows/MaterialMapperDialog.h"
#include "../Misc/AConfiguration.h"
#include "../App.h"
#include <wx/valnum.h>

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

#include <Tera/Cast.h>
#include <Tera/UMaterial.h>

#include <Tera/Utils/ALog.h>
#include <Tera/Utils/MeshUtils.h>
#include <Tera/Utils/TextureUtils.h>

#include <filesystem>
#include <functional>

#include "../resource.h"

static const osg::Vec3d YawAxis(0.0, 0.0, -1.0);
static const osg::Vec3d PitchAxis(0.0, -1.0, 0.0);
static const osg::Vec3d RollAxis(1.0, 0.0, 0.0);

static const osg::Vec4 BoneColor(1., 1., 1., 1.);
static const float BoneWidth = 2.f;

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

enum ExportMode {
  ExportGeometry = wxID_HIGHEST + 1,
  ExportFull
};

enum MaterialsMenuID {
  EditMaterials = wxID_HIGHEST + 1,
  ShowMaterials,
  FirstMaterial,
};

class SkelMeshExportOptions : public WXDialog {
public:
  SkelMeshExportOptions(wxWindow* parent, const FSkelMeshExportConfig& cfg)
    : WXDialog(parent, wxID_ANY, wxT("Export options"), wxDefaultPosition, wxSize(268, 270), wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU)
  {
    SetSize(FromDIP(GetSize()));
    SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxString ModeGroupChoices[] = { wxT("Geometry and weights"), wxT("Only geometry") };
    int ModeGroupNChoices = sizeof(ModeGroupChoices) / sizeof(wxString);
    ModeGroup = new wxRadioBox(this, wxID_ANY, wxT("Export:"), wxDefaultPosition, wxDefaultSize, ModeGroupNChoices, ModeGroupChoices, 1, wxRA_SPECIFY_COLS);
    ModeGroup->SetSelection(1);
    bSizer1->Add(ModeGroup, 0, wxALL | wxEXPAND, FromDIP(5));

    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* m_staticText2;
    m_staticText2 = new wxStaticText(this, wxID_ANY, wxT("Scale:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText2->Wrap(-1);
    bSizer2->Add(m_staticText2, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

    ScaleFactor = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    bSizer2->Add(ScaleFactor, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


    bSizer1->Add(bSizer2, 0, wxEXPAND, FromDIP(5));

    ExportTextures = new wxCheckBox(this, wxID_ANY, wxT("Export textures"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer1->Add(ExportTextures, 0, wxALL, FromDIP(5));

    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* m_staticText3;
    m_staticText3 = new wxStaticText(this, wxID_ANY, wxT("Texture format:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText3->Wrap(-1);
    bSizer5->Add(m_staticText3, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

    wxString TextureFormatChoices[] = { wxT("TGA"), wxT("PNG"), wxT("DDS") };
    int TextureFormatNChoices = sizeof(TextureFormatChoices) / sizeof(wxString);
    TextureFormat = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, TextureFormatNChoices, TextureFormatChoices, 0);
    TextureFormat->SetSelection(0);
    bSizer5->Add(TextureFormat, 1, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


    bSizer1->Add(bSizer5, 0, wxEXPAND, FromDIP(5));

    wxBoxSizer* bSizer3;
    bSizer3 = new wxBoxSizer(wxHORIZONTAL);


    bSizer3->Add(0, 0, 1, wxEXPAND, FromDIP(5));

    wxButton* m_button1;
    m_button1 = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer3->Add(m_button1, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

    wxButton* m_button2;
    m_button2 = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer3->Add(m_button2, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


    bSizer1->Add(bSizer3, 1, wxEXPAND, FromDIP(5));


    this->SetSizer(bSizer1);
    this->Layout();

    this->Centre(wxBOTH);

    ExportTextures->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(SkelMeshExportOptions::OnExportTexturesChanged), nullptr, this);
    m_button1->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SkelMeshExportOptions::OnOkClicked), nullptr, this);
    m_button2->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SkelMeshExportOptions::OnCancelClicked), nullptr, this);

    ScaleFactorValue = cfg.ScaleFactor;
    ScaleFactor->SetValidator(wxFloatingPointValidator<float>(2, &ScaleFactorValue, wxNUM_VAL_DEFAULT));

    ExportTextures->SetValue(cfg.ExportTextures);

    if (!HasAVX2())
    {
      TextureFormat->SetSelection(2);
      TextureFormat->Enable(false);
    }
    else
    {
      TextureFormat->SetSelection(cfg.TextureFormat);
      TextureFormat->Enable(cfg.ExportTextures);
    }

    ModeGroup->Select(cfg.Mode);
  }

  ~SkelMeshExportOptions()
  {}

  float GetScaleFactor() const
  {
    return ScaleFactorValue;
  }

  bool GetExportTextures() const
  {
    return ExportTextures->GetValue();
  }

  ExportMode GetExportMode() const
  {
    switch (GetExportModeIndex())
    {
    case 1:
      return ExportMode::ExportGeometry;
    }
    return ExportMode::ExportFull;
  }

  int32 GetExportModeIndex() const
  {
    return ModeGroup->GetSelection();
  }

  TextureProcessor::TCFormat GetTextureFormat() const
  {
    switch (GetTextureFormatIndex())
    {
    case 0:
      return TextureProcessor::TCFormat::TGA;
    case 1:
      return TextureProcessor::TCFormat::PNG;
    }
    return TextureProcessor::TCFormat::DDS;
  }

  int32 GetTextureFormatIndex() const
  {
    return TextureFormat->GetSelection();
  }

private:
  void OnOkClicked(wxCommandEvent&)
  {
    ScaleFactor->GetValidator()->TransferFromWindow();
    EndModal(wxID_OK);
  }

  void OnCancelClicked(wxCommandEvent&)
  {
    EndModal(wxID_CANCEL);
  }

  void OnExportTexturesChanged(wxCommandEvent&)
  {
    TextureFormat->Enable(ExportTextures->GetValue());
  }

protected:
  wxRadioBox* ModeGroup = nullptr;
  wxTextCtrl* ScaleFactor = nullptr;
  wxCheckBox* ExportTextures = nullptr;
  wxChoice* TextureFormat = nullptr;

  float ScaleFactorValue = 1.;
};

class SkelMeshImportOptions : public WXDialog {
public:
  SkelMeshImportOptions(wxWindow* parent)
    : WXDialog(parent, wxID_ANY, wxT("Import options"), wxDefaultPosition, wxSize(395, 527), wxDEFAULT_DIALOG_STYLE)
  {
    SetSize(FromDIP(GetSize()));
    SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxPanel* m_panel1;
    m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer(wxVERTICAL);

    ImportSkeleton = new wxCheckBox(m_panel1, wxID_ANY, wxT("Import skeleton"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer2->Add(ImportSkeleton, 0, wxALL, FromDIP(5));

    wxStaticText* m_StaticText1;
    m_StaticText1 = new wxStaticText(m_panel1, wxID_ANY, wxT("This option allows to import 3D models with any suitable skeleton. Disabling it results in strict FBX to GPK bone mapping."), wxDefaultPosition, wxDefaultSize, 0);
    m_StaticText1->Wrap(FromDIP(350));
    bSizer2->Add(m_StaticText1, 0, wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));


    m_panel1->SetSizer(bSizer2);
    m_panel1->Layout();
    bSizer2->Fit(m_panel1);
    bSizer1->Add(m_panel1, 0, wxALL | wxEXPAND, FromDIP(5));

    wxPanel* m_panel2;
    m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer21;
    bSizer21 = new wxBoxSizer(wxVERTICAL);

    ImportTangents = new wxCheckBox(m_panel2, wxID_ANY, wxT("Import normals"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer21->Add(ImportTangents, 0, wxALL, FromDIP(5));

    wxStaticText* m_StaticText11;
    m_StaticText11 = new wxStaticText(m_panel2, wxID_ANY, wxT("Import normals, binormals, and tangents from the FBX file."), wxDefaultPosition, wxDefaultSize, 0);
    m_StaticText11->Wrap(FromDIP(350));
    bSizer21->Add(m_StaticText11, 0, wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));


    m_panel2->SetSizer(bSizer21);
    m_panel2->Layout();
    bSizer21->Fit(m_panel2);
    bSizer1->Add(m_panel2, 0, wxALL | wxEXPAND, FromDIP(5));

    wxPanel* m_panel3;
    m_panel3 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer211;
    bSizer211 = new wxBoxSizer(wxVERTICAL);

    FlipBinormals = new wxCheckBox(m_panel3, wxID_ANY, wxT("Flip binormals"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer211->Add(FlipBinormals, 0, wxALL, FromDIP(5));

    wxStaticText* m_StaticText111;
    m_StaticText111 = new wxStaticText(m_panel3, wxID_ANY, wxT("Inverse binormal direction. This may help if you have seams between symmetrical parts."), wxDefaultPosition, wxDefaultSize, 0);
    m_StaticText111->Wrap(FromDIP(350));
    bSizer211->Add(m_StaticText111, 0, wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));


    m_panel3->SetSizer(bSizer211);
    m_panel3->Layout();
    bSizer211->Fit(m_panel3);
    bSizer1->Add(m_panel3, 0, wxEXPAND | wxALL, FromDIP(5));

    wxPanel* m_panel4;
    m_panel4 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer2111;
    bSizer2111 = new wxBoxSizer(wxVERTICAL);

    BinormalsUV = new wxCheckBox(m_panel4, wxID_ANY, wxT("Binormals by UV"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer2111->Add(BinormalsUV, 0, wxALL, FromDIP(5));

    wxStaticText* m_StaticText1111;
    m_StaticText1111 = new wxStaticText(m_panel4, wxID_ANY, wxT("Compute binormal direction based on vertex UV space."), wxDefaultPosition, wxDefaultSize, 0);
    m_StaticText1111->Wrap(FromDIP(350));
    bSizer2111->Add(m_StaticText1111, 0, wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));


    m_panel4->SetSizer(bSizer2111);
    m_panel4->Layout();
    bSizer2111->Fit(m_panel4);
    bSizer1->Add(m_panel4, 0, wxEXPAND | wxALL, FromDIP(5));

    wxPanel* m_panel5;
    m_panel5 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer21111;
    bSizer21111 = new wxBoxSizer(wxVERTICAL);

    UpdateBounds = new wxCheckBox(m_panel5, wxID_ANY, wxT("Update bounds"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer21111->Add(UpdateBounds, 0, wxALL, FromDIP(5));

    wxStaticText* m_StaticText11111;
    m_StaticText11111 = new wxStaticText(m_panel5, wxID_ANY, wxT("Calculate new mesh bounds for the 3D model. Tera uses these bounds to cull off objects obscured by the model."), wxDefaultPosition, wxDefaultSize, 0);
    m_StaticText11111->Wrap(FromDIP(350));
    bSizer21111->Add(m_StaticText11111, 0, wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));


    m_panel5->SetSizer(bSizer21111);
    m_panel5->Layout();
    bSizer21111->Fit(m_panel5);
    bSizer1->Add(m_panel5, 0, wxEXPAND | wxALL, FromDIP(5));

    wxPanel* m_panel6;
    m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
    wxBoxSizer* bSizer211111;
    bSizer211111 = new wxBoxSizer(wxVERTICAL);

    IndexBuffer = new wxCheckBox(m_panel6, wxID_ANY, wxT("Optimize index buffer"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer211111->Add(IndexBuffer, 0, wxALL, FromDIP(5));

    wxStaticText* m_StaticText111111;
    m_StaticText111111 = new wxStaticText(m_panel6, wxID_ANY, wxT("Optimize 3D model's index buffer. May slightly increase in-game performance."), wxDefaultPosition, wxDefaultSize, 0);
    m_StaticText111111->Wrap(FromDIP(350));
    bSizer211111->Add(m_StaticText111111, 0, wxBOTTOM | wxRIGHT | wxLEFT, FromDIP(5));


    m_panel6->SetSizer(bSizer211111);
    m_panel6->Layout();
    bSizer211111->Fit(m_panel6);
    bSizer1->Add(m_panel6, 0, wxEXPAND | wxALL, FromDIP(5));

    wxBoxSizer* bSizer14;
    bSizer14 = new wxBoxSizer(wxHORIZONTAL);


    bSizer14->Add(0, 0, 1, wxEXPAND, FromDIP(5));

    wxButton* okButton;
    okButton = new wxButton(this, wxID_ANY, wxT("Import"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer14->Add(okButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

    wxButton* cancelButton;
    cancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer14->Add(cancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


    bSizer1->Add(bSizer14, 1, wxEXPAND, FromDIP(5));


    SetSizer(bSizer1);
    Layout();

    Centre(wxBOTH);

    // Connect Events
    cancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SkelMeshImportOptions::OnCancelClicked), NULL, this);
    okButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SkelMeshImportOptions::OnOkClicked), NULL, this);

    FAppConfig cfg = App::GetSharedApp()->GetConfig();

    ImportSkeleton->SetValue(cfg.SkelMeshImportConfig.ImportSkeleton);
    ImportTangents->SetValue(cfg.SkelMeshImportConfig.ImportTangents);
    FlipBinormals->SetValue(cfg.SkelMeshImportConfig.FlipTangentY);
    BinormalsUV->SetValue(cfg.SkelMeshImportConfig.TangentYBasisByUV);
    IndexBuffer->SetValue(cfg.SkelMeshImportConfig.OptimizeIndexBuffer);
    UpdateBounds->SetValue(cfg.SkelMeshImportConfig.UpdateBounds);
  }

  bool GetImportSkeleton() const
  {
    return ImportSkeleton->GetValue();
  }

  bool GetImportTangents() const
  {
    return ImportTangents->GetValue();
  }

  bool GetFlipTangents() const
  {
    return FlipBinormals->GetValue();
  }

  bool GetTangentYByUVs() const
  {
    return BinormalsUV->GetValue();
  }

  bool GetOptimizeIndexBuffer() const
  {
    return IndexBuffer->GetValue();
  }

  bool GetUpdateBounds() const
  {
    return UpdateBounds->GetValue();
  }

protected:
  void OnOkClicked(wxCommandEvent&)
  {
    FAppConfig& cfg = App::GetSharedApp()->GetConfig();
    cfg.SkelMeshImportConfig.ImportSkeleton = ImportSkeleton->GetValue();
    cfg.SkelMeshImportConfig.ImportTangents = ImportTangents->GetValue();
    cfg.SkelMeshImportConfig.FlipTangentY = FlipBinormals->GetValue();
    cfg.SkelMeshImportConfig.TangentYBasisByUV = BinormalsUV->GetValue();
    cfg.SkelMeshImportConfig.UpdateBounds = UpdateBounds->GetValue();
    cfg.SkelMeshImportConfig.OptimizeIndexBuffer = IndexBuffer->GetValue();
    App::GetSharedApp()->SaveConfig();
    EndModal(wxID_OK);
  }

  void OnCancelClicked(wxCommandEvent&)
  {
    EndModal(wxID_CANCEL);
  }

protected:
  wxCheckBox* ImportSkeleton = nullptr;
  wxCheckBox* ImportTangents = nullptr;
  wxCheckBox* FlipBinormals = nullptr;
  wxCheckBox* BinormalsUV = nullptr;
  wxCheckBox* IndexBuffer = nullptr;
  wxCheckBox* UpdateBounds = nullptr;
};

SkelMeshEditor::SkelMeshEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  CreateRenderer();
  window->FixOSG();
}

SkelMeshEditor::~SkelMeshEditor()
{
  if (Renderer)
  {
    delete Renderer;
  }
}

void SkelMeshEditor::OnTick()
{
  if (Renderer && Renderer->isRealized() && Renderer->checkNeedToDoFrame())
  {
    Renderer->frame();
  }
}

void SkelMeshEditor::OnObjectLoaded()
{
  if (Loading || !Mesh)
  {
    Mesh = (USkeletalMesh*)Object;
    CreateRenderModel();
  }
  GenericEditor::OnObjectLoaded();
}

void SkelMeshEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (auto item = toolbar->FindById(eID_Import))
  {
    item->Enable(true);
  }
  toolbar->AddTool(eID_Materials, wxT("Materials"), wxBitmap(MAKE_IDB(IDB_ICO_MAT), wxBITMAP_TYPE_PNG_RESOURCE), "Model materials");
  if ((SkeletonTool = toolbar->AddCheckTool(eID_Skeleton, wxT("Skeleton"), wxBitmap(MAKE_IDB(IDB_SKEL), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_SKEL), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle skeleton")))
  {
    if (ShowSkeleton)
    {
      SkeletonTool->Toggle();
    }
  }
  toolbar->AddTool(eID_Refresh, wxT("Reload"), wxBitmap(MAKE_IDB(IDB_REFRESH), wxBITMAP_TYPE_PNG_RESOURCE), "Reload model and its textures");
}

void SkelMeshEditor::OnToolBarEvent(wxCommandEvent& e)
{
  GenericEditor::OnToolBarEvent(e);
  if (e.GetSkipped())
  {
    // The base class has processed the event. Unmark the event and exit
    e.Skip(false);
    return;
  }
  auto eId = e.GetId();
  if (eId == eID_Refresh)
  {
    OnRefreshClicked();
    Renderer->requestRedraw();
  }
  else if (eId == eID_Materials)
  {
    OnMaterialsClicked();
  }
  else if (eId == eID_Skeleton)
  {
    ShowSkeleton = SkeletonTool->IsToggled();
    osg::Vec3d center;
    osg::Vec3d eye;
    osg::Vec3d up;
    osgGA::TrackballManipulator* manipulator = (osgGA::TrackballManipulator*)Renderer->getCameraManipulator();
    if (manipulator)
    {
      manipulator->getTransformation(eye, center, up);
    }
    OnRefreshClicked();
    if (manipulator)
    {
      manipulator->setTransformation(eye, center, up);
    }
  }
}

void SkelMeshEditor::OnExportClicked(wxCommandEvent&)
{
  FAppConfig& appConfig = App::GetSharedApp()->GetConfig();
  SkelMeshExportOptions opts(this, appConfig.SkelMeshExportConfig);
  if (opts.ShowModal() != wxID_OK)
  {
    return;
  }
  appConfig.SkelMeshExportConfig.Mode = opts.GetExportModeIndex();
  appConfig.SkelMeshExportConfig.ScaleFactor = opts.GetScaleFactor();
  appConfig.SkelMeshExportConfig.ExportTextures = opts.GetExportTextures();
  appConfig.SkelMeshExportConfig.TextureFormat = opts.GetTextureFormatIndex();
  App::GetSharedApp()->SaveConfig();

  MeshExportContext ctx;
  ctx.ExportSkeleton = opts.GetExportMode() == ExportMode::ExportFull;
  wxString fbxpath = IODialog::SaveSkelMeshDialog(Window, Object->GetObjectNameString().WString());
  if (fbxpath.empty())
  {
    return;
  }
  ctx.Path = fbxpath.ToStdWstring();
  ctx.Scale3D = FVector(opts.GetScaleFactor());

  bool r = false;
  MeshExporterType exportType = MeshUtils::GetExporterTypeFromExtension(wxFileName(fbxpath).GetExt().Lower().ToStdString());
  auto utils = MeshUtils::CreateUtils(exportType);
  utils->SetCreatorInfo(App::GetSharedApp()->GetAppDisplayName().ToStdString(), GetAppVersion());
  appConfig.SkelMeshExportConfig.LastFormat = (int32)exportType;
  App::GetSharedApp()->SaveConfig();
  if (exportType == MET_Psk)
  {
    ProgressWindow progress(this, wxT("Exporting..."));
    progress.SetCanCancel(false);
    progress.SetCurrentProgress(-1);
    progress.SetActionText(wxT("Building ActorX..."));
    std::string errorText;
    std::thread([&] {
      try
      {
        r = utils->ExportSkeletalMesh((USkeletalMesh*)Object, ctx);
      }
      catch (const std::exception& e)
      {
        errorText = e.what();
        r = false;
      }
      SendEvent(&progress, UPDATE_PROGRESS_FINISH, r);
    }).detach();
    if (!progress.ShowModal())
    {
      if (errorText.size() && ctx.Error.empty())
      {
        ctx.Error = errorText;
      }
    }
  }
  else
  {
    try
    {
      r = utils->ExportSkeletalMesh((USkeletalMesh*)Object, ctx);
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what());
      return;
    }
  }

  if (!r)
  {
    REDialog::Error(ctx.Error);
    return;
  }
  if (opts.GetExportTextures())
  {
    fbxpath += ".textures";
    std::map<std::filesystem::path, UTexture*> textures;
    auto materials = Mesh->GetMaterials();
    for (UObject* obj : materials)
    {
      if (UMaterialInterface* mat = Cast<UMaterialInterface>(obj))
      {
        std::filesystem::path dir = fbxpath.ToStdWstring();
        dir /= mat->GetObjectNameString().WString();
        std::filesystem::create_directories(dir);
        auto textureParams = mat->GetTextureParameters();
        if (UMaterial* parent = Cast<UMaterial>(mat->GetParent()))
        {
          auto parentTexParams = parent->GetTextureParameters();
          for (auto& p : parentTexParams)
          {
            if (p.second.Texture && !textureParams.count(p.first))
            {
              textureParams[p.first].Texture = p.second.Texture;
            }
          }
        }
        for (auto& p : textureParams)
        {
          if (!p.second.Texture)
          {
            continue;
          }
          std::filesystem::path tpath = dir / p.second.Texture->GetObjectNameString().WString();
          if (!textures.count(tpath))
          {
            textures[tpath] = p.second.Texture;
          }
        }
        auto constTextures = mat->GetTextureSamples();
        for (UTexture* tex : constTextures)
        {
          if (!tex)
          {
            continue;
          }
          std::filesystem::path tpath = dir / tex->GetObjectNameString().WString();
          if (!textures.count(tpath))
          {
            textures[tpath] = tex;
          }
        }
      }
    }

    if (textures.size())
    {
      TextureProcessor::TCFormat outputFormat = HasAVX2() ? opts.GetTextureFormat() : TextureProcessor::TCFormat::DDS;
      std::string ext;
      switch (outputFormat)
      {
      case TextureProcessor::TCFormat::PNG:
        ext = "png";
        break;
      case TextureProcessor::TCFormat::TGA:
        ext = "tga";
        break;
      case TextureProcessor::TCFormat::DDS:
        ext = "dds";
        break;
      default:
        LogE("Invalid output format!");
        return;
      }

      for (auto& p : textures)
      {
        if (!p.second)
        {
          continue;
        }

        std::error_code err;
        std::filesystem::path path = p.first;
        path.replace_extension(ext);

        if (UTexture2D* texture = Cast<UTexture2D>(p.second))
        {
          TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
          switch (texture->Format)
          {
          case PF_DXT1:
            inputFormat = TextureProcessor::TCFormat::DXT1;
            break;
          case PF_DXT3:
            inputFormat = TextureProcessor::TCFormat::DXT3;
            break;
          case PF_DXT5:
            inputFormat = TextureProcessor::TCFormat::DXT5;
            break;
          case PF_A8R8G8B8:
            inputFormat = TextureProcessor::TCFormat::ARGB8;
            break;
          case PF_G8:
            inputFormat = TextureProcessor::TCFormat::G8;
            break;
          default:
            LogE("Failed to export texture %s. Invalid format!", texture->GetObjectNameString().UTF8().c_str());
            continue;
          }

          FTexture2DMipMap* mip = nullptr;
          for (FTexture2DMipMap* mipmap : texture->Mips)
          {
            if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
            {
              mip = mipmap;
              break;
            }
          }
          if (!mip)
          {
            LogE("Failed to export texture %s. No mipmaps!", texture->GetObjectNameString().UTF8().c_str());
            continue;
          }

          TextureProcessor processor(inputFormat, outputFormat);

          processor.SetInputData(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
          processor.SetOutputPath(W2A(path.wstring()));
          processor.SetInputDataDimensions(mip->SizeX, mip->SizeY);
          try
          {
            if (!processor.Process())
            {
              LogE("Failed to export %s: %s", texture->GetObjectNameString().UTF8().c_str(), processor.GetError().c_str());
              continue;
            }
          }
          catch (...)
          {
            LogE("Failed to export %s! Unknown texture processor error!", texture->GetObjectNameString().UTF8().c_str());
            continue;
          }
        }
        else if (UTextureCube* cube = Cast<UTextureCube>(p.second))
        {
          auto faces = cube->GetFaces();
          bool ok = true;
          TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
          for (UTexture2D* face : faces)
          {
            if (!face)
            {
              LogW("Failed to export a texture cube %s. Can't get one of the faces.", p.second->GetObjectPath().UTF8().c_str());
              ok = false;
              break;
            }
            TextureProcessor::TCFormat f = TextureProcessor::TCFormat::None;
            switch (face->Format)
            {
            case PF_DXT1:
              f = TextureProcessor::TCFormat::DXT1;
              break;
            case PF_DXT3:
              f = TextureProcessor::TCFormat::DXT3;
              break;
            case PF_DXT5:
              f = TextureProcessor::TCFormat::DXT5;
              break;
            case PF_A8R8G8B8:
              f = TextureProcessor::TCFormat::ARGB8;
              break;
            case PF_G8:
              f = TextureProcessor::TCFormat::G8;
              break;
            default:
              LogE("Failed to export texture cube %s.%s. Invalid face format!", p.second->GetObjectPath().UTF8().c_str(), face->GetObjectNameString().UTF8().c_str());
              ok = false;
              break;
            }
            if (inputFormat != TextureProcessor::TCFormat::None && inputFormat != f)
            {
              LogE("Failed to export texture cube %s.%s. Faces have different format!", p.second->GetObjectPath().UTF8().c_str(), face->GetObjectNameString().UTF8().c_str());
              ok = false;
              break;
            }
            inputFormat = f;
          }

          if (!ok)
          {
            continue;
          }

          // UE4 accepts texture cubes only in a DDS container with A8R8G8B8 format and proper flags. Export cubes this way regardless of the user's output format.
          path.replace_extension("dds");
          TextureProcessor processor(inputFormat, TextureProcessor::TCFormat::DDS);
          for (int32 faceIdx = 0; faceIdx < faces.size(); ++faceIdx)
          {
            FTexture2DMipMap* mip = nullptr;
            for (FTexture2DMipMap* mipmap : faces[faceIdx]->Mips)
            {
              if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
              {
                mip = mipmap;
                break;
              }
            }
            if (!mip)
            {
              LogE("Failed to export texture cube face %s.%s. No mipmaps!", cube->GetObjectPath().UTF8().c_str(), faces[faceIdx]->GetObjectNameString().UTF8().c_str());
              ok = false;
              break;
            }

            processor.SetInputCubeFace(faceIdx, mip->Data->GetAllocation(), mip->Data->GetBulkDataSize(), mip->SizeX, mip->SizeY);
          }

          if (!ok)
          {
            continue;
          }

          processor.SetOutputPath(W2A(path.wstring()));

          try
          {
            if (!processor.Process())
            {
              LogE("Failed to export %s: %s", cube->GetObjectPath().UTF8().c_str(), processor.GetError().c_str());
              continue;
            }
          }
          catch (...)
          {
            LogE("Failed to export %s! Unknown texture processor error!", cube->GetObjectPath().UTF8().c_str());
            continue;
          }
        }
        else
        {
          LogW("Failed to export a texture object %s(%s). Class is not supported!", p.second->GetObjectPath().UTF8().c_str(), p.second->GetClassNameString().UTF8().c_str());
        }
      }
    }
  }
}

void SkelMeshEditor::OnImportClicked(wxCommandEvent&)
{
  wxString path = IODialog::OpenMeshDialog(Window);
  if (path.empty())
  {
    return;
  }
  MeshImportContext ctx;

  SkelMeshImportOptions opts(this);
  if (opts.ShowModal() != wxID_OK)
  {
    return;
  }
  ctx.ImportData.ImportSkeleton = opts.GetImportSkeleton();
  ctx.ImportData.CalculateBounds = opts.GetUpdateBounds();
  ctx.ImportData.BinormalsByUV = opts.GetTangentYByUVs();
  ctx.ImportData.FlipBinormals = opts.GetFlipTangents();
  ctx.ImportData.ImportTangents = opts.GetImportTangents();
  ctx.ImportData.OptimizeIndexBuffer = opts.GetOptimizeIndexBuffer();

  ctx.Path = path.ToStdWstring();
  auto utils = MeshUtils::CreateUtils(MET_Fbx);
  utils->SetCreatorInfo(App::GetSharedApp()->GetAppDisplayName().ToStdString(), GetAppVersion());
  if (!utils->ImportSkeletalMesh(ctx))
  {
    REDialog::Error(ctx.Error);
    return;
  }

  if (ctx.ImportData.MissingNormals)
  {
    REDialog::Warning(wxT("This may result in incorrect light calculations and seams betwean model parts.\nTo fix the issue enable normals export feature in your 3D editor."), wxT("The 3D model has no normals!"), this);
  }
  if (ctx.ImportData.MissingTangents)
  {
    REDialog::Warning(wxT("The model may look too light/dark or have incorrect shadows.\nTo fix the issue enable tangents & binormals export feature in your 3D editor."), wxT("The 3D model has no tangents!"), this);
  }

  {
    FString error;
    bool askUser = false;
    int32 warningIndex = 0;
    while (1)
    {
      if (!Mesh->ValidateVisitor(&ctx.ImportData, 0, error, askUser, warningIndex))
      {
        REDialog::Error(error.WString());
        return;
      }
      if (error.Empty())
      {
        break;
      }
      warningIndex++;
      auto userResponse = REDialog::Warning(error.WString(), wxT("Warning!"), this, askUser ? wxYES_NO : wxOK);
      if (askUser && userResponse != wxYES)
      {
        return;
      }
    }
  }

  std::vector<FString> fbxMaterials = ctx.ImportData.Materials;
  std::vector<UObject*> objectMaterials = Mesh->GetMaterials();
  std::vector<std::pair<FString, UObject*>> matMap;
  MaterialMapperDialog::AutomaticallyMapMaterials(fbxMaterials, objectMaterials, matMap);
  MaterialMapperDialog mapper(this, Object, matMap, objectMaterials);
  if (mapper.ShowModal() != wxID_OK)
  {
    return;
  }
  matMap = mapper.GetMaterialMap();
  objectMaterials = mapper.GetObjectMaterials();
  ctx.ImportData.ObjectMaterials = objectMaterials;
  ctx.ImportData.MaterialMap = matMap;

  FString err;
  ProgressWindow progress(this, "Importing...");
  progress.SetCurrentProgress(-1);
  progress.SetActionText(wxT("Building new LOD..."));
  std::thread([&] {
    bool result = Mesh->AcceptVisitor(&ctx.ImportData, 0, err);
    SendEvent(&progress, UPDATE_PROGRESS_FINISH, result);
  }).detach();
  if (progress.ShowModal())
  {
    OnRefreshClicked();
  }
  else
  {
    if (err.Empty())
    {
      err = "Failed to create a LOD. Unknown error.";
    }
    REDialog::Error(err.WString());
  }
}

void SkelMeshEditor::SetNeedsUpdate()
{
  if (Renderer)
  {
    Renderer->requestRedraw();
  }
}

void SkelMeshEditor::CreateRenderer()
{
  int attrs[] = { int(WX_GL_DOUBLEBUFFER), WX_GL_RGBA, WX_GL_DEPTH_SIZE, 8, WX_GL_STENCIL_SIZE, 8, 0 };

  Canvas = new OSGCanvas(Window, this, wxID_ANY, wxDefaultPosition, GetSize(), wxNO_BORDER, wxT("OSGCanvas"), attrs);

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(Canvas, 1, wxALL | wxEXPAND);
  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);

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

void SkelMeshEditor::CreateRenderModel()
{
  if (!Mesh)
  {
    return;
  }

  osg::ref_ptr<osg::Geode> root = new osg::Geode;
  osg::ref_ptr<osgAnimation::Skeleton> skeleton = CreateSkeleton(Mesh->GetReferenceSkeleton());
  const FStaticLODModel* model = Mesh->GetLod(0);

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  osg::ref_ptr<osg::Vec2Array> uvs = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX);

  std::vector<FSoftSkinVertex> uvertices = model->GetVertices();
  for (int32 idx = 0; idx < uvertices.size(); ++idx)
  {
    FVector normal = uvertices[idx].TangentZ;
    normals->push_back(osg::Vec3(normal.X, -normal.Y, normal.Z));
    vertices->push_back(osg::Vec3(uvertices[idx].Position.X, -uvertices[idx].Position.Y, uvertices[idx].Position.Z));
    uvs->push_back(osg::Vec2(uvertices[idx].UVs[0].X, uvertices[idx].UVs[0].Y));
  }

  std::vector<const FSkelMeshSection*> sections = model->GetSections();
  const FMultiSizeIndexContainer* indexContainer = model->GetIndexContainer();
  for (const FSkelMeshSection* section : sections)
  {
    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES);
    for (uint32 faceIndex = 0; faceIndex < section->NumTriangles; ++faceIndex)
    {
      indices->push_back(indexContainer->GetIndex(section->BaseIndex + (faceIndex * 3) + 0));
      indices->push_back(indexContainer->GetIndex(section->BaseIndex + (faceIndex * 3) + 1));
      indices->push_back(indexContainer->GetIndex(section->BaseIndex + (faceIndex * 3) + 2));
    }
    geo->addPrimitiveSet(indices.get());
    geo->setVertexArray(vertices.get());
    geo->setNormalArray(normals.get());
    geo->setTexCoordArray(0, uvs.get());

    // TODO: Use MaterialMap to remap section materials to the global materials list
    if (Mesh->GetMaterials().size() > section->MaterialIndex)
    {
      if (UMaterialInterface* material = Cast<UMaterialInterface>(Mesh->GetMaterials()[section->MaterialIndex]))
      {
        if (UTexture2D* tex = material->GetDiffuseTexture())
        {
          osg::ref_ptr<osg::Image> img = new osg::Image;
          UTextureBitmapInfo info;
          if (tex->GetBitmapData(info) && info.IsValid())
          {
            img->setImage(info.Width, info.Height, 0, info.InternalFormat, info.Format, info.Type, (unsigned char*)info.Allocation, osg::Image::AllocationMode::NO_DELETE);
          }
          osg::ref_ptr<osg::Texture2D> osgtex = new osg::Texture2D(img);
          osgtex->setWrap(osg::Texture::WrapParameter::WRAP_S, osg::Texture::WrapMode::REPEAT);
          osgtex->setWrap(osg::Texture::WrapParameter::WRAP_T, osg::Texture::WrapMode::REPEAT);
          geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, osgtex.get());
          if (material->GetBlendMode() == EBlendMode::BLEND_Masked)
          {
            geo->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            geo->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
          }
        }
      }
    }
    root->addDrawable(geo.get());
  }


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
  else if (objName.StartsWith("ATTACH_"))
  {
    customRotation.Z = -90;
  }
  else if (objName.StartsWith("SWITCH_"))
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
    if (ShowSkeleton)
    {
      mt->addChild(skeleton);
    }
    Root = new osg::Geode;
    Root->addChild(mt);
  }
  else
  {
    Root = new osg::Geode;
    Root->addChild(root);
    if (ShowSkeleton)
    {
      Root->addChild(skeleton);
    }
  }
  

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
}

void SkelMeshEditor::OnRefreshClicked()
{
  CreateRenderModel();
}

void SkelMeshEditor::OnMaterialsClicked()
{
  auto mats = Mesh->GetMaterials();
  wxMenu menu;
  // Sub Menu
  {
    wxMenu* addMenu = new wxMenu;
    for (int32 idx = 0; idx < mats.size(); ++idx)
    {
      addMenu->Append(MaterialsMenuID::ShowMaterials + idx, mats[idx] ? mats[idx]->GetObjectNameString().WString() : wxT("NULL"));
    }
    menu.AppendSubMenu(addMenu, wxT("Show material"));
  }
  
  menu.Append(MaterialsMenuID::EditMaterials, wxT("Edit materials..."));
  dynamic_cast<wxMenuItem*>(menu.GetMenuItems().GetLast()->GetData())->Enable(false);

  int selection = GetPopupMenuSelectionFromUser(menu);
  if (selection >= MaterialsMenuID::ShowMaterials)
  {
    if (UObject* selectedObject = mats[selection - MaterialsMenuID::ShowMaterials])
    {
      Window->SelectObject(selectedObject);
    }
  }
}
