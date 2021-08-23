# Multithreaded-LUDO
co-creator: github.com/1shoaibazhar
It is a multi-threaded based LUDO game created by using the concept of detachable threads in Operating Systems.
Threads are running in parallel in
multithreading; they can be detached or joined with the parent thread depending on
relieving resources. This concept is used to run main thread, master controlling
threads, players and a few other threads in order to implement the game. We used
detached threads of players and joinable master thread in game implementation. The
threads have asynchronous or deferred cancellation. Deferred allows thread to be
cancelled at a safe point and asynchronous cancellation allows the thread to be
cancelled immediately.
