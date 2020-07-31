#include "UObjectRedirector.h"
#include "FPackage.h"
#include "FName.h"

void UObjectRedirector::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Object;
}

void UObjectRedirector::PostLoad()
{
  if (Object)
  {
    Object->Load();
  }
}
