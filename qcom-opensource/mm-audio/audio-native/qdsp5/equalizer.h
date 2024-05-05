/* equalizer.h - native equalizer test application header
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2010, The Linux Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define FILTER_GAIN_MAX	6
#define FILTER_GAIN_MIN	-6

#define MAX_PRESETS	17
#define MAX_BAND_COUNT	12

enum equalizers {
    NONE,
    BASS_BOOST,
    BASS_CUT,
    TREBLE_BOOST,
    TREBLE_CUT,
    BAND_BOOST,
    BAND_CUT,
};

enum eq_presets {
    BLANK,
    CLUB,
    DANCE,
    FULLBASS,
    FULLBASSTREBLE,
    FULLTREBLE,
    LAPTOP,
    LARGEHALL,
    LIVE,
    PARTY,
    POP,
    REGGAE,
    ROCK,
    SKA,
    SOFT,
    SOFTROCK,
    TECHNO,
};

int set_pcm_default_eq_values(int fd, int preset);

