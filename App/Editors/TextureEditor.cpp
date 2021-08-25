#include "TextureEditor.h"
#include "../App.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osg/BlendFunc>
#include <nvtt/nvtt.h>

#include "../Windows/PackageWindow.h"
#include "../Windows/TextureImporter.h"
#include "../Windows/ProgressWindow.h"
#include "../Windows/REDialogs.h"

#include <Utils/ALog.h>
#include <Tera/Cast.h>
#include <Tera/FPackage.h>
#include <Utils/TextureProcessor.h>
#include <Utils/TextureTravaller.h>

#include <thread>

#include <wx/filename.h>

#include "../resource.h"

TextureEditor::TextureEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  Mask = new osg::ColorMask(true, true, true, false);
  CreateRenderer();
  window->FixOSG();
}

TextureEditor::~TextureEditor()
{
  if (Renderer)
  {
    delete Renderer;
  }
}

void TextureEditor::OnObjectLoaded()
{
  if (Loading || !Texture)
  {
    Texture = (UTexture2D*)Object;
    CreateRenderTexture();
  }
  GenericEditor::OnObjectLoaded();
}

void TextureEditor::OnTick()
{
  if (Renderer && Renderer->isRealized() && Renderer->checkNeedToDoFrame())
  {
    Renderer->frame();
  }
}

void TextureEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (auto item = toolbar->FindById(eID_Import))
  {
    item->Enable(Texture->Format == PF_DXT1 || Texture->Format == PF_DXT3 || Texture->Format == PF_DXT5 ||
                 Texture->Format == PF_A8R8G8B8 || Texture->Format == PF_G8);
  }
  toolbar->AddSeparator();
  toolbar->AddCheckTool(eID_Texture2D_Channel_R, wxEmptyString, wxBitmap(MAKE_IDB(IDB_T2D_RED), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_T2D_RED), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle red channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_R, Mask->getRedMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_G, wxEmptyString, wxBitmap(MAKE_IDB(IDB_T2D_GREEN), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_T2D_GREEN), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle green channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_G, Mask->getGreenMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_B, wxEmptyString, wxBitmap(MAKE_IDB(IDB_T2D_BLUE), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_T2D_BLUE), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle blue channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_B, Mask->getBlueMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_A, wxEmptyString, wxBitmap(MAKE_IDB(IDB_T2D_ALPHA), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_T2D_ALPHA), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle alpha channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_A, Mask->getAlphaMask());
}

void TextureEditor::OnToolBarEvent(wxCommandEvent& e)
{
  GenericEditor::OnToolBarEvent(e);
  if (e.GetSkipped())
  {
    // The base class has processed the event. Unmark the event and exit
    e.Skip(false);
    return;
  }
  auto eId = e.GetId();
  bool state = e.GetInt();
  if (eId == eID_Texture2D_Channel_A)
  {
    Mask->setAlphaMask(state);
    OnAlphaMaskChange();
    Renderer->requestRedraw();
  }
  else if (eId == eID_Texture2D_Channel_R)
  {
    Mask->setRedMask(state);
    Renderer->requestRedraw();
  }
  else if (eId == eID_Texture2D_Channel_G)
  {
    Mask->setGreenMask(state);
    Renderer->requestRedraw();
  }
  else if (eId == eID_Texture2D_Channel_B)
  {
    Mask->setBlueMask(state);
    Renderer->requestRedraw();
  }
}

void TextureEditor::OnAlphaMaskChange()
{
  if (!Root)
  {
    return;
  }
  osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc;
  if (Mask->getAlphaMask())
  {
    blendFunc->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Root->getOrCreateStateSet()->setAttributeAndModes(blendFunc);
    Root->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
  }
  else
  {
    blendFunc->setFunction(GL_ONE, GL_ZERO);
    Root->getOrCreateStateSet()->setAttributeAndModes(blendFunc);
    Root->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
  }
}

