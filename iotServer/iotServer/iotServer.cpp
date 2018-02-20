// iotServer.cpp : Defines the IOT Server application
//

#include "iotServer.h"

bool iotServer::state;

/*
 * This method analyzes which method handler to be called
 * and calls the appropriate method handler and returns the
 * response to the client
 */
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

	// Reply back to the client
	req.reply(status_codes::OK, resp);
}

/*
 * This is the POST method handler
 * This method analyzes requests for different states 
 * This method is responsible for different state transitions
 * and for validating the user code.
 */
void iotServer::postHandler(http_request req)
{
	std::wostringstream stream;
	stream.str(std::wstring());
	stream << L"\nReceived request from client" << std::endl;
	stream << L"Handler type: " << req.method() << std::endl;
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

				count++;
			}

			auto pos = dictionary.find(key.serialize());
			if (pos == dictionary.end()) // key not found
			{
				// Any other code other than ON, OFF or USER_CODE will enter this path
				std::wcout << L"Unrecognized input" << std::endl;
				resp[key.serialize()] = json::value::string(L"Unrecognized input");
			}
			else // key found. Now process value of the key
			{
				// Switch on the device
				if ((key.serialize() == L"\"OFF\"") || (key.serialize() == L"\"ON\""))
				{
					std::wcout << L"Request client to enter code" << std::endl;
					resp[key.serialize()] = json::value::string(L"USER_CODE");

					if (key.serialize() == L"\"OFF\"")
						state = true; // arm request
					else
						state = false; // disarm request
				}
				else if (key.serialize() == L"\"USER_CODE\"")
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
							auto dict_val = dictionary[L"\"CODE_VALIDATED\""];
							std::wcout << L"CODE_VALIDATED : " << dict_val.serialize() << std::endl;
							if (state == true)
							{
								std::wcout << L"Device armed" << std::endl;
								resp[L"CODE_VALIDATED"] = 
									json::value::string(L"Device armed : " + value.serialize());
							}
							else
							{
								std::wcout << L"Device disarmed" << std::endl;
								resp[L"CODE_VALIDATED"] = 
									json::value::string(L"Device disarmed : " + value.serialize());
							}
						}
						else // Invalid code
						{
							// Send not ok response
							auto dict_val = dictionary[L"\"CODE_INVALIDATED\""];
							std::wcout << L"CODE_INVALIDATED : " << dict_val.serialize() << std::endl;
							if (state == true)
							{
								std::wcout << L"Device not armed" << std::endl;
								resp[L"CODE_INVALIDATED"] = 
									json::value::string(L"Device not armed : " + value.serialize());
							}
							else
							{
								std::wcout << L"Device not disarmed" << std::endl;
								resp[L"CODE_INVALIDATED"] =
									json::value::string(L"Device not disarmed : " + value.serialize());
							}
						}
					}
				}
			}
		}
		else
		{
			// Invalid input
			std::wcout << L"Invalid input" << std::endl;
			resp[L"INVALID"] = 
				json::value::string(L"Invalid input. Array with STATE and CODE expected!");
		}
	});
}

/*
 * This is the PUT method handler
 * This method adds all possible states to a dictionary
 */
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

				dictionary[key.serialize()] = value;

				std::wcout << L"Added - " << key.serialize() << L" : " <<
					value.serialize() << std::endl;
				resp[key.serialize()] = value;
			}
		}
	});
}

int main()
{
	// Listen to incoming connections at port :8080
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