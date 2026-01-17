#!/usr/bin/env python3
"""
Asyncio TCP stress client.
Usage:
  python3 stress_client.py --host 127.0.0.1 --port 8888 --connections 1000 --size 5000

What it does:
- opens N concurrent TCP connections
- sends a payload of given size on each connection
- waits for the server to echo back (reads until same length or EOF)
- records per-connection latency and overall throughput

Notes:
- raise `ulimit -n` before running if connections are large: e.g. `ulimit -n 20000`
"""
import argparse
import asyncio
import time
import statistics

async def worker(idx, host, port, payload, results, sem):
    start = time.perf_counter()
    try:
        reader, writer = await asyncio.open_connection(host, port)
        writer.write(payload)
        await writer.drain()
        total = 0
        expected = len(payload)
        while total < expected:
            data = await reader.read(min(4096, expected - total))
            if not data:
                # server closed early
                break
            total += len(data)
        writer.close()
        try:
            await writer.wait_closed()
        except Exception:
            pass
        end = time.perf_counter()
        results.append((idx, end - start, total))
    except Exception as e:
        end = time.perf_counter()
        results.append((idx, None, 0, str(e)))
    finally:
        if sem:
            sem.release()

async def run_all(host, port, connections, size, concurrency):
    payload = b'A' * size
    results = []
    sem = asyncio.Semaphore(concurrency) if concurrency and concurrency < connections else None
    tasks = []
    start_all = time.perf_counter()
    for i in range(connections):
        if sem:
            await sem.acquire()
            t = asyncio.create_task(worker(i, host, port, payload, results, sem))
        else:
            t = asyncio.create_task(worker(i, host, port, payload, results, None))
        tasks.append(t)
    await asyncio.gather(*tasks)
    total_time = time.perf_counter() - start_all
    return results, total_time

def print_stats(results, total_time, size):
    successes = [r for r in results if len(r) >= 3 and r[1] is not None]
    failures = [r for r in results if not (len(r) >=3 and r[1] is not None)]
    latencies = [r[1] for r in successes]
    received = sum(r[2] for r in successes)
    n = len(results)
    succ = len(successes)
    fail = len(failures)
    print(f"Connections requested: {n}")
    print(f"Succeeded: {succ}, Failed: {fail}")
    print(f"Total time: {total_time:.3f}s")
    print(f"Total bytes received: {received}")
    if latencies:
        print(f"Avg latency: {statistics.mean(latencies):.4f}s")
        print(f"Median latency: {statistics.median(latencies):.4f}s")
        print(f"Min latency: {min(latencies):.4f}s")
        print(f"Max latency: {max(latencies):.4f}s")
        print(f"P95 latency: {statistics.quantiles(latencies, n=20)[18]:.4f}s")
    if total_time > 0:
        print(f"Throughput (recv): {received/total_time:.2f} bytes/s  ({(received/total_time)/1024:.2f} KB/s)")

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--host', default='127.0.0.1')
    p.add_argument('--port', type=int, default=8888)
    p.add_argument('--connections', type=int, default=1000)
    p.add_argument('--size', type=int, default=4096)
    p.add_argument('--concurrency', type=int, default=200,
                   help='max concurrent connect/send tasks; helps avoid resource spikes')
    args = p.parse_args()
    print(f"Host={args.host} Port={args.port} connections={args.connections} size={args.size} concurrency={args.concurrency}")
    try:
        results, total_time = asyncio.run(run_all(args.host, args.port, args.connections, args.size, args.concurrency))
        print_stats(results, total_time, args.size)
    except KeyboardInterrupt:
        print('\nCancelled')

if __name__ == '__main__':
    main()
