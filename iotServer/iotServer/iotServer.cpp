// iotServer.cpp : Defines the entry point for the console application.
//

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <conio.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#include <iostream>
#include <map>
#include <set>
#include <string>
using namespace std;

map<utility::string_t, utility::string_t> dictionary;

void requestHandler(http_request req, 
			function<void(json::value const &, json::value &)> method)
{
	auto resp = json::value::object();

	req.extract_json().then([&resp, &method](pplx::task<json::value> task) {
		try
		{
			auto const & jvalue = task.get();
			std::wcout << L"\nRequest from client" << std::endl;
			std::wcout << jvalue.serialize() << std::endl;

			if (!jvalue.is_null())
			{
				std::wcout << L"\nReply from server" << std::endl;
				method(jvalue, resp);
			}
		}
		catch (http_exception const & e)
		{
			wcout << e.what() << endl;
		}
	}).wait();

	req.reply(status_codes::OK, resp);
}

void postHandler(http_request req)
{
	std::wostringstream stream;
	stream.str(std::wstring());
	stream << L"\nReceived request from client" << std::endl;
	stream << L"Content type: " << req.headers().content_type() << std::endl;
	stream << L"Content length: " << req.headers().content_length() << L"bytes" << std::endl;
	std::wcout << stream.str() << std::endl;

	requestHandler(req,	[](json::value const & jvalue, json::value & resp)
	{
		for (auto const & e : jvalue.as_array())
		{
			if (e.is_array())
			{
				auto value = json::value::array();
				int count = 0;
				for (auto const &e1 : e.as_array())
				{
					value[count++] = e1;
				}

				auto pos = dictionary.find(value[0].as_string());
				if (pos == dictionary.end())
				{
					std::wcout << json::value::string(L"<nil>") << std::endl;
					resp[value[0].as_string()] = json::value::string(L"<nil>");
				}
				else
				{
					std::wcout << pos->first << L" : " << json::value::string(pos->second) << std::endl;
					resp[pos->first] = json::value::string(pos->second);
				}
			}
		}
	});
}

void putHandler(http_request req)
{
	std::wostringstream stream;
	stream.str(std::wstring());
	stream << L"\nReceived request from client" << std::endl;
	stream << L"Content type: " << req.headers().content_type() << std::endl;
	stream << L"Content length: " << req.headers().content_length() << L"bytes" << std::endl;
	std::wcout << stream.str() << std::endl;

	requestHandler(req,	[](json::value const & jvalue, json::value & resp)
	{
		for (auto const &e : jvalue.as_array())
		{
			if (e.is_array())
			{
				auto value = json::value::array();
				int count = 0;
				for (auto const &e1 : e.as_array())
				{
					value[count++] = e1;
				}

				dictionary[value[0].as_string()] = value[1].as_string();

				std::wcout << L"Added - " << value[0].as_string() << L" : " << value[1].as_string() << std::endl;
			}
		}
	});
}

int main()
{
	http_listener listener(L"http://localhost:8080");

	listener.support(methods::PUT, putHandler);
	listener.support(methods::POST, postHandler);

	try
	{
		listener
			.open()
			.then([&listener]() { std::wcout << L"\nServer listening to incoming connections\n" << std::endl; })
			.wait();

		while (true);
	}
	catch (exception const & e)
	{
		wcout << e.what() << endl;
	}

	_getch();
	return 0;
}