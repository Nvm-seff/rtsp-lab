#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

struct FormatContext {
    AVFormatContext* ctx = nullptr;

    ~FormatContext() {
        if (ctx) {
            avformat_close_input(&ctx);
        }
    }
};

struct Packet {
    AVPacket* pkt = nullptr;

    Packet() {
        pkt = av_packet_alloc();
    }

    ~Packet() {
        if (pkt) {
            av_packet_free(&pkt);
        }
    }

    void unref() {
        av_packet_unref(pkt);
    }
};