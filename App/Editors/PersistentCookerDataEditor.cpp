#include "PersistentCookerDataEditor.h"

#include "../App.h"
#include "../Windows/REDialogs.h"
#include "../Windows/ProgressWindow.h"

#include <Tera/Cast.h>
#include <Tera/FPackage.h>
#include <Tera/UTexture.h>
#include <Tera/UClass.h>
#include <Tera/UProperty.h>

#include <filesystem>
#include <iostream>
#include <thread>
#include <pugixml/pugixml.hpp>

void PersistentCookerDataEditor::LoadObject()
{
  if (!Object->IsLoaded())
  {
    LoadingProgress = new ProgressWindow(this);
    LoadingProgress->SetCurrentProgress(-1);
    LoadingProgress->SetCanCancel(false);
    LoadingProgress->SetActionText(wxT("Reading UPersistentCookerData..."));
    std::thread([&] {
      GenericEditor::LoadObject();
    }).detach();
    LoadingProgress->ShowModal();
  }
  else
  {
    GenericEditor::LoadObject();
  }
}

void PersistentCookerDataEditor::OnObjectLoaded()
{
  if (LoadingProgress)
  {
    SendEvent(LoadingProgress, UPDATE_PROGRESS_FINISH);
    LoadingProgress = nullptr;
  }
  GenericEditor::OnObjectLoaded();
}

