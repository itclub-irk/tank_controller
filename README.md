# Tank controller

Firmware for esp8266 microcontroller. Can be used for controlling tank-like RC models (with separated left and right motor channels) using throttle and steering PWM channels. Provide output signals with a ramp.

## Configuration

### 1. PWM settings (endpoints, frequency)

Define endpoints using this constants. This values will be applied both input and output PWM signals

```c
const unsigned int DEFAULT_PULSE_WIDTH = 1500; // Pulse width when input /  output is 0%
const unsigned int MAX_PULSE_WIDTH = 1900; // Pulse width when input / output is 100%
const unsigned int MIN_PULSE_WIDTH = 1100; // Pulse width when input / output is -100%
```

Set output channels pulse period (microseconds) using this definition

```c
#define OUTPUT_PULSE_INTERVAL_MICROS 20000
```

### 2. Trimming

Set trimmers values (microseconds) using this definition. Trimer values will be added to corresponding motor channel pulse width

```c
const int LEFT_MOTOR_TRIMMER = 0;
const int RIGHT_MOTOR_TRIMMER = 0;
```

### 3. Mixing

Firmware mixes throttle and steering channels. Output signals is equal to the sum or difference of throttle and steering signal (depends on direction of steering wheel). Steering wheel direction can be reversed using this definition (1 for normal, -1 for reversed)

```c
#define MIXER_DIRECTION 1
```

As the throttle value increases, steering value weight decreases. Endpoint weights can be defined using this definition

```c
#define MIXER_GAIN_1_PERCENT 100 // Steering weight in percent when throttle is zero
#define MIXER_GAIN_2_PERCENT 10 // Steering weight in percent when full throttle
```

### 4. Output ramp

Output signals has a ramp function. Signal changing speed (in microseconds per OUTPUT_PULSE_INTERVAL_MICROS) can be specified using this definition

```c
#define RAMP_LIMIT 10
```
