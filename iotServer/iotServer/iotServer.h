#pragma once

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

class iotServer
{
public:
	static void requestHandler(http_request req,
		function<void(json::value const &, json::value &)> method);
	static void postHandler(http_request req);
	static void putHandler(http_request req);

};