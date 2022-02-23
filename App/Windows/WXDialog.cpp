#include "WXDialog.h"
#include "../App.h"

int WXDialog::ShowModal()
{
  App::GetSharedApp()->OnDialogWillOpen(this);
  int result = wxDialog::ShowModal();
  App::GetSharedApp()->OnDialogDidClose(this);
  return result;
}
