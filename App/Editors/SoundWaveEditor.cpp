#include "SoundWaveEditor.h"

void SoundWaveEditor::OnExportClicked(wxCommandEvent&)
{
  USoundNodeWave* wave = (USoundNodeWave*)Object;
  const void* soundData = wave->GetResourceData();
  const int32 soundDataSize = wave->GetResourceSize();
  if (!soundData || soundDataSize <= 0)
  {
    wxMessageBox(wxT("PC wave data is empty! Nothing to export!"), wxT("Error!"), wxICON_ERROR);
    return;
  }
  wxString path = wxSaveFileSelector("sound", wxT("OGG file|*.ogg"), Object->GetObjectName().WString(), this);
  if (path.empty())
  {
    return;
  }
  try
  {
    std::ofstream s(path.ToStdWstring(), std::ios::out | std::ios::trunc | std::ios::binary);
    s.write((const char*)soundData, soundDataSize);
  }
  catch (...)
  {
    wxMessageBox(wxT("Failed to save the file!"), wxT("Error!"), wxICON_ERROR);
  }
}
