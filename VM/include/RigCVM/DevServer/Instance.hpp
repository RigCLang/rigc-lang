#pragma once

#include RIGCVM_PCH

namespace ws = websocketpp;

namespace rigc::vm
{

using ServerBase = ws::server<ws::config::asio>;

class DevelopmentServer
{
public:
	using LogStreamPtr = std::ostream*;

	DevelopmentServer(LogStreamPtr loggingStream);

	void run();

	void enqueueMessage(String msg_);

private:
	using ConnectionSet = std::set<ws::connection_hdl, std::owner_less<ws::connection_hdl>>;

	auto setupLoggingTo(std::ostream* loggingStream) -> void;

	Queue<String>	_messageQueue;
	ConnectionSet	_connections;
	ServerBase		_endpoint;
};

extern DevelopmentServer* g_devServer;

}
