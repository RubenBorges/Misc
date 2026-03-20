//g++ remux.cpp -o remuxer -I/usr/include/ffmpeg4.4 -lavformat -lavutil

extern "C" {
#include <ffmpeg4.4/libavformat/avformat.h>
#include <ffmpeg4.4/libavutil/timestamp.h>
}

#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input> <output>" << std::endl;
        return 1;
    }

    AVFormatContext *ifmt_ctx = nullptr, *ofmt_ctx = nullptr;
    AVPacket *pkt = av_packet_alloc();

    // 1. Open Input
    avformat_open_input(&ifmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(ifmt_ctx, nullptr);

    // 2. Setup Output
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, argv[2]);

    // Map input streams to output streams
    std::vector<int> stream_mapping(ifmt_ctx->nb_streams);
    int stream_counter = 0;

    for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        // Only copy Video, Audio, and Subtitles
        if (in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        stream_mapping[i] = stream_counter++;
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, nullptr);
        
        // CRITICAL: Copy the parameters without re-encoding
        avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        out_stream->codecpar->codec_tag = 0; 
    }

    // 3. Open Output File & Write Header
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_open(&ofmt_ctx->pb, argv[2], AVIO_FLAG_WRITE);
    }
    avformat_write_header(ofmt_ctx, nullptr);

    // 4. The Packet Copy Loop
    while (av_read_frame(ifmt_ctx, pkt) >= 0) {
        AVStream *in_stream  = ifmt_ctx->streams[pkt->stream_index];
        int out_stream_idx = stream_mapping[pkt->stream_index];

        if (out_stream_idx < 0) {
            av_packet_unref(pkt);
            continue;
        }

        AVStream *out_stream = ofmt_ctx->streams[out_stream_idx];
        pkt->stream_index = out_stream_idx;

        // Rescale timestamps from input time_base to output time_base
        av_packet_rescale_ts(pkt, in_stream->time_base, out_stream->time_base);
        pkt->pos = -1;

        av_interleaved_write_frame(ofmt_ctx, pkt);
        av_packet_unref(pkt);
    }

    // 5. Cleanup
    av_write_trailer(ofmt_ctx);
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    av_packet_free(&pkt);

    return 0;
}