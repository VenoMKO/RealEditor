#include "USoundNode.h"

void USoundNodeWave::Serialize(FStream& s)
{
  Super::Serialize(s);
  EditorData.Serialize(s, this);
  PCData.Serialize(s, this);
  XBoxData.Serialize(s, this);
  PS3Data.Serialize(s, this);
  UnkData1.Serialize(s, this);
  UnkData2.Serialize(s, this);
  UnkData3.Serialize(s, this);
  UnkData4.Serialize(s, this);
  UnkData5.Serialize(s, this);
}

void USoundNodeWave::PostLoad()
{
  if ((ResourceSize = PCData.GetBulkDataSize()))
  {
    PCData.GetCopy(&ResourceData);
  }
}
