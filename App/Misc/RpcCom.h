#pragma once
// DDE does not allow user -> admin communications.
// So opening gpks from Explorer won't work if RE runs with admin rights.
// Disabling DDE forces WX to use TCP implementation. No DDE - no issue.
#define wxUSE_DDE_FOR_IPC 0

#include <wx/wx.h>
#include <wx/ipc.h>

class RpcConnection : public wxConnection {
protected:
  bool OnExec(const wxString& topic, const wxString& message) override;
};

class RpcServer : public wxServer {
public:
  void Run();
  wxConnectionBase* OnAcceptConnection(const wxString& topic) override;
};

class RpcClient : public wxClient {
public:
  static void SendRequest(const wxString& topic, const wxString& data);
};