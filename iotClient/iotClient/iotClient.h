#pragma once

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


class iotClient
{
public:
	static void request(http_client & client, method m, json::value const & jvalue);
	static void callMakeRequest(web::json::value getValue);
	static pplx::task<void> HTTPMethod();
};