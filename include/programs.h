#ifndef PROGRAMS_H
#define PROGRAMS_H

#include "effects.h"

// Define a uniform function pointer type for programs (no-arg functions)
using ProgramFn = void (*)();

// Program indices - keep these in sync with the PROGRAMS array below
enum ProgramIndex {
    PRG_FIRE_OFF = 0,
    PRG_RANDOM,
    PRG_FLOATING,
    PRG_FIRE_ON,
    PRG_SPIRAL,
    PRG_SPIRAL_MIRRORED,
    PRG_PULSES,
};

// Programs array: add new programs here. The array length determines
// PROGRAMS_COUNT so swapping/wrapping can be automatic.
static const ProgramFn PROGRAMS[] = {
    FireOff,           // PRG_FIRE_OFF
    Random,            // PRG_RANDOM
    Floating,          // PRG_FLOATING
    FireOn,            // PRG_FIRE_ON
    Spiral,            // PRG_SPIRAL
    SpiralMirrored,    // PRG_SPIRAL_MIRRORED
    Pulses             // PRG_PULSES
};

#define PROGRAMS_COUNT (sizeof(PROGRAMS) / sizeof(PROGRAMS[0]))

#endif // PROGRAMS_H
