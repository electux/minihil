#pragma once

namespace minihil {

class IServer {
public:
    virtual ~IServer() = default;

    // Starts the server listener (blocking or non-blocking depending on implementation)
    virtual bool start() = 0;

    // Gracefully stops the server listener and cleans up resources
    virtual void stop() = 0;
};

} // namespace minihil
