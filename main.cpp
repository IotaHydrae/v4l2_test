/**
 * @file main.cpp
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

#include <iostream>
#include <time.h>
#include <signal.h>
#include "v4l2_capturer.h"

v4l2_capturer capturer;

void capturer_sigint_handler( int signal )
{
        capturer.stop();
        exit( 1 );
}

/* TODO: listen SIGINT and call capturer.stop() when it come. */
int main( int argc, char const *argv[] )
{
        int frame_count = 0;
        std::cout << "Hello World!" << std::endl;
        
        /* register signal handler function. */
        signal( SIGINT, capturer_sigint_handler );
        
        capturer.init();
        capturer.query_supported_format_new();
        capturer.start();
        
        /*while(1){
            frame_count=capturer.get_frame();
            if(frame_count==25*10)break;
        }*/
        
        capturer.get_frame();
        capturer.stop();
        return 0;
}
