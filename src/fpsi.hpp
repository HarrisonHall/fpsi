// fpsi.hpp

#pragma once

#include "json.hpp"

using json = nlohmann::json;

#include "util/logging.hpp"
#include "session/session.hpp"



namespace fpsi {

const std::string version = "0.2";
const std::string author = "Harrison Hall";
const std::string url = "https://github.com/HarrisonHall/fpsi";
const std::string license = "MIT";

class Session;
extern Session *session;  // Global session pointer

}

