#include "PackageWindow.h"
#include "ProgressWindow.h"
#include "../App.h"
#include "REDialogs.h"

#include <filesystem>

#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/Cast.h>
#include <Tera/UObject.h>
#include <Tera/UObjectRedirector.h>
#include <Tera/UTexture.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/UStaticMesh.h>
#include <Tera/USoundNode.h>
#include <Tera/USpeedTree.h>

#include <Tera/Utils/APerfSamples.h>
#include <Tera/Utils/MeshUtils.h>
#include <Tera/Utils/TextureUtils.h>

void PackageWindow::OnBulkPackageExport(PACKAGE_INDEX objIndex)
{
  static const std::vector<std::string> filter = { UTexture2D::StaticClassName(), UTerrainWeightMapTexture::StaticClassName(), UTextureCube::StaticClassName(), UTextureFlipBook::StaticClassName(), USkeletalMesh::StaticClassName(), UStaticMesh::StaticClassName(), USoundNodeWave::StaticClassName(), USpeedTree::StaticClassName(), UAnimSet::StaticClassName() };
  int32 additionalCount = 0;
  std::function<void(FObjectExport*, std::vector<FObjectExport*>&)> rCollectExports;
  rCollectExports = [&](FObjectExport* exp, std::vector<FObjectExport*>& output) {
    FString className = exp->GetClassNameString();
    if (!exp)
    {
      return;
    }
    if (className == NAME_Package)
    {
      for (FObjectExport* inner : exp->Inner)
      {
        rCollectExports(inner, output);
      }
      return;
    }

    if (className == UObjectRedirector::StaticClassName())
    {
      output.push_back(exp);
      return;
    }

    if (std::find(filter.begin(), filter.end(), className.UTF8()) != filter.end())
    {
      output.push_back(exp);
      if (className == UAnimSet::StaticClassName())
      {
        additionalCount += (int32)exp->Inner.size();
      }
    }
    else
    {
      LogE("Unsupported class: %s", className.UTF8().c_str());
    }
  };
  std::vector<FObjectExport*> exports;
  FObjectExport* rootExport = nullptr;
  {
    ProgressWindow progress(this, wxT("Preparing..."));
    progress.SetCanCancel(false);
    progress.SetActionText(wxT("Collecting objects..."));
    std::thread([&] {
      if (objIndex == FAKE_EXPORT_ROOT)
      {
        std::vector<FObjectExport*> root = Package->GetRootExports();
        for (FObjectExport* exp : root)
        {
          rCollectExports(exp, exports);
        }
      }
      else
      {
        rootExport = Package->GetExportObject(objIndex);
        rCollectExports(rootExport, exports);
      }
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
    }).detach();
    progress.ShowModal();
  }
  

  if (exports.empty())
  {
    REDialog::Warning("The package has no supported objects to export.", "Nothing to export!");
    return;
  }

  wxDirDialog dlg(NULL, "Select a directory to extract packages to...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (dlg.ShowModal() != wxID_OK || dlg.GetPath().empty())
  {
    return;
  }

  const std::filesystem::path root = std::filesystem::path(dlg.GetPath().ToStdWstring()) / (rootExport ? rootExport->GetObjectNameString().WString() : Package->GetPackageName().WString());
  std::vector<FObjectExport*> failedExports;

  ProgressWindow progress(this, wxT("Exporting..."));
  additionalCount += (int32)exports.size();
  progress.SetMaxProgress(additionalCount);
  int32 count = 0;
  PERF_START(BulkExport);
  std::thread([&] {
    int progressCounter = 0;
    for (int idx = 0; idx < exports.size(); ++idx, ++progressCounter)
    {
      if (progress.IsCanceled())
      {
        SendEvent(&progress, UPDATE_PROGRESS_FINISH);
        return;
      }
      FObjectExport* exp = exports[idx];
      SendEvent(&progress, UPDATE_PROGRESS, progressCounter);
      SendEvent(&progress, UPDATE_PROGRESS_DESC, wxString("Exporting: ") + exp->GetObjectNameString().WString());
      std::filesystem::path dest(root);
      std::vector<std::wstring> pathComponents;
      FObjectExport* outer = exp->Outer;
      while (outer && outer != rootExport)
      {
        pathComponents.insert(pathComponents.begin(), outer->GetObjectNameString().WString());
        outer = outer->Outer;
      }
      for (const auto& component : pathComponents)
      {
        dest /= component;
      }

      std::error_code err;
      if (!std::filesystem::exists(dest, err))
      {
        if (!std::filesystem::create_directories(dest, err) && err)
        {
          failedExports.push_back(exp);
          LogE("Failed to create a directory to export %s", exp->GetObjectNameString().UTF8().c_str());
          continue;
        }
      }

      dest /= exp->GetObjectNameString().WString();

      UObject* obj = nullptr;
      try
      {
        if (exp->GetClassName() == UObjectRedirector::StaticClassName())
        {
          obj = Cast<UObjectRedirector>(exp->Package->GetObject(exp))->GetObject(true);
          DBreakIf(!obj);
        }
        else
        {
          obj = exp->Package->GetObject(exp);
        }
      }
      catch (...)
      {
        failedExports.push_back(exp);
        LogE("Failed to load %s", exp->GetObjectNameString().UTF8().c_str());
        continue;
      }

      if (!obj)
      {
        failedExports.push_back(exp);
        LogE("Failed to load %s", exp->GetObjectNameString().UTF8().c_str());
        continue;
      }

      if (obj->GetClassName() == UTexture2D::StaticClassName() || obj->GetClassName() == UTextureFlipBook::StaticClassName() || obj->GetClassName() == UTerrainWeightMapTexture::StaticClassName())
      {
        UTexture2D* texture = Cast<UTexture2D>(obj);
        if (!texture)
        {
          failedExports.push_back(exp);
          LogE("%s is not a texture", exp->GetObjectNameString().UTF8().c_str());
          continue;
        }
        FTexture2DMipMap* mip = nullptr;
        for (FTexture2DMipMap* mipmap : texture->Mips)
        {
          if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
          {
            mip = mipmap;
            break;
          }
        }
        if (!mip)
        {
          failedExports.push_back(exp);
          continue;
        }

        dest.replace_extension(IODialog::GetLastTextureExtension().ToStdString());

        TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
        TextureProcessor::TCFormat outputFormat = TextureProcessor::GetTcFormatByExtension(dest.extension().string());

        if (texture->Format == PF_DXT1)
        {
          inputFormat = TextureProcessor::TCFormat::DXT1;
        }
        else if (texture->Format == PF_DXT3)
        {
          inputFormat = TextureProcessor::TCFormat::DXT3;
        }
        else if (texture->Format == PF_DXT5)
        {
          inputFormat = TextureProcessor::TCFormat::DXT5;
        }
        else if (texture->Format == PF_A8R8G8B8)
        {
          inputFormat = TextureProcessor::TCFormat::ARGB8;
        }
        else if (texture->Format == PF_G8)
        {
          inputFormat = TextureProcessor::TCFormat::G8;
        }
        else
        {
          failedExports.push_back(exp);
          LogE("%s has unsupported pixel format!", exp->GetObjectNameString().UTF8().c_str());
          continue;
        }

        TextureProcessor processor(inputFormat, outputFormat);

        processor.SetInputData(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
        processor.SetOutputPath(W2A(dest.wstring()));
        processor.SetInputDataDimensions(mip->SizeX, mip->SizeY);

        try
        {
          if (!processor.Process())
          {
            failedExports.push_back(exp);
            LogE("Failed to export %s: %s", exp->GetObjectNameString().UTF8().c_str(), processor.GetError().c_str());
            continue;
          }
          count++;
        }
        catch (...)
        {
          failedExports.push_back(exp);
          LogE("Failed to export %s!", exp->GetObjectNameString().UTF8().c_str());
          continue;
        }
        continue;
      }
      if (obj->GetClassName() == UTextureCube::StaticClassName())
      {
        auto faces = Cast<UTextureCube>(obj)->GetFaces();
        bool invalidFace = faces.empty();
        TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
        for (UTexture2D* face : faces)
        {
          if (!face)
          {
            invalidFace = true;
            LogE("Failed to load the cube face!");
            break;
          }

          TextureProcessor::TCFormat f = TextureProcessor::TCFormat::None;
          switch (face->Format)
          {
          case PF_DXT1:
            f = TextureProcessor::TCFormat::DXT1;
            break;
          case PF_DXT3:
            f = TextureProcessor::TCFormat::DXT3;
            break;
          case PF_DXT5:
            f = TextureProcessor::TCFormat::DXT5;
            break;
          case PF_A8R8G8B8:
            f = TextureProcessor::TCFormat::ARGB8;
            break;
          case PF_G8:
            f = TextureProcessor::TCFormat::G8;
            break;
          default:
            continue;
          }
          if (inputFormat != TextureProcessor::TCFormat::None && inputFormat != f)
          {
            invalidFace = true;
            break;
          }
          inputFormat = f;
        }
        if (invalidFace)
        {
          continue;
        }
        wxString ext = IODialog::GetLastTextureExtension();
        if (ext.empty())
        {
          ext = wxT("dds");
        }
        ext.MakeLower();
        TextureProcessor::TCFormat outputFormat = TextureProcessor::TCFormat::None;
        if (ext == "png")
        {
          outputFormat = TextureProcessor::TCFormat::PNG;
        }
        else if (ext == "tga")
        {
          outputFormat = TextureProcessor::TCFormat::TGA;
        }
        else if (ext == "dds")
        {
          outputFormat = TextureProcessor::TCFormat::DDS;
        }
        else
        {
          continue;
        }

        TextureProcessor processor(inputFormat, outputFormat);
        bool noMip = false;
        for (int32 faceIdx = 0; faceIdx < faces.size(); ++faceIdx)
        {
          FTexture2DMipMap* mip = nullptr;
          for (FTexture2DMipMap* mipmap : faces[faceIdx]->Mips)
          {
            if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
            {
              mip = mipmap;
              break;
            }
          }
          if (!mip)
          {
            noMip = true;
            break;
          }
          processor.SetInputCubeFace(faceIdx, mip->Data->GetAllocation(), mip->Data->GetBulkDataSize(), mip->SizeX, mip->SizeY);
        }
        if (noMip)
        {
          continue;
        }

        processor.SetOutputPath(W2A(dest.replace_extension(ext.ToStdWstring()).wstring()));

        bool result = false;
        std::string err;
        try
        {
          if (!(result = processor.Process()))
          {
            err = processor.GetError();
            if (err.empty())
            {
              err = "Texture Processor: failed with an unknown error!";
            }
          }
          else
          {
            count++;
          }
        }
        catch (const std::exception& e)
        {
          err = e.what();
          result = false;
        }
        catch (...)
        {
          result = false;
          err = processor.GetError();
          if (err.empty())
          {
            err = "Texture Processor: Unexpected exception!";
          }
        }
        if (!result)
        {
          LogE("Failed to export: %s", err.c_str());
        }
        continue;
      }
      if (obj->GetClassName() == USoundNodeWave::StaticClassName())
      {
        dest.replace_extension("ogg");
        if (USoundNodeWave* wave = Cast<USoundNodeWave>(obj))
        {
          const void* soundData = wave->GetResourceData();
          const int32 soundDataSize = wave->GetResourceSize();
          std::ofstream s(dest, std::ios::out | std::ios::trunc | std::ios::binary);
          s.write((const char*)soundData, soundDataSize);
          count++;
        }
        else
        {
          failedExports.push_back(exp);
          LogE("%s is not a SoundNodeWave!", exp->GetObjectNameString().UTF8().c_str());
        }
        continue;
      }
      if (obj->GetClassName() == USpeedTree::StaticClassName())
      {
        dest.replace_extension("spt");
        if (USpeedTree* tree = Cast<USpeedTree>(obj))
        {
          void* sptData = nullptr;
          FILE_OFFSET sptDataSize = 0;
          if (!tree->GetSptData(&sptData, &sptDataSize, false) || !sptDataSize || !sptData)
          {
            failedExports.push_back(exp);
            LogE("Failed to export %s!", exp->GetObjectNameString().UTF8().c_str());
            continue;
          }
          std::ofstream s(dest, std::ios::out | std::ios::trunc | std::ios::binary);
          s.write((const char*)sptData, sptDataSize);
          free(sptData);
          count++;
        }
        else
        {
          LogE("F%s is not a SpeedTree!", exp->GetObjectNameString().UTF8().c_str());
          failedExports.push_back(exp);
        }
        continue;
      }
      if (obj->GetClassName() == UStaticMesh::StaticClassName() || obj->GetClassName() == USkeletalMesh::StaticClassName())
      {
        MeshExportContext ctx;
        FAppConfig& appConfig = App::GetSharedApp()->GetConfig();
        MeshExporterType exportType = MET_Fbx;
        if (obj->GetClassName() == UStaticMesh::StaticClassName())
        {
          exportType = (MeshExporterType)appConfig.StaticMeshExportConfig.LastFormat;
        }
        else
        {
          exportType = (MeshExporterType)appConfig.SkelMeshExportConfig.LastFormat;
        }

        switch (exportType)
        {
        case MET_Fbx:
          ctx.Path = dest.replace_extension("fbx").wstring();
          break;
        case MET_Psk:
          ctx.Path = dest.replace_extension("psk").wstring();
          break;
        }
        
        auto utils = MeshUtils::CreateUtils(exportType);
        utils->SetCreatorInfo(App::GetSharedApp()->GetAppDisplayName().ToStdString(), GetAppVersion());
        if (exp->GetClassName() == UStaticMesh::StaticClassName())
        {
          ctx.Scale3D *= appConfig.StaticMeshExportConfig.ScaleFactor;
          if (UStaticMesh* mesh = Cast<UStaticMesh>(obj))
          {
            if (!utils->ExportStaticMesh(mesh, ctx))
            {
              failedExports.push_back(exp);
              LogE("Failed to export %s!", exp->GetObjectNameString().UTF8().c_str());
              continue;
            }
            count++;
          }
          else
          {
            failedExports.push_back(exp);
            LogE("%s is not a StaticMesh!", exp->GetObjectNameString().UTF8().c_str());
          }
        }
        else
        {
          ctx.ExportSkeleton = appConfig.SkelMeshExportConfig.Mode;
          ctx.Scale3D *= appConfig.SkelMeshExportConfig.ScaleFactor;
          if (USkeletalMesh* mesh = Cast<USkeletalMesh>(obj))
          {
            if (!utils->ExportSkeletalMesh(mesh, ctx))
            {
              failedExports.push_back(exp);
              LogE("Failed to export %s!", exp->GetObjectNameString().UTF8().c_str());
              continue;
            }
            count++;
          }
          else
          {
            failedExports.push_back(exp);
            LogE("%s is not a SkeletalMesh!", exp->GetObjectNameString().UTF8().c_str());
          }
        }
        continue;
      }
      if (obj->GetClassName() == UAnimSet::StaticClassName())
      {
        MeshExportContext ctx;
        FAppConfig& appConfig = App::GetSharedApp()->GetConfig();
        ctx.ExportMesh = appConfig.AnimationExportConfig.ExportMesh;
        ctx.Scale3D = FVector(appConfig.AnimationExportConfig.ScaleFactor);
        ctx.CompressTracks = appConfig.AnimationExportConfig.Compress;
        ctx.ResampleTracks = appConfig.AnimationExportConfig.Resample;
        ctx.TrackRateScale = appConfig.AnimationExportConfig.RateFactor;
        if (appConfig.AnimationExportConfig.LastFormat == (int32)MeshExporterType::MET_Fbx)
        {
          ctx.SplitTakes = appConfig.AnimationExportConfig.Split;
        }
        else
        {
          ctx.SplitTakes = true;
          ctx.ExportMesh = false;
        }

        UAnimSet* set = Cast<UAnimSet>(obj);
        USkeletalMesh* source = nullptr;
        std::vector<FObjectExport*> allExports = obj->GetPackage()->GetAllExports();
        for (FObjectExport* exp : allExports)
        {
          if (exp->GetClassName() == USkeletalMesh::StaticClassName())
          {
            USkeletalMesh* skelMesh = Cast<USkeletalMesh>(obj->GetPackage()->GetObject(exp));
            if (set->GetSkeletalMeshMatchRatio(skelMesh))
            {
              source = skelMesh;
              break;
            }
          }
        }
        if (!source)
        {
          source = set->GetPreviewSkeletalMesh();
        }
        if (!source)
        {
          continue;
        }
        MeshExporterType exporterType = (MeshExporterType)appConfig.AnimationExportConfig.LastFormat;
        const char* ext = exporterType == MeshExporterType::MET_Fbx ? "fbx" : "psa";
        if (ctx.SplitTakes)
        {
          dest.replace_extension();
          std::error_code err;
          std::filesystem::create_directories(dest, err);
          std::vector<UObject*> inner = set->GetInner();
          int32 total = (int32)inner.size();
          auto utils = MeshUtils::CreateUtils(exporterType);
          utils->SetCreatorInfo(App::GetSharedApp()->GetAppDisplayName().ToStdString(), GetAppVersion());
          for (int32 idx = 0; idx < total; ++idx)
          {
            if (UAnimSequence* seq = Cast<UAnimSequence>(inner[idx]))
            {
              progressCounter++;
              count++;
              SendEvent(&progress, UPDATE_PROGRESS, progressCounter);
              ctx.Path = (dest / seq->SequenceName.String().WString()).replace_extension(ext).wstring();
              if (!utils->ExportAnimationSequence(source, seq, ctx))
              {
                break;
              }
            }
          }
        }
        else
        {
          ctx.Path = dest.replace_extension(ext).wstring();
          int32 lastCount = 0;
          ctx.ProgressFunc = [&](int32 prg) {
            SendEvent(&progress, UPDATE_PROGRESS, progressCounter + prg);
            lastCount = prg;
          };
          auto utils = MeshUtils::CreateUtils(exporterType);
          utils->SetCreatorInfo(App::GetSharedApp()->GetAppDisplayName().ToStdString(), GetAppVersion());
          utils->ExportAnimationSet(source, set, ctx);
          progressCounter += lastCount;
          count += lastCount;
        }
      }
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();
  progress.ShowModal();
  PERF_END(BulkExport);
  if (failedExports.empty())
  {
    if (!count)
    {
      REDialog::Warning("The package has no supported objects to export.", "Nothing to export!");
      return;
    }
    REDialog::Info(wxString::Format("Exported %d objects.", count), "Finished!");
  }
  else
  {
    wxString logMsg = wxT("Failed exports: ");
    for (FObjectExport* failed : failedExports)
    {
      if (failed)
      {
        logMsg += failed->GetObjectNameString().UTF8();
        logMsg += wxT("(");
        logMsg += failed->GetClassNameString().UTF8();
        logMsg += wxT("),");
      }
    }
    LogE("%s", logMsg.ToStdString().c_str());
    wxString desc = !count ? "Failed to export objects!" : "Failed to export some objects!";
    desc += " See the log for details.";
    REDialog::Warning(desc);
  }
}