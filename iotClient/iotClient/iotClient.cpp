// iotClient.cpp : Defines the IOT Client application
//

#include "iotClient.h"

/*
 * This is the request handler which requests response from the server 
 */
void iotClient::request(http_client& client, 
						method m, json::value const& jvalue)
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
			std::wcout << task.get().to_string() << std::endl << std::endl;

			// User validation required only for POST method
			if (m == L"POST")
				iotClient::callPostRequest(client, task.get());
		}
		catch (http_exception const & e)
		{
			std::wcout << e.what() << std::endl;
		}
	}).wait();
}

/*
 * The PUT request to the server to add all possible states
 * as defined in iot.json file
 */
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

/*
 * The Arm request to the server to arm the device
 */
void iotClient::callArmRequest(http_client client)
{
	auto value = json::value::array();

	// Request server to arm the device
	std::wcout << L"Request to Server: Switch on device" << std::endl;
	value[0] = json::value::string(L"OFF");
	value[1] = json::value::string(L"");
	iotClient::request(client, methods::POST, value);
}

/*
 * The Disarm request to the server to disarm the device
 */
void iotClient::callDisarmRequest(http_client client)
{
	auto value = json::value::array();

	// Request server to disarm the device
	std::wcout << L"Request to Server: Switch off device" << std::endl;
	value[0] = json::value::string(L"ON");
	value[1] = json::value::string(L"");
	iotClient::request(client, methods::POST, value);
}

/*
 * The POST request to the server to process client request 
 * and receive appropriate response. The POST request here
 * handles server requests for user code.
 */
void iotClient::callPostRequest(http_client client, web::json::value& _value)
{
	for (auto iter = _value.as_object().begin(); 
		iter != _value.as_object().end(); 
		iter++)
	{
		if (iter->second.serialize() == L"\"USER_CODE\"")
		{
			if (_value.serialize().find(L"USER_CODE") != std::wstring::npos)
			{
				// Request from server to enter code
				auto value = json::value::array();
				std::wcout << L"Request to Server: Validate code" << std::endl;
				value[0] = json::value::string(L"USER_CODE");
				value[1] = json::value::string(L"12345");
				iotClient::request(client, methods::POST, value);
			}
		}
	}
}

/*
 * The PUT request to the server to add all possible states from the json file
 */
void iotClient::callMakeRequest(web::json::value& value)
{
	http_client client(U("http://localhost:8080"));

	std::wcout << L"PUT request from client" << std::endl;
	iotClient::callPutRequest(client, value);
}

/*
 * The action request which initiates a series of POST requests
 * mainly here to arm and disarm the device. Other custom requests 
 * can be added here.
 */
void iotClient::callActionRequest()
{
	http_client client(U("http://localhost:8080"));

	std::wcout << L"Arm request from client" << std::endl;
	iotClient::callArmRequest(client);

	sleep_for(seconds(5));

	std::wcout << L"Disarm request from client" << std::endl;
	iotClient::callDisarmRequest(client);
}

/*
 * This method is responsible for requesting data from the json file
 * for all the state transitions and to make the server aware of all
 * possible states by calling the PUT request.
 */
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

int wmain()
{
	// This is the json file reader
	iotClient::HTTPMakeMethod().wait();

	// This is the call to request actions on the device
	iotClient::callActionRequest();

	_getch();
	return 0;
}