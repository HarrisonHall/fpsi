// fpsi.hpp

#pragma once

#include "json.hpp"

using json = nlohmann::json;

#include "util/logging.hpp"
#include "session/session.hpp"



namespace fpsi {

class Session;
extern Session *session;  // Global session pointer

}

