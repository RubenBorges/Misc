/*
INTEGRATION CODE:

if (myVideo.update()) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                    myVideo.getWidth(), myVideo.getHeight(), 
                    GL_RGB, GL_UNSIGNED_BYTE, myVideo.getPixels());
}
*/

#pragma once
extern "C" {
#include <ffmpeg4.4/libavformat/avformat.h>
#include <ffmpeg4.4/libavcodec/avcodec.h>
#include <ffmpeg4.4/libswscale/swscale.h>
}
#include <vector>
#include <string>

class VideoTexture {
public:
    VideoTexture(const std::string& path);
    ~VideoTexture();

    // Returns true if a new frame was decoded
    bool update(); 
    
    // Pointer to the raw RGB data for glTexSubImage2D
    const uint8_t* getPixels() const { return rgb_buffer.data(); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    AVFormatContext* fmt_ctx = nullptr;
    AVCodecContext* dec_ctx = nullptr;
    SwsContext* sws_ctx = nullptr;
    AVFrame *frame = nullptr, *rgb_frame = nullptr;
    AVPacket* pkt = nullptr;
    
    int video_stream_idx = -1;
    int width, height;
    std::vector<uint8_t> rgb_buffer;
};