#include "TextureEditor.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osg/BlendFunc>

#include "../PackageWindow.h"

#include <Tera/ALog.h>

TextureEditor::TextureEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  Mask = new osg::ColorMask(true, true, true, false);
  CreateRenderer();
  
  // Osg refuses to process events
  // Resizing the window for some reasone fixes the issue
  // TODO: fix the issue and get rid of the shitty hack below
  Window->Freeze();
  wxSize s = Window->GetSize();
  s.x += 1;
  Window->SetSize(s);
  s.x -= 1;
  Window->SetSize(s);
  Window->Thaw();
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
  if (Renderer && Renderer->isRealized())
  {
    Renderer->frame();
  }
}

void TextureEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  toolbar->AddSeparator();
  toolbar->AddCheckTool(eID_Texture2D_Channel_R, wxEmptyString, wxBitmap("#109", wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap("#109", wxBITMAP_TYPE_PNG_RESOURCE), "Toggle red channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_R, Mask->getRedMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_G, wxEmptyString, wxBitmap("#107", wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap("#107", wxBITMAP_TYPE_PNG_RESOURCE), "Toggle green channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_G, Mask->getGreenMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_B, wxEmptyString, wxBitmap("#105", wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap("#105", wxBITMAP_TYPE_PNG_RESOURCE), "Toggle blue channel");
  toolbar->ToggleTool(eID_Texture2D_Channel_B, Mask->getBlueMask());
  toolbar->AddCheckTool(eID_Texture2D_Channel_A, wxEmptyString, wxBitmap("#103", wxBITMAP_TYPE_PNG_RESOURCE), wxBitmap("#103", wxBITMAP_TYPE_PNG_RESOURCE), "Toggle alpha channel");
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
  }
  else if (eId == eID_Texture2D_Channel_R)
  {
    Mask->setRedMask(state);
  }
  else if (eId == eID_Texture2D_Channel_G)
  {
    Mask->setGreenMask(state);
  }
  else if (eId == eID_Texture2D_Channel_B)
  {
    Mask->setBlueMask(state);
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
  int attrs[] = { int(WX_GL_DOUBLEBUFFER), WX_GL_RGBA, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 16, 0 };

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
  osg::ref_ptr<osg::Image> image = Texture->GetTextureResource();
  image->setFileName(Texture->GetObjectName().String());
  if (!image)
  {
    return;
  }

  const float minV = std::min(std::max(GetSize().x, image->s()), std::max(GetSize().y, image->t()));
  const float canvasSizeX = GetSize().x / minV;
  const float canvasSizeY = GetSize().y / minV;
  const float textureSizeX = image->s() / minV;
  const float textureSizeY = image->t() / minV;

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
  geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::Texture2D(Texture->GetTextureResource()));
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

void TextureEditor::OnImportClicked(wxCommandEvent& e)
{
  wxString path = wxLoadFileSelector("texture", ".png; .dds", wxEmptyString, Window);
  // TODO: import data
}