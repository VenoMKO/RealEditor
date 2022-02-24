#include "../App.h"
#include "SoundCueEditor.h"
#include "../Windows/REDialogs.h"
#include "../Windows/LevelExportOptions.h"

#include <Tera/Cast.h>
#include <Tera/USoundNode.h>

#include <filesystem>

#include "../resource.h"

void SoundCueEditor::OnExportClicked(wxCommandEvent&)
{
  wxString path = IODialog::SaveSoundCueDialog(this);
  if (!path.size())
  {
    return;
  }
  const LevelExportContext tmpCtx;
  // Save the cue
  {
    FString cueData = "Game/";
    cueData += tmpCtx.DataDirName;
    cueData += "/" + Cast<USoundCue>(Object)->ExportCueToText(false, App::GetSharedApp()->GetConfig().MapExportConfig.GlobalScale);
    std::filesystem::path p(path.ToStdWstring());
    p /= Object->GetObjectNameString().WString() + L".cue";
    std::ofstream ofs(p);
    ofs << cueData.UTF8();
  }
  // Save related waves
  {
    
    std::filesystem::path p(path.ToStdWstring());
    p /= tmpCtx.DataDirName;

    std::vector<USoundNodeWave*> waves;
    Cast<USoundCue>(Object)->GetWaves(waves);

    std::error_code ec;
    for (USoundNodeWave* wave : waves)
    {
      wave->Load();
      if (uint32 size = wave->GetResourceSize())
      {
        std::filesystem::path wavep = p;
        wavep /= wave->GetLocalDir().UTF8();
        std::filesystem::create_directories(wavep, ec);
        wavep /= (wave->GetObjectNameString() + ".ogg").UTF8();
        std::ofstream ofs(wavep, std::ios::binary);
        ofs.write((const char*)wave->GetResourceData(), size);
      }
    }
  }
}
