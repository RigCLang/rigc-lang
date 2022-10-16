#pragma once

#include RIGCVM_PCH

#include <RigCVM/DevServer/Breakpoint.hpp>

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
	void stop();

	void enqueueMessage(String msg_);

	auto& getConnections() const {
		return _connections;
	}

	std::atomic_bool		suspended = true; // This doesn't have to be atomic
	std::atomic_uint64_t	suspensionId = 0;


	/// <summary>
	/// Suspends the current thread until `suspended` is false.
	/// </summary>
	void waitForContinue(ch::microseconds sleepInterval = ch::milliseconds(100));

	void waitForConnection(ch::microseconds sleepInterval = ch::milliseconds(100));


	std::function<void(DynArray<Breakpoint>)> onBreakpointsUpdated;
private:
	using ConnectionSet = Set<ws::connection_hdl, std::owner_less<ws::connection_hdl>>;

	auto setupLoggingTo(std::ostream* loggingStream) -> void;

	Queue<String>		_messageQueue;
	ConnectionSet		_connections;
	ServerBase			_endpoint;
	std::atomic_bool	_stopped = false;
};

extern DevelopmentServer* g_devServer;

}