void TextureEditor::CreateRenderer()
{
  int attrs[] = { int(WX_GL_DOUBLEBUFFER), WX_GL_RGBA, WX_GL_DEPTH_SIZE, 8, WX_GL_STENCIL_SIZE, 8, 0 };

  Canvas = new OSGCanvas(Window, this, wxID_ANY, wxDefaultPosition, {100, 100}, wxNO_BORDER, wxT("OSGCanvas"), attrs);
  Canvas->SetLockLmb(true);
  Canvas->SetLockScroll(true);

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(Canvas, 1, wxALL | wxEXPAND);
  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);

  OSGProxy = new OSGWindow(Canvas);
  Canvas->SetGraphicsWindow(OSGProxy);
  Renderer = new osgViewer::Viewer;
  Renderer->setRunFrameScheme(osgViewer::ViewerBase::FrameScheme::ON_DEMAND);
  Renderer->getCamera()->setNearFarRatio(2);
  Renderer->getCamera()->setClearColor({0, 0, 0, 1});
  Renderer->getCamera()->setGraphicsContext(OSGProxy);
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->getCamera()->setDrawBuffer(GL_BACK);
  Renderer->getCamera()->setReadBuffer(GL_BACK);
#if _DEBUG
  Renderer->addEventHandler(new osgViewer::StatsHandler);
#endif
  
  // TODO: create an orthographic manipulator
  osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
  manipulator->setAllowThrow(false);
  manipulator->setVerticalAxisFixed(true);
  Renderer->setCameraManipulator(manipulator);
  Renderer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
}

void TextureEditor::CreateRenderTexture()
{
  if (!Image)
  {
    Image = new osg::Image;
  }
  if (!Texture->RenderTo(Image.get()))
  {
    Image = nullptr;
    return;
  }

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

  // Apply blending
  OnAlphaMaskChange();
}

void TextureEditor::OnImportClicked(wxCommandEvent&)
{
  TextureImporter importer(this, Texture);
  if (importer.Run())
  {
    CreateRenderTexture();
    SendEvent(Window, UPDATE_PROPERTIES);
  }
}

void TextureEditor::OnExportClicked(wxCommandEvent&)
{
  LogI("Export a texture...");

  FTexture2DMipMap* mip = nullptr;
  for (FTexture2DMipMap* mipmap : Texture->Mips)
  {
    if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
    {
      mip = mipmap;
      break;
    }
  }

  if (!mip)
  {
    LogE("Export canceled: This texture object has no mipmaps!");
    REDialog::Error("This texture object has no mipmaps!");
    return;
  }

  wxString path = TextureImporterOptions::SaveImageDialog(Window, Object->GetObjectNameString().WString());
  if (path.empty())
  {
    LogI("Export canceled by user!");
    return;
  }
  wxString ext = wxFileName(path).GetExt();
  if (ext.empty())
  {
    ext = wxT("dds");
  }
  ext.MakeLower();

  TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
  TextureProcessor::TCFormat outputFormat = TextureProcessor::TCFormat::None;

  if (Texture->Format == PF_DXT1)
  {
    inputFormat = TextureProcessor::TCFormat::DXT1;
  }
  else if (Texture->Format == PF_DXT3)
  {
    inputFormat = TextureProcessor::TCFormat::DXT3;
  }
  else if (Texture->Format == PF_DXT5)
  {
    inputFormat = TextureProcessor::TCFormat::DXT5;
  }
  else if (Texture->Format == PF_A8R8G8B8)
  {
    inputFormat = TextureProcessor::TCFormat::ARGB8;
  }
  else if (Texture->Format == PF_G8)
  {
    inputFormat = TextureProcessor::TCFormat::G8;
  }
  else
  {
    std::string msg = std::string("Format ") + PixelFormatToString(Texture->Format).String() + " is not supported!";
    REDialog::Error(msg);
    return;
  }

  if (ext == "png")
  {
    outputFormat = TextureProcessor::TCFormat::PNG;
  }
  else if (ext == "tga")
  {
    outputFormat = TextureProcessor::TCFormat::TGA;
  }
  else if (ext == "dds")
  {
    outputFormat = TextureProcessor::TCFormat::DDS;
  }
  else
  {
    wxMessageBox("Unknown file extension!");
    return;
  }

  TextureProcessor processor(inputFormat, outputFormat);
  
  processor.SetInputData(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
  processor.SetOutputPath(W2A(path.ToStdWstring()));
  processor.SetInputDataDimensions(mip->SizeX, mip->SizeY);

  bool result = false;
  std::string err;
  try
  {
    if (!(result = processor.Process()))
    {
      err = processor.GetError();
      if (err.empty())
      {
        err = "Texture Processor: failed with an unknown error!";
      }
    }
  }
  catch (const std::exception& e)
  {
    err = e.what();
    result = false;
  }
  catch (...)
  {
    result = false;
    err = processor.GetError();
    if (err.empty())
    {
      err = "Texture Processor: Unexpected exception!";
    }
  }
  if (!result)
  {
    LogE("Failed to export: %s", err.c_str());
    REDialog::Error(err);
  }
  else
  {
    LogI("Exported texture to: %s", path.ToStdString().c_str());
  }
}

void TextureEditor::SetNeedsUpdate()
{
  if (Renderer)
  {
    Renderer->requestRedraw();
  }
}

TextureCubeEditor::TextureCubeEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  Mask = new osg::ColorMask(true, true, true, false);
  CreateRenderer();
  window->FixOSG();
}

