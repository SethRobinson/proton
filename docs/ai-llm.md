# shared/AI: LLM client

`shared/AI/LLMClient.h/.cpp` is a minimal client for OpenAI-compatible chat
completion servers (llama.cpp, vLLM, LM Studio, local gateways). First user:
RTGameBot (an LLM plays Infocom games). Header comment has a usage example.

## Classes

- `LLMMessage`: role ("user"/"assistant") + content.
- `LLMConversation`: system prompt + message history. `TrimToLastExchanges(n)`
  bounds the history (system prompt always kept, window starts on a user
  message), `RemoveLastMessage()` drops a user turn whose request got
  aborted. `BuildChatCompletionJSON(model, temperature, maxTokens, bStream)`
  builds the request body with cJSON (`bStream` adds `"stream":true` and
  `stream_options.include_usage`).
- `LLMStreamParser`: the incremental SSE parser streaming uses (pure, no
  network; see Streaming below). Feed it the body so far, drain the deltas.
- `LLMClient`: owns one `NetHTTP`, runs ONE request at a time.
  `Setup(server, port, "v1/chat/completions", modelId)`, then
  `SendAsync(convo)` (snapshots the JSON; false if busy), and poll `Update()`
  every frame. `SetParms(temperature, maxTokens, maxRetries)`,
  `SetTimeoutMS(ms)` (an idle cap per attempt, default 15s; a stream resets
  it with every fragment), `SetStreaming(bool)`, `Abort()`.
  `SetExtraBodyJSON(json)` merges arbitrary fields into every request body,
  e.g. `{"chat_template_kwargs":{"enable_thinking":false}}` for Qwen on
  vLLM. That one matters: with thinking left on, the model can burn the
  whole max_tokens budget in the `reasoning` field and return content:null
  (surfaced as "model produced only reasoning" through m_sig_error). With
  thinking on, raise max_tokens (the reasoning shares the budget) and turn
  streaming on.

## Logging and stats (for debug overlays)

- `SetLogFile(path, bAppend=false)` mirrors every request body, response body
  and error to a timestamped text file (truncated at Setup time unless
  bAppend). This is the exact bytes on the wire, which is what you want when
  a model starts misbehaving; `GetLogFile()` returns the path so an app can
  offer an "open the log" button.
- After each completed reply: `GetLastPromptTokens()` /
  `GetLastCompletionTokens()` (from the server's `usage` object, 0 if it
  didn't send one), `GetLastReplyMS()`, `GetLastTPS()` (completion tokens per
  second), `GetLastRequestBytes()`, `GetLastFinishReason()` ("stop",
  "length"...), `GetLastReasoning()` (the reasoning text a thinking model
  sent, from `message.reasoning` / `reasoning_content`, or a leading
  `<think>` block in the content; "" if none) and, for streamed replies,
  `GetLastFirstTokenMS()`. While a request is in flight, `GetInFlightMS()`
  gives its age (0 when idle).
- Timing uses `GetTick()`, so under the engine's deterministic screenshot mode
  (locked timestep) these durations count frames, not wall clock.

## Completion contract (boost::signals2, fired from Update)

- `m_sig_response`: `Get(0)` = assistant reply text. The client does NOT add
  it to your conversation; call `AddAssistantMessage` yourself if you want
  multi-turn memory. Fires for streamed replies too, with the whole content,
  once the stream ends.
- `m_sig_error`: `Get(0)` = error string, `Get(1)` = uint32 retries left.
  Fires once per failed attempt (transport error, timeout, unparsable body,
  a server-side error object, or a stream that ended without content).
  retriesLeft == 0 means the client gave up and is idle. Failed attempts
  resend the same snapshot with linear backoff. On parse/server errors the
  raw body (first 400 chars) is LogMsg'd.
- `m_sig_delta` (streaming only): `Get(0)` = a text fragment, `Get(1)` =
  uint32 `eLLMDeltaKind` (`LLM_DELTA_CONTENT` or `LLM_DELTA_REASONING`), in
  arrival order. A handler may `Abort()`. Apps that only want to show the
  live text can skip the signal and poll `GetStreamedReasoning()` /
  `GetStreamedContent()` each frame instead (RTGameBot does).

## Streaming (SSE), opt-in since Sep 2026

`SetStreaming(true)` makes the next `SendAsync` ask for a stream
(`"stream":true`, `stream_options.include_usage` so the usage stats still
arrive as the final chunk). The client puts its `NetHTTP` into stream mode
and, every frame, feeds the body received so far to an `LLMStreamParser`:
complete `data:` lines only (a partial line waits), `[DONE]` ends the reply
without waiting for the server to close, `choices[0].delta.content` is the
reply and `delta.reasoning` (newer vLLM) or `delta.reasoning_content`
(older vLLM, DeepSeek) the thinking. A server without a reasoning parser
sends the thinking inline as `<think>...</think>` at the start of the
content; the parser routes a leading block to reasoning even when the tags
are split across deltas (it holds back only the bytes that could still be a
tag). A body that turns out not to be SSE (an error object, HTTP 4xx/5xx, a
server that ignored `stream:true`) goes through the normal whole-reply
parse, so nothing changes for the caller. A stream that ends with reasoning
but no content (finish_reason "length": the budget ran out) fails with the
same "only reasoning" message as the non-streamed case. Each retry attempt
starts a fresh stream. The RESPONSE log block holds the reassembled
`[reasoning]` / `[content]` text (chunk count, first-token time), not the
raw SSE.

`NetHTTP::SetStreamMode(true)` (socket backend): the header is located every
frame (no 333 ms check delay), the socket buffer is drained into
`GetStreamBody()` as it arrives (`Transfer-Encoding: chunked` is decoded,
though an HTTP/1.0 request like ours normally gets a plain body), the
`"\n\n"` end-of-body heuristic of `END_OF_DATA_SIGNAL_HTTP` is off (it would
cut an SSE stream at its first event), and the reply ends on Content-Length,
the chunked terminator or the server closing the connection. `Reset()`
clears the flag, so set it AFTER `Reset()`/`Setup()` and BEFORE `Start()`
(redirects re-apply it). Not for use with `SetFileOutput`. The html5 and
libcurl backends ignore it: the body still arrives whole at
`STATE_FINISHED`, and `LLMClient` copes (no live deltas, the same
`m_sig_response` at the end). The idle timeout is per gap between
fragments, so a long reasoning phase no longer needs a long timeout, only
the wait for the first token does. RTGameBot's `-llmtest` exercises the
parser on canned bodies in 1-byte, 7-byte and whole pieces.

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
- Streaming is opt-in (see above); the default is still the whole reply at
  once, on the code path every existing caller uses.
- Non-streamed reply content is returned verbatim; a leading `<think>` block
  is copied into `GetLastReasoning()` but NOT stripped from the content
  (RTGameBot strips it, see its GameDirector::ParseAIResponse). Streamed
  content never carries the tags: the parser routes the block to the
  reasoning side.
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
