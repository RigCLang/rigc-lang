#pragma once

#include RIGCVM_PCH

namespace ws = websocketpp;

namespace rigc::vm
{

using ServerBase = ws::server<ws::config::asio>;

class DevelopmentServer
{
public:
	DevelopmentServer();

	void run();

	void enqueueMessage(std::string msg_);

private:
	using ConnectionSet = std::set<ws::connection_hdl,std::owner_less<ws::connection_hdl>>;

	std::queue<std::string> _messageQueue;
	ConnectionSet			_connections;
	ServerBase				_endpoint;
};

extern DevelopmentServer* g_devServer;

}