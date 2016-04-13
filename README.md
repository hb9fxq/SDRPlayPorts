# SDRPlayPorts
Ports of some parts of git://git.osmocom.org/rtl-sdr.git / http://sdr.osmocom.org/trac/wiki/rtl-sdr for the SDRPlay (http://www.sdrplay.com/) 


Incomplete!!! Actually not working yet ... Work in progress
Prototype demo:
* https://youtu.be/rDvxwpe5HT8 rtl_tcp first steps
* https://youtu.be/aYnz0Auqwho play_sdr first steps

# Installation

1. SDRPlay API
<br>Download the SDRPlay linux api.
* x86 32/64bit: http://www.sdrplay.com/linux.html
* RaspberryPI: http://www.sdrplay.com/raspberrypi2.html

and follow the instructions.

2. Build SDRPlayPorts:
<pre>
git clone https://github.com/krippendorf/SDRPlayPorts.git
cd SDRPlayPorts
mkdir build && cd build
cmake ..
make
sudo make install
</pre>

# Todo
* Test, refactor and enhance ;-)

## Samples
* Use with OpenWebRx / Config file:

```python
# ==== DSP/RX settings ====
dsp_plugin="csdr"
fft_fps=9
fft_size=4096*2
samp_rate = 1024000
center_freq = 3600000
rf_gain = 35 #in dB. For an RTL-SDR, rf_gain=0 will set the tuner to auto gain mode, else it will be in manual gain mode.
ppm = 0

audio_compression="adpcm" #valid values: "adpcm", "none"
fft_compression="adpcm" #valid values: "adpcm", "none"

start_rtl_thread=True

# ==== I/Q sources (uncomment the appropriate) ====

# >> RTL-SDR via rtl_sdr

start_rtl_command="play_sdr -b 600 -s {samp_rate} -f {center_freq} -x 16 -g {rf_gain} -y 0 -".format(rf_gain=rf_gain, center_freq=center_freq, samp_rate=samp_rate, ppm=ppm)
format_conversion="csdr convert_s16_f"

```
![SDRPlay with OpenWebRX, 16bit option set](https://raw.githubusercontent.com/krippendorf/SDRPlayPorts/master/doc/img/openwebrxcfg.png)
* Use with Baudline


```bash
timeout 15s play_sdr -s 8000000 -b 600 -f 3.6M -g 35 -l 0 -x 16 8000000_16bit.raw
```
![SDRPlay with Baudline, 16bit option set](https://raw.githubusercontent.com/krippendorf/SDRPlayPorts/master/doc/img/baudlinecfg_1.png)



# License

##SDRPlayPorts Licence


 SDRPlayPorts
 Ports of some parts of rtl-sdr for the SDRPlay (git://git.osmocom.org/rtl-sdr.git /)
 Fork by HB9FXQ (Frank Werner-Krippendorf, mail@hb9fxq.ch)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.



##rtl-sdr Licence


 rtl-sdr, turns your Realtek RTL2832 based DVB dongle into a SDR receiver
 Copyright (C) 2012 by Steve Markgraf <steve@steve-m.de>
 Copyright (C) 2012-2013 by Hoernchen <la@tfc-server.de>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


