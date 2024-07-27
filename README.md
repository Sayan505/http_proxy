# A Multi-Threaded HTTP Proxy Server with Cache

## Features:
- Multi-threaded (synchronized using semaphore and mutex)
- HTTP GET request support
- LRU Cache with time-to-live using a Doubly Linked-List
- Traffic caching provides a massive ~96% improvement in response times from avg. ~951ms to <40ms.

## Deployment:
- compile: `make release`
- run: `./build/release/proxy_server`&nbsp;&nbsp;&nbsp;&nbsp;`# -nocache: disable cache`
- config. browser proxy setting to `HTTP://127.0.0.1:5555` on the client side.

