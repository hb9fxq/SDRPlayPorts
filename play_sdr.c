/*
 * rtl-sdr, turns your Realtek RTL2832 based DVB dongle into a SDR receiver
 * Copyright (C) 2012 by Steve Markgraf <steve@steve-m.de>
 * Copyright (C) 2012-2013 by Hoernchen <la@tfc-server.de>
 *
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
 *
 * ************************************** THIS IS A FORK ******************* ORIGINAL COPYRIGHT SEE ABOVE.
 *
 *


 __/\\\________/\\\__/\\\\\\\\\\\\\________/\\\\\\\\\_____/\\\\\\\\\\\\\\\__/\\\_______/\\\________/\\\_______
 _\/\\\_______\/\\\_\/\\\/////////\\\____/\\\///////\\\__\/\\\///////////__\///\\\___/\\\/______/\\\\/\\\\____
  _\/\\\_______\/\\\_\/\\\_______\/\\\___/\\\______\//\\\_\/\\\_______________\///\\\\\\/______/\\\//\////\\\__
   _\/\\\\\\\\\\\\\\\_\/\\\\\\\\\\\\\\___\//\\\_____/\\\\\_\/\\\\\\\\\\\_________\//\\\\_______/\\\______\//\\\_
    _\/\\\/////////\\\_\/\\\/////////\\\___\///\\\\\\\\/\\\_\/\\\///////___________\/\\\\______\//\\\______/\\\__
     _\/\\\_______\/\\\_\/\\\_______\/\\\_____\////////\/\\\_\/\\\__________________/\\\\\\______\///\\\\/\\\\/___
      _\/\\\_______\/\\\_\/\\\_______\/\\\___/\\________/\\\__\/\\\________________/\\\////\\\______\////\\\//_____
       _\/\\\_______\/\\\_\/\\\\\\\\\\\\\/___\//\\\\\\\\\\\/___\/\\\______________/\\\/___\///\\\_______\///\\\\\\__
        _\///________\///__\/////////////______\///////////_____\///______________\///_______\///__________\//////___

 *
 *
 *
 *
 *  SDRPlayPorts
 *  Ports of some parts of rtl-sdr for the SDRPlay (original: git://git.osmocom.org/rtl-sdr.git /)
 *  2016: Fork by HB9FXQ (Frank Werner-Krippendorf, mail@hb9fxq.ch)
 *
 *  Code changes I've done:
 *  - removed rtl_sdr related calls and replaced them with mir_* to work with the SDRPlay
 *  - removed various local variables
 *
 *  This program is free software: you can redistribute it and/or modify
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
 *
 *
 *
 */

#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include "mirsdrapi-rsp.h"


#define DEFAULT_SAMPLE_RATE         2048000
#define DEFAULT_LNA                 0;
#define DEFAULT_AGC_SETPOINT        -30;
#define DEFAULT_FREQUENCY           100000000;
#define DEFAULT_RESULT_BITS         8; // more compatible with RTL_SDR

static int do_exit = 0;

int             samplesPerPacket;
int             resultBits = DEFAULT_RESULT_BITS;
int             flipComplex = 0;
uint8_t         *buffer8;
short           *buffer16;
FILE            *file;

void*           context = NULL; /* currently unused - let the API the thread stuff */


void adjust_bw(int bwHz, mir_sdr_Bw_MHzT *ptr);
void adjust_if(int ifFreq, mir_sdr_If_kHzT *ptr);
void adjust_result_bits(int bits, int *ptrResultBits);


double atofs(char *s)
/* standard suffixes */
{
    char last;
    int len;
    double suff = 1.0;
    len = strlen(s);
    last = s[len - 1];
    s[len - 1] = '\0';
    switch (last) {
        case 'g':
        case 'G':
            suff *= 1e3;
        case 'm':
        case 'M':
            suff *= 1e3;
        case 'k':
        case 'K':
            suff *= 1e3;
            suff *= atof(s);
            s[len - 1] = last;
            return suff;
    }
    s[len - 1] = last;
    return atof(s);
}


