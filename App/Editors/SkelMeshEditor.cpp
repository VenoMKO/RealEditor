#include "SkelMeshEditor.h"
#include "../Windows/PackageWindow.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
#include <osg/BlendFunc>

#include <Utils/FbxUtils.h>

enum ExportMode {
  ExportGeometry = wxID_HIGHEST + 1,
  ExportFull
};

SkelMeshEditor::SkelMeshEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
{
  CreateRenderer();
  window->FixOSG();
}

void SkelMeshEditor::OnTick()
{
  if (Renderer && Renderer->isRealized())
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

void SkelMeshEditor::OnExportMenuClicked(wxCommandEvent& e)
{
  FbxExportContext ctx;
  ctx.ExportSkeleton = e.GetId() == ExportMode::ExportFull;
  wxString path = wxSaveFileSelector("mesh", wxT("FBX file|*.fbx"), Object->GetObjectName().WString(), Window);
  if (path.empty())
  {
    return;
  }
  ctx.Path = path.ToStdWstring();
  FbxUtils utils;
  if (!utils.ExportSkeletalMesh((USkeletalMesh*)Object, ctx))
  {
    wxMessageBox(ctx.Error, wxT("Error!"), wxICON_ERROR);
  }
}

void SkelMeshEditor::OnExportClicked(wxCommandEvent&)
{
  wxMenu menu;
  menu.Append(ExportMode::ExportFull, wxT("Export rigged geometry"));
  menu.Append(ExportMode::ExportGeometry, wxT("Export geometry"));
  menu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(SkelMeshEditor::OnExportMenuClicked), NULL, this);
  PopupMenu(&menu);
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

  // TODO: use sections to build multi-part objects with different materials
  {
    std::vector<FSoftSkinVertex> uvertices = model->GetVertices();
    for (int32 idx = 0; idx < uvertices.size(); ++idx)
    {
      FVector normal = uvertices[idx].TangentZ;
      normals->push_back(osg::Vec3(normal.X, normal.Y, normal.Z));
      vertices->push_back(osg::Vec3(uvertices[idx].Position.X, uvertices[idx].Position.Y, uvertices[idx].Position.Z));
      uvs->push_back(osg::Vec2(uvertices[idx].UVs[0].X, uvertices[idx].UVs[0].Y));
    }
  }

  osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES);
  {
    const FMultiSizeIndexContainer* indexContainer = model->GetIndexContainer();
    for (int32 idx = 0; idx < indexContainer->GetElementCount(); idx+=3)
    {
      indices->push_back(indexContainer->GetIndex(idx + 0));
      indices->push_back(indexContainer->GetIndex(idx + 2));
      indices->push_back(indexContainer->GetIndex(idx + 1));
    }
  }
  
  osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
  geo->addPrimitiveSet(indices.get());
  geo->setVertexArray(vertices.get());
  geo->setNormalArray(normals.get());
  geo->setTexCoordArray(0, uvs.get());

  Root->addDrawable(geo.get());

  Renderer->setSceneData(Root.get());
  Renderer->getCamera()->setViewport(0, 0, GetSize().x, GetSize().y);
}
