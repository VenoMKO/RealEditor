#include "../App.h"

const char* RpcPort = "4545";
const char* RpcHost = "localhost";

bool RpcConnection::OnExec(const wxString& topic, const wxString& message)
{
  if (wxTheApp && topic == "open")
  {
    App::GetSharedApp()->OnRpcOpenFile(message);
  }
  return true;
}

void RpcClient::SendRequest(const wxString& topic, const wxString& data)
{
  RpcClient* client = new RpcClient;
  wxConnectionBase* conn = client->MakeConnection(RpcHost, RpcPort, topic);
  if (conn)
  {
    conn->Execute(data);
    delete conn;
  }
  delete client;
}

void RpcServer::Run()
{
  Create(RpcPort);
}

wxConnectionBase* RpcServer::OnAcceptConnection(const wxString& topic)
{
  return new RpcConnection;
}
