#include "LandscapeEditor.h"
#include <Tera/Utils/ALog.h>
#include <Tera/FPackage.h>
#include <osg/Image>
#include <osg/Texture2D>
#include <Tera/Utils/TextureUtils.h>

#include "../Windows/REDialogs.h"
#include "../Windows/TextureImporter.h"

#include <wx/filename.h>
#include "../resource.h"

void LandscapeProxyEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (Object && !toolbar->FindById(eID_Export))
  {
    toolbar->AddTool(eID_Export, "Export", wxBitmap(MAKE_IDB(IDB_EXPORT), wxBITMAP_TYPE_PNG_RESOURCE), "Export object data...");
    if (!Object->GetPackage()->GetPackageFlag(PKG_ROAccess))
    {
      toolbar->AddTool(eID_Import, "Import", wxBitmap(MAKE_IDB(IDB_IMPORT), wxBITMAP_TYPE_PNG_RESOURCE), "Import object data...");
      toolbar->FindById(eID_Import)->Enable(false);
    }
  }
  toolbar->AddTool(eID_ExportWeightMap, "Export Weights", wxBitmap(MAKE_IDB(IDB_EXPORT), wxBITMAP_TYPE_PNG_RESOURCE), "Export weight maps...");
}

void LandscapeProxyEditor::OnExportClicked(wxCommandEvent&)
{
  LogI("Export landscape...");
  wxString path = TextureImporterOptions::SaveImageDialog((wxWindow*)Window, Object->GetObjectNameString().WString());
  if (path.empty())
  {
    LogI("Export canceled by user!");
    return;
  }

  std::filesystem::path dst(path.ToStdWstring());
  dst.replace_extension(HasAVX2() ? "png" : "dds");

  TextureProcessor processor(TextureProcessor::TCFormat::G16, HasAVX2() ? TextureProcessor::TCFormat::PNG : TextureProcessor::TCFormat::DDS);
  processor.SetOutputPath(W2A(dst.wstring()));
  processor.SetInputData(HeightMapData.Allocation, HeightMapData.Size);
  processor.SetInputDataDimensions(HeightMapData.Width, HeightMapData.Height);

  if (!processor.Process())
  {
    LogW("Failed to export heights");
    REDialog::Error(processor.GetError());
  }
}

wxString LandscapeProxyEditor::OnExportWeightsClicked(wxCommandEvent&)
{
  wxDirDialog* openPanel = new wxDirDialog(this, wxS("Select a folder to export weight maps..."));
  openPanel->Center();
  wxString path;
  if (openPanel->ShowModal() == wxID_OK)
  {
    path = openPanel->GetPath();
  }
  openPanel->Destroy();
  return path;
}

void LandscapeProxyEditor::OnToolBarEvent(wxCommandEvent& e)
{
  if (e.GetId() == eID_ExportWeightMap)
  {
    OnExportWeightsClicked(e);
  }
  else
  {
    TextureEditor::OnToolBarEvent(e);
  }
}

