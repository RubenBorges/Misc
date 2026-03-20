/* HOW TO BUILD:
g++ temp.cpp -o vid_info \
    -I/usr/include/ffmpeg4.4 \
    -lavformat -lavcodec -lavutil
*/
extern "C" {
#include <ffmpeg4.4/libavformat/avformat.h>
#include <ffmpeg4.4/libavcodec/avcodec.h>
#include <ffmpeg4.4/libavutil/avutil.h>
}

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video_file>" << std::endl;
        return 1;
    }

    const std::string filename = argv[1];

    // 1. Allocate Format Context
    AVFormatContext* format_ctx = avformat_alloc_context();
    if (!format_ctx) {
        std::cerr << "Could not allocate format context." << std::endl;
        return 1;
    }

    // 2. Open input stream
    if (avformat_open_input(&format_ctx, filename.c_str(), nullptr, nullptr) != 0) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return 1;
    }

    // 3. Retrieve stream information
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) {
        std::cerr << "Could not find stream information." << std::endl;
        return 1;
    }

    // 4. Dump information to console (standard FFmpeg debug output)
    av_dump_format(format_ctx, 0, filename.c_str(), 0);

    std::cout << "\n--- Custom Metadata ---" << std::endl;
    std::cout << "Format: " << format_ctx->iformat->long_name << std::endl;
    std::cout << "Duration: " << format_ctx->duration / AV_TIME_BASE << " seconds" << std::endl;
    std::cout << "Number of streams: " << format_ctx->nb_streams << std::endl;

    // 5. Cleanup
    avformat_close_input(&format_ctx);
    avformat_free_context(format_ctx);

    return 0;
}