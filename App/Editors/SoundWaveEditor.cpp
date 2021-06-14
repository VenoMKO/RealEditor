#include "SoundWaveEditor.h"
#include "../Windows/PackageWindow.h"
#include "../App.h"

#include <Utils/SoundTravaller.h>

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
  wxString path = wxSaveFileSelector("sound", wxT("OGG file|*.ogg"), Object->GetObjectNameString().WString(), this);
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

void SoundWaveEditor::OnImportClicked(wxCommandEvent&)
{
  wxString ext = "OGG files (*.ogg)|*.ogg";
  wxString path = wxFileSelector("Import a sound file", wxEmptyString, wxEmptyString, ext, ext, wxFD_OPEN, Window);
  if (path.empty())
  {
    return;
  }
  void* soundData = nullptr;
  FILE_OFFSET size = 0;
  SoundTravaller travaller;
  
  try
  {
    std::ifstream s(path.ToStdWstring(), std::ios::in | std::ios::binary);
    size_t tmpPos = s.tellg();
    s.seekg(0, std::ios::end);
    size = (FILE_OFFSET)s.tellg();
    s.seekg(tmpPos);
    if (size > 0)
    {
      soundData = malloc(size);
      s.read((char*)soundData, size);
      travaller.SetData(soundData, size);
    }
  }
  catch (...)
  {
    wxMessageBox(wxT("Failed to read the OGG file!"), wxT("Error!"), wxICON_ERROR);
    return;
  }

  if (size <= 0)
  {
    wxMessageBox(wxT("Invalid OGG file size!"), wxT("Error!"), wxICON_ERROR);
    return;
  }

  try
  {
    if (!travaller.Visit((USoundNodeWave*)Object))
    {
      wxMessageBox(travaller.GetError(), wxT("Error!"), wxICON_ERROR);
      return;
    }
  }
  catch (...)
  {
    wxMessageBox("Unexpected error!", wxT("Error!"), wxICON_ERROR);
    return;
  }
  
  SendEvent(Window, UPDATE_PROPERTIES);
}

void SoundWaveEditor::PopulateToolBar(wxToolBar* toolbar)
{
  GenericEditor::PopulateToolBar(toolbar);
  if (auto item = toolbar->FindById(eID_Import))
  {
    item->Enable(true);
  }
}