void usage(void) {
    fprintf(stderr,
            "play_sdr, an I/Q recorder for SDRplay RSP receivers\n\n"
                    "Usage:\t -f frequency_to_tune_to (Hz)\n"
                    "\t[-s samplerate (default: 2048000 Hz)]\n"
                    "\t[-b Band width in kHz (default: 1536) possible values: 200 300 600 1536 5000 6000 7000 8000]\n"
                    "\t[-i IF in kHz (default: 0 (=Zero IF)) possible values: 0 450 1620 2048]\n"
                    "\t[-r ADC set point is in dBfs\n"
                    "(decibels relative to full scale) and will normally lie in the range 0dBfs (full scale) to -50dBfs (50dB below\n"
                    "full scale). Default: -30)]\n\n"
                    "\t[-l RSP LNA enable (default: 0, disabled)]\n"
                    "\t[-y Flipcomplex I-Q => Q-I (default: 0, disabled) 1 = enabled\n"
                    "\t[-x Result I/Q bit resolution (uint8 / short) (default: 8, possible values: 8 16)]\n"
                    "\t[-v Verbose mode, prints debug information. Default 0, 1 = enabled\n"
                    "\tfilename (a '-' dumps samples to stdout)\n\n");
    exit(1);
}

void callbackGC(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext)
{
   return;
}

unsigned int firstBufferCallback = 1;

void streamCallback (short *xi, short *xq, unsigned int firstSampleNum, int grChanged, int rfChanged, int fsChanged, unsigned int numSamples, unsigned int reset, void *cbContext){


    if(firstBufferCallback == 1 || reset == 1){

        if (resultBits == 8) {
            buffer8 = malloc(numSamples * 2 * sizeof(uint8_t));
        }
        else {
            buffer16 = malloc(numSamples * 2 * sizeof(short));
        }

        firstBufferCallback = 0;
    }

    int j = 0;
    int i = 0;

    for (i = 0; i < numSamples; i++) {
        if (resultBits == 8) {
            if (flipComplex == 0) {
                buffer8[j++] = (unsigned char) (xi[i] >> 8);
                buffer8[j++] = (unsigned char) (xq[i] >> 8);
            } else {
                buffer8[j++] = (unsigned char) (xq[i] >> 8);
                buffer8[j++] = (unsigned char) (xi[i] >> 8);
            }
        }
        else {
            if (flipComplex == 0) {
                buffer16[j++] = (xi[i]);
                buffer16[j++] = (xq[i]);
            } else {
                buffer16[j++] = (xq[i]);
                buffer16[j++] = (xi[i]);
            }
        }
    }

    if (resultBits == 8) {
        if (fwrite(buffer8, sizeof(uint8_t), numSamples*2, file) != (size_t) numSamples*2) {
            fprintf(stderr, "Short write, samples lost, exiting 8!\n"); // TODO: take firstSampleNum into account
            return;
        }
    }
    else {
        if (fwrite(buffer16, sizeof(short), numSamples * 2, file) != (size_t) numSamples*2) {
            fprintf(stderr, "Short write, samples lost, exiting 16!\n"); // TODO: take firstSampleNum into account
            return;
        }
    }
}

static void sighandler(int signum) {
    fprintf(stderr, "Signal (%d) caught, exiting!\n", signum);
    do_exit = 1;
}

