/*
g++ decoder_rgb.cpp -o decoder_rgb \
    -I/usr/include/ffmpeg4.4 \
    -lavformat -lavcodec -lswscale -lavutil
*/

extern "C" {
#include <ffmpeg4.4/libavformat/avformat.h>
#include <ffmpeg4.4/libavcodec/avcodec.h>
#include <ffmpeg4.4/libswscale/swscale.h>
#include <ffmpeg4.4/libavutil/imgutils.h>
}

#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) return -1;

    // 1. Standard Decoder Setup (same as previous example)
    AVFormatContext* fmt_ctx = nullptr;
    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);
    AVCodec* decoder = avcodec_find_decoder(fmt_ctx->streams[0]->codecpar->codec_id);
    AVCodecContext* dec_ctx = avcodec_alloc_context3(decoder);
    
    dec_ctx->thread_count = 0; // 0 tells FFmpeg to use all available CPU cores
    dec_ctx->thread_type = FF_THREAD_FRAME;
   
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[0]->codecpar);
    avcodec_open2(dec_ctx, decoder, nullptr);

    // 2. Initialize the Scaler (SwsContext)
    // This defines: Source (Decoder) -> Destination (RGB)
    struct SwsContext* sws_ctx = sws_getContext(
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt, // From Decoder
        dec_ctx->width, dec_ctx->height, AV_PIX_FMT_RGB24, // To RGB
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    // 3. Prepare the Output RGB Frame
    AVFrame* rgb_frame = av_frame_alloc();
    rgb_frame->format = AV_PIX_FMT_RGB24;
    rgb_frame->width  = dec_ctx->width;
    rgb_frame->height = dec_ctx->height;
    av_frame_get_buffer(rgb_frame, 0); // Allocate memory for the RGB planes

    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    // 4. The Combined Loop
    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (avcodec_send_packet(dec_ctx, pkt) == 0) {
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                
                // --- THE CONVERSION STEP ---
                // This takes the YUV data and writes it into our rgb_frame
                sws_scale(sws_ctx, 
                          frame->data, frame->linesize, 0, dec_ctx->height, 
                          rgb_frame->data, rgb_frame->linesize);

                // Now rgb_frame->data[0] is a contiguous array of RGB pixels!
                // Ready for: glTexImage2D(..., rgb_frame->data[0]);
                
                std::cout << "Frame " << dec_ctx->frame_number << " converted to RGB." << std::endl;
            }
        }
        av_packet_unref(pkt);
    }

    // 5. Cleanup
    sws_freeContext(sws_ctx);
    av_frame_free(&rgb_frame);
    av_frame_free(&frame);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);

    return 0;
}