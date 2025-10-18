#ifndef EFFECTS_H
#define EFFECTS_H

#include "config.h"
#include "led_panel.h"

// Effects API used by Nano_crystal.cpp
void Random();
void Fire(bool flip);
void Floating();
void RandomizeTime();
void CoolAll(double max, double min);
uint8_t wrap(uint8_t num, uint8_t limit);
void Spread(double factor);
void SpreadHeight(double factor);
void UpdateLedHeat(int row, int col);

// Effects -> runtime API: request FPS changes. Nano_crystal reads and applies
// the delta each loop via effects_get_and_clear_fps_delta().
void effects_init();
double effects_get_and_clear_fps_delta();

#endif // EFFECTS_H
