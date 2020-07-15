#include "AConfiguration.h"
#include "FStream.h"

#define SerializeKey(key) { uint16 k = key; s << k; } //
#define SerializeKeyValue(key, value) { uint16 k = key; s << k << value; } //
#define CheckKey(key) { if (!s.IsGood()) { return s; } uint16 k = key; s << k; if (k != key) { s.Close(); return s; }} //

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

FAppConfig AConfiguration::GetDefaultConfig() const
{
  return FAppConfig();
}

FAppConfig AConfiguration::GetConfig() const
{
  return Config;
}

void AConfiguration::SetConfig(const FAppConfig& cfg)
{
  Config = cfg;
}

FStream& operator<<(FStream& s, FLogConfig& c)
{
  if (s.IsReading())
  {
    while (s.IsGood())
    {
      uint16 key = 0;
      s << key;
      switch (key)
      {
      case FLogConfig::CFG_ShowLog:
        s << c.ShowLog;
        break;
      case FLogConfig::CFG_LogPos:
        s << c.LogPosition;
        break;
      case FLogConfig::CFG_LogSize:
        s << c.LogSize;
        break;
      case FLogConfig::CFG_End:
        return s;
      default:
        s.Close();
        return s;
      }
    }
  }
  else
  {
    SerializeKeyValue(FLogConfig::CFG_ShowLog, c.ShowLog);
    SerializeKeyValue(FLogConfig::CFG_LogPos, c.LogPosition);
    SerializeKeyValue(FLogConfig::CFG_LogSize, c.LogSize);

    // End
    SerializeKey(FLogConfig::CFG_End);
  }
  return s;
}

FStream& operator<<(FStream& s, FAppConfig& c)
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
    uint16 key = 0;
    while (s.GetPosition() < (FILE_OFFSET)c.Size)
    {
      s << key;
      switch (key)
      {
      case FAppConfig::CFG_RootDir:
        s << c.RootDir;
        break;
      case FAppConfig::CFG_LogBegin:
        s << c.LogConfig;
        CheckKey(FAppConfig::CFG_LogEnd);
        break;
      case FAppConfig::CFG_End:
        return s;
      default:
        s.Close();
        return s;
      }
    }
  }
  else
  {
    // Writing
    // General
    SerializeKeyValue(FAppConfig::CFG_RootDir, c.RootDir);

    // Log settings
    SerializeKey(FAppConfig::CFG_LogBegin);
    s << c.LogConfig;
    SerializeKey(FAppConfig::CFG_LogEnd);

    // End
    SerializeKey(FAppConfig::CFG_End);

    // Fixup storage size
    c.Size = s.GetSize();
    s.SetPosition(6);
    s << c.Size;
  }
  return s;
}
