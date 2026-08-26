#pragma once

#include "core/irpc_handler.hpp"
#include <map>
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

namespace minihil {

class JsonRpcRouter : public IRpcHandler {
public:
    // Callback handler signature: returns result JSON or error structure
    using MethodHandler = std::function<nlohmann::json(const nlohmann::json& params, const nlohmann::json& id)>;

    JsonRpcRouter();
    ~JsonRpcRouter() override;

    // Process incoming raw JSON string and route to registered methods (implementation of IRpcHandler)
    std::string processRequest(const std::string& rawRequest) override;

    // Registers a callback method handler (Open/Closed Principle)
    void registerMethod(const std::string& name, MethodHandler handler);

private:
    std::map<std::string, MethodHandler> m_methods;

    nlohmann::json processSingleRequest(const nlohmann::json& reqJson);
    nlohmann::json makeErrorResponse(const nlohmann::json& id, int code, const std::string& message) const;
    nlohmann::json makeSuccessResponse(const nlohmann::json& id, const nlohmann::json& result) const;
};

} // namespace minihil
