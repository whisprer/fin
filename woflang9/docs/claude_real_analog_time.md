\#pragma once



\#include "analog\_modes.hpp"

\#include <chrono>

\#include <immintrin.h>

\#include <cmath>



namespace woflang {



// --- Time State for Analog Simulation ---

struct TimeState {

&nbsp;   float dt = 0.001f;  // Default 1ms time step

&nbsp;   float current\_time = 0.0f;

&nbsp;   bool auto\_advance = false;

&nbsp;   

&nbsp;   void set\_timestep(float new\_dt) {

&nbsp;       dt = (new\_dt > 0.0f) ? new\_dt : 0.001f;

&nbsp;   }

&nbsp;   

&nbsp;   void advance() {

&nbsp;       current\_time = clamp\_analog(current\_time + dt);

&nbsp;   }

&nbsp;   

&nbsp;   void reset() {

&nbsp;       current\_time = 0.0f;

&nbsp;   }

};



inline TimeState time\_state;



// --- Analog Integration (Euler method) ---

inline float analog\_integrate(float current\_value, float derivative) {

&nbsp;   float result = current\_value + (derivative \* time\_state.dt);

&nbsp;   return clamp\_analog(result);

}



// --- SIMD Integration ---

inline \_\_m256 analog\_integrate(\_\_m256 current\_value, \_\_m256 derivative) {

&nbsp;   \_\_m256 dt\_vec = \_mm256\_set1\_ps(time\_state.dt);

&nbsp;   \_\_m256 increment = \_mm256\_mul\_ps(derivative, dt\_vec);

&nbsp;   \_\_m256 result = \_mm256\_add\_ps(current\_value, increment);

&nbsp;   return clamp\_analog(result);

}



// --- Analog Differentiation (simple backward difference) ---

inline float analog\_differentiate(float current\_value, float previous\_value) {

&nbsp;   if (time\_state.dt == 0.0f) return clamp\_analog(0.0f);

&nbsp;   float result = (current\_value - previous\_value) / time\_state.dt;

&nbsp;   return clamp\_analog(result);

}



// --- SIMD Differentiation ---

inline \_\_m256 analog\_differentiate(\_\_m256 current\_value, \_\_m256 previous\_value) {

&nbsp;   \_\_m256 dt\_vec = \_mm256\_set1\_ps(time\_state.dt);

&nbsp;   \_\_m256 diff = \_mm256\_sub\_ps(current\_value, previous\_value);

&nbsp;   \_\_m256 result = \_mm256\_div\_ps(diff, dt\_vec);

&nbsp;   return clamp\_analog(result);

}



// --- RC Circuit Simulation (exponential decay) ---

inline float analog\_rc\_step(float current\_voltage, float target\_voltage, float rc\_constant) {

&nbsp;   if (rc\_constant <= 0.0f) return clamp\_analog(target\_voltage);

&nbsp;   

&nbsp;   float alpha = time\_state.dt / rc\_constant;

&nbsp;   if (alpha > 1.0f) alpha = 1.0f;  // Prevent overshoot

&nbsp;   

&nbsp;   float result = current\_voltage + alpha \* (target\_voltage - current\_voltage);

&nbsp;   return clamp\_analog(result);

}



// --- SIMD RC Circuit ---

inline \_\_m256 analog\_rc\_step(\_\_m256 current\_voltage, \_\_m256 target\_voltage, \_\_m256 rc\_constant) {

&nbsp;   \_\_m256 dt\_vec = \_mm256\_set1\_ps(time\_state.dt);

&nbsp;   \_\_m256 alpha = \_mm256\_div\_ps(dt\_vec, rc\_constant);

&nbsp;   \_\_m256 one = \_mm256\_set1\_ps(1.0f);

&nbsp;   alpha = \_mm256\_min\_ps(alpha, one);  // Clamp alpha to 1.0

&nbsp;   

&nbsp;   \_\_m256 diff = \_mm256\_sub\_ps(target\_voltage, current\_voltage);

&nbsp;   \_\_m256 increment = \_mm256\_mul\_ps(alpha, diff);

&nbsp;   \_\_m256 result = \_mm256\_add\_ps(current\_voltage, increment);

&nbsp;   return clamp\_analog(result);

}



// --- Simple Oscillator (sine wave generator) ---

struct AnalogOscillator {

&nbsp;   float phase = 0.0f;

&nbsp;   float frequency = 1.0f;  // Hz

&nbsp;   

&nbsp;   float step() {

&nbsp;       float two\_pi = 6.28318530717958647692f;

&nbsp;       phase += two\_pi \* frequency \* time\_state.dt;

&nbsp;       

&nbsp;       // Wrap phase to prevent accumulation errors

&nbsp;       while (phase >= two\_pi) phase -= two\_pi;

&nbsp;       while (phase < 0.0f) phase += two\_pi;

&nbsp;       

&nbsp;       float result = sinf(phase);

&nbsp;       return clamp\_analog(result);

&nbsp;   }

&nbsp;   

&nbsp;   void set\_frequency(float freq) {

&nbsp;       frequency = clamp\_analog(freq);

&nbsp;   }

&nbsp;   

&nbsp;   void reset\_phase() {

&nbsp;       phase = 0.0f;

&nbsp;   }

};



} // namespace woflang

