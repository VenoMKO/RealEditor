#include "FlagsDialog.h"

struct FlagEntry {
  FlagEntry() = default;
  FlagEntry(const char* name, uint64 flag, bool state)
    : Name(name)
    , Flag(flag)
    , Enabled(state)
  {}

  wxString Name;
  uint64 Flag = 0;
  bool Enabled = false;
};

class FlagEntryModel : public wxDataViewVirtualListModel {
public:
  enum
  {
    Col_Check = 0,
    Col_Name,
    Col_Max
  };

  FlagEntryModel(const std::vector<FlagEntry>& entries)
    : Rows(entries)
  {}

  unsigned int GetColumnCount() const override
  {
    return Col_Max;
  }

  wxString GetColumnType(unsigned int col) const override
  {
    if (col == Col_Check)
    {
      return "bool";
    }
    return wxDataViewCheckIconTextRenderer::GetDefaultType();
  }

  unsigned int GetCount() const override
  {
    return Rows.size();
  }

  void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override
  {
    switch (col)
    {
    case Col_Check:
      variant = Rows[row].Enabled;
      break;
    case Col_Name:
      variant = Rows[row].Name;
      break;
    }
  }

  bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override
  {
    return false;
  }

  bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override
  {
    if (col == Col_Check)
    {
      Rows[row].Enabled = variant.GetBool();
      return true;
    }
    return false;
  }

  std::vector<FlagEntry> GetRows() const
  {
    return Rows;
  }

  uint64 GetFlagsMask() const
  {
    uint64 result = 0;
    for (const auto& item : Rows)
    {
      if (item.Enabled)
      {
        result |= item.Flag;
      }
    }
    return result;
  }

private:
  std::vector<FlagEntry> Rows;
};

FlagsDialog::FlagsDialog(wxWindow* parent, const wxString& title)
  : WXDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(403, 514), wxDEFAULT_DIALOG_STYLE)
{
  SetSize(FromDIP(GetSize()));
  SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Flags:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText1->Wrap(-1);
  bSizer1->Add(m_staticText1, 0, wxALL, FromDIP(5));

  FlagsTable = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
  bSizer1->Add(FlagsTable, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxHORIZONTAL);

  DefaultsButton = new wxButton(this, wxID_ANY, wxT("Default"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(DefaultsButton, 0, wxALL, FromDIP(5));


  bSizer3->Add(0, 0, 1, wxEXPAND, FromDIP(5));

  OkButton = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(OkButton, 0, wxALL, FromDIP(5));

  CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(CancelButton, 0, wxALL, FromDIP(5));


  bSizer2->Add(bSizer3, 1, wxEXPAND | wxTOP | wxBOTTOM, FromDIP(15));


  bSizer1->Add(bSizer2, 0, wxEXPAND, FromDIP(5));


  SetSizer(bSizer1);
  Layout();

  Centre(wxBOTH);

  FlagsTable->AppendToggleColumn(_(""), FlagEntryModel::Col_Check, wxDATAVIEW_CELL_ACTIVATABLE, FromDIP(25));
  FlagsTable->AppendTextColumn(_("Flag"), FlagEntryModel::Col_Name, wxDATAVIEW_CELL_INERT, FromDIP(300), wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);

  // TODO: create default presets
  DefaultsButton->Enable(false);
  DefaultsButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(FlagsDialog::OnDefaultClicked), NULL, this);
  OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(FlagsDialog::OnOkClicked), NULL, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(FlagsDialog::OnCancelClicked), NULL, this);
}

