#pragma once
#include <string>

namespace minihil {

class IRpcHandler {
public:
    virtual ~IRpcHandler() = default;

    // Receives a raw input request payload and returns a raw response payload
    virtual std::string processRequest(const std::string& rawRequest) = 0;
};

} // namespace minihil
