#pragma once

class GenericEditor : public wxPanel
{
public:
  static GenericEditor* CreateEditor(wxPanel *parent, UObject* object);
  GenericEditor(wxPanel* parent);

  virtual void LoadObject();
  
  inline UObject* GetObject()
  {
    return Object;
  }
private:
  UObject* Object = nullptr;
};