FlagsDialog* FlagsDialog::ExportFlagsDialog(EFExportFlags flags, wxWindow* parent, const wxString& title)
{
  FlagsDialog* d = new FlagsDialog(parent, title);
  std::vector<FlagEntry> list;

  list.emplace_back("ForcedExport", EF_ForcedExport, EF_ForcedExport & flags);
  list.emplace_back("ScriptPatcherExport", EF_ScriptPatcherExport, EF_ScriptPatcherExport & flags);
  list.emplace_back("MemberFieldPatchPending", EF_MemberFieldPatchPending, EF_MemberFieldPatchPending & flags);
  list.emplace_back("Composite", EF_Composit, EF_Composit & flags);

  wxDataViewModel* model = new FlagEntryModel(list);
  d->FlagsTable->AssociateModel(model);
  model->DecRef();
  return d;
}

FlagsDialog* FlagsDialog::PackageFlagsDialog(EPackageFlags flags, wxWindow* parent, const wxString& title)
{
  FlagsDialog* d = new FlagsDialog(parent, title);
  std::vector<FlagEntry> list;

  list.emplace_back("AllowDownload", PKG_AllowDownload, PKG_AllowDownload & flags);
  list.emplace_back("ClientOptional", PKG_ClientOptional, PKG_ClientOptional & flags);
  list.emplace_back("Compiling", PKG_Compiling, PKG_Compiling & flags);
  list.emplace_back("ContainsDebugInfo", PKG_ContainsDebugInfo, PKG_ContainsDebugInfo & flags);
  list.emplace_back("ContainsFaceFXData", PKG_ContainsFaceFXData, PKG_ContainsFaceFXData & flags);
  list.emplace_back("ContainsInlinedShaders", PKG_ContainsInlinedShaders, PKG_ContainsInlinedShaders & flags);
  list.emplace_back("DisallowLazyLoading", PKG_DisallowLazyLoading, PKG_DisallowLazyLoading & flags);
  list.emplace_back("Need", PKG_Need, PKG_Need & flags);
  list.emplace_back("NoExportAllowed", PKG_NoExportAllowed, PKG_NoExportAllowed & flags);
  list.emplace_back("PendingDeletion", PKG_PendingDeletion, PKG_PendingDeletion & flags);
  list.emplace_back("PlayInEditor", PKG_PlayInEditor, PKG_PlayInEditor & flags);
  list.emplace_back("RequireImportsAlreadyLoaded", PKG_RequireImportsAlreadyLoaded, PKG_RequireImportsAlreadyLoaded & flags);
  list.emplace_back("SavedWithNewerVersion", PKG_SavedWithNewerVersion, PKG_SavedWithNewerVersion & flags);
  list.emplace_back("SelfContainedLighting", PKG_SelfContainedLighting, PKG_SelfContainedLighting & flags);
  list.emplace_back("ServerSideOnly", PKG_ServerSideOnly, PKG_ServerSideOnly & flags);
  list.emplace_back("StrippedSource", PKG_StrippedSource, PKG_StrippedSource & flags);
  list.emplace_back("Trash", PKG_Trash, PKG_Trash & flags);
  list.emplace_back("Unsecure", PKG_Unsecure, PKG_Unsecure & flags);

#if INTERNAL_FLAGS
  list.emplace_back("Cooked", PKG_Cooked, PKG_Cooked & flags);
  list.emplace_back("Dirty", PKG_Dirty, PKG_Dirty & flags);
  list.emplace_back("NoSource", PKG_NoSource, PKG_NoSource & flags);
  list.emplace_back("StoreCompressed", PKG_StoreCompressed, PKG_StoreCompressed & flags);
  list.emplace_back("StoreFullyCompressed", PKG_StoreFullyCompressed, PKG_StoreFullyCompressed & flags);
  list.emplace_back("ContainsMap", PKG_ContainsMap, PKG_ContainsMap & flags);
  list.emplace_back("ContainsScript", PKG_ContainsScript, PKG_ContainsScript & flags);
#else
  if (PKG_Cooked & flags)
  {
    d->InternalFlags |= (uint64)PKG_Cooked;
  }
  if (PKG_Dirty & flags)
  {
    d->InternalFlags |= (uint64)PKG_Dirty;
  }
  if (PKG_NoSource & flags)
  {
    d->InternalFlags |= (uint64)PKG_NoSource;
  }
  if (PKG_NoSource & flags)
  {
    d->InternalFlags |= (uint64)PKG_NoSource;
  }
  if (PKG_StoreCompressed & flags)
  {
    d->InternalFlags |= (uint64)PKG_StoreCompressed;
  }
  if (PKG_StoreFullyCompressed & flags)
  {
    d->InternalFlags |= (uint64)PKG_StoreFullyCompressed;
  }
  if (PKG_StoreFullyCompressed & flags)
  {
    d->InternalFlags |= (uint64)PKG_StoreFullyCompressed;
  }
  if (PKG_ContainsMap & flags)
  {
    d->InternalFlags |= (uint64)PKG_ContainsMap;
  }
  if (PKG_ContainsScript & flags)
  {
    d->InternalFlags |= (uint64)PKG_ContainsScript;
  }
#endif

  wxDataViewModel* model = new FlagEntryModel(list);
  d->FlagsTable->AssociateModel(model);
  model->DecRef();
  return d;
}

