#pragma once
#include <wx/dialog.h>

struct WXDialogObserver {
  virtual ~WXDialogObserver()
  {}

  virtual void OnDialogWillOpen(class WXDialog* dialog) = 0;
  virtual void OnDialogDidClose(class WXDialog* dialog) = 0;
};

class WXDialog : public wxDialog {
public:
  using wxDialog::wxDialog;

  int ShowModal() wxOVERRIDE;
};