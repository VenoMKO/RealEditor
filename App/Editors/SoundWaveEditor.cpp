#include "SoundWaveEditor.h"
#include "../Windows/PackageWindow.h"
#include "../Windows/REDialogs.h"
#include "../App.h"

#include <Utils/ALog.h>
#include <Utils/SoundTravaller.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "../resource.h"

wxDEFINE_EVENT(UPDATE_STATE, wxCommandEvent);

void SoundWaveEditor::OnExportClicked(wxCommandEvent&)
{
  USoundNodeWave* wave = (USoundNodeWave*)Object;
  const void* soundData = wave->GetResourceData();
  const int32 soundDataSize = wave->GetResourceSize();
  if (!soundData || soundDataSize <= 0)
  {
    REDialog::Error("PC wave data is empty! Nothing to export!");
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
    REDialog::Error("Failed to save the file!");
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
  App::GetSharedAudioDevice()->Stop(SoundId);
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
    REDialog::Error("Failed to read the OGG file!");
    return;
  }

  if (size <= 0)
  {
    REDialog::Error("Invalid OGG file size!");
    return;
  }

  try
  {
    if (!travaller.Visit((USoundNodeWave*)Object))
    {
      REDialog::Error(travaller.GetError());
      return;
    }
  }
  catch (...)
  {
    REDialog::Error("Unexpected error!");
    return;
  }
  App::GetSharedAudioDevice()->RemoveSoundSource(SoundId);
  SendEvent(Window, UPDATE_PROPERTIES);
  SoundId = App::GetSharedAudioDevice()->AddSoundSource(this, (USoundNodeWave*)Object);
  UpdateToolBar();
}

void SoundWaveEditor::OnPlayClicked(wxCommandEvent&)
{
  if (!App::GetSharedAudioDevice()->Play(SoundId))
  {
    LogE("Failed to start the sound.");
  }
}

void SoundWaveEditor::OnPauseClicked(wxCommandEvent&)
{
  App::GetSharedAudioDevice()->Pause(SoundId);
}

void SoundWaveEditor::OnStopClicked(wxCommandEvent&)
{
  App::GetSharedAudioDevice()->Stop(SoundId);
}

void SoundWaveEditor::OnToolBarEvent(wxCommandEvent& e)
{
  GenericEditor::OnToolBarEvent(e);
  if (e.GetSkipped())
  {
    // The base class has processed the event. Unmark the event and exit
    e.Skip(false);
    return;
  }
  auto eId = e.GetId();
  if (eId == eID_SoundPlay)
  {
    OnPlayClicked(e);
  }
  else if (eId == eID_SoundPause)
  {
    OnPauseClicked(e);
  }
  else if (eId == eID_SoundStop)
  {
    OnStopClicked(e);
  }
}

void SoundWaveEditor::OnSoundStarted(size_t id)
{
  SendEvent(this, UPDATE_STATE);
}

void SoundWaveEditor::OnSoundStopped(size_t id)
{
  SendEvent(this, UPDATE_STATE);
}

void SoundWaveEditor::OnSoundPaused(size_t id)
{
  SendEvent(this, UPDATE_STATE);
}

void SoundWaveEditor::UpdateToolBar()
{
  std::scoped_lock<std::mutex> l(ToolbarMutex);
  if (!Toolbar)
  {
    return;
  }
  auto state = App::GetSharedAudioDevice()->GetSoundState(SoundId);
  if (wxToolBarToolBase* tool = Toolbar->FindById(eID_SoundStop))
  {
    Toolbar->EnableTool(eID_SoundStop, state != ALSoundSource::SoundState::STOPPED);
  }
  if (wxToolBarToolBase* tool = Toolbar->FindById(eID_SoundPlay))
  {
    Toolbar->EnableTool(eID_SoundPlay, state != ALSoundSource::SoundState::PLAYING);
  }
  if (wxToolBarToolBase* tool = Toolbar->FindById(eID_SoundPause))
  {
    Toolbar->EnableTool(eID_SoundPause, state == ALSoundSource::SoundState::PLAYING);
  }
}

void SoundWaveEditor::OnUpdateState(wxCommandEvent&)
{
  UpdateToolBar();
}

SoundWaveEditor::SoundWaveEditor(wxPanel* parent, PackageWindow* window)
  : GenericEditor(parent, window)
  , PlayBitmap(MAKE_IDB(IDB_PLAY), wxBITMAP_TYPE_PNG_RESOURCE)
  , StopBitmap(MAKE_IDB(IDB_STOP), wxBITMAP_TYPE_PNG_RESOURCE)
  , PauseBitmap(MAKE_IDB(IDB_PAUSE), wxBITMAP_TYPE_PNG_RESOURCE)
{
  Connect(UPDATE_STATE, wxCommandEventHandler(SoundWaveEditor::OnUpdateState), NULL, this);
}

SoundWaveEditor::~SoundWaveEditor()
{
  if (SoundId)
  {
    App::GetSharedAudioDevice()->RemoveSoundSource(SoundId);
  }
}

void SoundWaveEditor::OnObjectLoaded()
{
  if (!SoundId && Object)
  {
    SoundId = App::GetSharedAudioDevice()->AddSoundSource(this, (USoundNodeWave*)Object);
  }
  GenericEditor::OnObjectLoaded();
}

void SoundWaveEditor::PopulateToolBar(wxToolBar* toolbar)
{
  {
    std::scoped_lock<std::mutex> l(ToolbarMutex);
    GenericEditor::PopulateToolBar(toolbar);
    if (auto item = toolbar->FindById(eID_Import))
    {
      item->Enable(true);
    }
    toolbar->AddTool(eID_SoundPlay, "Play", PlayBitmap);
    toolbar->AddTool(eID_SoundPause, "Pause", PauseBitmap);
    toolbar->AddTool(eID_SoundStop, "Stop", StopBitmap);
  }
  UpdateToolBar();
}

void SoundWaveEditor::ClearToolbar()
{
  {
    std::scoped_lock<std::mutex> l(ToolbarMutex);
    Toolbar = nullptr;
  }
  App::GetSharedAudioDevice()->Stop(SoundId);
}