FlagsDialog* FlagsDialog::ObjectFlagsDialog(EObjectFlags flags, wxWindow* parent, const wxString& title)
{
  FlagsDialog* d = new FlagsDialog(parent, title);
  std::vector<FlagEntry> list;

  list.emplace_back("ArchetypeObject", RF_ArchetypeObject, RF_ArchetypeObject & flags);
  list.emplace_back("AsyncLoading", RF_AsyncLoading, RF_AsyncLoading & flags);
  list.emplace_back("BeginDestroyed", RF_BeginDestroyed, RF_BeginDestroyed & flags);
  list.emplace_back("Cooked", RF_Cooked, RF_Cooked & flags);
  list.emplace_back("DebugBeginDestroyed", RF_DebugBeginDestroyed, RF_DebugBeginDestroyed & flags);
  list.emplace_back("DebugFinishDestroyed", RF_DebugFinishDestroyed, RF_DebugFinishDestroyed & flags);
  list.emplace_back("DebugPostLoad", RF_DebugPostLoad, RF_DebugPostLoad & flags);
  list.emplace_back("DebugSerialize", RF_DebugSerialize, RF_DebugSerialize & flags);
  list.emplace_back("DisregardForGC", RF_DisregardForGC, RF_DisregardForGC & flags);
  list.emplace_back("EdSelected", RF_EdSelected, RF_EdSelected & flags);
  list.emplace_back("ErrorShutdown", RF_ErrorShutdown, RF_ErrorShutdown & flags);
  list.emplace_back("FinishDestroyed", RF_FinishDestroyed, RF_FinishDestroyed & flags);
  list.emplace_back("ForceTagExp", RF_ForceTagExp, RF_ForceTagExp & flags);
  list.emplace_back("InEndState", RF_InEndState, RF_InEndState & flags);
  list.emplace_back("InitializedProps", RF_InitializedProps, RF_InitializedProps & flags);
  list.emplace_back("InSingularFunc", RF_InSingularFunc, RF_InSingularFunc & flags);
  list.emplace_back("IsCrossLevelReferenced", RF_IsCrossLevelReferenced, RF_IsCrossLevelReferenced & flags);
  list.emplace_back("LoadForClient", RF_LoadForClient, RF_LoadForClient & flags);
  list.emplace_back("LoadForEdit", RF_LoadForEdit, RF_LoadForEdit & flags);
  list.emplace_back("LoadForServer", RF_LoadForServer, RF_LoadForServer & flags);
  list.emplace_back("LocalizedResource", RF_LocalizedResource, RF_LocalizedResource & flags);
  list.emplace_back("MarkedByCooker", RF_MarkedByCooker, RF_MarkedByCooker & flags);
  list.emplace_back("MisalignedObject", RF_MisalignedObject, RF_MisalignedObject & flags);
  list.emplace_back("NeedLoad", RF_NeedLoad, RF_NeedLoad & flags);
  list.emplace_back("NeedPostLoad", RF_NeedPostLoad, RF_NeedPostLoad & flags);
  list.emplace_back("NeedPostLoadSubobjects", RF_NeedPostLoadSubobjects, RF_NeedPostLoadSubobjects & flags);
  list.emplace_back("NotForClient", RF_NotForClient, RF_NotForClient & flags);
  list.emplace_back("NotForEdit", RF_NotForEdit, RF_NotForEdit & flags);
  list.emplace_back("NotForServer", RF_NotForServer, RF_NotForServer & flags);
  list.emplace_back("Obsolete", RF_Obsolete, RF_Obsolete & flags);
  list.emplace_back("PendingFieldPatches", RF_PendingFieldPatches, RF_PendingFieldPatches & flags);
  list.emplace_back("PendingKill", RF_PendingKill, RF_PendingKill & flags);
  list.emplace_back("PerObjectLocalized", RF_PerObjectLocalized, RF_PerObjectLocalized & flags);
  list.emplace_back("Protected", RF_Protected, RF_Protected & flags);
  list.emplace_back("Public", RF_Public, RF_Public & flags);
  list.emplace_back("RootSet", RF_RootSet, RF_RootSet & flags);
  list.emplace_back("Saved", RF_Saved, RF_Saved & flags);
  list.emplace_back("Standalone", RF_Standalone, RF_Standalone & flags);
  list.emplace_back("StateChanged", RF_StateChanged, RF_StateChanged & flags);
  list.emplace_back("Suppress", RF_Suppress, RF_Suppress & flags);
  list.emplace_back("TagExp", RF_TagExp, RF_TagExp & flags);
  list.emplace_back("TagGarbage", RF_TagGarbage, RF_TagGarbage & flags);
  list.emplace_back("TagImp", RF_TagImp, RF_TagImp & flags);
  list.emplace_back("TokenStreamAssembled", RF_TokenStreamAssembled, RF_TokenStreamAssembled & flags);
  list.emplace_back("Transactional", RF_Transactional, RF_Transactional & flags);
  list.emplace_back("Transient", RF_Transient, RF_Transient & flags);
  list.emplace_back("Unreachable", RF_Unreachable, RF_Unreachable & flags);
  list.emplace_back("ZombieComponent", RF_ZombieComponent, RF_ZombieComponent & flags);

#if INTERNAL_FLAGS
  list.emplace_back("ClassDefaultObject", RF_ClassDefaultObject, RF_ClassDefaultObject & flags);
  list.emplace_back("Marked", RF_Marked, RF_Marked & flags);
  list.emplace_back("Native", RF_Native, RF_Native & flags);
  list.emplace_back("HasStack", RF_HasStack, RF_HasStack & flags);
#else
  if (RF_ClassDefaultObject & flags)
  {
    d->InternalFlags |= (uint64)RF_ClassDefaultObject;
  }
  if (RF_Marked & flags)
  {
    d->InternalFlags |= (uint64)RF_Marked;
  }
  if (RF_Native & flags)
  {
    d->InternalFlags |= (uint64)RF_Native;
  }
  if (RF_HasStack & flags)
  {
    d->InternalFlags |= (uint64)RF_HasStack;
  }
#endif

  wxDataViewModel* model = new FlagEntryModel(list);
  d->FlagsTable->AssociateModel(model);
  model->DecRef();
  return d;
}

void FlagsDialog::OnDefaultClicked(wxCommandEvent&)
{
}

void FlagsDialog::OnOkClicked(wxCommandEvent&)
{
  EndModal(wxID_OK);
}

void FlagsDialog::OnCancelClicked(wxCommandEvent&)
{
  EndModal(wxID_CANCEL);
}

uint64 FlagsDialog::GetUntypedFlags() const
{
  return ((FlagEntryModel*)FlagsTable->GetModel())->GetFlagsMask() | InternalFlags;
}