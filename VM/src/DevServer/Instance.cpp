#include RIGCVM_PCH

#include <RigCVM/DevServer/Instance.hpp>
#include <RigCVM/DevServer/Utils.hpp>

namespace rigc::vm
{

DevelopmentServer* g_devServer = nullptr;

static auto sendMtx = std::mutex();

DevelopmentServer::DevelopmentServer(LogStreamPtr loggingStream)
{
	// Set logging settings
	if(loggingStream)
		setupLoggingTo(loggingStream);

	// Initialize Asio
	_endpoint.init_asio();
}

void DevelopmentServer::run()
{
	_endpoint.set_http_handler([&](ws::connection_hdl hdl) {
			ServerBase::connection_ptr con = _endpoint.get_con_from_hdl(hdl);

			auto res = con->get_request_body();

			con->set_body(fmt::format("got HTTP request with {} bytes of body data.", res.size()));
			con->set_status(ws::http::status_code::ok);
		});
	_endpoint.set_fail_handler([&](ws::connection_hdl hdl) {
			ServerBase::connection_ptr con = _endpoint.get_con_from_hdl(hdl);

			// fmt::print("Fail handler: {} {}\n", con->get_ec().value(), con->get_ec().message());
		});
	_endpoint.set_open_handler([&](ws::connection_hdl hdl) {
			{
				auto lock = std::scoped_lock(sendMtx);
				_connections.insert(hdl);
			}
			// fmt::print("Opened a new connection\n");
		});
	_endpoint.set_close_handler([&](ws::connection_hdl hdl) {
			{
				auto lock = std::scoped_lock(sendMtx);
				_connections.erase(hdl);
			}
			// fmt::print("Closed a connection\n");
		});
	_endpoint.set_validate_handler([&](ws::connection_hdl) {
			return true;
		});

	_endpoint.set_message_handler([&](ws::connection_hdl hdl, ServerBase::message_ptr msg) {
			// fmt::print("Got message:\n{}\n", msg->get_payload());

			auto json = json::parse(msg->get_payload());
			auto type = json.value("type", String(""));
			auto action = json.value("action", String(""));
			if (type == "session")
			{
				if (action == "continue" && std::stoull(json.value("suspensionId", "0")) == suspensionId)
				{
					++suspensionId;
					suspended = false;
				}
			}
			else if (type == "breakpoints")
			{
				auto breakpoints = DynArray<Breakpoint>();

				for (auto [_, bp] : json["breakpoints"].items()) {
					auto breakpoint		= Breakpoint();
					breakpoint.id		= bp.value("id", 0);
					breakpoint.line		= bp.value("line", size_t(0));
					breakpoint.column	= bp.value("column", size_t(0));
					breakpoint.verified	= bp.value("verified", false);

					breakpoints.emplace_back( std::move(breakpoint) );
				}

				if (onBreakpointsUpdated) {
					onBreakpointsUpdated(std::move(breakpoints));
				}
			}
			// _endpoint.send(hdl, msg->get_payload(), msg->get_opcode());
		});

	// Listen on port 9002
	_endpoint.listen(9002);

	// Queues a connection accept operation
	_endpoint.start_accept();

	auto broadcastMessage = [&](String const& msg_) {
		for (auto hdl : _connections)
		{
			_endpoint.get_con_from_hdl(hdl)->send(msg_);
		}
	};

	auto sender = std::jthread([&]{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds{100});
			bool queueEmpty = false;
			{
				auto lock = std::scoped_lock(sendMtx);
				queueEmpty = _messageQueue.empty();
			}

			while (!queueEmpty) {

				auto msg = String();

				{
					auto lock = std::scoped_lock(sendMtx);
					msg = std::move(_messageQueue.front());
					_messageQueue.pop();
				}

				broadcastMessage(msg);

				{
					auto lock = std::scoped_lock(sendMtx);
					queueEmpty = _messageQueue.empty();
				}
			}
		}
	});

	// Start the Asio io_service run loop
	_endpoint.run();
}

void DevelopmentServer::enqueueMessage(String msg_)
{
	auto lock = std::scoped_lock(sendMtx);
	_messageQueue.emplace( std::move(msg_) );
}

auto DevelopmentServer::setupLoggingTo(std::ostream* stream) -> void
{
	_endpoint.set_error_channels(ws::log::elevel::all);
	_endpoint.set_access_channels(ws::log::alevel::all ^ ws::log::alevel::frame_payload);

	_endpoint.get_alog().set_ostream(stream);
	_endpoint.get_elog().set_ostream(stream);
}

void DevelopmentServer::waitForContinue(ch::microseconds sleepInterval)
{
	while(suspended) {
		tt::sleep_for(sleepInterval);
	}
}

void DevelopmentServer::waitForConnection(ch::microseconds sleepInterval)
{
	while(_connections.empty()) {
		tt::sleep_for(sleepInterval);
	}
}


}