void PersistentCookerDataEditor::OnExportClicked(wxCommandEvent& e)
{
  const wxString path = IODialog::OpenDirectoryDialog(this, wxEmptyString, wxT("Select a folder to save files..."));
  if (!path.size())
  {
    return;
  }

  UPersistentCookerData* data = Cast<UPersistentCookerData>(Object);
  if (!data)
  {
    return;
  }

  std::filesystem::path root(path.ToStdWstring());

  std::thread classMapThread = std::thread([&] {
    
    std::unordered_map<FString, std::vector<FPacakgeTreeEntry>> classMap = data->GetClassMap();
    if (classMap.size())
    {
      pugi::xml_document doc;
      for (const auto& it : classMap)
      {
        pugi::xml_node node = doc.append_child("Package");
        {
          pugi::xml_attribute attr = node.append_attribute("Name");
          attr.set_value(W2A(it.first.WString()).c_str());
        }
        for (const auto& vit : it.second)
        {
          const FString fullName = vit.FullObjectName.GetString();
          std::vector<FString> comps = fullName.Split(' ');
          std::vector<FString> comps2 = comps.back().Split('.');
          pugi::xml_node child = node.append_child("Object");
          {
            pugi::xml_attribute attr = child.append_attribute("Name");
            attr.set_value(W2A(comps2.back().WString()).c_str());
          }
          {
            pugi::xml_attribute attr = child.append_attribute("Class");
            attr.set_value(W2A(comps.front().WString()).c_str());
          }
          {
            pugi::xml_attribute attr = child.append_attribute("Path");
            attr.set_value(W2A(comps.back().WString()).c_str());
          }
        }
      }
      std::filesystem::path p(root / L"ClassMap.xml");
      doc.save_file(p.wstring().c_str());
    }
  });
  std::thread cookedBulkDataInfoMapThread = std::thread([&]{
    pugi::xml_document doc;
    std::unordered_map<FString, FCookedBulkDataInfo> cookedBulkDataInfoMap = data->GetCookedBulkDataInfoMap();
    for (const auto& it : cookedBulkDataInfoMap)
    {
      pugi::xml_node node = doc.append_child("Info");
      {
        pugi::xml_attribute attr = node.append_attribute("Item");
        attr.set_value(W2A(it.first.WString()).c_str());
      }
      {
        pugi::xml_attribute attr = node.append_attribute("TFC");
        attr.set_value(W2A(it.second.TextureFileCacheName.String().WString()).c_str());
      }
      {
        pugi::xml_attribute attr = node.append_attribute("Flags");
        attr.set_value(BulkDataFlagsToString(it.second.SavedBulkDataFlags).UTF8().c_str());
      }
      {
        pugi::xml_attribute attr = node.append_attribute("ElementCount");
        attr.set_value(it.second.SavedElementCount);
      }
      {
        pugi::xml_attribute attr = node.append_attribute("Offset");
        attr.set_value(it.second.SavedBulkDataOffsetInFile);
      }
      {
        pugi::xml_attribute attr = node.append_attribute("Size");
        attr.set_value(it.second.SavedBulkDataSizeOnDisk);
      }
    }
    std::filesystem::path p(root / L"BulkDataInfoMap.xml");
    doc.save_file(p.wstring().c_str());
  });
  std::thread cookedTextureFileCacheInfoThread = std::thread([&] {
    std::unordered_map<FString, FCookedTextureFileCacheInfo> textureFileCacheInfoMap = data->GetCookedTextureFileCacheInfoMap();
    if (textureFileCacheInfoMap.size())
    {
      pugi::xml_document doc;
      for (const auto& it : textureFileCacheInfoMap)
      {
        pugi::xml_node node = doc.append_child("Info");
        {
          pugi::xml_attribute attr = node.append_attribute("GUID");
          attr.set_value(it.second.TextureFileCacheGuid.String().UTF8().c_str());
        }
        {
          pugi::xml_attribute attr = node.append_attribute("TFC");
          attr.set_value(W2A(it.second.TextureFileCacheName.String().WString()).c_str());
        }
        {
          pugi::xml_attribute attr = node.append_attribute("LastSaved");
          attr.set_value(it.second.LastSaved);
        }
      }
      std::filesystem::path p(root / L"TfcInfoMap.xml");
      doc.save_file(p.wstring().c_str());
    }
  });
  std::thread textureUsageInfoThread = std::thread([&] {
    std::unordered_map<FString, FCookedTextureUsageInfo> textureUsageInfoMap = data->GetTextureUsageInfos();
    if (textureUsageInfoMap.size())
    {
      UByteProperty* formatProperty = (UByteProperty*)FPackage::FindClass(UTexture2D::StaticClassName())->GetProperty("Format");
      UByteProperty* lodGroupProperty = (UByteProperty*)FPackage::FindClass(UTexture2D::StaticClassName())->GetProperty("LODGroup");
      pugi::xml_document doc;
      {
        pugi::xml_node comment = doc.append_child(pugi::node_comment);
        comment.set_value("The exported data does not include lists of packages that use these textures!!!");
      }
      for (const auto& it : textureUsageInfoMap)
      {
        pugi::xml_node node = doc.append_child("Info");
        {
          pugi::xml_attribute attr = node.append_attribute("Item");
          attr.set_value(W2A(it.first.WString()).c_str());
        }
        {
          pugi::xml_attribute attr = node.append_attribute("Format");
          attr.set_value(formatProperty->Enum->GetEnum(it.second.Format).String().UTF8().c_str());
        }
        {
          pugi::xml_attribute attr = node.append_attribute("LODGroup");
          attr.set_value(lodGroupProperty->Enum->GetEnum(it.second.LODGroup).String().UTF8().c_str());
        }
        {
          pugi::xml_attribute attr = node.append_attribute("SizeX");
          attr.set_value(it.second.SizeX);
        }
        {
          pugi::xml_attribute attr = node.append_attribute("SizeY");
          attr.set_value(it.second.SizeY);
        }
        {
          pugi::xml_attribute attr = node.append_attribute("StoredOnceMipSize");
          attr.set_value(it.second.StoredOnceMipSize);
        }
        {
          pugi::xml_attribute attr = node.append_attribute("DuplicatedMipSize");
          attr.set_value(it.second.DuplicatedMipSize);
        }
        /* Requires absurd amount of disk spaces and time
        for (const FStringRef& name : it.second.PackageNames)
        {
          node.append_child(name.GetString().UTF8().c_str());
        }*/
      }
      std::filesystem::path p(root / L"TextureUsageInfoMap.xml");
      doc.save_file(p.wstring().c_str());
    }
  });
  std::thread versionMapThread = std::thread([&] {
    pugi::xml_document doc;
    std::unordered_map<FString, int32> versionMap = data->GetFilenameToCookedVersion();
    for (const auto& it : versionMap)
    {
      pugi::xml_node node = doc.append_child("File");
      {
        pugi::xml_attribute attr = node.append_attribute(W2A(it.first.WString()).c_str());
        attr.set_value(it.second);
      }
      {
        pugi::xml_attribute attr = node.append_attribute("Version");
        attr.set_value(it.second);
      }
    }
    std::filesystem::path p(root / L"VersionMap.xml");
    doc.save_file(p.wstring().c_str());
  });

  ProgressWindow progress(this, wxT("Exporting"));
  progress.SetCurrentProgress(-1);
  progress.SetCanCancel(false);
  progress.SetActionText(wxT("Exporting UPersistentCookerData..."));

  std::thread([&] {
    cookedBulkDataInfoMapThread.join();
    classMapThread.join();
    cookedTextureFileCacheInfoThread.join();
    textureUsageInfoThread.join();
    versionMapThread.join();
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();

  progress.ShowModal();
}
