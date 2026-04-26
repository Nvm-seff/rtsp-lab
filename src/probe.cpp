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

    av_log_set_level(AV_LOG_PANIC); // to suppress ffmpeg logs, warnings hi khaali    
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

    const char* profile_name = avcodec_profile_name(cp->codec_id, cp->profile);
    
    // Format Level (e.g., 41 -> 4.1)
    std::string level_str = (cp->level != FF_LEVEL_UNKNOWN) 
                            ? std::to_string(cp->level / 10) + "." + std::to_string(cp->level % 10)
                            : "Unknown";


    double fps_adv = av_q2d(vs->avg_frame_rate);
    int bitrate_adv = fmt.ctx->bit_rate / 1000;

    int width = cp->width;
    int height = cp->height;

    // --- packet loop ---
    Packet pkt;

    int packets = 0;
    long long total_bytes = 0;

    std::vector<double> key_times;

    double first_pts_sec = -1.0; 
    double last_pts_sec = 0.0;

    bool first_seen = false;
    auto start = std::chrono::steady_clock::now();
    auto first_pkt_time = start;
    double media_duration = 0.0;

    while (running) {
        // auto now = std::chrono::steady_clock::now();
        // double elapsed =
        //     std::chrono::duration<double>(now - start).count();

        // if (elapsed >= duration_sec) break;

        ret = av_read_frame(fmt.ctx, pkt.pkt);
        if (ret < 0) break;

        if (pkt.pkt->stream_index == video_idx) {
            // 1. Check if the packet actually has a valid timestamp
            if (pkt.pkt->pts == AV_NOPTS_VALUE) {
                pkt.unref();
                continue; 
            }

            double pts_sec = pkt.pkt->pts * av_q2d(vs->time_base);

            if (!first_seen) {
                first_pts_sec = pts_sec;
                first_pkt_time = std::chrono::steady_clock::now();
                first_seen = true;
            }
            
            // Stop if the current media time exceeds the requested duration
            if (duration_sec > 0 && (pts_sec - first_pts_sec) >= duration_sec) {
                pkt.unref();
                break;
            }

            packets++;
            total_bytes += pkt.pkt->size;
            last_pts_sec = pts_sec;
            media_duration = last_pts_sec - first_pts_sec;

            if (pkt.pkt->flags & AV_PKT_FLAG_KEY) {
                key_times.push_back(pts_sec);
                //std::cerr << "[DEBUG] Keyframe found at PTS: " << pts_sec << "s. Total keyframes: " << key_times.size() << std::endl;
            }
        }

        pkt.unref();
    }

    if (!running) {
                std::cout << "\r\033[K" << "Interrupt received, calculating results for the captured " 
                << media_duration << " seconds..."<< std::endl; // fancy way of removing that annoying ^C 
        }

    double fps_meas = 0;
    double bitrate_meas = 0;

    if (media_duration > 0) {
        fps_meas = packets / media_duration;
        bitrate_meas = (total_bytes * 8.0 / 1000.0) / media_duration;
    }
    
    double key_interval = calculate_key_interval(key_times);

    double ttf_ms =
        std::chrono::duration<double, std::milli>(
            first_pkt_time - open_start).count();

    // --- output ---
    if (json) {
        std::cout << "{\n";
        std::cout << "\"codec\": \"" << codec << "\", \"profile\": \"" << (profile_name ? profile_name : "Unknown") << "\", \"level\": \"" << level_str << "\",\n";
        std::cout << "\"resolution\": {\"width\": " << width
                  << ", \"height\": " << height << "},\n";
        std::cout << "\"fps_advertised\": " << fps_adv << ", \"fps_measured\": " << fps_meas << ",\n";
        std::cout << "\"bitrate_advertised_kbps\": " << bitrate_adv << ",\n";
        std::cout << "\"bitrate_measured_kbps\": " << bitrate_meas << ",\n";
        std::cout << "\"keyframe_interval_s\": " << key_interval << ",\n";
        std::cout << "\"time_to_first_frame_ms\": " << ttf_ms << ",\n";
        std::cout << "\"packets_received\": " << packets << "\n";
        std::cout << "}\n";
    } else {
        //std::cout << "\n";
        std::cout << "=== rtsp-lab probe ===\n";
        std::cout << "Input: " << input << "\n";
        std::cout << "Codec:       " << codec << " (" << (profile_name ? profile_name : "Unknown") << ", Level " << level_str << ")\n";
        std::cout << "Resolution: " << width << "x" << height << "\n";
        std::cout << "FPS (advertised): " << fps_adv << "\n";
        std::cout << "FPS (measured): " << fps_meas << "\n";
        std::cout << "Bitrate (adv.): " << bitrate_adv << " kbps\n";
        std::cout << "Bitrate (meas.): " << bitrate_meas << " kbps\n";
        std::cout << "Keyframe interval: " << key_interval << " s\n";
        std::cout << "Time to first frame: " << ttf_ms << " ms\n";
        std::cout << "Packets received: " << packets << "\n";
    }

    return 0;
}