int main(int argc, char **argv) {

    struct sigaction    sigact;
    char *filename = NULL;
    mir_sdr_ErrT r;
    int opt;
    int agcSetPoint = DEFAULT_AGC_SETPOINT;
    int verbose = 0;

    uint32_t frequency = DEFAULT_FREQUENCY;
    uint32_t samp_rate = DEFAULT_SAMPLE_RATE;
    int rspLNA = DEFAULT_LNA;

    int i, j;

    mir_sdr_Bw_MHzT bandwidth = mir_sdr_BW_1_536;
    mir_sdr_If_kHzT ifKhz = mir_sdr_IF_Zero;

    while ((opt = getopt(argc, argv, "f:g:s:n:l:b:i:x:y:v:")) != -1) {
        switch (opt) {
            case 'f':
                frequency = (uint32_t) atofs(optarg);
                break;
            case 'g':
                agcSetPoint = (int) atof(optarg);
                break;
            case 's':
                samp_rate = (uint32_t) atofs(optarg);
                break;
            case 'l':
                rspLNA = atoi(optarg);
                break;
            case 'b':
                adjust_bw(atoi(optarg), &bandwidth);
                break;
            case 'i':
                adjust_if(atoi(optarg), &ifKhz);
                break;
            case 'x':
                adjust_result_bits(atoi(optarg), &resultBits);
                break;
            case 'y':
                flipComplex = atoi(optarg);
                break;
            case 'v':
                verbose = atoi(optarg);
                break;
            default:
                usage();
                break;
        }
    }

    if (argc <= optind) {
        usage();
    } else {
        filename = argv[optind];
    }

    if (verbose == 1) {
        fprintf(stderr, "[DEBUG] *************** play_sdr init summary *********************\n");
        fprintf(stderr, "[DEBUG] LNA: %d\n", rspLNA);
        fprintf(stderr, "[DEBUG] samp_rate: %d\n", samp_rate);
        fprintf(stderr, "[DEBUG] agcSetPoint: %d\n", agcSetPoint);
        fprintf(stderr, "[DEBUG] frequency: [Hz] %d / [MHz] %f\n", frequency, frequency / 1e6);
        fprintf(stderr, "[DEBUG] bandwidth: [kHz] %d\n", bandwidth);
        fprintf(stderr, "[DEBUG] IF: %d\n", ifKhz);
        fprintf(stderr, "[DEBUG] Result I/Q bit resolution (bit): %d\n", resultBits);
        fprintf(stderr, "[DEBUG] *************************************************************\n");
    }


    sigact.sa_handler = sighandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGPIPE, &sigact, NULL);


    if (strcmp(filename, "-") == 0) { /* Write samples to stdout */
        file = stdout;
    } else {
        file = fopen(filename, "wb");
        if (!file) {
            fprintf(stderr, "Failed to open %s\n", filename);
            goto out;
        }
    }


    mir_sdr_AgcControl(1, agcSetPoint, 0, 0, 0, 0, rspLNA);

    int infoOverallGr;
    r = mir_sdr_StreamInit(&agcSetPoint, (samp_rate/1e6), (frequency/1e6), mir_sdr_BW_1_536, mir_sdr_IF_Zero, rspLNA, &infoOverallGr, 1 /* use internal gr tables acording to band */, &samplesPerPacket, streamCallback,
                       callbackGC, &context);


    if (r != mir_sdr_Success) {
        fprintf(stderr, "Failed to start SDRplay RSP device.\n");
        exit(r);
    }


    mir_sdr_SetDcMode(4, 0);
    mir_sdr_SetDcTrackTime(63);


    fprintf(stderr, "Writing samples...\n");
    while (!do_exit) {
        sleep(1);
    }

    if (do_exit)
        fprintf(stderr, "\nUser cancel, exiting...\n");
    else
        fprintf(stderr, "\nLibrary error %d, exiting...\n", r);

    if (file != stdout)
        fclose(file);


    if (resultBits == 8) {
        free(buffer8);
    }
    else {
        free(buffer16);
    }

    out:
    return r >= 0 ? r : -r;
}

void adjust_result_bits(int bits, int *ptrResultBits) {
    switch (bits) {
        case 8:
        case 16:
            *ptrResultBits = bits;
            return;
    }

    fprintf(stderr, "Invalid result I/Q resolution (-x) !\n");
    usage();
}

void adjust_if(int ifFreq, mir_sdr_If_kHzT *ptrIFKhz) {
    switch (ifFreq) {
        case 0:
        case 450:
        case 1620:
        case 2048:
            *ptrIFKhz = ifFreq;
            return;
    }

    fprintf(stderr, "Invalid IF frequency (-i) !\n");
    usage();

}

void adjust_bw(int bwHz, mir_sdr_Bw_MHzT *ptrBw) {

    switch (bwHz) {
        case 200:
        case 300:
        case 600:
        case 1536:
        case 5000:
        case 6000:
        case 8000:
            *ptrBw = bwHz;
            return;
    }

    fprintf(stderr, "Invalid Band Width (-b) !\n");
    usage();
}