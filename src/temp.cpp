#include "probe.hpp"
#include "ffmpeg_raii.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <csignal>
#include <atomic>

extern "C" {
#include <libavutil/error.h>
}

static std::atomic<bool> running(true);

void handle_sigint(int) {
    running = false;
}

std::string err2str(int err) {
    char buf[256];
    av_strerror(err, buf, sizeof(buf));
    return std::string(buf);
}

int run_probe(const std::string& input,
              int duration_sec,
              int timeout_sec,
              bool json) {

    signal(SIGINT, handle_sigint);

    FormatContext fmt;

    // --- timeout ---
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "stimeout",
                std::to_string(timeout_sec * 1000000).c_str(), 0);

    auto open_start = std::chrono::steady_clock::now();

    int ret = avformat_open_input(&fmt.ctx, input.c_str(), nullptr, &opts);
    if (ret < 0) {
        if (json)
            std::cout << "{ \"error\": \"" << err2str(ret) << "\" }\n";
        else
            std::cerr << "Error: " << err2str(ret) << "\n";
        return 1;
    }

    ret = avformat_find_stream_info(fmt.ctx, nullptr);
    if (ret < 0) {
        if (json)
            std::cout << "{ \"error\": \"" << err2str(ret) << "\" }\n";
        else
            std::cerr << "Error: " << err2str(ret) << "\n";
        return 1;
    }

    // --- find video stream ---
    int video_idx = -1;
    for (unsigned i = 0; i < fmt.ctx->nb_streams; i++) {
        if (fmt.ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_idx = i;
            break;
        }
    }

    if (video_idx == -1) {
        std::cerr << "No video stream found\n";
        return 1;
    }

    AVStream* vs = fmt.ctx->streams[video_idx];
    AVCodecParameters* cp = vs->codecpar;

    const char* codec = avcodec_get_name(cp->codec_id);

    double fps_adv = av_q2d(vs->avg_frame_rate);
    int bitrate_adv = fmt.ctx->bit_rate / 1000;

    int width = cp->width;
    int height = cp->height;

// --- packet loop ---
    Packet pkt;
    int packets = 0;
    long long total_bytes = 0;
    std::vector<double> key_times;

    bool first_seen = false;
    double first_pts_sec = 0.0; // Track the start of the video
    double last_pts_sec = 0.0;  // Track the end of the video
    
    //auto open_start = std::chrono::steady_clock::now();
    auto start = std::chrono::steady_clock::now();
    auto first_pkt_time = start;

    while (running) {
        ret = av_read_frame(fmt.ctx, pkt.pkt);
        if (ret < 0) break;

        if (pkt.pkt->stream_index == video_idx) {
            // CONVERT PTS TO SECONDS: This is the most important part
            double current_pts_sec = pkt.pkt->pts * av_q2d(vs->time_base);

            if (!first_seen) {
                first_pkt_time = std::chrono::steady_clock::now();
                first_pts_sec = current_pts_sec;
                first_seen = true;
            }
            
            // Check if we have reached the user-requested duration based on video time
            if ((current_pts_sec - first_pts_sec) >= duration_sec) {
                pkt.unref(); // Don't forget to unref before breaking
                break;
            }

            packets++;
            total_bytes += pkt.pkt->size;
            last_pts_sec = current_pts_sec;

            if (pkt.pkt->flags & AV_PKT_FLAG_KEY) {
                // Record the actual media time of the keyframe
                key_times.push_back(current_pts_sec);
            }
        }

        pkt.unref();
    }

    // --- NEW CALCULATIONS ---
    double media_duration = last_pts_sec - first_pts_sec;

    // Use media_duration for FPS and Bitrate instead of total_time
    double fps_meas = (media_duration > 0) ? (packets / media_duration) : 0;
    double bitrate_meas = (media_duration > 0) ? ((total_bytes * 8.0 / 1000.0) / media_duration) : 0;

    double key_interval = 0.0;
    if (key_times.size() > 1) {
        double sum = 0;
        for (size_t i = 1; i < key_times.size(); i++) {
            sum += key_times[i] - key_times[i - 1];
        }
        key_interval = sum / (key_times.size() - 1);
    }

    double ttf_ms =
        std::chrono::duration<double, std::milli>(
            first_pkt_time - open_start).count();

    // --- output ---
    if (json) {
        std::cout << "{\n";
        std::cout << "\"codec\": \"" << codec << "\",\n";
        std::cout << "\"resolution\": {\"width\": " << width
                  << ", \"height\": " << height << "},\n";
        std::cout << "\"fps_advertised\": " << fps_adv << ",\n";
        std::cout << "\"fps_measured\": " << fps_meas << ",\n";
        std::cout << "\"bitrate_advertised_kbps\": " << bitrate_adv << ",\n";
        std::cout << "\"bitrate_measured_kbps\": " << bitrate_meas << ",\n";
        std::cout << "\"keyframe_interval_s\": " << key_interval << ",\n";
        std::cout << "\"time_to_first_frame_ms\": " << ttf_ms << ",\n";
        std::cout << "\"packets_received\": " << packets << "\n";
        std::cout << "}\n";
    } else {
        std::cout << "\n";
        std::cout << "=== rtsp-lab probe ===\n";
        std::cout << "Input: " << input << "\n";
        std::cout << "Codec: " << codec << "\n";
        std::cout << "Resolution: " << width << "x" << height << "\n";
        std::cout << "FPS (adv): " << fps_adv << "\n";
        std::cout << "FPS (meas): " << fps_meas << "\n";
        std::cout << "Bitrate (adv): " << bitrate_adv << " kbps\n";
        std::cout << "Bitrate (meas): " << bitrate_meas << " kbps\n";
        std::cout << "Keyframe interval: " << key_interval << " s\n";
        std::cout << "Time to first frame: " << ttf_ms << " ms\n";
        std::cout << "Packets: " << packets << "\n";
    }

    return 0;
}