// Tank controller board
// Mixes steering and throttle channels
// Provide output signals with a ramp
//

#include <Arduino.h>

#define STEERING_PIN 5     // D1
#define THROTTLE_PIN 4     // D2
#define LEFT_MOTOR_PIN 14  // D5
#define RIGHT_MOTOR_PIN 12 // D6

// 3.3v   ________________________________________                 _______
//        |                                       |                |     |
//        |                                       |                |     |
// _______|<-- (MIN, MAX, DEFAULT) PULSE_WIDTH -->|________________|     |____
//
//        |<-- OUTPUT_PULSE_INTERVAL_MICROS ---------------------->|

#define OUTPUT_PULSE_INTERVAL_MICROS 20000

// How much steering (in percent) will be added to channels
#define MIXER_GAIN_1_PERCENT 100 // when throttle is zero (throttle pulse width = DEFAULT_PULSE_WIDTH)
#define MIXER_GAIN_2_PERCENT 10  // when throttle is full (throttle pulse width = MIN_PULSE_WIDTH or throttle pulse width = MAX_PULSE_WIDTH)
#define MIXER_DIRECTION 1        // 1 - steering added to left motor channel, minused from right motor channel, -1 for inversion
#define DEAD_ZONE 2              // dead zone near DEFAULT_PULSE_WIDTH, microseconds
#define RAMP_LIMIT 10            // output signals changing speed, microseconds per OUTPUT_PULSE_INTERVAL_MICROS

const unsigned int DEFAULT_PULSE_WIDTH = 1500;
const unsigned int MAX_PULSE_WIDTH = 1900;
const unsigned int MIN_PULSE_WIDTH = 1100;
const int LEFT_MOTOR_TRIMMER = 0;
const int RIGHT_MOTOR_TRIMMER = 0;

unsigned long steering_pulse_start_micros = 0;
unsigned long throttle_pulse_start_micros = 0;
unsigned long left_motor_pulse_start_micros = 0;
unsigned long right_motor_pulse_start_micros = 0;

unsigned int steering_pulse_width = DEFAULT_PULSE_WIDTH;
unsigned int throttle_pulse_width = DEFAULT_PULSE_WIDTH;
unsigned int left_motor_pulse_width = DEFAULT_PULSE_WIDTH;
unsigned int right_motor_pulse_width = DEFAULT_PULSE_WIDTH;

bool is_steering_pulse = false;
bool is_throttle_pulse = false;
bool is_left_motor_pulse = false;
bool is_right_motor_pulse = false;

unsigned long output_pulse_start_micros = 0;

void read_pulse_width(uint8_t pin, unsigned long *start_micros, unsigned int *pulse_width, bool *is_pulse)
{
  unsigned long current_micros = micros();
  if (digitalRead(pin) == HIGH)
  {
    if (!*is_pulse)
    {
      *is_pulse = true;
      *start_micros = current_micros;
    }
  }
  else
  {
    if (*is_pulse)
    {
      *is_pulse = false;

      *pulse_width = current_micros - *start_micros;
      *start_micros = current_micros;
    }
  }
}

void read_input_signals()
{
  read_pulse_width(STEERING_PIN, &steering_pulse_start_micros, &steering_pulse_width, &is_steering_pulse);
  read_pulse_width(THROTTLE_PIN, &throttle_pulse_start_micros, &throttle_pulse_width, &is_throttle_pulse);
}

void output_pulse_width(uint8_t pin, unsigned int pulse_width)
{
  unsigned long current_micros = micros();
  if (current_micros - output_pulse_start_micros >= pulse_width)
  {
    digitalWrite(pin, LOW);
  }
}

unsigned int normalize_output_pulse_width(unsigned int pulse_width)
{
  if (pulse_width > MAX_PULSE_WIDTH)
  {
    return MAX_PULSE_WIDTH;
  }
  else if (pulse_width <= DEFAULT_PULSE_WIDTH + DEAD_ZONE && pulse_width >= DEFAULT_PULSE_WIDTH - DEAD_ZONE)
  {
    return DEFAULT_PULSE_WIDTH;
  };
  return pulse_width;
}

unsigned int ramp(unsigned int setpoint, unsigned int current_value)
{
  if (current_value >= setpoint)
  {
    if (current_value - setpoint >= RAMP_LIMIT)
    {
      return current_value - RAMP_LIMIT;
    }
  }
  else if (current_value < setpoint)
  {
    if (setpoint - current_value >= RAMP_LIMIT)
    {
      return current_value + RAMP_LIMIT;
    }
  }
  return setpoint;
}

void mix_channels()
{
  long multiplicator = 0;
  if (throttle_pulse_width >= DEFAULT_PULSE_WIDTH)
  {
    multiplicator = map(throttle_pulse_width, DEFAULT_PULSE_WIDTH, MAX_PULSE_WIDTH, MIXER_GAIN_1_PERCENT, MIXER_GAIN_2_PERCENT);
  }
  else
  {
    multiplicator = map(throttle_pulse_width, MIN_PULSE_WIDTH, DEFAULT_PULSE_WIDTH, MIXER_GAIN_2_PERCENT, MIXER_GAIN_1_PERCENT);
  };

  double mix_value = 0;
  if (steering_pulse_width >= DEFAULT_PULSE_WIDTH)
  {
    mix_value = multiplicator * 0.01 * (steering_pulse_width - DEFAULT_PULSE_WIDTH);
  }
  else
  {
    mix_value = -1 * multiplicator * 0.01 * (DEFAULT_PULSE_WIDTH - steering_pulse_width);
  }

  unsigned int left_motor_setpoint = normalize_output_pulse_width(throttle_pulse_width + MIXER_DIRECTION * mix_value + LEFT_MOTOR_TRIMMER);
  unsigned int right_motor_setpoint = normalize_output_pulse_width(throttle_pulse_width - MIXER_DIRECTION * mix_value + RIGHT_MOTOR_TRIMMER);

  left_motor_pulse_width = ramp(left_motor_setpoint, left_motor_pulse_width);
  right_motor_pulse_width = ramp(right_motor_setpoint, right_motor_pulse_width);
}

void output_signals()
{
  unsigned long current_micros = micros();
  if (current_micros - output_pulse_start_micros >= OUTPUT_PULSE_INTERVAL_MICROS)
  {
    mix_channels();
    output_pulse_start_micros = micros();
    digitalWrite(LEFT_MOTOR_PIN, HIGH);
    digitalWrite(RIGHT_MOTOR_PIN, HIGH);
  }

  output_pulse_width(LEFT_MOTOR_PIN, left_motor_pulse_width);
  output_pulse_width(RIGHT_MOTOR_PIN, right_motor_pulse_width);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STEERING_PIN, INPUT);
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(LEFT_MOTOR_PIN, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN, OUTPUT);
}

void loop()
{
  read_input_signals();
  output_signals();
}
