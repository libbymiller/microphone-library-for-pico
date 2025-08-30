/*
 * Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * This examples creates a USB Microphone device using the TinyUSB
 * library and captures data from a PDM microphone using a sample
 * rate of 16 kHz, to be sent the to PC.
 * 
 * The USB microphone code is based on the TinyUSB audio_test example.
 * 
 * https://github.com/hathach/tinyusb/tree/master/examples/device/audio_test
 */

#include "pico/pdm_microphone.h"
#include "usb_microphone.h"

#include "hardware/clocks.h"
#include "hardware/pll.h"

// High pass filter from pipistrello / https://www.pippyg.com 
// analogue microphone code.
//
#include "filter.h"

// configuration
const struct pdm_microphone_config config = {
  .gpio_data = 2,
  .gpio_clk = 3,
  .pio = pio0,
  .pio_sm = 0,
  .sample_rate = SAMPLE_RATE,
  .sample_buffer_size = SAMPLE_BUFFER_SIZE,
};

// variables
int16_t sample_buffer[SAMPLE_BUFFER_SIZE];
int16_t out_buffer[SAMPLE_BUFFER_SIZE];

// callback functions
void on_pdm_samples_ready();
void on_usb_microphone_tx_ready();

int
setCPUclock(const int32_t kHz)
{
    uint vco_freq;
    uint post_div1;
    uint post_div2;
    
    printf ( "setCPUclock : kHz %d\n", kHz );
    
    if (check_sys_clock_khz(kHz, &vco_freq, &post_div1, &post_div2)) {
        printf ( "check_sys_clock_khz : PASSED : kHz %d\n", kHz );
        clock_configure(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                        48 * MHZ,
                        48 * MHZ);
        
        pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
        uint32_t freq = vco_freq / (post_div1 * post_div2);
        
        // Configure clocks
        // CLK_REF = XOSC (12MHz) / 1 = 12MHz
        clock_configure(clk_ref,
                        CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                        0, // No aux mux
                        12 * MHZ,
                        12 * MHZ);
        
        clock_configure(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                        freq, freq);
        
        // DO NOT SET PERIPHERAL CLOCK - gives me more SPI flexibility
        return 1;
    }
    else {
        printf ( "check_sys_clock_khz : FAILED : kHz %d\n", kHz );
        return 0;
    }
}

int main(void)
{
  setCPUclock(2*108000); // seems to minimize energy bands

  // initialize and start the PDM microphone
  pdm_microphone_init(&config);
  pdm_microphone_set_samples_ready_handler(on_pdm_samples_ready);
  pdm_microphone_start();

  // initialize the USB microphone interface
  usb_microphone_init();
  usb_microphone_set_tx_ready_handler(on_usb_microphone_tx_ready);

  // Initalise filter from pipistrello / https://www.pippyg.com 
  // analogue microphone code.
  //
  HPFilterInit(12);

  while (1) {
    // run the USB microphone task continuously
    usb_microphone_task();
  }

  return 0;
}

void on_pdm_samples_ready()
{
  // Callback from library when all the samples in the library
  // internal sample buffer are ready for reading.
  //
  // Read new samples into local buffer.

  // Above 192 kHz sampling- this call takes too much
  // time. It does the PDM thing in software.
  //
  pdm_microphone_read(sample_buffer, SAMPLE_BUFFER_SIZE);
}

void on_usb_microphone_tx_ready()
{
  // Callback from TinyUSB library when all data is ready
  // to be transmitted.
  //

#if 0
  // Do not apply a filter - pass straight on.
  //
  for(uint32_t i = 0; i < SAMPLE_BUFFER_SIZE; i++) 
	out_buffer[i] = sample_buffer[i];
#endif

#if 1
  // Apply a high pass filter.
  //
  HPFilterBuffer (out_buffer, sample_buffer, SAMPLE_BUFFER_SIZE);
#endif

#if 0
  // Simpy generate a square wave - every (1<<3) == 8 samples.
  //
  //730 hz -- 1<<5, 48Khz -- 48khz / 2x32 =~ 750 Hz ??
  //          1<<6  96khz  ok
  //          1<<7  192kHz ok
  //          1<<8  384kHz ok withouth microphone polling

  static uint32_t j = 0;
  for(uint32_t i = 0; i < SAMPLE_BUFFER_SIZE; i++, j++) 
	out_buffer[i] = 0x8000 + ((j & (1<<3)) ? 0x500 : -0x500) + 3*(i & 0xFF);

#endif

  // Write local buffer to the USB microphone
  //
  usb_microphone_write(out_buffer, sizeof(out_buffer));
}
