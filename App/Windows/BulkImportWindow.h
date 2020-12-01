#pragma once
#include <wx/wx.h>
#include <wx/filepicker.h>
#include <wx/dataview.h>

#include <sstream>

#include "../Misc/BulkImportOperation.h"

#include <Tera/Core.h>


class BulkImportWindow : public wxFrame {
public:
	BulkImportWindow(wxWindow* parent);
	BulkImportWindow(wxWindow* parent, const wxString& className, const wxString& objectName);
	void AddOperation(const wxString& className, const wxString& objectName);
	~BulkImportWindow();

protected:
	void OnFirstIdle(wxIdleEvent&);
	void OnPathChanged(wxFileDirPickerEvent& event);
	void OnStartEditingOperation(wxDataViewEvent& event);
	void OnOperationSelected(wxDataViewEvent& event);
	void OnAddOperationClicked(wxCommandEvent& event);
	void OnEditOperationClicked(wxCommandEvent& event);
	void OnRemoveOperationClicked(wxCommandEvent& event);
	void OnClearOperationsClicked(wxCommandEvent& event);
	void OnContinueClicked(wxCommandEvent& event);
	void OnCancelClicked(wxCommandEvent& event);
	void OnOperationsListContextMenu(wxDataViewEvent& event);
	void OnOperationDoubleClick(wxDataViewEvent& event);

	void UpdateControls();
	bool LoadBuffer();

protected:
	wxFilePickerCtrl* PathPicker = nullptr;
	wxDataViewCtrl* OperationsList = nullptr;
	wxButton* AddOperationButton = nullptr;
	wxButton* EditOperationButton = nullptr;
	wxButton* RemoveOperationButton = nullptr;
	wxButton* ClearOperationsButton = nullptr;
	wxCheckBox* EnableTfc = nullptr;
	wxButton* ContinueButton = nullptr;
	wxButton* CancelButton = nullptr;

	wxString PreviousStreamPath;
	std::stringstream ObjectDumpBuffer;
	bool BufferLoaded = false;

	std::vector<BulkImportAction> Actions;

	wxString FirstStartClass;
	wxString FirstStartName;
};