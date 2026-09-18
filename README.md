# TinyEngine

This is a personal project where I learn how to set up a small inference server running a tiny model locally on mac/metal.

## Goals

1. Build a small concurrent server from first principles.
2. Understand principles of cache management and request batching.
3. Build profiling harness to measure and optimize throughput and latency.
4. Tinker with metal kernels.

## First Protocol Experiment

`tinyengine_client` sends an OpenAI-compatible chat-completion request to a
local `llama-server`. The JSON body is deliberately small:

```json
{
  "model": "local-model",
  "messages": [{"role": "user", "content": "Say hello"}],
  "max_tokens": 64,
  "temperature": 0.7,
  "stream": false
}
```

Start the model server in one terminal:

```sh
llama serve -hf ggml-org/Qwen3.5-0.8B-GGUF --port 8080
```

Then build and run the client in another:

```sh
cmake -S . -B build
cmake --build build
./build/tinyengine_client --prompt "Explain KV caching in one sentence."
```

By default, the client sends a POST request to
`http://127.0.0.1:8080/v1/chat/completions`. Use the optional flags when the
server is running somewhere else or you want different sampling settings:

```sh
./build/tinyengine_client \
  --host 127.0.0.1 \
  --port 8080 \
  --model local-model \
  --max-tokens 128 \
  --temperature 0.4 \
  --prompt "Write a short haiku about local inference."
```

The client prints the raw HTTP response so the wire protocol stays visible.
