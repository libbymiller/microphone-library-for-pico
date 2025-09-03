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

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#include "pico/pdm_microphone.h"

#include "usb_microphone.h"

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
uint16_t sample_buffer[SAMPLE_BUFFER_SIZE];

// callback functions
void on_pdm_samples_ready();
void on_usb_microphone_tx_ready();

int main(void)
{
  stdio_init_all();
  printf(__FILE__ " at " __DATE__ " " __TIME__ " Started.\n");

  printf("Sampling rate: %d\n", SAMPLE_RATE);

  // initialize and start the PDM microphone
  pdm_microphone_init(&config);
  pdm_microphone_set_samples_ready_handler(on_pdm_samples_ready);
  pdm_microphone_start();

  // initialize the USB microphone interface
  usb_microphone_init();
  usb_microphone_set_tx_ready_handler(on_usb_microphone_tx_ready);

  printf("entering loop.\n");

  for(uint64_t i = 0; 1; i++) {
#if 0
    static uint32_t lst = 0;
    uint32_t n = to_ms_since_boot(get_absolute_time());
    if (lst == 0 || n - lst > 1000) {
	printf("d, ",i);
	lst = n;
	};
#endif
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
  pdm_microphone_read(sample_buffer, SAMPLE_BUFFER_SIZE);
}

void on_usb_microphone_tx_ready()
{
  // Callback from TinyUSB library when all data is ready
  // to be transmitted.
  //
  // Write local buffer to the USB microphone
  usb_microphone_write(sample_buffer, sizeof(sample_buffer));
}
