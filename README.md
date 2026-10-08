# UDP Client / Server in C

[![CI](https://github.com/ia-maiga/c-udp-client-server/actions/workflows/ci.yml/badge.svg)](https://github.com/ia-maiga/c-udp-client-server/actions/workflows/ci.yml)

> A minimal client/server pair that talks over **UDP** with the **BSD sockets API**, in pure C.
> The client sends lines typed by the user; the server prints each datagram with the address of
> its sender.

Lab work from the *Networks* course (Université Paris-Saclay), cleaned up and extended.

---

## ✨ Demo

Terminal 1, the server:

```console
$ ./bin/udp_server
UDP server listening on port 9600 (Ctrl+C to stop)
[127.0.0.1:43433] hello
[127.0.0.1:43433] UDP is connectionless
```

Terminal 2, the client:

```console
$ ./bin/udp_client localhost
Connected to localhost:9600. Type messages, Ctrl+D to quit.
> hello
> UDP is connectionless
> ^D
```

The client can also read from a pipe:

```bash
echo "sensor=42" | ./bin/udp_client localhost 9600
```

---

## 🧠 How it works

```
        Server                          Client
        ------                          ------
        socket(SOCK_DGRAM)              socket(SOCK_DGRAM)
        bind(port 9600)                 gethostbyname("localhost")
                                        fgets(line)
        recvfrom()   <--- datagram ---  sendto(server address)
        print "[ip:port] message"
        (loop)                          (loop until end of input)
```

- **UDP is connectionless**: there is no `listen()`, `accept()` or `connect()`. Each `sendto()`
  carries the destination address, and each `recvfrom()` returns the sender's address.
- **Each line is one datagram**: message boundaries are kept, unlike with TCP, which is a byte
  stream.
- **No delivery guarantee**: UDP does not retransmit, reorder or de-duplicate packets. That is
  why it suits real-time data (sensor readings, telemetry, video) where a late packet is useless
  anyway.
- **Byte order**: ports and addresses are converted with `htons` / `htonl` (host → network) and
  back with `ntohs` / `inet_ntop`.

---

## 🚀 Build & run

Requirements: Linux or macOS, `gcc` (or `clang`) and `make`.

```bash
git clone https://github.com/ia-maiga/c-udp-client-server.git
cd c-udp-client-server

make                               # builds bin/udp_server and bin/udp_client
make test                          # end-to-end test: starts the server, sends messages, checks output

./bin/udp_server [port]            # default port: 9600
./bin/udp_client <host> [port]     # e.g. localhost, or the IP of another machine
```

To try it between two computers on the same network, run the server on one machine and use its
IP address as `<host>` on the other.

---

## ✅ Robustness

- Port given on the command line and validated (1-65535).
- Every system call is checked (`socket`, `bind`, `recvfrom`, `sendto`, `gethostbyname`).
- The server null-terminates what it receives, so it never prints past the buffer.
- `Ctrl+C` stops the server cleanly (`SIGINT` handler, socket closed).
- Compiles with `-Wall -Wextra -pedantic -Werror`, checked by GitHub Actions on every push.

---

## 📂 Structure

```
├── src/udp_server.c    UDP server
├── src/udp_client.c    UDP client
├── tests/run_test.sh   end-to-end test
└── Makefile
```

---

## 👤 Author

**Ibrahim Aboubakarine Maiga**, Double Bachelor's in Mathematics & Computer Science,
Université Paris-Saclay.

The lab also had a TCP part, written by my lab partner Walid Bouzid; it is not included here.
