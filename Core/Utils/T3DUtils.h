#pragma once
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include <Tera/FStructs.h>
#include <Tera/FString.h>

class T3DFile {
public:
  inline void InitializeMap()
  {
    Begin("Map");
    Begin("Level");
  }

  inline void FinalizeMap()
  {
    End();
    End();
  }

  const std::string& GetBody()
  {
    return Body;
  }

  inline bool Save(const std::wstring& path)
  {
    try
    {
      std::ofstream s(path, std::ios::out);
      s.write(Body.c_str(), Body.size());
    }
    catch (...)
    {
      return false;
    }
    return true;
  }

  inline void Begin(const char* type, const char* objectClass = nullptr, const char* objectName = nullptr)
  {
    Body += Padding() + "Begin " + type;
    ObjectStack.push_back(type);
    if (objectClass)
    {
      Body += " Class=/Script/Engine.";
      Body += objectClass;
    }
    if (objectName)
    {
      Body += " Name=\"";
      Body += objectName;
      Body += "\"";
    }
    Body += "\n";
  }

  inline void End()
  {
    std::string end = "End " + ObjectStack.back() + "\n";
    ObjectStack.pop_back();
    Body += Padding() + end;
  }

  inline void AddBool(const char* name, bool value)
  {
    AddParameter(name, value ? "True" : "False");
  }

  inline void AddFloat(const char* name, float value)
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << value;
    std::string mystring = ss.str();
    AddParameter(name, ss.str().c_str());
  }

  inline void AddGuid(const char* name, const FGuid& value)
  {
    AddParameter(name, value.String().UTF8().c_str());
  }

  inline void AddColor(const char* name, const FColor& value)
  {
    AddParameter(name, FString::Sprintf("(B=%d,G=%d,R=%d,A=%d)",int(value.B), int(value.G), int(value.R), int(value.A)).UTF8().c_str());
  }

  inline void AddLinearColor(const char* name, const FLinearColor& value)
  {
    AddParameter(name, FString::Sprintf("(B=%06f,G=%06f,R=%06f,A=%06f)", int(value.B), int(value.G), int(value.R), int(value.A)).UTF8().c_str());
  }

  inline void AddVector(const char* name, const FVector& value)
  {
    AddParameter(name, FString::Sprintf("(X=%06f,Y=%06f,Z=%06f)", value.X, value.Y, value.Z).UTF8().c_str());
  }

  inline void AddRotator(const char* name, const FRotator& value)
  {
    FVector e = value.Normalized().Euler();
    AddParameter(name, FString::Sprintf("(Pitch=%06f,Yaw=%06f,Roll=%06f)", e.Y, e.Z, e.X).UTF8().c_str());
  }

  inline void AddString(const char* name, const char* value)
  {
    AddParameter(name, (std::string("\"") + value + "\"").c_str());
  }

  inline void AddCustom(const char* name, const char* value)
  {
    AddParameter(name, value);
  }

  inline void AddPosition(const FVector& value)
  {
    AddVector("RelativeLocation", value);
  }

  inline void AddRotation(const FRotator& value)
  {
    AddRotator("RelativeRotation", value);
  }

  inline void AddScale(const FVector& value, float scale = 1.)
  {
    AddVector("RelativeScale3D", value * scale);
  }

  inline void AddStaticMesh(const char* path)
  {
    AddParameter("StaticMesh", FString::Sprintf("StaticMesh'\"/Game/%s\"'", path).UTF8().c_str());
  }

  inline void AddSkeletalMesh(const char* path)
  {
    AddParameter("SkeletalMesh", FString::Sprintf("SkeletalMesh'\"/Game/%s\"'", path).UTF8().c_str());
  }

protected:
  inline std::string Padding() const
  {
    return ObjectStack.empty() ? std::string() : std::string(ObjectStack.size() * 3, ' ');
  }

  inline void AddParameter(const char* name, const char* value)
  {
    Body += Padding() + name + "=" + value + "\n";
  }

private:
  std::string Body;
  std::vector<std::string> ObjectStack;
};