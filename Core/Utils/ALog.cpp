#include "ALog.h"

ALog* SharedLogger = nullptr;

ALog* ALog::SharedLog()
{
  if (!SharedLogger)
  {
    SharedLogger = new ALog();
  }
  return SharedLogger;
}

void ALog::Log(const std::string& msg, ALogEntry::Type channel)
{
  if (SharedLogger)
  {
    SharedLogger->Push({ std::time(nullptr), msg, channel });
  }
}

void ALog::ILog(const std::string& msg)
{
  Log(msg, ALogEntry::Type::INFO);
}

void ALog::ELog(const std::string& msg)
{
  Log(msg, ALogEntry::Type::ERR);
}

void ALog::WLog(const std::string& msg)
{
  Log(msg, ALogEntry::Type::WARN);
}

void ALog::GetEntries(std::vector<ALogEntry>& output, size_t& index)
{
  std::scoped_lock<std::recursive_mutex> lock(EntriesMutex);
  while (index < Entries.size())
  {
    output.push_back(Entries[index]);
    index++;
  }
}

void ALog::Push(const ALogEntry& entry)
{
  std::scoped_lock<std::recursive_mutex> lock(EntriesMutex);
  Entries.push_back(entry);
}