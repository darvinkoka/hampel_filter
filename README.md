# Embedded Hampel Filter in C

A lightweight, dependency-free **streaming Hampel filter** (outlier rejection) written in plain C, designed for microcontrollers such as **STM32** where Python/NumPy is not an option and every byte and cycle counts.

Originally developed for an ultrasonic distance/level sensor (PoliMi Sailing Team) to remove spikes from noisy readings in real time.

## Why

The textbook Hampel filter re-sorts the whole window at every sample. Here the window is kept in **three intrusive doubly linked lists** that are updated incrementally:

| List | Order | Purpose |
|------|-------|---------|
| FIFO | arrival time | evict the oldest sample |
| Sorted | value | median in a few pointer steps |
| DevSorted | \|value − median\| | MAD |

Mean and variance are also updated **incrementally** (no pass over the window).

## Features

- Pure C99, only `<stdlib.h>`, `<math.h>`, `<stdio.h>`
- Fixed memory footprint: `SIZE` nodes × (1 `double` + 6 pointers) → ~32 B/node on 32-bit ARM (≈ 290 B for the default window of 9)
- Median pointer moves by at most one step per sample (no re-sorting)
- Running mean and variance in O(1) per sample
- Robust scale estimate: `MAD · K`, with `K = 1.4826` (consistent with σ for Gaussian data)
- No Python, no external libraries, no dynamic structures beyond the window nodes

## How it works

For each new sample `x`, once the window is full (`SIZE` samples):

1. The oldest node leaves the FIFO and the sorted list; the median pointer is shifted accordingly.
2. The DevSorted list is rebuilt around the new median and the MAD is read at its middle element.
3. The **center** sample of the window is flagged as an outlier if

   `|x_c − median| > K · NDEV · MAD`

   and, if so, replaced by the median.

The output is therefore delayed by `(SIZE − 1) / 2` samples.

## Parameters

Defined at the top of `filters.c`:

```c
int    SIZE = 9;       // window length (odd)
double NDEV = 2.0;     // threshold in scaled MADs
double K    = 1.4826;  // MAD -> sigma consistency constant
```

## Usage

```c
#include "filters.h"

Queue buffer;
initQueue(&buffer);

for (;;) {
    double x = read_sensor();
    enqueue(&buffer, x);

    if (buffer.count >= SIZE) {
        double clean = hampel_filter(&buffer);
        // buffer.med->value : median
        // buffer.mean, buffer.variance : window statistics
    }
}
```

## API

| Function | Description |
|----------|-------------|
| `initQueue(Queue*)` | Initialize an empty window |
| `enqueue(Queue*, double)` | Push a sample, evict the oldest when full |
| `hampel_filter(Queue*)` | Return the filtered center sample (in-place correction) |
| `update_params`, `update_med`, `update_MAD` | Internal incremental updates |

## Complexity (window size `W`)

| Operation | Cost |
|-----------|------|
| Median update | O(1) pointer move (+ O(W) sorted insertion) |
| Mean / variance | O(1) |
| Memory | O(W) |


## Build

```bash
gcc -O2 -c filters.c -lm
```

On STM32 simply add `filters.c` / `filters.h` to your project (link with `-lm`).

## Roadmap

- [ ] Static node pool (no `malloc`/`free` at runtime)
- [ ] Compile-time `SIZE`
- [ ] Incremental MAD update
- [ ] `float` variant for FPU-less targets
- [ ] Unit tests and benchmarks

## License

MIT
