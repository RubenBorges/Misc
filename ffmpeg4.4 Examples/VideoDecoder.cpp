

extern "C" {
#include <ffmpeg4.4/libavformat/avformat.h>
#include <ffmpeg4.4/libavcodec/avcodec.h>
#include <ffmpeg4.4/libavutil/imgutils.h>
}

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) return -1;

    // 1. Open File & Find Stream
    AVFormatContext* fmt_ctx = nullptr;
    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);

    AVCodec* decoder = nullptr;
    int video_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);

    // 2. Initialize Decoder Context
    AVCodecContext* dec_ctx = avcodec_alloc_context3(decoder);
    
    dec_ctx->thread_count = 0; // 0 tells FFmpeg to use all available CPU cores
    dec_ctx->thread_type = FF_THREAD_FRAME;

    
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_idx]->codecpar);
    
    
    avcodec_open2(dec_ctx, decoder, nullptr);

    // 3. Prepare Containers
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    // 4. Decoding Loop
    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_idx) {
            
            // Send compressed packet to the decoder "machine"
            int response = avcodec_send_packet(dec_ctx, pkt);
            if (response < 0) break;

            while (response >= 0) {
                // Pull raw frames out of the machine
                response = avcodec_receive_frame(dec_ctx, frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
                } else if (response < 0) {
                    return -1; // Actual error
                }

                // --- RAW DATA IS READY HERE ---
                // frame->data[0], [1], [2] contain the Y, U, and V planes.
                // In your 3D Viewer, you'd call glTexSubImage2D() here.
                
                std::cout << "Decoded Frame " << dec_ctx->frame_number 
                          << " [PTS: " << frame->pts << "]" << std::endl;
            }
        }
        av_packet_unref(pkt);
    }

    // 5. Cleanup
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);

    return 0;
}