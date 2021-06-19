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

class AObjectProperty : public wxLongStringProperty {
public:
  AObjectProperty(FPropertyValue* value, int32 idx);
  bool ValidateValue(wxVariant& value, wxPGValidationInfo& validationInfo) const override;

protected:
  bool DisplayEditorDialog(wxPropertyGrid* pg, wxVariant& value) override;
  void OnShowObjectClicked();
  void OnChangeObjectClicked(wxPropertyGrid* pg);

protected:
  FPropertyValue* Value = nullptr;
  bool AllowChanges = false;
};

class AByteArrayProperty : public wxLongStringProperty {
public:
  AByteArrayProperty(FPropertyValue* value, int32 idx);
  bool ValidateValue(wxVariant& value, wxPGValidationInfo& validationInfo) const override;

protected:
  bool DisplayEditorDialog(wxPropertyGrid* pg, wxVariant& value) override;
  void OnExportClicked(wxPropertyGrid* pg);
  void OnImportClicked(wxPropertyGrid* pg);

protected:
  FPropertyValue* Value = nullptr;
  bool AllowChanges = false;
};

class AGuideProperty : public wxStringProperty {
public:
  AGuideProperty(FPropertyValue* value, int32 idx);
  bool ValidateValue(wxVariant& value, wxPGValidationInfo& validationInfo) const override;

  void OnSetValue() override;

protected:
  FPropertyValue* Value = nullptr;
  bool AllowChanges = false;
};