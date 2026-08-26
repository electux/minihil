#include "protocol/jsonrpc_router.hpp"
#include <iostream>

namespace minihil {

JsonRpcRouter::JsonRpcRouter() = default;
JsonRpcRouter::~JsonRpcRouter() = default;

void JsonRpcRouter::registerMethod(const std::string& name, MethodHandler handler) {
    m_methods[name] = handler;
    std::cout << "[JsonRpcRouter] Registered method: " << name << std::endl;
}

std::string JsonRpcRouter::processRequest(const std::string& rawRequest) {
    nlohmann::json rootRequest;
    
    // Parse JSON
    try {
        rootRequest = nlohmann::json::parse(rawRequest);
    } catch (const nlohmann::json::parse_error& e) {
        return makeErrorResponse(nullptr, -32700, std::string("Parse error: ") + e.what()).dump() + "\n";
    }

    nlohmann::json rootResponse;

    // Handle batch requests
    if (rootRequest.is_array()) {
        if (rootRequest.empty()) {
            return makeErrorResponse(nullptr, -32600, "Invalid Request: Empty batch").dump() + "\n";
        }
        
        nlohmann::json batchResponse = nlohmann::json::array();
        for (const auto& singleReq : rootRequest) {
            nlohmann::json singleRes = processSingleRequest(singleReq);
            if (!singleRes.is_null()) {
                batchResponse.push_back(singleRes);
            }
        }
        
        if (batchResponse.empty()) {
            return ""; // Notifications only
        }
        rootResponse = batchResponse;
    } else {
        rootResponse = processSingleRequest(rootRequest);
        if (rootResponse.is_null()) {
            return ""; // Notification
        }
    }

    return rootResponse.dump() + "\n";
}

nlohmann::json JsonRpcRouter::processSingleRequest(const nlohmann::json& reqJson) {
    nlohmann::json id = nullptr;
    
    if (reqJson.contains("id")) {
        id = reqJson["id"];
    }

    // Validate request structure
    if (!reqJson.is_object() || !reqJson.contains("jsonrpc") || reqJson["jsonrpc"] != "2.0" || !reqJson.contains("method")) {
        return makeErrorResponse(id, -32600, "Invalid Request: Missing required JSON-RPC 2.0 fields");
    }

    std::string method = reqJson["method"];
    nlohmann::json params = reqJson.contains("params") ? reqJson["params"] : nlohmann::json::object();

    try {
        auto it = m_methods.find(method);
        if (it != m_methods.end()) {
            nlohmann::json result = it->second(params, id);
            
            // If handler returned an error object structured like {"error": ...}, wrap it properly
            if (result.is_object() && result.contains("error") && result.contains("code")) {
                return makeErrorResponse(id, result["code"].get<int>(), result["error"].get<std::string>());
            }
            
            // Otherwise it's a success response
            return makeSuccessResponse(id, result);
        } else {
            return makeErrorResponse(id, -32601, "Method not found: " + method);
        }
    } catch (const std::exception& e) {
        return makeErrorResponse(id, -32603, std::string("Internal error: Exception occurred in handler: ") + e.what());
    }
}

nlohmann::json JsonRpcRouter::makeErrorResponse(const nlohmann::json& id, int code, const std::string& message) const {
    nlohmann::json res;
    res["jsonrpc"] = "2.0";
    res["error"] = {{"code", code}, {"message", message}};
    res["id"] = id;
    return res;
}

nlohmann::json JsonRpcRouter::makeSuccessResponse(const nlohmann::json& id, const nlohmann::json& result) const {
    nlohmann::json res;
    res["jsonrpc"] = "2.0";
    res["result"] = result;
    res["id"] = id;
    return res;
}

} // namespace minihil
