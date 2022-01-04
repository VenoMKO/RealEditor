#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>

// Helper to find the decryption key from a running instance of Tera
struct DCKeyTool {
  // Return nullptr if no processes were found.
  // You must close the handle unless passed to the DCKeyTool ctor
  static HANDLE GetTeraProcess();
  // Passed handle will be closed in the dtor
  DCKeyTool(HANDLE process);
  
  ~DCKeyTool();

  bool FindKey();

  std::vector<std::pair<std::string, std::string>> GetResults() const
  {
    return Result;
  }

  bool IsX86();

protected:
  PMEMORY_BASIC_INFORMATION GetNextRegion();

protected:
  HANDLE Process = nullptr;
  LPVOID BaseAddress = nullptr;
  LPVOID MaxAddress = nullptr;
  LPVOID CurrentAddress = nullptr;
  std::vector<std::pair<std::string, std::string>> Result;
};