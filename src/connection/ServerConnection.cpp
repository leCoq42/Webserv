#include "ServerConnection.hpp"

ServerConnection::ServerConnection() {}

ServerConnection::~ServerConnection() 
{
  for (size_t i = 0; i < _connectedServers.size(); i++) {
    _log.logServerConnection("Closing server", _connectedServers[i].serverID, _connectedServers[i].serverFD, _connectedServers[i].serverPort);
    close(_connectedServers[i].serverFD);
  }
}

void ServerConnection::initServerInfo(ServerStruct &serverStruct, ServerInfo &info, std::list<std::string>::iterator it) 
{
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	info.serverPort = atoi(it->c_str());
	info.serverID = serverStruct._id;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(it->c_str()));
	info.server_addr = server_addr;
}

void ServerConnection::createServerSocket(ServerInfo &info) 
{
  info.serverFD = socket(AF_INET, SOCK_STREAM, 0);
  if (info.serverFD == -1)
    _log.logServerError("Failed to create server socket", info.serverID, info.serverPort);
}

void ServerConnection::bindServerSocket(ServerInfo &info) 
{
  const int reuse = 1;
  if (setsockopt(info.serverFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) != 0) {
    close(info.serverFD);
    _log.logError("Could not configure socket options: " +
             std::string(std::strerror(errno)));
  }
  if (bind(info.serverFD, (struct sockaddr *)&info.server_addr,
           sizeof(info.server_addr)) == -1) {
    _log.logServerError("Failed to bind server", info.serverID, info.serverPort);
    close(info.serverFD);
  }
}

void ServerConnection::listenIncomingConnections(ServerInfo &info) {
  if (listen(info.serverFD, BACKLOG) == -1) {
    close(info.serverFD);
    _log.logServerError("Failed to listen for connection on server", info.serverID, info.serverPort);
  }
}

bool	already_in_servers(int port_number, std::vector<ServerInfo>	_connectedServers)
{
	for (ServerInfo connected : _connectedServers){
		if (connected.serverPort == port_number)
			return (true);
	}
	return (false);
}

void ServerConnection::setUpServerConnection(ServerStruct &serverStruct) 
{
  std::list<std::string>::iterator it = serverStruct._port.content_list.begin();
  for (; it != serverStruct._port.content_list.end(); ++it) {
	if (!already_in_servers(atoi(it->c_str()), _connectedServers))
	{
		if (atoi(it->c_str()) > 0 && atoi(it->c_str()) < 65536) {
			ServerInfo info;
			initServerInfo(serverStruct, info, it);
			createServerSocket(info);
			bindServerSocket(info);
			listenIncomingConnections(info);
			_connectedServers.push_back(info);
			_log.logServerConnection("Server created", info.serverID, info.serverFD, info.serverPort);
		} 
		else {
			_log.logServerError("Invalid port number", serverStruct._id, atoi(it->c_str()));
		}
	}
  }
}
