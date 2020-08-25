#pragma once
#include <wx/wx.h>
#include <wx/ipc.h>

class App;

class RpcConnection : public wxConnection {
public:
  void SetDelegate(App* delegate)
  {
    MainApplication = delegate;
  }

protected:
  bool OnExec(const wxString& topic, const wxString& message);

private:
  App* MainApplication = NULL;
};

class RpcServer : public wxServer {
public:
  void RunWithDelegate(App* delegate);

  wxConnectionBase* OnAcceptConnection(const wxString& topic) 
  {
    RpcConnection* conn = new RpcConnection;
    conn->SetDelegate(MainApplication);
    return conn;
  }
private:
  App* MainApplication = NULL;
};

class RpcClient : public wxClient {
public:
  static void SendRequest(const wxString& topic, const wxString& data);
};