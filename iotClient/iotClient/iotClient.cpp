// iotClient.cpp : Defines the entry point for the console application.
//

#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include <iostream>
#include <sstream>
#include <conio.h>
#include <string>

using namespace ::pplx;

using namespace web;
using namespace web::http;
using namespace web::http::client;

void request(http_client & client, method m, json::value const & jvalue)
{
	client.request(m, L"/iot", jvalue).then([](http_response res)
	{
		if (res.status_code() == status_codes::OK)
		{
			return res.extract_json();
		}
		return pplx::task_from_result(json::value());
	}).then([](pplx::task<json::value> task)
	{
		try
		{
			std::wcout << L"Reply from Server:" << std::endl;
			std::wcout << task.get().serialize() << std::endl << std::endl;
		}
		catch (http_exception const & e)
		{
			std::wcout << e.what() << std::endl;
		}
	}).wait();
}

void callMakeRequest(web::json::value getValue)
{
	http_client client(U("http://localhost:8080"));
	std::wcout << L"PUT request from client" << std::endl;
	request(client, methods::PUT, getValue);

	std::wcout << L"POST request from client" << std::endl;
	request(client, methods::POST, getValue);
}

pplx::task<void> HTTPMethod()
{
	http_client client(U("http://localhost/examples/"));

	http_request req(methods::GET);
	req.headers().set_content_type(L"application/json");
	req.set_request_uri(U("iot"));

	return client
		.request(req)
		.then([](http_response res) -> pplx::task<json::value>
	{
		if (res.status_code() == status_codes::OK)
		{
			res.headers().set_content_type(L"application/json");
			return res.extract_json();
		}
		else
		{
			return pplx::task_from_result(json::value());
		}
	}).then([](pplx::task<json::value> task)
	{
		try
		{
			json::value const & value = task.get();

			if (!value.is_null())
			{
				auto getvalue = json::value::array();

				for (auto const & e : value.as_array())
				{
					auto pairvalue1 = json::value::array();
					pairvalue1[0] = json::value(utility::string_t(L"STATE"));
					pairvalue1[1] = e.at(utility::string_t(L"STATE"));

					getvalue[0] = pairvalue1;

					auto pairvalue2 = json::value::array();
					pairvalue2[0] = json::value(utility::string_t(L"TRANSITION"));
					pairvalue2[1] = e.at(utility::string_t(L"TRANSITION"));

					getvalue[1] = pairvalue2;

					auto pairvalue3 = json::value::array();
					pairvalue3[0] = json::value(utility::string_t(L"ACTION"));
					pairvalue3[1] = e.at(utility::string_t(L"ACTION"));

					getvalue[2] = pairvalue3;

					auto pairvalue4 = json::value::array();
					pairvalue4[0] = json::value(utility::string_t(L"CODE"));
					pairvalue4[1] = e.at(utility::string_t(L"CODE"));

					getvalue[3] = pairvalue4;

					std::wcout << L"Request to Server:" << std::endl;
					std::wcout << getvalue.serialize() << std::endl << std::endl;
					callMakeRequest(getvalue);
				}
			}
		}
		catch (http_exception const & e)
		{
			std::wcout << e.what() << std::endl;
		}
	});
}

int wmain(int argc, char *args[])
{
	HTTPMethod().wait();

	_getch();
	return 0;
}