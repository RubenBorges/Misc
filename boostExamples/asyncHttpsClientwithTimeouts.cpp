//g++ temp.cpp -o https_client -lssl -lcrypto -lpthread
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp> // Fix for the error you saw
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
namespace ssl = net::ssl;               
using tcp = net::ip::tcp;               

// Performs an HTTP GET and prints the response
class Session : public std::enable_shared_from_this<Session> {
    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_; 
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;

public:
    explicit Session(net::io_context& ioc, ssl::context& ctx)
        : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc), ctx) {}

    void run(char const* host, char const* port, char const* target) {
        // Set SNI Hostname (important for HTTPS!)
        if(!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            return fail(ec, "set_tlsext_host_name");
        }

        req_.version(11);
        req_.method(http::verb::get);
        req_.target(target);
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        resolver_.async_resolve(host, port,
            beast::bind_front_handler(&Session::on_resolve, shared_from_this()));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if(ec) return fail(ec, "resolve");

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        beast::get_lowest_layer(stream_).async_connect(results,
            beast::bind_front_handler(&Session::on_connect, shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if(ec) return fail(ec, "connect");

        stream_.async_handshake(ssl::stream_base::client,
            beast::bind_front_handler(&Session::on_handshake, shared_from_this()));
    }

    void on_handshake(beast::error_code ec) {
        if(ec) return fail(ec, "handshake");

        http::async_write(stream_, req_,
            beast::bind_front_handler(&Session::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if(ec) return fail(ec, "write");
        
        http::async_read(stream_, buffer_, res_,
            beast::bind_front_handler(&Session::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if(ec) return fail(ec, "read");

        // Check for HTTP Redirect (301/302)
        if (res_.result() == http::status::moved_permanently || res_.result() == http::status::found) {
            std::cout << "Redirected to: " << res_["Location"] << "\n";
        } else {
            std::cout << res_ << std::endl;
        }

        // Gracefully close
        stream_.async_shutdown(beast::bind_front_handler(&Session::on_shutdown, shared_from_this()));
    }

    void on_shutdown(beast::error_code ec) {
        if(ec == net::error::eof || ec == ssl::error::stream_truncated) ec = {};
        if(ec) return fail(ec, "shutdown");
    }

    void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
};

int main() {
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    ctx.set_default_verify_paths();

    std::make_shared<Session>(ioc, ctx)->run("www.google.com", "443", "/");

    ioc.run();
    return 0;
}