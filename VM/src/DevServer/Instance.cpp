#include RIGCVM_PCH

#include <RigCVM/DevServer/Instance.hpp>

namespace rigc::vm
{

DevelopmentServer* g_devServer = nullptr;

static auto sendMtx = std::mutex();

DevelopmentServer::DevelopmentServer()
{
	// Set logging settings
	_endpoint.set_error_channels(ws::log::elevel::all);
	_endpoint.set_access_channels(ws::log::alevel::all ^ ws::log::alevel::frame_payload);

	// Initialize Asio
	_endpoint.init_asio();
}

void DevelopmentServer::run()
{
	_endpoint.set_http_handler([&](ws::connection_hdl hdl) {
			ServerBase::connection_ptr con = _endpoint.get_con_from_hdl(hdl);

			std::string res = con->get_request_body();

			std::stringstream ss;
			ss << "got HTTP request with " << res.size() << " bytes of body data.";

			con->set_body(ss.str());
			con->set_status(ws::http::status_code::ok);
		});
	_endpoint.set_fail_handler([&](ws::connection_hdl hdl) {
			ServerBase::connection_ptr con = _endpoint.get_con_from_hdl(hdl);

			std::cout << "Fail handler: " << con->get_ec() << " " << con->get_ec().message()  << std::endl;
		});
	_endpoint.set_open_handler([&](ws::connection_hdl hdl) {
			{
				auto lock = std::scoped_lock(sendMtx);
				_connections.insert(hdl);
			}
			std::cout << "Opened a new connection" << std::endl;
		});
	_endpoint.set_close_handler([&](ws::connection_hdl hdl) {
			{
				auto lock = std::scoped_lock(sendMtx);
				_connections.erase(hdl);
			}
			std::cout << "Closed a connection" << std::endl;
		});
	_endpoint.set_validate_handler([&](ws::connection_hdl) {
			//sleep(6);
			return true;
		});

	// Listen on port 9002
	_endpoint.listen(9002);

	// Queues a connection accept operation
	_endpoint.start_accept();

	auto broadcastMessage = [&](std::string const& msg_) {
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

				auto msg = std::string();

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

void DevelopmentServer::enqueueMessage(std::string msg_)
{
	auto lock = std::scoped_lock(sendMtx);
	_messageQueue.emplace( std::move(msg_) );
}

}
