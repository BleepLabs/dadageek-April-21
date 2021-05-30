#include "Arduino.h"
#include "AudioStream.h"
#include "utility/dspinst.h"

#define SAMPLES_PER_MSEC (AUDIO_SAMPLE_RATE_EXACT/1000.0)

class AudioEffectEnvelopeAdjustable : public AudioStream
{
  public:
    AudioEffectEnvelopeAdjustable() : AudioStream(1, inputQueueArray) {
      state = 0;
      delay(0);  // default values...
      attack(10.5f);
      hold(0);
      decay(35.0f);
      sustain(0.5f);
      release(300.0f);
      releaseNoteOn(5.0f);
      shape(0.5f);
    }
    void noteOn();
    void trigger();
    void noteOff();
    void lutSelect(uint16_t *arrayy) {
      lut = arrayy;
    }

    void delay(float milliseconds) {
      delay_count = milliseconds2count(milliseconds);
    }
    void attack(float milliseconds) {
      if (milliseconds < min_millis) {
        milliseconds = min_millis;
      }
      attack_count = milliseconds2count(milliseconds);
      if (attack_count == 0) attack_count = 1;
    }
    void hold(float milliseconds) {
      if (milliseconds < .1) {
        milliseconds = .1; //otherwise there are issues w it not doing decay
      }
      hold_count = milliseconds2count(milliseconds);
    }
    void decay(float milliseconds) {
      if (milliseconds < min_millis) {
        milliseconds = min_millis;
      }
      decay_count = milliseconds2count(milliseconds);
      if (decay_count == 0) decay_count = 1;
    }

    void sustain(float level) {
      if (level < 0.0) level = 0;
      else if (level > 1.0) level = 1.0;
      input_sus_level=level;

    }

    void release(float milliseconds) {
      release_count = milliseconds2count(milliseconds);
      if (release_count == 0) release_count = 1;
    }
    void releaseNoteOn(float milliseconds) {
      release_forced_count = milliseconds2count(milliseconds);
      if (release_count == 0) release_count = 1;
    }

    void shape(float s1) {
      attackShape(s1);
      decayShape(s1);
      releaseShape(s1);
    }

    void attackShape(float s1) {
      if (s1 < -1.0) {
        s1 = -1.0;
      }
      if (s1 > 1.0) {
        s1 = 1.0;
      }
      if (s1 > 0) {
        //the log shape does an odd fade out if it's up too high.
        //musically, I think having a sharp exp is may more useful than a bulbus log anyway.
        s1 *= log_max;
      }
      attack_shape = s1;
    }
    void decayShape(float s1) {
      if (s1 < -1.0) {
        s1 = -1.0;
      }
      if (s1 > 1.0) {
        s1 = 1.0;
      }
      if (s1 > 0) {
        //the log shape does an odd fade out if it's up too high.
        //musically, I think having a sharp exp is may more useful than a bulbus log anyway.
        s1 *= log_max;
      }
      decay_shape = s1;
    }
    void releaseShape(float s1) {
      if (s1 < -1.0) {
        s1 = -1.0;
      }
      if (s1 > 1.0) {
        s1 = 1.0;
      }
      if (s1 > 0) {
        //the log shape does an odd fade out if it's up too high.
        //musically, I think having a sharp exp is may more useful than a bulbus log anyway.
        s1 *= log_max;
      }
      release_shape = s1;
    }

    bool isActive();
    bool isSustain();
    using AudioStream::release;
    virtual void update(void);
  private:
    uint16_t milliseconds2count(float milliseconds) {
      if (milliseconds < min_millis) milliseconds = min_millis;
      uint32_t c = ((uint32_t)(milliseconds * SAMPLES_PER_MSEC) + 7) >> 3;
      if (c > 65535) c = 65535; // allow up to 11.88 seconds
      return c;
    }

    //powf is fast http://openaudio.blogspot.com/2017/02/faster-log10-and-pow.html
    int32_t fscale(float inputValue, float curve) {
      curve = curve * -1.0; //log positive, expo negateive idk thats jsut how it is
      curve = powf(10, curve); // convert linear scale into logarithmic exponent for other pow function
      float normalizedCurVal  =  inputValue / 65600.0;   // normalize to 0 - 1 float. well jsuta little under 1.0
      int32_t rangedValue =  (powf(normalizedCurVal, curve) * 65600.0);
      return (rangedValue);
    }

