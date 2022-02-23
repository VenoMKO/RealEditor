#pragma once

#include "GenericEditor.h"
#include <Tera/Utils/ALDevice.h>
#include <Tera/USoundNode.h>

class SoundWaveEditor 
  : public GenericEditor
  , public ALSoundObserver {
public:
  SoundWaveEditor(wxPanel* parent, PackageWindow* window);
  ~SoundWaveEditor();

  void OnObjectLoaded() override;
  void PopulateToolBar(wxToolBar* toolbar) override;
  void ClearToolbar() override;
  void OnExportClicked(wxCommandEvent&) override;
  void OnImportClicked(wxCommandEvent&) override;
  void OnPlayClicked(wxCommandEvent&);
  void OnPauseClicked(wxCommandEvent&);
  void OnStopClicked(wxCommandEvent&);
  
  void OnToolBarEvent(wxCommandEvent& e) override;

  void OnSoundStarted(size_t id) override;
  void OnSoundStopped(size_t id) override;
  void OnSoundPaused(size_t id) override;

protected:
  void UpdateToolBar();
  void OnUpdateState(wxCommandEvent&);

private:
  std::mutex ToolbarMutex;
  size_t SoundId = 0;
  wxBitmap PlayBitmap;
  wxBitmap StopBitmap;
  wxBitmap PauseBitmap;
};