#pragma once
#include <wx/wx.h>
#include <wx/propgrid/manager.h>

#include <Tera/FPropertyTag.h>

extern void CreateProperty(wxPropertyGridManager* mgr, wxPropertyCategory* cat, const std::vector<FPropertyTag*>& tags);
extern void CreateProperty(wxPropertyGridManager* mgr, wxPropertyCategory* cat, FPropertyValue* value, int32 idx = -1);

class AIntProperty : public wxIntProperty {
public:
  AIntProperty(FPropertyValue* value, int32 idx);

  void OnSetValue() override;

protected:
  FPropertyValue* Value = nullptr;
};

class AFloatProperty : public wxFloatProperty {
public:
  AFloatProperty(FPropertyValue* value, int32 idx);

  void OnSetValue() override;

protected:
  FPropertyValue* Value = nullptr;
};

class ABoolProperty : public wxBoolProperty {
public:
  ABoolProperty(FPropertyValue* value, int32 idx);

  void OnSetValue() override;

protected:
  FPropertyValue* Value = nullptr;
};

class AByteProperty : public wxIntProperty {
public:
  AByteProperty(FPropertyValue* value, int32 idx);

  void OnSetValue() override;

protected:
  FPropertyValue* Value = nullptr;
};

class AEnumProperty : public wxEnumProperty {
public:
  AEnumProperty(FPropertyValue* value, int32 idx);

  void OnSetValue() override;

protected:
  FPropertyValue* Value = nullptr;
};

class AStringProperty : public wxStringProperty {
public:
  AStringProperty(FPropertyValue* value, int32 idx);

  void OnSetValue() override;

protected:
  FPropertyValue* Value = nullptr;
};

class ANameProperty : public wxStringProperty {
public:
  ANameProperty(FPropertyValue* value, int32 idx);

  void OnSetValue() override;

protected:
  FPropertyValue* Value = nullptr;
};

// Temporary solution
class AObjectProperty : public wxStringProperty {
public:
  AObjectProperty(FPropertyValue* value, int32 idx);
protected:
  FPropertyValue* Value = nullptr;
};