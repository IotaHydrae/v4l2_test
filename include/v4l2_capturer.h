/**
 * @file v4l2_capturer.h
 * @author IotaHydrae (writeforever@foxmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-06
 * 
 * MIT License
 * 
 * Copyright 2022 IotaHydrae(writeforever@foxmail.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * 
 */

#ifndef __V4L2_CAPTURER
#define __V4L2_CAPTURER

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include <fstream>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#define DEFAULT_CAMERA_PATH "/dev/video0"
#define DEFAULT_BUFFER_COUNT 4

#define LOG_DEBUG(msg)printf(msg"\n")
#define CHECK_FMT_IF_SUPPORTED(pixelformat, v4l2_fmt)if(pixelformat == v4l2_fmt)printf(#v4l2_fmt" supported\n");

struct video_buffer {
        unsigned int length;
        unsigned int offset;
        unsigned char *start;
};

using namespace std;

class v4l2_capturer
{
public:

        v4l2_capturer();
        ~v4l2_capturer();
        int init();
        int start();
        int stop();
        int get_frame();
        int save_fbdata_to_file( const char *path );
        int save_fbdata_to_file_by_mmap( const char *path );
        void query_supported_format_new();
        
        bool frameSaveImage( const char *file_path );
        
private:
        int                        m_fd;
        int                        m_rb_count;
        int                        m_rb_current;
        int                        m_total_bytes;
        struct v4l2_capability     m_cap;
        struct v4l2_format         m_fmt;
        struct v4l2_fmtdesc        m_desc_fmt;
        struct v4l2_requestbuffers m_rb;
        struct v4l2_buffer         m_buf;
        struct v4l2_frmsizeenum    m_frmsize;
        struct video_buffer       *m_video_buffers;
        unsigned int               m_video_buff_size;
};


#endif
