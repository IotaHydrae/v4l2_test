/**
 * @file v4l2_capturer.cpp
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

#include "v4l2_capturer.h"

v4l2_capturer::v4l2_capturer(): m_fd{-1}
{
        LOG_DEBUG( "v4l2_capturer created." );
}

v4l2_capturer::~v4l2_capturer()
{
        LOG_DEBUG( "v4l2_capturer destoryed." );
        
        for( int i = 0; i < DEFAULT_BUFFER_COUNT; i++ ) {
                if( m_video_buffers[i].start ) {
                        munmap( m_video_buffers[i].start, m_video_buff_size );
                }
        }
        
        if( m_video_buffers ) {
                free( m_video_buffers );
        }
        
        if( m_fd != -1 ) {
                close( m_fd );
        }
}

int v4l2_capturer::init()
{
        int ret = -1;
        m_fd = open( DEFAULT_CAMERA_PATH, O_RDWR );
        
        if( m_fd < 0 ) {
                perror( "open failed." );
                return -1;
        }
        
        memset( &m_cap, 0, sizeof( m_cap ) );
        
        if( ioctl( m_fd, VIDIOC_QUERYCAP, &m_cap ) < 0 ) {
                perror( "ioctl failed" );
                return -1;
        }
        
        if( m_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) {
                printf( "V4L2_CAP_VIDEO_CAPTURE supportted\n" );
        }
        
        if( m_cap.capabilities & V4L2_CAP_STREAMING ) {
                printf( "V4L2_CAP_STREAMING supportted\n" );
        }
        
        if( m_cap.capabilities & V4L2_CAP_READWRITE ) {
                printf( "V4L2_CAP_READWRITE supportted\n" );
        }
        
        /* set format */
        memset( &m_fmt, 0x0, sizeof( m_fmt ) );
        m_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_fmt.fmt.pix.width = DEFAULT_CAPTURER_PIX_WIDTH;
        m_fmt.fmt.pix.height = DEFAULT_CAPTURER_PIX_HEIGHT;
        m_fmt.fmt.pix.pixelformat = DEFAULT_CAPTURER_PIX_FORMAT;
        m_fmt.fmt.pix.field       = DEFAULT_CAPTURER_PIX_FIELD;
        
        if( ioctl( m_fd, VIDIOC_S_FMT, &m_fmt ) < 0 ) {
                perror( "ioctl VIDIOC_S_FMT failed!" );
                return -1;
        }
        
        /* request framebuffer */
        memset( &m_rb, 0x0, sizeof( m_rb ) );
        m_rb.count = DEFAULT_BUFFER_COUNT;
        m_rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_rb.memory = V4L2_MEMORY_MMAP;
        
        if( ioctl( m_fd, VIDIOC_REQBUFS, &m_rb ) < 0 ) {
                perror( "ioctl VIDIOC_REQBUFS failed!" );
                return -1;
        }
        
        m_video_buffers = ( struct video_buffer * )calloc( m_rb.count,
                                                           sizeof( struct video_buffer ) );
                                                           
        /* map the framebuffer to userspace */
        for( int index_buf = 0; index_buf < m_rb.count; index_buf++ ) {
                /* calloc the userspace buffer */
                memset( &m_buf, 0x0, sizeof( m_buf ) );
                m_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                m_buf.memory = V4L2_MEMORY_MMAP;
                m_buf.index = index_buf;
                
                if( ioctl( m_fd, VIDIOC_QUERYBUF, &m_buf ) < 0 ) {
                        perror( "ioctl VIDIOC_QUERYBUF failed!" );
                        return -1;
                }
                
                m_video_buff_size = m_buf.length;
                /* map to userspace  */
                m_video_buffers[index_buf].start = ( unsigned char * )mmap( NULL,
                                                                            m_buf.length,
                                                                            PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, m_buf.m.offset );
                                                                            
                if( m_video_buffers[index_buf].start == MAP_FAILED ) {
                        perror( "map video buf failed!" );
                        return -1;
                }
                
                //memset(m_video_buffers[index_buf].start, 0x0, m_buf.length);
                /* queue the buffer */
                memset( &m_buf, 0x0, sizeof( m_buf ) );
                m_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                m_buf.memory = V4L2_MEMORY_MMAP;
                m_buf.index = index_buf;
                
                if( ioctl( m_fd, VIDIOC_QBUF, &m_buf ) < 0 ) {
                        perror( "ioctl VIDIOC_QBUF failed!" );
                        return -1;
                }
        }
        
        printf( "v4l2_buf size: %d\n", m_buf.length );
        return 0;
}

