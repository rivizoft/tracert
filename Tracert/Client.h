#pragma once

#include <string>
#include <boost/beast.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace http = boost::beast::http;

class Client
{
public:
	Client(std::string url, std::string arguments)
	{
		_url = url;
		_arguments = arguments;
	}

	std::string getResponse()
	{
		boost::asio::io_context ioc;
		boost::asio::ip::tcp::resolver resolver(ioc);
		boost::asio::ip::tcp::socket socket(ioc);

		boost::asio::connect(socket, resolver.resolve(_url, "80"));
		
		http::request<http::string_body> req(http::verb::get, _arguments, 11);
		req.set(http::field::host, _url);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		http::write(socket, req);

		std::string response;
		//read response
		{
			boost::beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(socket, buffer, res);
			response = boost::beast::buffers_to_string(res.body().data());
		}

		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		return response;
	}

private: 
	std::string _url;
	std::string _arguments;
};

