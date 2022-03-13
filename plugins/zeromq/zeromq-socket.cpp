// tcp-socket.cpp
// TODO - use a thread to retry connections after failure

#include "plugin/plugin.hpp"

#include <cassert>
#include <cstring>
#include <string>
#include <thread>

#include "zmq.h"

#include "fpsi/src/session/session.hpp"
#include "fpsi/src/data/datahandler.hpp"
#include "fpsi/src/util/logging.hpp"


namespace fpsi {

class ZeroMQSocket : public Plugin {
public:
  ZeroMQSocket(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) :
		Plugin(plugin_name, plugin_path, plugin_config) {
		
		// Read config
		this->is_client = plugin_config.value<bool>("is_client", false);
		this->is_server = plugin_config.value<bool>("is_server", false);
		assert(("Can't be client and server (or neither)", is_client != is_server));
		this->port = plugin_config.value<uint32_t>("port", 7331);
		this->ip = std::string("tcp://") + plugin_config.value<std::string>("ip", "127.0.0.1") + ":" + std::to_string(this->port);

		util::log(util::debug, "ZeroMQ Settings: %s", this->ip.c_str());
		
		this->context = zmq_ctx_new();
		
		if (is_server) {
			// Finish binding
			this->responder = zmq_socket(this->context, ZMQ_PAIR);
			int rc = zmq_bind(this->responder, this->ip.c_str());
			assert (rc == 0);
			
			// Continue looking for messages
			auto server_loop_function = [this]() {
				const size_t BUFF_SIZE = 1024 * 10;  // 10kb buffer size
				char buffer[BUFF_SIZE];
				while (!::fpsi::session->exiting) {
					const std::lock_guard<std::mutex> recv_lock(this->comm_lock);
					int bytes_read = zmq_recv(this->responder, buffer, BUFF_SIZE, ZMQ_DONTWAIT);
					if (bytes_read > 0) {
						std::vector<uint8_t> v;
						v.resize(bytes_read);
						memcpy(v.data(), buffer, bytes_read);
						json message = json::from_bson(v);
						::fpsi::session->receive(message);  // Notify fpsi
					}
				}
			};
				
			this->recv_thread = std::thread(server_loop_function);
		}
		
		if (is_client) {
			// Finish connecting
			this->requester = zmq_socket(this->context, ZMQ_PAIR);
			if (zmq_connect(this->requester, this->ip.c_str()) == -1) {
				util:log(util::debug, "Connect error: %d", zmq_errno());
			}
			
			// Continue receiving messages
			auto client_loop_function = [this]() {
				const size_t BUFF_SIZE = 1024 * 10;  // 10kb buffer size
				char buffer[BUFF_SIZE];
				while (!::fpsi::session->exiting) {
					const std::lock_guard<std::mutex> recv_lock(this->comm_lock);
					int bytes_read = zmq_recv(this->requester, buffer, BUFF_SIZE, ZMQ_DONTWAIT);
					if (bytes_read > 0) {
						util::log(util::debug, "Received message! %d bytes", bytes_read);
						std::vector<uint8_t> v;
						v.resize(bytes_read);
						memcpy(v.data(), buffer, bytes_read);
						json message = json::from_bson(v);
						::fpsi::session->receive(message);  // Notify fpsi
					}
				}			
			};
				
			this->recv_thread = std::thread(client_loop_function);
		}
  }

  ~ZeroMQSocket() {
		if (this->recv_thread.joinable())
			this->recv_thread.join();

		void *sender = is_client ? this->requester : this->responder;
		if (sender != nullptr) {
			zmq_close(sender);
		}
		zmq_ctx_destroy(this->context);
  }

	virtual void broadcast(const json &message) {
		if (!this->is_socket_setup()) return;
		void *sender = is_client ? this->requester : this->responder;
		assert(("Sender has been initialized", sender != nullptr));

		const std::lock_guard<std::mutex> send_lock(this->comm_lock);

		auto message_bson = json::to_bson(message);
		int sent_bytes = zmq_send(sender, message_bson.data(), message_bson.size(), 0);
		if (sent_bytes != message_bson.size()) {
			util::log(util::error, "zmq-socket: Unable to send correct number of bytes: Sent %d/%lu", sent_bytes, message_bson.size());
			util::log(util::error, "zmq errno: %d", zmq_errno());
		}
	}

	virtual void receive(const json &message) {
		// No reason for a pure-communicator to care about what data is received
	}


private:
	std::string ip;
	uint32_t port;
	void *context = nullptr;
	bool is_client = false;
	void *responder = nullptr;
	bool is_server = false;
	void *requester = nullptr;
	std::thread recv_thread;
	std::mutex comm_lock;

	bool is_socket_setup() {
		return ((this->responder != nullptr) ^ (this->requester != nullptr));
	}
  
};

}

extern "C" fpsi::Plugin *construct_plugin(const std::string &plugin_name, const std::string &plugin_path, const json &plugin_config) {
  return new fpsi::ZeroMQSocket(plugin_name, plugin_path, plugin_config);
}