int v4l2_capturer::start()
{
        LOG_DEBUG( "***start stream." );
        enum v4l2_buf_type type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
        if( ioctl( m_fd, VIDIOC_STREAMON, &type ) < 0 ) {
                perror( "ioctl VIDIOC_STREAMON failed!" );
                return -1;
        }
        
        return 0;
}

int v4l2_capturer::stop()
{
        LOG_DEBUG( "***stop stream." );
        enum v4l2_buf_type type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
        if( ioctl( m_fd, VIDIOC_STREAMOFF, &type ) < 0 ) {
                perror( "ioctl VIDIOC_STREAMOFF failed!" );
                return -1;
        }
        
        return 0;
}

int v4l2_capturer::get_frame()
{
        int ret;
        static int frame_count = 1;
        struct v4l2_buffer v4lbuffer;
        
        /* pop fb from queue */
        memset( &v4lbuffer, 0x0, sizeof( v4lbuffer ) );
        v4lbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4lbuffer.memory = V4L2_MEMORY_MMAP;
        
        if( ioctl( m_fd, VIDIOC_DQBUF, &v4lbuffer ) < 0 ) {
                perror( "ioctl VIDIOC_DQBUF failed!" );
                return ret;
        }
        
        m_rb_current = v4lbuffer.index;
        m_total_bytes = v4lbuffer.bytesused;
        
        
        int out_fd;
        unsigned char *file_base;
        char out_fname[50];
        sprintf( out_fname, "./abc_%04d.jpeg", frame_count );
        
        LOG_DEBUG( "***save_fbdata_to_file by mmap." );
        out_fd = open( out_fname, O_RDWR | O_CREAT | O_TRUNC, 0755 );
        
        if( out_fd < 0 ) {
                perror( "open path failed!" );
                return out_fd;
        }
        
        /* make file length */
        lseek( out_fd, m_total_bytes - 1, SEEK_END );
        write( out_fd, "", 1 ); /* because of COW? */
        printf( "total_bytes:%d\n", m_total_bytes );
        printf( "current:%d\n", m_rb_current );
        
        /* write operation here */
        file_base = ( unsigned char * )mmap( NULL, m_total_bytes,
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED,
                                             out_fd, 0 );
                                             
        if( file_base == MAP_FAILED ) {
                perror( "mmap file failed!" );
                return -1;
        }
        
        LOG_DEBUG( "memcpying..." );
        memcpy( file_base, m_video_buffers[m_rb_current].start, m_total_bytes );
        /* release the resource */
        munmap( file_base, m_total_bytes );
        close( out_fd );
        frame_count++;
        
        /* push back fb to queue */
        LOG_DEBUG( "push back fb to queue" );
        memset( &v4lbuffer, 0x0, sizeof( v4lbuffer ) );
        v4lbuffer.index = m_rb_current;
        v4lbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4lbuffer.memory = V4L2_MEMORY_MMAP;
        
        if( ioctl( m_fd, VIDIOC_QBUF, &v4lbuffer ) < 0 ) {
                perror( "ioctl VIDIOC_QBUF failed!" );
                return -1;
        }
        
        //if(frame_count > 24)exit(1);
        return frame_count;
}

