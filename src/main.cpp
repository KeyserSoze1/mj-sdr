/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <complex>
#include <iostream>
#include <thread>
#include <vector>

#include <rtl-sdr.h>

#include "CLI11.hpp"

#define U8_F(x) ( (((float)(x)) - 127.4f) / 128.0f ) // 127.4 for tuner DC bias in rtl
#define SAMPLE_RATE 2048000
#define CENTER_FREQ 1575420000
#define GAIN_MODE 1 // 0 = Automatic, 1 = Manual
// Supported manual gain values: 0.0 0.9 1.4 2.7 3.7 7.7 8.7 12.5 14.4 15.7 16.6 19.7
//                               20.7 22.9 25.4 28.0 29.7 32.8 33.8 36.4 37.2 38.6 40.2
//                               42.1 43.4 43.9 44.5 48.0 49.6
#define GAIN 402 // Manual gain value in tenths of dB

typedef std::complex<float> cfloat;

void input_cb(uint8_t *buf, uint32_t length, void *arg)
{    
    // TODO: Move function to another file
    
    assert(length % 2 == 0);
    
    uint32_t num_samples = length / 2;
    std::vector<cfloat> converted_samples(num_samples);
    for (uint32_t i = 0; i < num_samples; i++)
    {
        converted_samples.push_back(cfloat(U8_F(buf[i*2]), U8_F(buf[i*2 + 1])));
    }
    
    // TODO: Create a processing function for the converted samples
    std::cout << "Number of samples: " << num_samples << "\n";
}

void read_samples(rtlsdr_dev_t *dev)
{
    rtlsdr_read_async(dev,
                      input_cb, // Function for reading samples
                      NULL, // Used to pass something to the callback function
                      0, // Default number of buffers
                      0); // Default buffer size
}

int main(int argc, char *argv[])
{
    rtlsdr_dev_t *dev;
    int retval = 0;

    // Parse command line
    CLI::App app{"mj-sdr"};
    int sample_rate = SAMPLE_RATE;
    int center_freq = CENTER_FREQ;
    int gain_mode = GAIN_MODE;
    int gain = GAIN;
    int device = 0;

    app.add_option("-d,--device", device, "RTL-SDR device id", true);
    app.add_option("-s,--sample_rate", sample_rate, "Sampling Rate in Hz", true);
    app.add_option("-c,--center_freq", center_freq, "Center Frequency in Hz", true);
    app.add_option("-m,--gain_mode", gain_mode, "Gain Mode (0=Automatic, 1=Manual)", true);
    app.add_option("-g,--gain", gain, "Manual gain in tenths of dB", true);

    try 
    {
        app.parse(argc, argv);
    } 
    catch (const CLI::ParseError &e) 
    {
        return app.exit(e);
    }
	
    // Find rtl-sdr devices
    int devices = rtlsdr_get_device_count();
    for(int n=0; n < devices; n++)
    {
        std::cout << "Device " << n << ": " << rtlsdr_get_device_name(n) << "\n\n";
    }

    if (devices <= 0)
    {
        std::cout << "No Devices Found!\n";
        return 0;
    }

    retval = rtlsdr_open(&dev, device);
    if (retval != 0)
    {
        std::cout << "Failed to open device: " << device << "\n";
        return 0;
    }
	
    // Configure rtlsdr settings
    retval = rtlsdr_set_sample_rate(dev, sample_rate);
    if (retval == 0) std::cout << "Sample rate set to: " << sample_rate << " Hz\n";
    else std::cout << "Setting sample rate failed!\n";
    
    retval = rtlsdr_set_center_freq(dev, center_freq);    
    if (retval == 0) std::cout << "Center frequency set to: " << center_freq << " Hz\n";
    else std::cout << "Setting center frequency failed!\n";
    
    retval = rtlsdr_set_tuner_gain_mode(dev, gain_mode);
    if (retval == 0) std::cout << "Gain mode set to: " << ((gain_mode == 0) ? "Automatic\n" : "Manual\n");
    else std::cout << "Setting gain mode failed!\n";
    
    if (gain_mode != 0)
    {    
        retval = rtlsdr_set_tuner_gain(dev, gain);
        if (retval == 0) std::cout << "Set gain to: " << gain / 10.0f << " dB\n";
        else std::cout << "Setting gain failed!\n";
    } 
    
    retval = rtlsdr_reset_buffer(dev);
	
    // Start reading samples
    std::thread sample_reader(read_samples, dev);
	
    // Wait for user to press q to quit 
    char input_char = 0;
    while (input_char != 'q')
    {
        std::cin >> input_char;
    }
	
    // Cancel sample reading and quit
    rtlsdr_cancel_async(dev);
    sample_reader.join();
    rtlsdr_close(dev);
    return 0;
}