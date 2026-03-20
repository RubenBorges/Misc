//g++ PingGoogle.cpp -o https_client -lssl -lcrypto -lpthread
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = net::ssl;           // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

int main() {
    try {
        std::string const host = "www.google.com";
        std::string const port = "443";
        int version = 11; // HTTP 1.1

        // 1. The io_context is required for all I/O
        net::io_context ioc;

        // 2. The SSL context is required for HTTPS
        ssl::context ctx{ssl::context::tlsv12_client};
        ctx.set_default_verify_paths(); // Use system certificates

        // 3. These objects perform our I/O
        tcp::resolver resolver{ioc};
        beast::ssl_stream<beast::tcp_stream> stream{ioc, ctx};

        // 4. Set SNI Hostname (essential for modern HTTPS)
        if(!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        // 5. Look up the domain name
        auto const results = resolver.resolve(host, port);

        // 6. Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream).connect(results);

        // 7. Perform the SSL handshake
        stream.handshake(ssl::stream_base::client);

        // 8. Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, "/", version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // 9. Send the HTTP request to the remote host
        http::write(stream, req);

        // 10. This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // 11. Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // 12. Receive the HTTP response
        http::read(stream, buffer, res);

        // 13. Write the message to standard out
        std::cout << "Response received! Status: " << res.result_int() << " " << res.reason() << std::endl;

        // 14. Gracefully close the stream
        beast::error_code ec;
        stream.shutdown(ec);
        if(ec == net::error::eof) ec = {}; // Rationale: http://stackoverflow.com/questions/25587403/
    }
    catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}