#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

// Macro to quickly disable copying for RAII structs
#define DISABLE_COPY_AND_MOVE(Classname) \
    Classname(const Classname&) = delete; \
    Classname& operator=(const Classname&) = delete; \
    Classname(Classname&&) = delete; \
    Classname& operator=(Classname&&) = delete;

struct FormatContext {
    AVFormatContext* ctx = nullptr;

    FormatContext() {
        ctx = avformat_alloc_context();
    }

    ~FormatContext() {
        if (ctx) {
            // avformat_close_input also calls avformat_free_context
            avformat_close_input(&ctx);
        }
    }

    DISABLE_COPY_AND_MOVE(FormatContext)
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

    // AVPacket requires frequent unreferencing in loops
    void unref() {
        av_packet_unref(pkt);
    }

    DISABLE_COPY_AND_MOVE(Packet)
};

struct CodecContext {
    AVCodecContext* ctx = nullptr;

    // Usually initialized via avcodec_alloc_context3(codec)
    explicit CodecContext(AVCodecContext* c) : ctx(c) {}

    ~CodecContext() {
        if (ctx) {
            avcodec_free_context(&ctx);
        }
    }

    DISABLE_COPY_AND_MOVE(CodecContext)
};

struct Frame {
    AVFrame* frame = nullptr;

    Frame() {
        frame = av_frame_alloc();
    }

    ~Frame() {
        if (frame) {
            av_frame_free(&frame);
        }
    }

    DISABLE_COPY_AND_MOVE(Frame)
};