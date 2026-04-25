# RTSP Lab Probe

A high-performance C++20 tool for probing RTSP streams and local video files. This tool measures actual stream performance (bitrate, FPS, keyframe intervals) rather than just reporting advertised metadata.

## Prerequisites

You will need FFmpeg development libraries installed:

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake pkg-config \
                 libavformat-dev libavcodec-dev libavutil-dev
```

## Building the Project
The project uses a modern CMake workflow. Run the following command from the project root:

```Bash
cmake -S . -B build && cmake --build build -j
```
This will create a build directory and compile the rtsp-lab executable inside it.

## Setting up a Test Stream
To test RTSP functionality locally:

### Start MediaMTX: Download MediaMTX and run the binary.

Publish a stream:

```Bash
ffmpeg -re -i sample.mp4 -c copy -f rtsp rtsp://localhost:8554/test
```

### Usage
The binary supports several subcommands and flags.

1. Probe a Stream (Default)
```Bash
./build/rtsp-lab probe rtsp://127.0.0.1:8554/test
```
2. Set Duration and Output JSON
Capture data for 15 seconds and output in JSON format:

```Bash
./build/rtsp-lab probe rtsp://127.0.0.1:8554/test --duration 15 --json
```
3. Handle Timeouts
Exit if the stream doesn't connect within 2 seconds:

```Bash
./build/rtsp-lab probe rtsp://does.not.exist:554/none --timeout 2 --json
```
4. Run Tests
The project includes automated tests via CTest:

```Bash
cd build && ctest --output-on-failure
```
### Technical Implementation
#### RAII Architecture
We manage FFmpeg resources using custom RAII wrappers to ensure zero memory leaks even during early interrupts:

```C++
struct Packet {
    AVPacket* pkt;
    Packet() { pkt = av_packet_alloc(); }
    ~Packet() { av_packet_free(&pkt); }
    void unref() { av_packet_unref(pkt); }
};
```

```Markdown
### Final Checklist for you:
1.  **Executable Name**: In your `CMakeLists.txt`, make sure the `add_executable` target is named `rtsp-lab` (with a hyphen), as that's what the requirement uses.
2.  **Location**: Always run that `cmake` command from the folder where `CMakeLists.txt` lives.
3.  **Clean Start**: If you have an old `build` folder, delete it (`rm -rf build`) before run
```