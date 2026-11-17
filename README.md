# Hampel Filter in C for STM32

I wrote this Hampel filter for the ultrasonic sensor of the **PoliMi Sailing Team**, to remove spikes from noisy height readings of a moth.

It runs on an **STM32** microcontroller, so Python was never an option: memory is tight and the filtering has to happen in real time. That's why I hardcoded everything in pure C and designed it around efficiency, both in computation and in memory.

Instead of re-sorting the whole window at every sample, I keep it in linked lists and update the median, mean and variance incrementally. No dependencies, fixed memory footprint.
