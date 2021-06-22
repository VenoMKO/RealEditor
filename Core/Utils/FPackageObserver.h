#pragma once

struct FPackageObserver {
  virtual ~FPackageObserver() = default;
  virtual void OnObjectDirty(class FObjectExport* obj) = 0;
  virtual void OnExportAdded(class FObjectExport* obj) = 0;
  virtual void OnImportAdded(class FObjectImport* imp) = 0;
  virtual void OnExportRemoved(PACKAGE_INDEX index) = 0;
};