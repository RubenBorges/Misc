#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <deque>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class PersistentSession : public std::enable_shared_from_this<PersistentSession> {
    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    std::deque<std::string> target_queue_;
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    std::string host_;

public:
    PersistentSession(net::io_context& ioc, ssl::context& ctx)
        : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc), ctx) {}

    void run(const std::string& host, std::deque<std::string> targets) {
        host_ = host;
        target_queue_ = std::move(targets);

        resolver_.async_resolve(host_, "443",
            beast::bind_front_handler(&PersistentSession::on_resolve, shared_from_this()));
    }

private:
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if(ec) return fail(ec, "resolve");
        beast::get_lowest_layer(stream_).async_connect(results,
            beast::bind_front_handler(&PersistentSession::on_connect, shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if(ec) return fail(ec, "connect");
        SSL_set_tlsext_host_name(stream_.native_handle(), host_.c_str());
        stream_.async_handshake(ssl::stream_base::client,
            beast::bind_front_handler(&PersistentSession::on_handshake, shared_from_this()));
    }

    void on_handshake(beast::error_code ec) {
        if(ec) return fail(ec, "handshake");
        send_next_request();
    }

    void send_next_request() {
        if (target_queue_.empty()) return graceful_shutdown();

        req_ = {};
        req_.version(11);
        req_.method(http::verb::get);
        req_.target(target_queue_.front());
        req_.set(http::field::host, host_);
        // FIX: Replaced macro with literal string
        req_.set(http::field::user_agent, "BoostBeastClient/1.0");
req_.keep_alive(true); // Correct: This is a built-in helper method
        target_queue_.pop_front();

        http::async_write(stream_, req_,
            beast::bind_front_handler(&PersistentSession::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t) {
        if(ec) return fail(ec, "write");
        res_ = {};
        http::async_read(stream_, buffer_, res_,
            beast::bind_front_handler(&PersistentSession::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t) {
        if(ec) return fail(ec, "read");
        std::cout << "Response for " << req_.target() << ": " << res_.result_int() << std::endl;
        
        if (res_.keep_alive()) send_next_request();
        else graceful_shutdown();
    }

    void graceful_shutdown() {
        stream_.async_shutdown(beast::bind_front_handler(&PersistentSession::on_shutdown, shared_from_this()));
    }

    void on_shutdown(beast::error_code ec) {
        if(ec == net::error::eof || ec == ssl::error::stream_truncated) ec = {};
    }

    void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << std::endl;
    }
};

int main() {
    try {
        net::io_context ioc;
        ssl::context ctx{ssl::context::tlsv12_client};
        ctx.set_default_verify_paths();

        auto session = std::make_shared<PersistentSession>(ioc, ctx);
        session->run("www.google.com", {"/", "/robots.txt"});

        ioc.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}