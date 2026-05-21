# 3DSware

3DSware is a lightweight, stable, and highly efficient C++ library for Nintendo 3DS consoles (homebrew).
It provides a high-level, object-oriented framework for interacting with the console's hardware, eliminating the complexity of low-level libctru APIs. It automatically handles buffers, conversions, allocations, and prevents memory leaks or system crashes.

## Features
- **Camera (`hardware_camera`):** Safe initialization of `cam:u`, acquisition of linear `RGB565` frames, and ultra-high-performance direct transfer to VRAM using the DMA module (PICA200 `GX_DisplayTransfer`), completely eliminating glitches and tearing.
- **Image Export (`image_export`):** Isolated module to export frames to pure `BMP` RGB888 format (without any extra dependencies) or compressed formats. It natively handles padding and bottom-up orientation for the highest output quality.
- **Microphone (`hardware_mic`):** Total management of `mic:u`. Captures audio using a circular ring buffer, fully supporting real-time streaming for VoIP applications (e.g., Discord) or buffered recordings to the filesystem.
- **File System (`hardware_fs`):** Essential and crash-proof utilities to check, read, recursively create directories, and delete files from the SD card (handles missing SD cards safely).

## License
This project is licensed under the **MIT License**.
It is open, free for both commercial and non-commercial use, and can be integrated into your closed-source or open-source projects without any viral copyleft constraints. See the `LICENSE` file for details.