TextureCubeEditor::~TextureCubeEditor()
{
  if (Renderer)
  {
    delete Renderer;
  }
}

void TextureCubeEditor::OnObjectLoaded()
{
  if (Loading || !Cube)
  {
    Cube = (UTextureCube*)Object;
    CreateRenderTexture();
  }
  GenericEditor::OnObjectLoaded();
}

void TextureCubeEditor::OnTick()
{
  if (Renderer && Renderer->isRealized() && Renderer->checkNeedToDoFrame())
  {
    Renderer->frame();
  }
}

void TextureCubeEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (auto item = toolbar->FindById(eID_Import))
  {
    item->Enable(false);
  }
  toolbar->AddSeparator();
  toolbar->AddCheckTool(eID_Texture2D_Channel_R, wxEmptyString, wxBitmap(MAKE_IDB(IDB_T2D_RED), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_T2D_RED), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle red channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_R, Mask->getRedMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_G, wxEmptyString, wxBitmap(MAKE_IDB(IDB_T2D_GREEN), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_T2D_GREEN), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle green channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_G, Mask->getGreenMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_B, wxEmptyString, wxBitmap(MAKE_IDB(IDB_T2D_BLUE), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_T2D_BLUE), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle blue channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_B, Mask->getBlueMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_A, wxEmptyString, wxBitmap(MAKE_IDB(IDB_T2D_ALPHA), wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap(MAKE_IDB(IDB_T2D_ALPHA), wxBITMAP_TYPE_PNG_RESOURCE), "Toggle alpha channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_A, Mask->getAlphaMask());
}

void TextureCubeEditor::OnToolBarEvent(wxCommandEvent& e)
{
  GenericEditor::OnToolBarEvent(e);
  if (e.GetSkipped())
  {
    // The base class has processed the event. Unmark the event and exit
    e.Skip(false);
    return;
  }
  auto eId = e.GetId();
  bool state = e.GetInt();
  if (eId == eID_Texture2D_Channel_A)
  {
    Mask->setAlphaMask(state);
    OnAlphaMaskChange();
    Renderer->requestRedraw();
  }
  else if (eId == eID_Texture2D_Channel_R)
  {
    Mask->setRedMask(state);
    Renderer->requestRedraw();
  }
  else if (eId == eID_Texture2D_Channel_G)
  {
    Mask->setGreenMask(state);
    Renderer->requestRedraw();
  }
  else if (eId == eID_Texture2D_Channel_B)
  {
    Mask->setBlueMask(state);
    Renderer->requestRedraw();
  }
}

void TextureCubeEditor::OnExportClicked(wxCommandEvent&)
{
  LogI("Export a texture...");

  UTextureCube* cube = Cast<UTextureCube>(Object);

  if (!cube)
  {
    REDialog::Error("Failed to load the cube!");
    return;
  }

  auto faces = cube->GetFaces();
  TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
  for (UTexture2D* face : faces)
  {
    if (!face)
    {
      REDialog::Error("Failed to load the cube face!");
      return;
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
      REDialog::Error(wxString::Format("Failed to export texture cube %s.%s. Invalid face format!", cube->GetObjectPath().UTF8().c_str(), face->GetObjectNameString().UTF8().c_str()));
      return;
    }
    if (inputFormat != TextureProcessor::TCFormat::None && inputFormat != f)
    {
      REDialog::Error(wxString::Format("Failed to export texture cube %s.%s. Faces have different format!", cube->GetObjectPath().UTF8().c_str(), face->GetObjectNameString().UTF8().c_str()));
      return;
    }
    inputFormat = f;
  }

  wxString path = TextureImporterOptions::SaveImageDialog(Window, Object->GetObjectNameString().WString());
  if (path.empty())
  {
    LogI("Export canceled by user!");
    return;
  }

  wxString ext = wxFileName(path).GetExt();
  if (ext.empty())
  {
    ext = wxT("dds");
  }
  ext.MakeLower();

  TextureProcessor::TCFormat outputFormat = TextureProcessor::TCFormat::None;
  if (ext == "png")
  {
    outputFormat = TextureProcessor::TCFormat::PNG;
  }
  else if (ext == "tga")
  {
    outputFormat = TextureProcessor::TCFormat::TGA;
  }
  else if (ext == "dds")
  {
    outputFormat = TextureProcessor::TCFormat::DDS;
  }
  else
  {
    REDialog::Error("Unknown file extension!");
    return;
  }

  TextureProcessor processor(inputFormat, outputFormat);
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
      REDialog::Error(wxString::Format("Failed to export texture cube face %s.%s. No mipmaps!", cube->GetObjectPath().UTF8().c_str(), faces[faceIdx]->GetObjectNameString().UTF8().c_str()));
      return;
    }

    processor.SetInputCubeFace(faceIdx, mip->Data->GetAllocation(), mip->Data->GetBulkDataSize(), mip->SizeX, mip->SizeY);
  }

  processor.SetOutputPath(W2A(path.ToStdWstring()));

  bool result = false;
  std::string err;
  try
  {
    if (!(result = processor.Process()))
    {
      err = processor.GetError();
      if (err.empty())
      {
        err = "Texture Processor: failed with an unknown error!";
      }
    }
  }
  catch (const std::exception& e)
  {
    err = e.what();
    result = false;
  }
  catch (...)
  {
    result = false;
    err = processor.GetError();
    if (err.empty())
    {
      err = "Texture Processor: Unexpected exception!";
    }
  }
  if (!result)
  {
    LogE("Failed to export: %s", err.c_str());
    REDialog::Error(err);
  }
  else
  {
    LogI("Exported texture to: %s", path.ToStdString().c_str());
  }
}

