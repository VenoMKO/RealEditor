#include "UObjectRedirector.h"
#include "FPackage.h"
#include "FObjectResource.h"
#include "FName.h"

void UObjectRedirector::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, Object);
  DBreakIf(ObjectRefIndex && !Object);
}