extern "C" {
#include <ffmpeg4.4/libavformat/avformat.h>
#include <ffmpeg4.4/libavcodec/avcodec.h>
#include <ffmpeg4.4/libavutil/imgutils.h>
}

#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) return -1;

    // 1. Initialize Format Context and Open Input
    AVFormatContext* fmt_ctx = nullptr;
    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);

    // 2. Find the first Video Stream and its Decoder
    AVCodec* codec = nullptr;
    int video_stream_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    
    // 3. Setup Codec Context
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_idx]->codecpar);
    
    avcodec_open2(codec_ctx, codec, nullptr);

    // 4. Prepare Packet and Frame containers
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    std::cout << "Processing frames from: " << argv[1] << " (" 
              << codec_ctx->width << "x" << codec_ctx->height << ")" << std::endl;

    // 5. The Main Processing Loop
    int frame_count = 0;
    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_idx) {
            
            // Send packet to decoder
            if (avcodec_send_packet(codec_ctx, pkt) == 0) {
                // Receive decoded frame (one packet can produce 0, 1, or multiple frames)
                while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    
                    // --- BEGIN PROCESSING ---
                    // Example: Calculate average Luminance (Y plane)
                    // In YUV420P, data[0] is the brightness channel
                    uint64_t sum = 0;
                    for (int y = 0; y < codec_ctx->height; y++) {
                        for (int x = 0; x < codec_ctx->width; x++) {
                            sum += frame->data[0][y * frame->linesize[0] + x];
                        }
                    }
                    double avg_brightness = (double)sum / (codec_ctx->width * codec_ctx->height);
                    
                    if (frame_count % 30 == 0) { // Log every 30th frame
                        std::cout << "Frame " << frame_count << " | Avg Brightness: " << avg_brightness << std::endl;
                    }
                    frame_count++;
                    // --- END PROCESSING ---
                }
            }
        }
        av_packet_unref(pkt);
    }

    // Flush decoder (send null packet to signal EOF)
    avcodec_send_packet(codec_ctx, nullptr);

    // Cleanup
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    return 0;
}