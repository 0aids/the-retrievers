# Shared and Impls
While the code is shared, there are a couple functions that need to be defined in order for the
shared code to work. These functions are `utils_sleepms` and `utils_log`.

Furthermore, for the lora, per-device implementations are required. therefore these are separated
to ensure proper encapsulation.
