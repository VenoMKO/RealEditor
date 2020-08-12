#pragma once
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

  virtual std::vector<FPropertyTag*> GetObjectProperties();

private:
  UObject* Object = nullptr;
  PackageWindow* Window = nullptr;
  bool Loading = false;
};