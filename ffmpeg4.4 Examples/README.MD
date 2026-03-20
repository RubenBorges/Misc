On Arch, the ffmpeg4.4 package usually puts its libraries in /usr/lib/ffmpeg4.4/. If the program compiles but fails to run with a "loading shared libraries" error, you might need to run it like this:
Bash

LD_LIBRARY_PATH=/usr/lib/ffmpeg4.4 ./decoder my_video.mp4


-----------------
OPENGL IMPLEMENTATION SNIPPETS:
------------------- 

Instead, you should use glTexSubImage2D. This updates the pixel data of an already existing texture on the GPU.
The Efficient Upload Strategy
1. One-Time Setup (Initialization)

Before your main loop starts, create the texture object. This is where you define the dimensions and the format.
C++

GLuint textureID;
glGenTextures(1, &textureID);
glBindTexture(GL_TEXTURE_2D, textureID);

// Set filtering for scaling (Bilinear is usually best for video)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// Pre-allocate the storage (no data uploaded yet)
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
             myVideo.getWidth(), myVideo.getHeight(), 
             0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

2. The Per-Frame Update

Inside your render loop, once the swscale conversion is done, you "push" the new pixels to that allocated space.
C++

if (myVideo.update()) { // Our class method that calls sws_scale
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Crucial for performance: Tell OpenGL how the CPU data is aligned.
    // FFmpeg's linesize is often padded, so we set this to 1.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                    myVideo.getWidth(), myVideo.getHeight(), 
                    GL_RGB, GL_UNSIGNED_BYTE, myVideo.getPixels());
}

Pro-Tip: PBOs (Pixel Buffer Objects)

Since you are a power user on Arch with an i5-1135G7, you can take this a step further. glTexSubImage2D is synchronous—it makes the CPU wait while the GPU copies the data.

To prevent your 3D viewer's UI from stuttering, you can use PBOs. This allows you to "map" a piece of GPU memory, have swscale write directly into it, and then tell the GPU to update the texture asynchronously.
Why glPixelStorei is your Best Friend

You might notice that without glPixelStorei(GL_UNPACK_ALIGNMENT, 1), your video looks slanted or has static lines through it. This is because OpenGL defaults to a 4-byte alignment, but video frames (especially RGB24) often don't fall on clean 4-byte boundaries. Setting it to 1 ensures OpenGL reads the exact byte-stream swscale produced.
...................
PIXEL BUFFER OBJECT VERSION
----------------------
Using PBOs (Pixel Buffer Objects) is the "pro" way to handle video textures because it offloads the data transfer to the GPU's DMA (Direct Memory Access) engine. Instead of the CPU waiting for glTexSubImage2D to finish copying, you "map" a piece of GPU memory, let swscale write to it, and the transfer happens in the background.
----------
The PBO Streaming Architecture
---------
1. Initialization

You'll need two PBOs to "ping-pong" between frames. While the GPU is reading from one, the CPU is writing the next decoded frame into the other.
------------
CPP
-----------

GLuint pbos[2];
int pbo_index = 0;
size_t data_size = width * height * 3; // RGB24

glGenBuffers(2, pbos);
for(int i = 0; i < 2; i++) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[i]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, data_size, nullptr, GL_STREAM_DRAW);
}
glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
_______________________

2. The Asynchronous Upload Loop

In your render loop, the logic shifts from "sending data" to "mapping memory."
___________
Cpp
__________

if (myVideo.decodeNextFrame()) { // Your decoder fills the internal AVFrame
    pbo_index = (pbo_index + 1) % 2;
    int next_index = (pbo_index + 1) % 2;

    // 1. Start the transfer from the current PBO to the texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[pbo_index]);
    
    // When a PBO is bound, the last param is an offset (0), not a pointer!
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // 2. Map the NEXT PBO for swscale to write into
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[next_index]);
    
    // glMapBuffer returns a pointer directly to GPU-accessible memory
    uint8_t* ptr = (uint8_t*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        // Point swscale's destination to the mapped PBO memory
        uint8_t* dest_data[4] = { ptr, nullptr, nullptr, nullptr };
        int dest_linesize[4] = { width * 3, 0, 0, 0 };

        sws_scale(sws_ctx, frame->data, frame->linesize, 0, height, dest_data, dest_linesize);
        
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); 
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
___________________________

Why This Wins on Arch + Intel

    Zero-Copy (ish): swscale writes directly into memory that the GPU can see. You avoid the extra memcpy from an intermediate std::vector to the driver's internal buffer.

    CPU/GPU Parallelism: Your i5-1135G7 can start decoding the next frame while the Iris Xe is still busy drawing the previous one.

    Modern OpenGL Standards: Since you're on 4.3+, you could even use glBufferStorage with GL_MAP_PERSISTENT_BIT for an even more advanced "Zero-Copy" approach, but PBOs are the standard sweet spot for compatibility and speed.

Practical Tip for your 3D Viewer

If you're using Indirect Drawing, you can now treat these video textures as part of your material system. You could even use an Array Texture (GL_TEXTURE_2D_ARRAY) if you wanted to play multiple videos at once on different masonry blocks in your 3D scene.

Would you like me to show you how to handle the audio sync so the video doesn't play too fast for the PBOs?
_________________
AUDIO SYNCING:
_________________
#include <chrono>

// 1. Initialize the clock when playback starts
auto start_time = std::chrono::high_resolution_clock::now();

// --- Inside your Render Loop ---
auto current_time = std::chrono::high_resolution_clock::now();
double elapsed = std::chrono::duration<double>(current_time - start_time).count();

// Convert the frame's PTS to seconds
// Note: time_base is from the AVStream (e.g., 1/24000)
double frame_pts_seconds = frame->pts * av_q2d(video_stream->time_base);

if (frame_pts_seconds <= elapsed) {
    // 2. The frame is due! Update the PBO and Texture
    updatePBOAndTexture(); 
    
    // 3. Decode the NEXT frame so it's ready in the buffer
    decodeNextFrame(); 
} else {
    // 4. The frame is in the future. 
    // Just render the 3D scene with the PREVIOUS texture.
    render3DSceneWithExistingTexture();
}
____________________________
Handling the "Drift"

Since you are a developer, you might notice that av_usleep or std::this_thread::sleep_for isn't always accurate on Linux due to OS scheduling. In your Arch Linux environment, it is better to let the OpenGL VSync (glfwSwapBuffers) act as your primary regulator and only update the texture when the clock says it's time.
Why this matters for your 3D Viewer

By using a Master Clock, your 3D models can rotate, scale, and move at a smooth 60fps (or 144fps), while the video texture playing on their surface stays perfectly synced at its native 24fps or 30fps.
Critical Note: The "Time Base"

Every video file has a different time_base. Never assume pts is in seconds or milliseconds. Always multiply by av_q2d(stream->time_base) to get the real-world time.
_________________________________