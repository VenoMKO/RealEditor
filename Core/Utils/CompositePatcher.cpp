#include "CompositePatcher.h"

#include <array>
#include <string_view>

#include <Tera/FMapper.h>

CompositePatcher::CompositePatcher(const std::wstring& path)
  : Path(path)
{}

void CompositePatcher::Load()
{
  GDecrytMapperFile(Path, Decrypted);
}

void CompositePatcher::Apply()
{
  GEncrytMapperFile(Path, Decrypted);
}

bool CompositePatcher::DeleteEntry(const std::string& compositePackageName)
{
  std::string prefix = compositePackageName + '.';
  size_t start = Decrypted.find(prefix);
  if (start == std::string::npos)
  {
    return false;
  }
  size_t end = Decrypted.find('|', start);
  if (end == std::string::npos)
  {
    return false;
  }
  
  end++;

  if (start >= end)
  {
    return false;
  }

  memmove(&Decrypted[start], &Decrypted[end], Decrypted.size() - end);
  Decrypted.resize(Decrypted.size() - (end - start));

  if (Decrypted[start] == '!')
  {
    // This as the last file entry. Remove the empty file with trailing "?!"
    size_t fstart;
    size_t fend = Decrypted.rfind('?', start);
    if (fend == std::string::npos)
    {
      return false;
    }
    fstart = Decrypted.rfind('!', fend);
    if (fstart == std::string::npos)
    {
      fstart = 0;
    }
    memmove(&Decrypted[fstart], &Decrypted[start], Decrypted.size() - start);
    Decrypted.resize(Decrypted.size() - (start - fstart));
  }
  return false;
}

std::string CompositePatcher::Patch(const std::string& compositePackageName, const CompositeEntry & dest)
{
  if (Decrypted.empty())
  {
    throw "Composite map is empty!";
  }
  std::string prefix = compositePackageName + '.';
  size_t start = Decrypted.find(prefix);
  if (start == std::string::npos)
  {
    throw "Failed to find the entry " + compositePackageName;
  }
  size_t end = Decrypted.find('|', start);
  if (end == std::string::npos)
  {
    throw "Failed to find the end of the entry " + compositePackageName;
  }
  end++;
  std::string previousEntry = Decrypted.substr(start, end - start);

  if (previousEntry.empty())
  {
    throw "Failed to find the entry " + compositePackageName;
  }

  // Find previousFilename
  std::string previousFilename;
  size_t fstart;
  {
    size_t fend = Decrypted.rfind('?', start);
    if (fend == std::string::npos)
    {
      throw "Failed to find the entries filename";
    }
    fstart = Decrypted.rfind('!', fend);
    if (fstart == std::string::npos)
    {
      fstart = 0;
    }
    previousFilename = Decrypted.substr(fstart + 1, fend - fstart - 1);
  }

  if (!dest.Size)
  {
    // Delete the entry and return
    memmove(&Decrypted[start], &Decrypted[end], Decrypted.size() - end);
    Decrypted.resize(Decrypted.size() - previousEntry.size());
    return previousFilename + previousEntry;
  }

  std::string newEntry = dest.ToString();

  if (previousFilename == dest.Filename)
  {
    // Just changing values

    if (previousEntry.size() == newEntry.size())
    {
      memcpy(&Decrypted[start], &newEntry[0], newEntry.size());
    }
    else if (previousEntry.size() > newEntry.size())
    {
      // Copy new value
      memcpy(&Decrypted[start], &newEntry[0], newEntry.size());
      // Fit trailing data
      memmove(&Decrypted[start + newEntry.size()], &Decrypted[end], Decrypted.size() - end);
      // Shrink
      Decrypted.resize(Decrypted.size() - (previousEntry.size() - newEntry.size()));
    }
    else
    {
      // Enlarge
      Decrypted.resize(Decrypted.size() + (newEntry.size() - previousEntry.size()));
      // Move trailing data to the end
      memmove(&Decrypted[start + newEntry.size()], &Decrypted[end], Decrypted.size() - (end + (newEntry.size() - previousEntry.size())));
      // Copy new value
      memcpy(&Decrypted[start], &newEntry[0], newEntry.size());
    }
  }
  else
  {
    // Moving to a different storage

    // But first delete the old entry
    memmove(&Decrypted[start], &Decrypted[end], Decrypted.size() - end);
    Decrypted.resize(Decrypted.size() - previousEntry.size());

    if (Decrypted[start] == '!')
    {
      // This as the last file entry. Remove the empty file with trailing "?!"
      memmove(&Decrypted[fstart], &Decrypted[start], Decrypted.size() - start);
      Decrypted.resize(Decrypted.size() - (start - fstart));
    }

    size_t newFileNameStart = Decrypted.find('!' + dest.Filename + '?');
    if (newFileNameStart == std::string::npos)
    {
      // Check if the file name is the first entry(has no '!')
      newFileNameStart = Decrypted.find(dest.Filename + '?');
      if (newFileNameStart != 0)
      {
        // Npos or we found something, but it's not out filename cos newFileNameStart expected to be 0
        newFileNameStart = std::string::npos;
      }
    }
    else
    {
      // Skip '!'
      newFileNameStart++;
    }

    if (newFileNameStart == std::string::npos)
    {
      // Create a new storage
      Decrypted += dest.Filename + '?' + dest.ToString() + '!';
    }
    else
    {
      // Add to the end of an existing storage
      size_t entryStart = Decrypted.find('!', newFileNameStart);
      if (entryStart == std::string::npos)
      {
        throw "Failed to insert new value!";
      }

      std::string newEntry = dest.ToString();
      Decrypted.resize(Decrypted.size() + newEntry.size());
      memmove(&Decrypted[entryStart + newEntry.size()], &Decrypted[entryStart], Decrypted.size() - (entryStart + newEntry.size()));
      memcpy(&Decrypted[entryStart], &newEntry[0], newEntry.size());
    }
  }
  return previousFilename + previousEntry;
}