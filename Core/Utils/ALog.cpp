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