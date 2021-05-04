#include "SkelMeshEditor.h"
#include "../Windows/IODialogs.h"
#include "../Windows/PackageWindow.h"
#include "../App.h"
#include <wx/valnum.h>

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osg/BlendFunc>
#include <osg/Depth>

#include <Tera/Cast.h>
#include <Tera/UMaterial.h>

#include <Utils/ALog.h>
#include <Utils/FbxUtils.h>
#include <Utils/AConfiguration.h>
#include <Utils/TextureProcessor.h>

#include <filesystem>
#include <functional>

enum ExportMode {
  ExportGeometry = wxID_HIGHEST + 1,
  ExportFull
};

enum MaterialsMenuID {
  EditMaterials = wxID_HIGHEST + 1,
  ShowMaterials,
  FirstMaterial,
};

class SkelMeshExportOptions : public wxDialog {
public:
  SkelMeshExportOptions(wxWindow* parent, const FSkelMeshExportConfig& cfg)
    : wxDialog(parent, wxID_ANY, wxT("Export options"), wxDefaultPosition, wxSize(268, 270), wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU)
  {
    SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxString ModeGroupChoices[] = { wxT("Geometry and weights"), wxT("Only geometry") };
    int ModeGroupNChoices = sizeof(ModeGroupChoices) / sizeof(wxString);
    ModeGroup = new wxRadioBox(this, wxID_ANY, wxT("Export:"), wxDefaultPosition, wxDefaultSize, ModeGroupNChoices, ModeGroupChoices, 1, wxRA_SPECIFY_COLS);
    ModeGroup->SetSelection(1);
    bSizer1->Add(ModeGroup, 0, wxALL | wxEXPAND, 5);

    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* m_staticText2;
    m_staticText2 = new wxStaticText(this, wxID_ANY, wxT("Scale:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText2->Wrap(-1);
    bSizer2->Add(m_staticText2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    ScaleFactor = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    bSizer2->Add(ScaleFactor, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


    bSizer1->Add(bSizer2, 0, wxEXPAND, 5);

    ExportTextures = new wxCheckBox(this, wxID_ANY, wxT("Export textures"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer1->Add(ExportTextures, 0, wxALL, 5);

    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* m_staticText3;
    m_staticText3 = new wxStaticText(this, wxID_ANY, wxT("Texture format:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText3->Wrap(-1);
    bSizer5->Add(m_staticText3, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    wxString TextureFormatChoices[] = { wxT("TGA"), wxT("PNG"), wxT("DDS") };
    int TextureFormatNChoices = sizeof(TextureFormatChoices) / sizeof(wxString);
    TextureFormat = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, TextureFormatNChoices, TextureFormatChoices, 0);
    TextureFormat->SetSelection(0);
    bSizer5->Add(TextureFormat, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);


    bSizer1->Add(bSizer5, 0, wxEXPAND, 5);

    wxBoxSizer* bSizer3;
    bSizer3 = new wxBoxSizer(wxHORIZONTAL);


    bSizer3->Add(0, 0, 1, wxEXPAND, 5);

    wxButton* m_button1;
    m_button1 = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer3->Add(m_button1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    wxButton* m_button2;
    m_button2 = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer3->Add(m_button2, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


    bSizer1->Add(bSizer3, 1, wxEXPAND, 5);


    this->SetSizer(bSizer1);
    this->Layout();

    this->Centre(wxBOTH);

    ExportTextures->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(SkelMeshExportOptions::OnExportTexturesChanged), nullptr, this);
    m_button1->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SkelMeshExportOptions::OnOkClicked), nullptr, this);
    m_button2->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SkelMeshExportOptions::OnCancelClicked), nullptr, this);

    ScaleFactorValue = cfg.ScaleFactor;
    ScaleFactor->SetValidator(wxFloatingPointValidator<float>(1, &ScaleFactorValue, wxNUM_VAL_DEFAULT));

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
  toolbar->AddTool(eID_Materials, wxT("Materials"), wxBitmap("#125", wxBITMAP_TYPE_PNG_RESOURCE), "Model materials");
  toolbar->AddTool(eID_Refresh, wxT("Reload"), wxBitmap("#122", wxBITMAP_TYPE_PNG_RESOURCE), "Reload model and its textures");
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

  FbxExportContext ctx;
  ctx.ExportSkeleton = opts.GetExportMode() == ExportMode::ExportFull;
  wxString fbxpath = IODialog::SaveMeshDialog(Window, Object->GetObjectName().WString());
  if (fbxpath.empty())
  {
    return;
  }
  ctx.Path = fbxpath.ToStdWstring();
  ctx.Scale3D = FVector(opts.GetScaleFactor());
  FbxUtils utils;
  if (!utils.ExportSkeletalMesh((USkeletalMesh*)Object, ctx))
  {
    wxMessageBox(ctx.Error, wxT("Error!"), wxICON_ERROR);
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
        dir /= mat->GetObjectName().WString();
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
          std::filesystem::path tpath = dir / p.second.Texture->GetObjectName().WString();
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
          std::filesystem::path tpath = dir / tex->GetObjectName().WString();
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
            LogE("Failed to export texture %s. Invalid format!", texture->GetObjectName().UTF8().c_str());
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
            LogE("Failed to export texture %s. No mipmaps!", texture->GetObjectName().UTF8().c_str());
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
              LogE("Failed to export %s: %s", texture->GetObjectName().UTF8().c_str(), processor.GetError().c_str());
              continue;
            }
          }
          catch (...)
          {
            LogE("Failed to export %s! Unknown texture processor error!", texture->GetObjectName().UTF8().c_str());
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
              LogE("Failed to export texture cube %s.%s. Invalid face format!", p.second->GetObjectPath().UTF8().c_str(), face->GetObjectName().UTF8().c_str());
              ok = false;
              break;
            }
            if (inputFormat != TextureProcessor::TCFormat::None && inputFormat != f)
            {
              LogE("Failed to export texture cube %s.%s. Faces have different format!", p.second->GetObjectPath().UTF8().c_str(), face->GetObjectName().UTF8().c_str());
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
              LogE("Failed to export texture cube face %s.%s. No mipmaps!", cube->GetObjectPath().UTF8().c_str(), faces[faceIdx]->GetObjectName().UTF8().c_str());
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
          LogW("Failed to export a texture object %s(%s). Class is not supported!", p.second->GetObjectPath().UTF8().c_str(), p.second->GetClassName().UTF8().c_str());
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
  FbxImportContext ctx;
  ctx.ImportTangents = true;
  ctx.Path = path.ToStdWstring();
  FbxUtils utils;
  if (!utils.ImportSkeletalMesh(ctx))
  {
    wxMessageBox(ctx.Error, wxT("Error!"), wxICON_ERROR);
    return;
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
  Root = new osg::Geode;

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
    for (int32 faceIndex = 0; faceIndex < section->NumTriangles; ++faceIndex)
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
          tex->RenderTo(img.get());
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
    Root->addDrawable(geo.get());
  }
  Renderer->setSceneData(Root.get());
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
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
      addMenu->Append(MaterialsMenuID::ShowMaterials + idx, mats[idx] ? mats[idx]->GetObjectName().WString() : wxT("NULL"));
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
