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
- `NetHTTP::GetResultCode()` (Aug 2026) returns the reply's HTTP status (0
  until the header arrived); only 404 is turned into a NetHTTP error, so a
  500 with a JSON body looks like a successful download otherwise.
- The reply-header lookup in `NetHTTP::ScanDownloadedHeader` is
  case-insensitive since Aug 2026 (`GetHeaderValue`). Before that,
  `Content-Length` was matched exactly and uvicorn's lowercase
  `content-length` was never seen, so the `END_OF_DATA_SIGNAL_HTTP` path
  ended a body at the first `"\n\n"` in it: harmless for JSON, fatal for
  binary audio (found by TTSClient). `Location` redirects had the same bug.
- Windows NetSocket fix that this depends on: sockets are now made
  non-blocking with `ioctlsocket(FIONBIO)`; the old
  `WSAAsyncSelect(GetForegroundWindow(), ...)` silently left the socket
  BLOCKING whenever the app window wasn't foreground, freezing the main
  thread in recv().

# shared/AI: TTS client

`shared/AI/TTSClient.h/.cpp` speaks text through an HTTP text-to-speech
server: it form-POSTs the text plus any fields you set and writes the audio
reply to a file, ready for `AudioManager::Play`. It is a request pool (see
Queueing below). First user: RTGameBot's
narrator/player voices (its `docs/speech.md`). Built for hal's `POST /tts`
(fields `text`, `voice`, `scene`; reply = a WAV) but any endpoint that takes
an `application/x-www-form-urlencoded` POST and answers with the audio bytes
plus a `Content-Length` works. Header comment has a usage example. Compiles
with the same files as LLMClient minus cJSON.

## API

- `Setup(server, port, "tts")`; `SetField(name, value)` for the client-wide
  form fields (`""` removes one); `SetTextFieldName` (default `text`);
  `SetTimeoutMS` (idle cap per attempt, default 90 s: generation is silent
  until the audio is done); `SetMaxRetries` (default 1, transport failures);
  `SetMaxParallel(n)` (requests on the wire at once, default 1).
- `Speak(text, outFile, priority = 0)` and `Speak(text, outFile, fields,
  priority)` (per-request fields override the client-wide ones, so one pool
  serves several voices) return a request id (0 = not set up / empty text).
  `Cancel(id)` / `CancelAll()`; `IsPending(id)` / `IsInFlight(id)`;
  `Update()` every frame; `IsBusy()`, `GetInFlightCount()`,
  `GetQueuedCount()`.
- `m_sig_ready`: `Get(0)` = the file path, `Get(1)` = request id (uint32),
  `Get(2)` = the text, `Get(3)` = generation ms, `Get(4)` = audio ms (from
  the WAV header via the public static `GetWavDurationMS`, 0 if unreadable).
  `m_sig_error`: `Get(0)` = error string, `Get(1)` = request id. Both fire
  from `Update()`, after the freed slot has been handed to the next queued
  line. A cancelled request fires nothing.
- Stats of the most recently finished line: `GetLastReplyMS()`,
  `GetLastAudioBytes()`, `GetLastAudioMS()`.

## Queueing: a pool

`Speak()` queues; up to `SetMaxParallel` requests are on the wire, the rest
wait in a priority queue (higher priority first, FIFO within one). `Cancel`
drops a queued line or aborts an in-flight one (the socket is reset and the
slot freed; the server still finishes generating, nothing can stop that).
hal serializes generation: three concurrent short lines all came back
together after 3x the time of one, so more than 1 in parallel gains nothing
there and makes the first reply arrive later (it only comes back with the
batch). To keep a long text flowing, split it into short chunks and keep a
couple queued so the first plays while the rest generate; RTGameBot's
`SpeechDirector` does exactly that (its `docs/speech.md`). A caller that
wants "only the newest line" does `CancelAll()` before `Speak()`.

## Reply validation

`GetResultCode() >= 400` is an error carrying the start of the body (hal
answers an unknown voice with a 500 and a JSON `detail`). Otherwise
`TTSClient::ValidateAudio` (public static, so tests can feed it buffers)
requires >= 64 bytes and a known signature (`RIFF`, `OggS`, `fLaC`, `ID3`,
an MP3 frame) and, for RIFF, that the body is at least as long as the size
field claims (0 / 0xFFFFFFFF, what streaming encoders write, is accepted),
so a truncated download is rejected instead of played.

## Gotchas

- The `AudioManager` backends cache sound objects by file name and would
  replay the OLD audio for a rewritten file: use a fresh name per line, or
  call `AudioManager::DeleteSoundObjectByFileName` (base-class virtual, Aug
  2026; every backend already had the method) before reusing one. Stop the
  sound first.
- A server that is down but whose host resolves costs the full idle timeout
  per attempt: `NetSocket` reads a refused connection as "not ready", not
  as a disconnect. Probe the server first if that matters (RTGameBot does a
  `GET /voices` with a 5 s idle timeout and switches the voices off on
  failure).
- The whole reply is held in memory before the file is written (a paragraph
  of 24 kHz speech is around a megabyte); `NetHTTP`'s 333 ms end-of-data poll
  adds up to that much latency per line.
