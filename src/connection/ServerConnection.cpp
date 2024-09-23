#include "ServerConnection.hpp"

ServerConnection::ServerConnection(std::shared_ptr<Log> log) : _log(log){}

ServerConnection::~ServerConnection() 
{
  for (size_t i = 0; i < _connectedPorts.size(); i++) {
	_log->logServerConnection("Closing server", _connectedPorts[i].serverID, _connectedPorts[i].serverFD, _connectedPorts[i].serverPort);
	close(_connectedPorts[i].serverFD);
  }
  _log->logAdd("All servers closed.");
  _log->logAdd("Webserv is closed correctly.");
}

void ServerConnection::initSocketInfo(ServerStruct &serverStruct, ServerInfo &info, std::list<std::string>::iterator it) 
{
	struct sockaddr_in server_addr;

	info.serverPort = atoi(it->c_str());
	info.serverID = serverStruct._id;
	info.MaxBodySize = std::stoll(*serverStruct._clientMaxBodySize.content_list.begin());
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(it->c_str()));
	info.server_addr = server_addr;
}

int	ServerConnection::createSocket(ServerInfo &info) 
{
  info.serverFD = socket(AF_INET, SOCK_STREAM, 0);
  if (info.serverFD == -1) {
		_log->logServerError("Failed to create server socket", info.serverID, info.serverPort);
		return (failed);
	}
	return (success);
}

int ServerConnection::bindPort(ServerInfo &info) 
{
  const int reuse = 1;
  if (setsockopt(info.serverFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) != 0) {
	close(info.serverFD);
	_log->logError("Could not configure socket options: " +
			 std::string(std::strerror(errno)));
  }

  int ret = bind(info.serverFD, (struct sockaddr *)&info.server_addr,
		   sizeof(info.server_addr));
	if (ret == -1) {
		_log->logServerError("Failed to bind server", info.serverID, info.serverPort);
		close(info.serverFD);
		return (failed);
  }
  return (success);
}

int	ServerConnection::listenOnPort(ServerInfo &info) 
{
	int ret = listen(info.serverFD, BACKLOG);

	if (ret == -1) {
		close(info.serverFD);
		_log->logServerError("Failed to listen for connection on server", info.serverID, info.serverPort);
		return (failed);
  }
  return (success);
}

bool	portIsConnected(int port_number, std::vector<ServerInfo>	_connectedPorts)
{
	for (ServerInfo connected : _connectedPorts){
		if (connected.serverPort == port_number)
			return (true);
	}
	return (false);
}

int	ServerConnection::setUpPorts(ServerStruct &serverStruct) 
{
  std::list<std::string>::iterator it = serverStruct._port.content_list.begin();
  for (; it != serverStruct._port.content_list.end(); ++it) {
	if (!portIsConnected(atoi(it->c_str()), _connectedPorts)) {
	  if (atoi(it->c_str()) > 0 && atoi(it->c_str()) < 65536) {
		ServerInfo info;
		initSocketInfo(serverStruct, info, it);
		if (createSocket(info) == failed)
		  return (failed);
		if (bindPort(info) == failed)
		  return (failed);
		if (listenOnPort(info) == failed)
		  return (failed);
		_connectedPorts.push_back(info);
		_log->logServerConnection("Port connection created", info.serverID, info.serverFD, info.serverPort);
	  } 
	  else {
		_log->logServerError("Invalid port number", serverStruct._id, atoi(it->c_str()));
	  }
	}
  }
  return (success);
}
