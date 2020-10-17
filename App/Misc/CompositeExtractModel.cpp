#include "CompositeExtractModel.h"

void CompositeExtractModel::GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const
{
  switch (col)
  {
  case Col_Check:
    variant = Rows[row].Enabled;
    break;
  case Col_Package:
    variant = Rows[row].PackageName;
    break;
  case Col_Path:
    variant = Rows[row].ObjectPath;
    break;
  }
}

bool CompositeExtractModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const
{
  return false;
}

bool CompositeExtractModel::SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col)
{
  if (col == Col_Check)
  {
    Rows[row].Enabled = variant.GetBool();
    return true;
  }
  return false;
}