int v4l2_capturer::save_fbdata_to_file( const char *path )
{
        int fd;
        struct v4l2_buffer v4lbuffer;
        LOG_DEBUG( "***save_fbdata_to_file." );
        fd = open( path, O_RDWR | O_CREAT | O_TRUNC, 0755 );
        
        if( fd < 0 ) {
                perror( "open path failed!" );
                return fd;
        }
        
        printf( "total_bytes:%d\n", m_total_bytes );
        printf( "current:%d\n", m_rb_current );
        write( fd, m_video_buffers[m_rb_current].start, m_total_bytes );
        close( fd );
        /* push back fb to queue */
        /* queue the buffer */
        memset( &v4lbuffer, 0x0, sizeof( v4lbuffer ) );
        v4lbuffer.index = m_rb_current;
        v4lbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4lbuffer.memory = V4L2_MEMORY_MMAP;
        
        if( ioctl( m_fd, VIDIOC_QBUF, &v4lbuffer ) < 0 ) {
                perror( "ioctl VIDIOC_QBUF failed!" );
                return -1;
        }
        
        
        return 0;
}

int v4l2_capturer::save_fbdata_to_file_by_mmap( const char *path )
{
        int fd;
        unsigned char *file_base;
        struct v4l2_buffer v4lbuffer;
        LOG_DEBUG( "***save_fbdata_to_file by mmap." );
        fd = open( path, O_RDWR | O_CREAT | O_TRUNC, 0755 );
        
        if( fd < 0 ) {
                perror( "open path failed!" );
                return fd;
        }
        
        /* make file length */
        lseek( fd, m_total_bytes - 1, SEEK_END );
        write( fd, "", 1 ); /* because of COW? */
        printf( "total_bytes:%d\n", m_total_bytes );
        printf( "current:%d\n", m_rb_current );
        /* write operation here */
        file_base = ( unsigned char * )mmap( NULL, m_total_bytes,
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED,
                                             fd, 0 );
                                             
        if( file_base == MAP_FAILED ) {
                perror( "mmap file failed!" );
                return -1;
        }
        
        LOG_DEBUG( "memcpying..." );
        memcpy( file_base, m_video_buffers[m_rb_current].start, m_total_bytes );
        /* release the resource */
        munmap( file_base, m_total_bytes );
        close( fd );
        /* push back fb to queue */
        LOG_DEBUG( "push back fb to queue" );
        memset( &v4lbuffer, 0x0, sizeof( v4lbuffer ) );
        v4lbuffer.index = m_rb_current;
        v4lbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4lbuffer.memory = V4L2_MEMORY_MMAP;
        
        if( ioctl( m_fd, VIDIOC_QBUF, &v4lbuffer ) < 0 ) {
                perror( "ioctl VIDIOC_QBUF failed!" );
                return -1;
        }
        
        return 0;
}


void v4l2_capturer::query_supported_format_new()
{
        struct v4l2_fmtdesc fmt_desc;
        struct v4l2_frmsizeenum frmsize;
        
        if( m_fd < 0 ) {
                printf( "device haven't been openned!\n" );
                return;
        }
        
        memset( &fmt_desc, 0x0, sizeof( fmt_desc ) );
        fmt_desc.index = 0;
        fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        printf( "Format supportted:\n" );
        
        while( ioctl( m_fd, VIDIOC_ENUM_FMT, &fmt_desc ) != -1 ) {
                printf( "\t%d. Type: %s\n", fmt_desc.index, fmt_desc.description );

                m_frmsize.index=0;
                m_frmsize.pixel_format = fmt_desc.pixelformat;
                printf("\tResolutions:\n");
		while( ioctl(m_fd, VIDIOC_ENUM_FRAMESIZES, &m_frmsize ) >= 0 ) {
                        printf("\t\t%d x %d\n", m_frmsize.discrete.width,
                                m_frmsize.discrete.height );
                        m_frmsize.index++;
                }
                fmt_desc.index++;
        }
}
