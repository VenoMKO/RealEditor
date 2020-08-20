#pragma once
#include <wx/wx.h>
#include <Tera/FPropertyTag.h>


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

  virtual void OnTick()
  {}

  virtual std::vector<FPropertyTag*> GetObjectProperties();

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
  PackageWindow* Window = nullptr;
  bool Loading = false;
};