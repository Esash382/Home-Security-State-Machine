// iotClient.cpp : Defines the entry point for the console application.
//

#include "iotClient.h"

void iotClient::request(http_client& client, method m, json::value const& jvalue)
{
	client.request(m, L"/iot", jvalue).then([](http_response res)
	{
		if (res.status_code() == status_codes::OK)
		{
			return res.extract_json();
		}
		return pplx::task_from_result(json::value());
	}).then([=](pplx::task<json::value> task)
	{
		try
		{
			std::wcout << L"Reply from Server:" << std::endl;
			std::wcout << task.get().serialize() << std::endl << std::endl;

			iotClient::callPostRequest(client, task.get());
		}
		catch (http_exception const & e)
		{
			std::wcout << e.what() << std::endl;
		}
	}).wait();
}

void iotClient::callPutRequest(http_client client, web::json::value& value)
{
	auto getvalue = json::value::array();
	int counter = 0;

	for (auto const & e : value.as_array())
	{
		auto value = json::value::array();
		value[0] = e.at(utility::string_t(L"STATE"));
		value[1] = e.at(utility::string_t(L"TRANSITION"));
		value[2] = e.at(utility::string_t(L"ACTION"));
		value[3] = e.at(utility::string_t(L"CODE"));

		getvalue[counter++] = value;
	}

	std::wcout << L"Request to Server:" << std::endl;
	std::wcout << getvalue.serialize() << std::endl << std::endl;
	iotClient::request(client, methods::PUT, getvalue);
}

void iotClient::callArmRequest(http_client client)
{
	auto value = json::value::array();

	// Request server to arm the device
	std::wcout << L"Request to Server: Switch on device" << std::endl;
	value[0] = json::value::string(L"OFF");
	value[1] = json::value::string(L"");
	iotClient::request(client, methods::POST, value);
}

void iotClient::callDisarmRequest(http_client client)
{
	auto value = json::value::array();

	// Request server to disarm the device
	std::wcout << L"Request to Server: Switch off device" << std::endl;
	value[0] = json::value::string(L"ON");
	value[1] = json::value::string(L"");
	iotClient::request(client, methods::POST, value);
}

void iotClient::callPostRequest(http_client client, web::json::value& _value)
{
	auto value = json::value::array();

	if (_value.serialize().find(L"USER_CODE") != std::wstring::npos)
	{
		// Request from server to enter code
		std::wcout << L"Request to Server: Validate code" << std::endl;
		value[0] = json::value::string(L"USER_CODE");
		value[1] = json::value::string(L"12345");
		iotClient::request(client, methods::POST, value);
	}
}

void iotClient::callMakeRequest(web::json::value& value)
{
	http_client client(U("http://localhost:8080"));

	std::wcout << L"PUT request from client" << std::endl;
	iotClient::callPutRequest(client, value);
}

void iotClient::callActionRequest()
{
	http_client client(U("http://localhost:8080"));

	std::wcout << L"Arm request from client" << std::endl;
	iotClient::callArmRequest(client);

	std::wcout << L"Disarm request from client" << std::endl;
	iotClient::callDisarmRequest(client);
}

pplx::task<void> iotClient::HTTPMakeMethod()
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
			json::value& value = task.get();

			if (!value.is_null())
			{
				iotClient::callMakeRequest(value);
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
	iotClient::HTTPMakeMethod().wait();
	iotClient::callActionRequest();

	_getch();
	return 0;
}