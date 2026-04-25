# Project Writeup: RTSP Lab Probe

### What I Built
I developed a high-performance C++20 command-line utility designed to analyze RTSP streams and local video files. Unlike basic metadata readers, this tool utilizes the FFmpeg `libav` suite to inspect the actual bitstream. It measures real-time performance metrics, including frame rate (FPS), bitrate, and keyframe intervals, while providing a clean JSON output for automated processing.

### Difficulties Faced
 
1. **Signal Interruption Management**: Handling `SIGINT` (Ctrl+C) while maintaining valid JSON output was difficult. I used `std::atomic` flags to safely break the packet loop and ensure the "Interrupted" state still resulted in a calculated report rather than a crash or empty file.
2. **Synchronous Timeouts**: Standard FFmpeg network calls can block indefinitely. I had to implement a dictionary-based `stimeout` configuration to ensure the probe exits within the required 3-second window for non-existent streams, satisfying strict timing requirements.
3. **RTSP Keyframe Identification**: A major challenge was the lack of reliable keyframe flags in raw RTSP packets. if the `--duration` flag is set to less than time needed for 2 Keyframes to pass, the Keyframe interval would output `0`. Figured this out by adding a DEBUG statement.
4. **Clock Synchronization**: I discovered that calculating metrics using `std::chrono` (real-world time) resulted in wildly inaccurate data for local files, as they are processed faster than real-time. I pivoted to using the difference between the first and last packet PTS (Presentation Timestamps) to derive the true media duration, ensuring consistent results for both live RTSP and static file inputs.

### Future Improvements
- **Advanced Jitter Analysis**: With more time, I would implement timestamp analysis to calculate network jitter and packet loss, providing a clearer picture of network health.
- **Multithreaded Probing**: I would move the network reading and packet parsing into separate threads to prevent the analysis logic from occasionally slowing down the packet ingestion during high-bitrate bursts.