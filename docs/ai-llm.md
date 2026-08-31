# shared/AI: LLM client

`shared/AI/LLMClient.h/.cpp` is a minimal client for OpenAI-compatible chat
completion servers (llama.cpp, vLLM, LM Studio, local gateways). First user:
RTGameBot (an LLM plays Infocom games). Header comment has a usage example.

## Classes

- `LLMMessage`: role ("user"/"assistant") + content.
- `LLMConversation`: system prompt + message history. `TrimToLastExchanges(n)`
  bounds the history (system prompt always kept, window starts on a user
  message). `BuildChatCompletionJSON(model, temperature, maxTokens)` builds
  the request body with cJSON.
- `LLMClient`: owns one `NetHTTP`, runs ONE request at a time.
  `Setup(server, port, "v1/chat/completions", modelId)`, then
  `SendAsync(convo)` (snapshots the JSON; false if busy), and poll `Update()`
  every frame. `SetParms(temperature, maxTokens, maxRetries)`,
  `SetTimeoutMS(ms)` (per-attempt, default 15s), `Abort()`.
  `SetExtraBodyJSON(json)` merges arbitrary fields into every request body,
  e.g. `{"chat_template_kwargs":{"enable_thinking":false}}` for Qwen on
  vLLM. That one matters: with thinking left on, the model can burn the
  whole max_tokens budget in the `reasoning` field and return content:null
  (surfaced as "model produced only reasoning" through m_sig_error).

## Logging and stats (for debug overlays)

- `SetLogFile(path, bAppend=false)` mirrors every request body, response body
  and error to a timestamped text file (truncated at Setup time unless
  bAppend). This is the exact bytes on the wire, which is what you want when
  a model starts misbehaving; `GetLogFile()` returns the path so an app can
  offer an "open the log" button.
- After each completed reply: `GetLastPromptTokens()` /
  `GetLastCompletionTokens()` (from the server's `usage` object, 0 if it
  didn't send one), `GetLastReplyMS()`, `GetLastTPS()` (completion tokens per
  second) and `GetLastRequestBytes()`. While a request is in flight,
  `GetInFlightMS()` gives its age (0 when idle).
- Timing uses `GetTick()`, so under the engine's deterministic screenshot mode
  (locked timestep) these durations count frames, not wall clock.

## Completion contract (boost::signals2, fired from Update)

- `m_sig_response`: `Get(0)` = assistant reply text. The client does NOT add
  it to your conversation; call `AddAssistantMessage` yourself if you want
  multi-turn memory.
- `m_sig_error`: `Get(0)` = error string, `Get(1)` = uint32 retries left.
  Fires once per failed attempt (transport error, timeout, unparsable body,
  or a server-side error object). retriesLeft == 0 means the client gave up
  and is idle. Failed attempts resend the same snapshot with linear backoff.
  On parse/server errors the raw body (first 400 chars) is LogMsg'd.

## What a project must compile (engine is source-only)

`shared/AI/LLMClient.cpp`, `shared/Network/NetHTTP.cpp`,
`shared/Network/NetSocket.cpp`, `shared/Network/NetUtils.cpp`,
`shared/util/cJSON.c` (PCH off for the .c), and link `Ws2_32.lib` on Windows.

## Limits / notes

- The default socket NetHTTP backend is plain HTTP (no TLS) and HTTP/1.0:
  fine for LAN servers (vLLM/uvicorn reply Content-Length + close). For
  HTTPS an app would need the `RT_USE_LIBCURL` backend, which currently
  can't send custom headers (`SetPostHeaderOverride` isn't implemented
  there), so it would need a small patch first.
- Non-streaming: the whole reply arrives at once. Streaming (SSE) would need
  a data callback hook in the NetHTTP backends.
- Reply content is returned verbatim; reasoning-model artifacts like
  `<think>` blocks are the caller's problem (RTGameBot strips them, see its
  GameDirector::ParseAIResponse).
- `NetHTTP::SetIdleTimeoutMS(ms)` was added for this (public, default 25s).
- Windows NetSocket fix that this depends on: sockets are now made
  non-blocking with `ioctlsocket(FIONBIO)`; the old
  `WSAAsyncSelect(GetForegroundWindow(), ...)` silently left the socket
  BLOCKING whenever the app window wasn't foreground, freezing the main
  thread in recv().
