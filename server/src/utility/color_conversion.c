// Copyright (c) 2014, Jan Winkler <winkler@cs.uni-bremen.de>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Universität Bremen nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/* Author: Jan Winkler */
/* Modified by Erick Folckemer to run in C18*/
#include "color_conversion.h"
#include <stdio.h>
#include <math.h>

void RGBtoHSV(const RGBColor* in_rgb, HSVColor* out_hsv)
{
    float fCMax = fmax(fmax(in_rgb->R, in_rgb->G), in_rgb->B);
    float fCMin = fmin(fmin(in_rgb->R, in_rgb->G), in_rgb->B);
    float fDelta = fCMax - fCMin;

    if (fDelta > 0)
    {
        if (fCMax == in_rgb->R)
        {
            out_hsv->H = 60 * (fmod(((in_rgb->G - in_rgb->B) / fDelta), 6));
        }
        else if (fCMax == in_rgb->G)
        {
            out_hsv->H = 60 * (((in_rgb->B - in_rgb->R) / fDelta) + 2);
        }
        else if (fCMax == in_rgb->B)
        {
            out_hsv->H = 60 * (((in_rgb->R - in_rgb->G) / fDelta) + 4);
        }

        if (fCMax > 0)
        {
            out_hsv->S = fDelta / fCMax;
        }
        else
        {
            out_hsv->S = 0;
        }

        out_hsv->V = fCMax;
    }
    else
    {
        out_hsv->H = 0;
        out_hsv->S = 0;
        out_hsv->V = fCMax;
    }

    if (out_hsv->H < 0)
    {
        out_hsv->H = 360 + out_hsv->H;
    }
}

void HSVtoRGB(const HSVColor* in_hsv, RGBColor* out_rgb)
{
    float fC = in_hsv->V * in_hsv->S; // Chroma
    float fHPrime = fmod(in_hsv->H / 60.0, 6);
    float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    float fM = in_hsv->V - fC;

    if (0 <= fHPrime && fHPrime < 1)
    {
        out_rgb->R = fC;
        out_rgb->G = fX;
        out_rgb->B = 0;
    }
    else if (1 <= fHPrime && fHPrime < 2)
    {
        out_rgb->R = fX;
        out_rgb->G = fC;
        out_rgb->B = 0;
    }
    else if (2 <= fHPrime && fHPrime < 3)
    {
        out_rgb->R = 0;
        out_rgb->G = fC;
        out_rgb->B = fX;
    }
    else if (3 <= fHPrime && fHPrime < 4)
    {
        out_rgb->R = 0;
        out_rgb->G = fX;
        out_rgb->B = fC;
    }
    else if (4 <= fHPrime && fHPrime < 5)
    {
        out_rgb->R = fX;
        out_rgb->G = 0;
        out_rgb->B = fC;
    }
    else if (5 <= fHPrime && fHPrime < 6)
    {
        out_rgb->R = fC;
        out_rgb->G = 0;
        out_rgb->B = fX;
    }
    else
    {
        out_rgb->R = 0;
        out_rgb->G = 0;
        out_rgb->B = 0;
    }

    out_rgb->R += fM;
    out_rgb->G += fM;
    out_rgb->B += fM;
}

void color_conversion_test()
{
    RGBColor rgb = {0.0f, 0.0f, 0.0f};
    HSVColor hsv = {146.0f, 0.19f, 0.66f};
    HSVtoRGB(&hsv, &rgb);
    RGBtoHSV(&rgb, &hsv);
    printf("[RGB] - %f %f %f", rgb.R, rgb.G, rgb.B);
    printf("[HSV] - %f %f %f", hsv.H, hsv.S, hsv.V);

    rgb.R = 136.0;
    rgb.G = 168.0;
    rgb.B = 150.0;    
    RGBtoHSV(&rgb, &hsv);
    HSVtoRGB(&hsv, &rgb);
    printf("[RGB] - %f %f %f", rgb.R, rgb.G, rgb.B);
    printf("[HSV] - %f %f %f", hsv.H, hsv.S, hsv.V);
}