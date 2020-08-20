#include "TextureEditor.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include "../PackageWindow.h"

TextureEditor::~TextureEditor()
{
  if (Renderer)
  {
    delete Renderer;
  }
}

void TextureEditor::OnObjectLoaded()
{
  if (Loading)
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
  Renderer->getCamera()->setClearColor({.65, .65, .65, 1});
  Renderer->getCamera()->setGraphicsContext(OSGProxy);
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->getCamera()->setDrawBuffer(GL_BACK);
  Renderer->getCamera()->setReadBuffer(GL_BACK);
  Renderer->addEventHandler(new osgViewer::StatsHandler);
  
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

  const float minV = std::max(std::max(GetSize().x, image->s()), std::max(GetSize().y, image->t()));
  const float canvasSizeX = GetSize().x / minV;
  const float canvasSizeY = GetSize().y / minV;
  const float textureSizeX = image->s() / minV;
  const float textureSizeY = image->t() / minV;

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

  osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
  geometry->setVertexArray(verticies.get());
  geometry->setNormalArray(normals.get());
  geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
  geometry->setTexCoordArray(0, uvs.get());
  geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
  geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

  osg::ref_ptr<osg::Geode> root = new osg::Geode;
  root->addDrawable(geometry.get());
  root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
  root->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::Texture2D(Texture->GetTextureResource()));

  Renderer->setSceneData(root.get());
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
  Renderer->getCamera()->setProjectionMatrixAsOrtho2D(canvasSizeX * -.5f, canvasSizeX * .5f, canvasSizeY * -.5f, canvasSizeY * .5f);
  Renderer->getCamera()->setViewMatrixAsLookAt(osg::Vec3f(0.f, minV, 0.f), osg::Vec3f(0, 0.5f, 0), osg::Vec3f(0.f, 0.f, minV));
}
