import socket
import time
import threading

HOST = "127.0.0.1"
PORT = 5000
NUM_REQUESTS = 1000
THREADS = 10

def send_command(cmd):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    s.sendall((cmd + "\n").encode())
    data = s.recv(4096)
    s.close()
    return data.decode().strip()

def worker(thread_id, results):
    for i in range(NUM_REQUESTS // THREADS):
        key = f"key{thread_id}_{i}"
        val = f"value{i}"
        send_command(f"PUT {key} {val}")
        resp = send_command(f"GET {key}")
        results.append(resp)

def main():
    threads = []
    results = []
    start = time.time()

    for t in range(THREADS):
        thr = threading.Thread(target=worker, args=(t, results))
        threads.append(thr)
        thr.start()

    for thr in threads:
        thr.join()

    end = time.time()
    total_time = end - start
    print(f"Sent {NUM_REQUESTS} requests in {total_time:.2f}s")
    print(f"Throughput: {NUM_REQUESTS/total_time:.2f} req/s")

if __name__ == "__main__":
    main()
