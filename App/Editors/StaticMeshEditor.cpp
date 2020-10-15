#include "StaticMeshEditor.h"
#include "../Windows/PackageWindow.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osg/BlendFunc>
#include <osg/Depth>

#include <Utils/FbxUtils.h>

#include <Tera/Cast.h>
#include <Tera/UMaterial.h>

StaticMeshEditor::StaticMeshEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  CreateRenderer();
  window->FixOSG();
}

StaticMeshEditor::~StaticMeshEditor()
{
  if (Renderer)
  {
    delete Renderer;
  }
}

void StaticMeshEditor::OnTick()
{
  if (Renderer && Renderer->isRealized() && Renderer->checkNeedToDoFrame())
  {
    Renderer->frame();
  }
}

void StaticMeshEditor::OnObjectLoaded()
{
  if (Loading || !Mesh)
  {
    Mesh = (UStaticMesh*)Object;
    CreateRenderModel();
  }
  GenericEditor::OnObjectLoaded();
}

void StaticMeshEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  toolbar->AddTool(eID_Refresh, wxT("Reload"), wxBitmap("#122", wxBITMAP_TYPE_PNG_RESOURCE), "Reload model and its textures");
}

void StaticMeshEditor::OnToolBarEvent(wxCommandEvent& e)
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
}

void StaticMeshEditor::OnExportClicked(wxCommandEvent&)
{
  FbxExportContext ctx;
  ctx.ExportSkeleton = false;
  wxString path = wxSaveFileSelector("mesh", wxT("FBX file|*.fbx"), Object->GetObjectName().WString(), Window);
  if (path.empty())
  {
    return;
  }
  ctx.Path = path.ToStdWstring();
  FbxUtils utils;
  if (!utils.ExportStaticMesh(Mesh, ctx))
  {
    wxMessageBox(ctx.Error, wxT("Error!"), wxICON_ERROR);
  }
}

void StaticMeshEditor::SetNeedsUpdate()
{
  if (Renderer)
  {
    Renderer->requestRedraw();
  }
}

void StaticMeshEditor::CreateRenderer()
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

void StaticMeshEditor::CreateRenderModel()
{
  if (!Mesh)
  {
    return;
  }
  Root = new osg::Geode;

  const FStaticMeshRenderData* model = Mesh->GetLod(0);

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  osg::ref_ptr<osg::Vec2Array> uvs = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX);

  std::vector<FStaticVertex> uvertices = model->GetVertices();
  for (int32 idx = 0; idx < uvertices.size(); ++idx)
  {
    FVector normal = uvertices[idx].TangentZ;
    normals->push_back(osg::Vec3(normal.X, normal.Y, normal.Z));
    vertices->push_back(osg::Vec3(uvertices[idx].Position.X, uvertices[idx].Position.Y, uvertices[idx].Position.Z));
    uvs->push_back(osg::Vec2(uvertices[idx].UVs[0].X, uvertices[idx].UVs[0].Y));
  }

  std::vector<FStaticMeshElement> elements = model->GetElements();
  const FRawIndexBuffer& indexContainer = model->IndexBuffer;
  for (const FStaticMeshElement& section : elements)
  {
    if (!section.NumTriangles)
    {
      continue;
    }
    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES);
    for (int32 faceIndex = 0; faceIndex < section.NumTriangles; ++faceIndex)
    {
      indices->push_back(indexContainer.GetIndex(section.FirstIndex + (faceIndex * 3) + 0));
      indices->push_back(indexContainer.GetIndex(section.FirstIndex + (faceIndex * 3) + 2));
      indices->push_back(indexContainer.GetIndex(section.FirstIndex + (faceIndex * 3) + 1));
    }
    geo->addPrimitiveSet(indices.get());
    geo->setVertexArray(vertices.get());
    geo->setNormalArray(normals.get());
    geo->setTexCoordArray(0, uvs.get());

    if (UMaterialInterface* material = Cast<UMaterialInterface>(section.Material))
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

    Root->addDrawable(geo.get());
  }

  Renderer->setSceneData(Root.get());
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
}

void StaticMeshEditor::OnRefreshClicked()
{
  CreateRenderModel();
}
