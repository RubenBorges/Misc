extern "C" {
#include <ffmpeg4.4/libavformat/avformat.h>
#include <ffmpeg4.4/libavcodec/avcodec.h>
#include <ffmpeg4.4/libavutil/time.h>
}

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    // 1. Open Input
    AVFormatContext* ifmt_ctx = nullptr;
    avformat_open_input(&ifmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(ifmt_ctx, nullptr);

    // 2. Setup Output (UDP Stream)
    // We use "mpegts" because it's the standard for streaming raw UDP video
    const char* output_url = "udp://127.0.0.1:1234?pkt_size=1316";
    AVFormatContext* ofmt_ctx = nullptr;
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, "mpegts", output_url);

    // Copy stream settings from input to output (Remuxing)
    for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream* in_stream = ifmt_ctx->streams[i];
        AVStream* out_stream = avformat_new_stream(ofmt_ctx, nullptr);
        avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        out_stream->codecpar->codec_tag = 0; 
    }

    // 3. Open Network Socket
    avio_open(&ofmt_ctx->pb, output_url, AVIO_FLAG_WRITE);
    avformat_write_header(ofmt_ctx, nullptr);

    // 4. Serving Loop (with Rate Limiting)
    AVPacket* pkt = av_packet_alloc();
    int64_t start_time = av_gettime();

    while (av_read_frame(ifmt_ctx, pkt) >= 0) {
        AVStream* in_stream = ifmt_ctx->streams[pkt->stream_index];
        AVStream* out_stream = ofmt_ctx->streams[pkt->stream_index];

        // --- REAL-TIME THROTTLING ---
        // Without this, the program sends the whole file instantly.
        // We calculate when this packet *should* be sent.
        int64_t pts_time = av_rescale_q(pkt->dts, in_stream->time_base, AV_TIME_BASE_Q);
        int64_t now_time = av_gettime() - start_time;
        if (pts_time > now_time) {
            av_usleep(pts_time - now_time);
        }

        // Rescale timestamps for the MPEG-TS container
        av_packet_rescale_ts(pkt, in_stream->time_base, out_stream->time_base);
        pkt->pos = -1;

        if (av_interleaved_write_frame(ofmt_ctx, pkt) < 0) {
            break; // Network error or socket closed
        }
        av_packet_unref(pkt);
    }

    // 5. Cleanup
    av_write_trailer(ofmt_ctx);
    avformat_close_input(&ifmt_ctx);
    avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    av_packet_free(&pkt);

    return 0;
}