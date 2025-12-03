# Concurrency

## Async First (SHOULD)
- Prefer async, non-blocking I/O
- Maximize single-thread performance first
- Add parallelism only after async optimization
- Threads are last resort

## Synchronization Priority (SHOULD)
1. Lock-free data structures
2. RCU (Read-Copy-Update)
3. Atomics
4. Traditional locks (avoid if possible)

## Guidelines (SHOULD)
- Minimize shared mutable state
- Avoid lock contention in critical paths
- Profile before optimizing

**Rationale:**
Single-threaded async maximizes throughput,
lock-free minimizes contention.
