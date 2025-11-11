# ClusterStore

ClusterStore is a simple distributed key-value store written in C++.  
It lets you run many servers that share data between them. You can save, get, 
and delete key-value pairs from any server, and the update will be copied to 
other servers so data is not lost if one stops.

## Features
- Thread-safe in-memory store
- Commands: `PUT key value`, `GET key`, `DELETE key`
- Hashing to decide which server owns a key
- Replication so all servers share updates
- Client program to talk to any server

## Build

```bash
make
```

This makes two programs in `bin/`:
- `node` — starts a server
- `client` — simple client to send commands

## Run Example

Open 3 terminals:

```bash
./bin/node --host 127.0.0.1 --port 5000 --peers 127.0.0.1:5001,127.0.0.1:5002
./bin/node --host 127.0.0.1 --port 5001 --peers 127.0.0.1:5000,127.0.0.1:5002
./bin/node --host 127.0.0.1 --port 5002 --peers 127.0.0.1:5000,127.0.0.1:5001
```

Now connect with the client:
```bash
./bin/client --host 127.0.0.1 --port 5000
> PUT name Kirtan
OK
> GET name
Kirtan
> DELETE name
OK
```

## Simple Design
- Each server has its own key-value map
- Hashing decides which server stores each key
- Servers send copies of new keys to each other
- Client connects with plain text commands


## Benchmark
Run the Python benchmark to send many requests at once:


python3 benchmark.py
```
