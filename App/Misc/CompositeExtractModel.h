#pragma once
#include <wx/dataview.h>
#include <vector>

struct CompositeExtractModelNode {
  wxString ObjectPath;
  bool Enabled = true;
  wxString PackageName;
  int ObjectIndex = 0;
};

class CompositeExtractModel : public wxDataViewVirtualListModel {
public:
  enum
  {
    Col_Check = 0,
    Col_Package,
    Col_Path,
    Col_Max
  };

  CompositeExtractModel(std::vector<CompositeExtractModelNode> entries)
    : Rows(entries)
  {}

  unsigned int GetColumnCount() const override
  {
    return Col_Max;
  }

  wxString GetColumnType(unsigned int col) const override
  {
    if (col == Col_Check)
    {
      return "bool";
    }
    return wxDataViewCheckIconTextRenderer::GetDefaultType();
  }

  unsigned int GetCount() const override
  {
    return Rows.size();
  }

  void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override;

  bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override;

  bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override;

  std::vector<CompositeExtractModelNode> GetRows() const
  {
    return Rows;
  }

private:
  std::vector<CompositeExtractModelNode> Rows;
};