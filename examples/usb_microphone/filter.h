/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * (c) Phil Atkin 2022-2024 as part of * the Pippitrelle code on 
 * https://www.pippyg.com.
 *
 * Seperated out high pass filter (dirkx, August 2025) to reuse with a
 * directly connected *a PDM microphone (the Pipistrello uses an analog 
 * microphone with an analog pre-amplifier).
 */

#ifndef _PICO_HIGHPASS_FILTER_HPP_
#define _PICO_HIGHPASS_FILTER_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#if HPF_DEBUG
void dumbHPFbuffer ( short *outbuf, const short *inbuf, const int n );
#endif

// High pass filter
//
//  Possible value for the filter are:
//   3:		for  -3 dB
//   6:		for  -6 dB
//   9:		for  -9 dB
//  12: 	for   12 dB
//  32:		for   32 dB
//  Any other value will default to -3 dB
//
void HPFilterInit(int32_t dBcut);

// this one is in use HPF followed by bandCut around 24kHz to suppress noisy microphone, 12-bits in, shorts out
//
void HPFilterBuffer (short *outp, const short *inp, int32_t n );

#define HPFCOEFF_DEFAULT (32)
void setHPFcoeff ( int32_t c);

#endif
