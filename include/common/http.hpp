#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <websocket_context.hpp>

namespace http {

//  Accumulates a full HTTP request body from uWS's chunked onData delivery,
//  then invokes `callback` with the complete body string.
//
//  Why this is non-trivial in uWS:
//    uWS never blocks. Bodies arrive in one or more string_view chunks via
//    the onData callback. The bool `isLast` signals the final chunk. Callers
//    must therefore buffer every chunk themselves and act only when isLast
//    is true — which is exactly what this helper does.
//
//  The shared_ptr<bool> is_alive guard handles connection abortion:
//    If the client disconnects mid-transfer, uWS fires onAborted and the
//    `res` pointer becomes dangling. The guard flips to false inside
//    onAborted so that any subsequent onData invocation (which can race on
//    some platforms) detects the abort and returns immediately without
//    touching `res`.
//
//  @param res        The response object for the current HTTP request.
//                    Must remain valid until `callback` is invoked or the
//                    connection is aborted — uWS guarantees this.
//  @param max_bytes  Hard cap on body size. Responds 413 and stops if
//                    exceeded. Callers should pass their domain-specific
//                    limit rather than relying on a global default.
//  @param callback   Invoked exactly once with the complete body, or never
//                    if the connection is aborted or the size cap is hit.
inline void ReadBody(AppResponse* res, size_t max_bytes, std::function<void(const std::string&)> callback) {
    auto is_alive = std::make_shared<bool>(true);
    auto buffer   = std::make_shared<std::string>();

    res->onAborted([is_alive] { *is_alive = false; });

    res->onData([is_alive, buffer, max_bytes, callback = std::move(callback), res](std::string_view chunk, bool isLast) mutable {
        if (!*is_alive) return;

        buffer->append(chunk.data(), chunk.size());

        if (buffer->size() > max_bytes) {
            res->writeStatus("413 Payload Too Large")->end();
            return;
        }

        if (isLast) callback(*buffer);
    });
}

}
