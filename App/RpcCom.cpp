#include "App.h"

const char* RpcPort = "4545";

bool RpcConnection::OnExec(const wxString& topic, const wxString& message)
{
  if (MainApplication)
  {
    if (topic == "open")
    {
      MainApplication->OnRpcOpenFile(message);
    }
  }
  return true;
}

void RpcClient::SendRequest(const wxString& topic, const wxString& data)
{
  RpcClient* client = new RpcClient;
  wxConnectionBase* conn = client->MakeConnection("localhost", RpcPort, topic);
  if (conn)
  {
    conn->Execute(data);
    delete conn;
  }
  delete client;
}

void RpcServer::RunWithDelegate(App* delegate)
{
  MainApplication = delegate;
  Create(RpcPort);
}