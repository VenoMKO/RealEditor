#pragma once
#include <wx/wx.h>
#include <wx/filename.h>

#include "../Windows/ProgressWindow.h"

#include <Tera/Core.h>

struct BulkImportOperationData {
	struct Entry {
		wxString ObjectPath;
		wxString PackageName;
		PACKAGE_INDEX Index = 0;
		bool Enabled = true;
		FPackage* Package = nullptr;

		inline bool IsValid() const
		{
			return ObjectPath.size() && PackageName.size() && Index > 0 && Enabled;
		}
	};

	wxString ClassName;
	wxString ObjectName;
	std::vector<Entry> Entries;

	wxString ImportPath;
	wxString RedirectPath;

	inline wxString GetChangeString() const
	{
		wxString fname;
		if (ImportPath.size())
		{
			wxFileName::SplitPath(ImportPath, nullptr, nullptr, &fname, nullptr);
		}
		else
		{
			fname = RedirectPath;
		}
		if (fname.empty())
		{
			return wxT("None");
		}
		return fname;
	}

	inline bool IsValid() const
	{
		bool hasValidEntries = false;
		for (const Entry& object : Entries)
		{
			if (object.IsValid())
			{
				hasValidEntries = true;
				break;
			}
		}
		return ClassName.size() && ObjectName.size() && hasValidEntries;
	}
};

class BulkImportOperation {
public:
	BulkImportOperation(const std::vector<BulkImportOperationData>& ops, const wxString& path)
		: Path(path)
		, Operations(ops)
	{}

	bool Execute(ProgressWindow& progress);

	inline std::vector<std::pair<wxString, wxString>> GetErrors() const
	{
		return Errors;
	}

	inline bool HasErrors() const
	{
		return Errors.size();
	}

protected:
	void AddError(const wxString& source, const wxString& error);
	void ImportTexture(FPackage* package, class UTexture2D* tobject, const wxString& source);
	void ImportSound(FPackage* package, class USoundNodeWave* tobject, const wxString& source);
	void ImportUntyped(FPackage* package, class UObject* tobject, const wxString& source);

protected:
	wxString Path;
	std::vector<BulkImportOperationData> Operations;
	std::vector<std::pair<wxString, wxString>> Errors;
};