void LandscapeProxyEditor::CreateRenderTexture()
{
  if (!Image)
  {
    Image = new osg::Image;
  }
  if (!HeightMapData.Allocation)
  {
    return;
  }

  Image->setImage(HeightMapData.Width, HeightMapData.Height, 0, HeightMapData.InternalFormat, HeightMapData.Format, HeightMapData.Type, (unsigned char*)HeightMapData.Allocation, osg::Image::AllocationMode::USE_MALLOC_FREE);

  const float minV = std::min(std::max(GetSize().x, Image->s()), std::max(GetSize().y, Image->t()));
  const float canvasSizeX = GetSize().x / minV;
  const float canvasSizeY = GetSize().y / minV;
  const float textureSizeX = Image->s() / minV;
  const float textureSizeY = Image->t() / minV;

  // Texture plane
  osg::ref_ptr<osg::Vec3Array> verticies = new osg::Vec3Array;
  verticies->push_back(osg::Vec3(textureSizeX * -.5, 0.0f, textureSizeY * -.5));
  verticies->push_back(osg::Vec3(textureSizeX * .5, 0.0f, textureSizeY * -.5));
  verticies->push_back(osg::Vec3(textureSizeX * .5, 0.0f, textureSizeY * .5));
  verticies->push_back(osg::Vec3(textureSizeX * -.5, 0.0f, textureSizeY * .5));

  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
  normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));

  osg::ref_ptr<osg::Vec2Array> uvs = new osg::Vec2Array;
  uvs->push_back(osg::Vec2(0.0f, 1.0f));
  uvs->push_back(osg::Vec2(1.0f, 1.0f));
  uvs->push_back(osg::Vec2(1.0f, 0.0f));
  uvs->push_back(osg::Vec2(0.0f, 0.0f));

  osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
  colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

  osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
  geo->setVertexArray(verticies.get());
  geo->setNormalArray(normals.get());
  geo->setNormalBinding(osg::Geometry::BIND_OVERALL);
  geo->setTexCoordArray(0, uvs.get());
  geo->setColorArray(colors);
  geo->setColorBinding(osg::Geometry::BIND_OVERALL);
  geo->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
  geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::Texture2D(Image));
  geo->getOrCreateStateSet()->setAttribute(Mask);

  Root = new osg::Geode;
  Root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
  Root->addDrawable(geo.get());

  Renderer->setSceneData(Root.get());
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->getCamera()->setProjectionMatrixAsOrtho2D(canvasSizeX * -.5f, canvasSizeX * .5f, canvasSizeY * -.5f, canvasSizeY * .5f);
  Renderer->getCamera()->setViewMatrixAsLookAt(osg::Vec3f(0.f, minV, 0.f), osg::Vec3f(0, 0.5f, 0), osg::Vec3f(0.f, 0.f, minV));

  // Texture border

  verticies = new osg::Vec3Array;
  verticies->push_back(osg::Vec3(textureSizeX * -.5, -0.1f, textureSizeY * -.5));
  verticies->push_back(osg::Vec3(textureSizeX * .5, -0.1f, textureSizeY * -.5));

  verticies->push_back(osg::Vec3(textureSizeX * .5, -0.1f, textureSizeY * -.5));
  verticies->push_back(osg::Vec3(textureSizeX * .5, -0.1f, textureSizeY * .5));

  verticies->push_back(osg::Vec3(textureSizeX * .5, -0.1f, textureSizeY * .5));
  verticies->push_back(osg::Vec3(textureSizeX * -.5, -0.1f, textureSizeY * .5));

  verticies->push_back(osg::Vec3(textureSizeX * -.5, -0.1f, textureSizeY * .5));
  verticies->push_back(osg::Vec3(textureSizeX * -.5, -0.1f, textureSizeY * -.5));

  normals = new osg::Vec3Array;
  normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));

  colors = new osg::Vec4Array;
  colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

  geo = new osg::Geometry;

  geo->setVertexArray(verticies.get());
  geo->setNormalArray(normals.get());
  geo->setNormalBinding(osg::Geometry::BIND_OVERALL);
  geo->setColorArray(colors);
  geo->setColorBinding(osg::Geometry::BIND_OVERALL);
  geo->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 8));

  Root->addDrawable(geo.get());
}

void LandscapeEditor::OnObjectLoaded()
{
  if (Loading || !Landscape)
  {
    Landscape = (ULandscape*)Object;
    if (Landscape && Landscape->LandscapeComponents.size())
    {
      Landscape->GetHeightMapData(HeightMapData);
      CreateRenderTexture();
    }
  }
  GenericEditor::OnObjectLoaded();
}

wxString LandscapeEditor::OnExportWeightsClicked(wxCommandEvent& e)
{
  wxString path = LandscapeProxyEditor::OnExportWeightsClicked(e);
  if (path.IsEmpty())
  {
    return wxEmptyString;
  }
  std::vector<LandscapeLayerStruct> LayerInfoObjs = Landscape->LayerInfoObjs;
  for (const LandscapeLayerStruct& l : LayerInfoObjs)
  {
    if (l.LayerInfoObj)
    {
      l.LayerInfoObj->Load();
      UTextureBitmapInfo bitmap;
      Landscape->GetWeighMapData(l.LayerInfoObj->LayerName, bitmap);

      std::filesystem::path dst(path.ToStdWstring());
      FString name = l.LayerInfoObj->LayerName.GetString();
      dst /= name.WString() + (HasAVX2() ? L".png" : L".dds");

      TextureProcessor processor(TextureProcessor::TCFormat::G8, HasAVX2() ? TextureProcessor::TCFormat::PNG : TextureProcessor::TCFormat::DDS);
      processor.SetOutputPath(W2A(dst.wstring()));
      processor.SetInputData(bitmap.Allocation, bitmap.Size);
      processor.SetInputDataDimensions(bitmap.Width, bitmap.Height);
      processor.Process();

      free(bitmap.Allocation);
    }
  }
  return path;
}

void LandscapeComponentEditor::OnObjectLoaded()
{
  if (Loading || !Component)
  {
    Component = (ULandscapeComponent*)Object;
    if (Component)
    {
      Component->GetHeightMapData(HeightMapData);
      CreateRenderTexture();
    }
  }
  GenericEditor::OnObjectLoaded();
}