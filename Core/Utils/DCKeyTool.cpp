#include "DCKeyTool.h"

#include <algorithm>
#include <functional>
#include <numeric>

#include <psapi.h>
#include <tchar.h>
#include <Sysinfoapi.h>
#include <Tera/FString.h>

#include <Zydis/Zydis.h>

const std::vector<uint8_t> SearchPattern = { 0x48, 0x33, 0xC4, 0x48, 0x89, 0x84, 0x24, 0x40, 0x01, 0x00, 0x00, 0x4C, 0x8B, 0xF9, 0x41, 0xC7, 0x43 };

HANDLE DCKeyTool::GetTeraProcess()
{
  DWORD pid = 0;
  DWORD processes[2048];
  DWORD arraySize = 0;
  if (!EnumProcesses(processes, sizeof(processes), &arraySize))
  {
    return nullptr;
  }
  DWORD count = arraySize / sizeof(DWORD);
  for (unsigned int idx = 0; idx < count; ++idx)
  {
    TCHAR processName[MAX_PATH] = TEXT("None");
    if (HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[idx]))
    {
      HMODULE hMod;
      DWORD cbNeeded;
      if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
      {
        if (GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(TCHAR)))
        {
          if (!_tcscmp(processName, _T("TERA.exe")))
          {
            return hProcess;
          }
        }
      }
      CloseHandle(hProcess);
    }
  }
  return nullptr;
}

DCKeyTool::DCKeyTool(HANDLE process)
  : Process(process)
{
}

DCKeyTool::~DCKeyTool()
{
  if (Process)
  {
    CloseHandle(Process);
  }
}

bool DCKeyTool::FindKey()
{
  if (!Process)
  {
    return false;
  }
  Result.clear();
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  BaseAddress = info.lpMinimumApplicationAddress;
  MaxAddress = info.lpMaximumApplicationAddress;
  CurrentAddress = BaseAddress;
  auto hasValidRights = [](DWORD protect) { return (protect & PAGE_EXECUTE) || (protect & PAGE_EXECUTE_READ) || (protect & PAGE_EXECUTE_READWRITE);};
  while (PMEMORY_BASIC_INFORMATION region = GetNextRegion())
  {
    if (!hasValidRights(region->Protect))
    {
      delete region;
      continue;
    }
    std::vector<uint8_t> memory(region->RegionSize);
    size_t readSize = 0;
    if (!ReadProcessMemory(Process, (LPVOID)region->BaseAddress, memory.data(), memory.size(), &readSize) || !readSize || readSize < SearchPattern.size())
    {
      delete region;
      continue;
    }
    if (readSize < memory.size())
    {
      memory.resize(readSize);
    }
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
    auto it = std::search(memory.begin(), memory.end(), std::boyer_moore_searcher(SearchPattern.begin(), SearchPattern.end()));
    while (it != memory.end())
    {
      ZydisDecodedInstruction instruction;
      std::vector<ZyanU32> raw;
      while (it != memory.end() && ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, &*it, memory.end() - it, &instruction)))
      {
        if (instruction.mnemonic == ZYDIS_MNEMONIC_MOV && instruction.operands[0].mem.base == ZYDIS_REGISTER_R11)
        {
          raw.emplace_back((ZyanU32)instruction.operands[1].imm.value.u);
        }
        else if (raw.size() == 8)
        {
          std::string key(35, '-');
          std::string vec(35, '-');
          int intIdx = 0;
          for (; intIdx < 4; ++intIdx)
          {
            FString::BytesToString((char*)&raw[intIdx], sizeof(ZyanU32), &key[intIdx * (sizeof(ZyanU32) * 2 + 1)]);
          }
          for (int chIdx = 0; intIdx < 8; ++intIdx, ++chIdx)
          {
            FString::BytesToString((char*)&raw[intIdx], sizeof(ZyanU32), &vec[chIdx * (sizeof(ZyanU32) * 2 + 1)]);
          }
          Result.emplace_back(std::make_pair(key, vec));
          break;
        }
        it += instruction.length;
      }
      it = std::search(it, memory.end(), std::boyer_moore_searcher(SearchPattern.begin(), SearchPattern.end()));
    }
    delete region;
  }
  return Result.size();
}

PMEMORY_BASIC_INFORMATION DCKeyTool::GetNextRegion()
{
  while (CurrentAddress < MaxAddress)
  {
    PMEMORY_BASIC_INFORMATION result = new MEMORY_BASIC_INFORMATION;
    if (!VirtualQueryEx(Process, CurrentAddress, result, sizeof(MEMORY_BASIC_INFORMATION)))
    {
      delete result;
      return nullptr;
    }
    *(uint64_t*)&CurrentAddress += result->RegionSize;
    return result;
  }
  return nullptr;
}
