// iotServer.cpp : Defines the entry point for the console application.
//

#include "iotServer.h"

bool iotServer::state;

void iotServer::requestHandler(http_request req,
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

void iotServer::postHandler(http_request req)
{
	std::wostringstream stream;
	stream.str(std::wstring());
	stream << L"\nReceived request from client" << std::endl;
	stream << L"Content type: " << req.headers().content_type() << std::endl;
	stream << L"Content length: " << 
		req.headers().content_length() << L"bytes" << std::endl;
	std::wcout << stream.str() << std::endl;

	requestHandler(req, [](json::value const & jvalue, json::value & resp)
	{
		if (jvalue.is_array())
		{
			auto key = json::value();
			auto value = json::value();
			int count = 0;
			// The array (always size 2) can only be key and code if provided.
			for (auto const & e : jvalue.as_array()) 
			{
				if (count == 0)
					key = e;
				else
					value = e;
			}

			auto pos = dictionary.find(key.as_string());
			if (pos == dictionary.end()) // key not found
			{
				std::wcout << json::value::string(L"<nil>") << std::endl;
				resp[key.serialize()] = json::value::string(L"<nil>");
			}
			else // key found. Now process value of the key
			{
				// Switch on the device
				if ((key.serialize() == L"OFF") || (key.serialize() == L"ON"))
				{
					std::wcout << L"Request client to enter code" << std::endl;
					resp[key.serialize()] = json::value::string(L"USER_CODE");

					if (key.serialize() == L"OFF")
						state = false; // disarm request
					else
						state = true; // arm request
				}
				else if (key.serialize() == L"USER_CODE")
				{
					// Check if code is present
					if (value.serialize() == L"")
					{
						std::wcout << json::value::string(L"<nil>") << std::endl;
						resp[key.serialize()] = json::value::string(L"<nil>");
					}
					else // If code is present, validate the code
					{
						auto code = dictionary[key.serialize()][2];
						if (value.serialize() == code.serialize()) // Valid code
						{
							// Send ok response
							auto dict_val = dictionary[L"CODE_VALIDATED"];
							std::wcout << L"CODE_VALIDATED : " << dict_val.serialize() << std::endl;
							if (state == true)
								std::wcout << L"Device armed" << std::endl;
							else
								std::wcout << L"Device disarmed" << std::endl;
							resp[L"CODE_VALIDATED"] = code;
						}
						else // Invalid code
						{
							// Send not ok response
							auto dict_val = dictionary[L"CODE_INVALIDATED"];
							std::wcout << L"CODE_INVALIDATED : " << dict_val.serialize() << std::endl;
							if (key.serialize() == L"OFF")
								std::wcout << L"Device not armed" << std::endl;
							else
								std::wcout << L"Device not disarmed" << std::endl;
							resp[L"CODE_INVALIDATED"] = code;
						}
					}
				}
				else
				{
					// Any other code other than ON or OFF will enter this control path
					std::wcout << L"Unrecognized input" << std::endl;
					resp[key.serialize()] = json::value::string(L"<nil>");
				}
			}
		}
		else
		{
			// Invalid input
			std::wcout << L"Invalid input" << std::endl;
			resp[L"INVALID"] = json::value::string(L"<nil>");
		}
	});
}

void iotServer::putHandler(http_request req)
{
	std::wostringstream stream;
	stream.str(std::wstring());
	stream << L"\nReceived request from client" << std::endl;
	stream << L"Content type: " << req.headers().content_type() << std::endl;
	stream << L"Content length: " << 
			req.headers().content_length() << L"bytes" << std::endl;
	std::wcout << stream.str() << std::endl;

	requestHandler(req,	[](json::value const & jvalue, json::value & resp)
	{
		for (auto const &e : jvalue.as_array())
		{
			if (e.is_array())
			{
				auto value = json::value::array();
				auto key = json::value();
				int count = 0;

				for (auto const &e1 : e.as_array())
				{
					if (count == 0)
						key = e1;
					else
						value[count - 1] = e1;
					count++;
				}

				dictionary[key.as_string()] = value;

				std::wcout << L"Added - " << key.serialize() << L" : " <<
					value.serialize() << std::endl;
				resp[key.as_string()] = value;
			}
		}
	});
}

int main()
{
	http_listener listener(L"http://localhost:8080");

	listener.support(methods::PUT, iotServer::putHandler);
	listener.support(methods::POST, iotServer::postHandler);

	try
	{
		listener
			.open()
			.then([&listener]() { std::wcout << 
				L"\nServer listening to incoming connections\n" << std::endl; })
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