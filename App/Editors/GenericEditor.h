#pragma once
#include <wx/wx.h>
#include <Tera/FPropertyTag.h>

enum ToolEventID : int {
  eID_Export = 0,
  eID_Import,
  eID_Composite,
  eID_Class,
  eID_Origin,
  eID_Refresh,
  eID_Materials,
  eID_Texture2D_Channel_R,
  eID_Texture2D_Channel_G,
  eID_Texture2D_Channel_B,
  eID_Texture2D_Channel_A,
  eID_Level_Load,
  eID_StreamingLevel_Source,
  eID_SoundPlay,
  eID_SoundStop,
  eID_SoundPause,
};

class UObject;
class PackageWindow;
class GenericEditor : public wxPanel
{
public:
  static GenericEditor* CreateEditor(wxPanel *parent, PackageWindow* window, UObject* object);
  GenericEditor(wxPanel* parent, PackageWindow* window);

  virtual void LoadObject();

  virtual void OnObjectLoaded();
  
  virtual UObject* GetObject()
  {
    return Object;
  }

  std::string GetEditorId() const;

  inline bool IsLoading() const
  {
    return Loading;
  }

  virtual void SetNeedsUpdate()
  {}

  virtual void OnTick()
  {}

  virtual std::vector<FPropertyTag*> GetObjectProperties();

  virtual void PopulateToolBar(wxToolBar* toolbar);

  virtual void ClearToolbar();
  
  virtual void OnToolBarEvent(wxCommandEvent& e);

  virtual void OnExportClicked(wxCommandEvent& e);

  virtual void OnImportClicked(wxCommandEvent& e);

  virtual void OnSourceClicked(wxCommandEvent& e);

  virtual void OnClassClicked(wxCommandEvent& e);

protected:
  virtual void OnObjectSet()
  {
  }

  void SetObject(UObject* object)
  {
    if ((Object = object))
    {
      OnObjectSet();
    }
  }

protected:
  UObject* Object = nullptr;
  wxToolBar* Toolbar = nullptr;
  wxString CompositeObjectPath;
  PackageWindow* Window = nullptr;
  bool Loading = false;
  bool NeedsUpdate = false;
};