void TextureCubeEditor::OnAlphaMaskChange()
{
  if (!Root)
  {
    return;
  }
  osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc;
  if (Mask->getAlphaMask())
  {
    blendFunc->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Root->getOrCreateStateSet()->setAttributeAndModes(blendFunc);
    Root->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
  }
  else
  {
    blendFunc->setFunction(GL_ONE, GL_ZERO);
    Root->getOrCreateStateSet()->setAttributeAndModes(blendFunc);
    Root->getOrCreateStateSet()->setRenderingHint(osg::StateSet::OPAQUE_BIN);
  }
}

void TextureCubeEditor::CreateRenderer()
{
  int attrs[] = { int(WX_GL_DOUBLEBUFFER), WX_GL_RGBA, WX_GL_DEPTH_SIZE, 8, WX_GL_STENCIL_SIZE, 8, 0 };

  Canvas = new OSGCanvas(Window, this, wxID_ANY, wxDefaultPosition, { 100, 100 }, wxNO_BORDER, wxT("OSGCanvas"), attrs);
  Canvas->SetLockLmb(true);
  Canvas->SetLockScroll(true);

  wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(Canvas, 1, wxALL | wxEXPAND);
  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);

  OSGProxy = new OSGWindow(Canvas);
  Canvas->SetGraphicsWindow(OSGProxy);
  Renderer = new osgViewer::Viewer;
  Renderer->setRunFrameScheme(osgViewer::ViewerBase::FrameScheme::ON_DEMAND);
  Renderer->getCamera()->setNearFarRatio(2);
  Renderer->getCamera()->setClearColor({ 0, 0, 0, 1 });
  Renderer->getCamera()->setGraphicsContext(OSGProxy);
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->getCamera()->setDrawBuffer(GL_BACK);
  Renderer->getCamera()->setReadBuffer(GL_BACK);
#if _DEBUG
  Renderer->addEventHandler(new osgViewer::StatsHandler);
#endif

  // TODO: create an orthographic manipulator
  osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
  manipulator->setAllowThrow(false);
  manipulator->setVerticalAxisFixed(true);
  Renderer->setCameraManipulator(manipulator);
  Renderer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
}

void TextureCubeEditor::CreateRenderTexture()
{
  if (!Image)
  {
    Image = new osg::Image;
  }
  if (!Cube->RenderTo(Image.get()))
  {
    Image = nullptr;
    return;
  }

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

  // Apply blending
  OnAlphaMaskChange();
}
