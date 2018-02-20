#pragma once

#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include <iostream>
#include <sstream>
#include <conio.h>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

using namespace ::pplx;

using namespace web;
using namespace web::http;
using namespace web::http::client;

using namespace std::this_thread;
using namespace std::chrono;

class iotClient
{
public:
	static void request(http_client & client, method m, json::value const & jvalue);
	static void callMakeRequest(web::json::value& value);
	static void callActionRequest();
	static pplx::task<void> HTTPMakeMethod();

	static void callPutRequest(http_client client, web::json::value& value);
	static void callPostRequest(http_client client, web::json::value& value);

	static void callArmRequest(http_client client);
	static void callDisarmRequest(http_client client);
};