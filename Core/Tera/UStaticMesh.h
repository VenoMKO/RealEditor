#pragma once
#include "UObject.h"

class UStaticMesh : public UObject {
public:
  DECL_UOBJ(UStaticMesh, UObject);
  // Construct StaticMesh class and link properties
  static void ConfigureClassObject(UClass* object);
};