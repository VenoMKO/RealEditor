#include "AConfiguration.h"
#include "FStream.h"

enum ConfigKeys : uint16 {
  CFG_RootDir = 1,
  CFG_End = 0xFFFF
};

AConfiguration::AConfiguration(const std::string& path)
  : Path(path)
{}

bool AConfiguration::Load()
{
  FStream* s = new FReadStream(Path);
  uint32 size = (uint32)s->GetSize();
  if (!s->IsGood() || !size)
  {
    delete s;
    return false;
  }

  (*s) << Config;
  
  bool result = s->IsGood();
  delete s;
  return result;
}

bool AConfiguration::Save()
{
  FStream* s = new FWriteStream(Path);
  if (!s->IsGood())
  {
    delete s;
    return false;
  }

  (*s) << Config;

  delete s;
  return true;
}

FConfig AConfiguration::GetDefaultConfig() const
{
  return FConfig();
}

FConfig AConfiguration::GetConfig() const
{
  return Config;
}

void AConfiguration::SetConfig(const FConfig& cfg)
{
  Config = cfg;
}

FStream& operator<<(FStream& s, FConfig& c)
{
  s << c.Magic;
  if (c.Magic != PACKAGE_MAGIC)
  {
    s.Close();
  }
  s << c.Version;
  s << c.Size;
  if (s.IsReading())
  {
    while (s.GetPosition() < c.Size)
    {
      uint16 key = 0;
      s << key;
      switch (key)
      {
      case CFG_RootDir:
        s << c.RootDir;
        break;
      case CFG_End:
        return s;
      default:
        s.Close();
        return s;
      }
    }
  }
  else
  {
#define SK(key) { uint16 k = key; s << k; } //
#define SKV(key, value) { uint16 k = key; s << k << value; } //

    SKV(CFG_RootDir, c.RootDir);

    // End
    SK(CFG_End);
    // Fixup storage size
    c.Size = s.GetSize();
    s.SetPosition(6);
    s << c.Size;
  }
  return s;
}