    int32_t lerpLUT (uint16_t mult_in, float shape_in) {
      float mult_float = mult_in / 255.0;
      int mult_int = mult_float;
      float mult_decimal_rem = mult_float - mult_int;

      int bank_a_sel;
      int bank_b_sel;
      float mult_lerp;
      int offs = 4;
      int dir;
      float shape_float;
      int shape_int;

      if (shape_in < 0) {
        shape_float = abs(shape_in) * 4.0;
        shape_int = shape_float;
        bank_a_sel = shape_int + offs;
        bank_b_sel = bank_a_sel + 1;
      }
      if (shape_in > 0) {

        shape_float = ((shape_in) * (4.0 / log_max));
        shape_int = shape_float;
        bank_a_sel = offs - shape_int;
        bank_b_sel = bank_a_sel - 1;
      }
      float shape_decimal_rem = abs(shape_float) - shape_int;


      if (bank_a_sel > 7) {
        bank_a_sel = 7;
      }
      if (bank_b_sel > 7) {
        bank_b_sel = 7;
      }
      if (bank_a_sel < 0) {
        bank_a_sel = 0;
      }
      if (bank_b_sel < 0) {
        bank_b_sel = 0;
      }
      print_tick_l++;
      if (print_tick_l > 500 && 0) {
        print_tick_l = 0;
        Serial.print(shape_int);
        Serial.print(" ");
        Serial.print(bank_a_sel);
        Serial.print(" ");
        Serial.print(bank_b_sel);
        Serial.print(" ");
        Serial.println(shape_int);
      }

      //float shape_decimal_rem = .5;
      if (mult_int < 255) {
        int ta = (bank_a_sel * 256);
        int mult_pos_a_1 = lut[ta + mult_int];
        int mult_pos_a_2 = lut[ta + mult_int + 1];
        int mult_lerp_a = (mult_pos_a_1 * (1.0 - mult_decimal_rem)) + (mult_pos_a_2 * (mult_decimal_rem));
        int tb = (bank_b_sel * 256);

        int mult_pos_b_1 = lut[tb + mult_int];
        int mult_pos_b_2 = lut[tb + mult_int + 1];
        int mult_lerp_b = (mult_pos_b_1 * (1.0 - mult_decimal_rem)) + (mult_pos_b_2 * (mult_decimal_rem));

        int bank_lerp = (mult_lerp_a * (1.0 - shape_decimal_rem)) + (mult_lerp_b * (shape_decimal_rem));
        mult_lerp = bank_lerp;
      }

      if (mult_int >= 255) {
        mult_lerp = 65535;
      }
      if (mult_int < 1) {
        mult_lerp = 0;
      }
      return mult_lerp;
    }



    audio_block_t *inputQueueArray[1];
    // state
    uint8_t  state, first_rel;     // idle, delay, attack, hold, decay, sustain, release, forced
    uint16_t count;      // how much time remains in this state, in 8 sample units
    int32_t  mult_hires; // attenuation, 0=off, 0x40000000=unity gain
    int32_t  inc_hires;  // amount to change mult_hires every 8 samples

    // settings
    float attack_shape, decay_shape, release_shape, shape_sel;
    uint16_t delay_count;
    uint16_t attack_count;
    uint16_t hold_count;
    uint16_t decay_count;
    int32_t  sustain_mult;
    uint16_t release_count;
    uint16_t release_forced_count;
    uint16_t print_tick, ptb, print_tick_l;
    int32_t pcnt, rel_begin_curve, mt1;
    float recalc_level, sus_level, atten_level;
    float min_millis = .9;
    float log_max = 1.0; //log isn't super useful after .5 i think
    byte triggerMode;
    byte first_calc;
    int32_t curve, start_curve, end_curve, lerp_curve, lerp_curve2;
    int32_t lerpsteps[8];
    int32_t step_size;
    int32_t mult, inc;
    short calc_mode = 1; //0 is fscale , 1 LUT
    uint16_t *lut;
    float input_sus_level;
    float input_attack_len;
    float input_decay_len;
    float input_rel_len;


};
