extern "C" {
#include <ffmpeg4.4/libavformat/avformat.h>
#include <ffmpeg4.4/libavcodec/avcodec.h>
#include <ffmpeg4.4/libavutil/opt.h>
}

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input> <output.mp4>" << std::endl;
        return 1;
    }

    // 1. INPUT SETUP (Demuxer & Decoder)
    AVFormatContext* ifmt_ctx = nullptr;
    avformat_open_input(&ifmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(ifmt_ctx, nullptr);

    AVCodec* decoder = nullptr;
    int stream_idx = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    AVCodecContext* dec_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(dec_ctx, ifmt_ctx->streams[stream_idx]->codecpar);
    avcodec_open2(dec_ctx, decoder, nullptr);

    // 2. OUTPUT SETUP (Encoder & Muxer)
    AVFormatContext* ofmt_ctx = nullptr;
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, argv[2]);

    // Find H.264 encoder
    AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    AVCodecContext* enc_ctx = avcodec_alloc_context3(encoder);

    // Set basic encoding parameters
    enc_ctx->height = dec_ctx->height;
    enc_ctx->width = dec_ctx->width;
    enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P; // Standard for H.264
    enc_ctx->time_base = av_inv_q(dec_ctx->framerate); // Match input framerate
    
    // Some formats (like MP4) require global headers
    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    avcodec_open2(enc_ctx, encoder, nullptr);

    // Create a new stream in the output file
    AVStream* out_stream = avformat_new_stream(ofmt_ctx, nullptr);
    avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);

    // Open the output file for writing
    avio_open(&ofmt_ctx->pb, argv[2], AVIO_FLAG_WRITE);
    avformat_write_header(ofmt_ctx, nullptr);

    // 3. THE TRANSCODING LOOP
    AVPacket* in_pkt = av_packet_alloc();
    AVPacket* out_pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    while (av_read_frame(ifmt_ctx, in_pkt) >= 0) {
        if (in_pkt->stream_index == stream_idx) {
            // DECODE
            if (avcodec_send_packet(dec_ctx, in_pkt) == 0) {
                while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                    
                    // ENCODE
                    frame->pts = frame->best_effort_timestamp; // Manage timing
                    if (avcodec_send_frame(enc_ctx, frame) == 0) {
                        while (avcodec_receive_packet(enc_ctx, out_pkt) == 0) {
                            // Rescale timestamps from encoder to muxer
                            av_packet_rescale_ts(out_pkt, enc_ctx->time_base, out_stream->time_base);
                            out_pkt->stream_index = out_stream->index;
                            av_interleaved_write_frame(ofmt_ctx, out_pkt);
                            av_packet_unref(out_pkt);
                        }
                    }
                }
            }
        }
        av_packet_unref(in_pkt);
    }

    // 4. CLEANUP & FLUSH
    av_write_trailer(ofmt_ctx);
    
    av_frame_free(&frame);
    av_packet_free(&in_pkt);
    av_packet_free(&out_pkt);
    avcodec_free_context(&dec_ctx);
    avcodec_free_context(&enc_ctx);
    avformat_close_input(&ifmt_ctx);
    avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    return 0;
}