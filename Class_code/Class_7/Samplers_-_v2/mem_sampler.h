
#ifndef synth_MemSampler_h_
#define synth_MemSampler_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"

#define AUDIO_SAMPLE_RATE_ROUNDED (44118)


class MemSampler :
  public AudioStream
{

  public:
    MemSampler(void) :
      AudioStream(1, inputQueueArray) {
    }

    void frequency(float t_freq) {
      t_freq = norm * t_freq * speed_offset; //when recording straiight into the array the speed needs to be doubled and i dont know why. maybe its due to storing it as 16 isntead of 32 It's not recording at a higher or lower sample rate but the same thign waas happeing w queue hmmmmmm
      if (t_freq < 0.0) t_freq = 0.0;
      else if (t_freq > AUDIO_SAMPLE_RATE_EXACT / 2) t_freq = AUDIO_SAMPLE_RATE_EXACT / 2;
      increment = (t_freq * (0x80000000LL / AUDIO_SAMPLE_RATE_EXACT)) + 0.5;
    }


    void begin(unsigned int *in_sample, uint32_t len) {
      frequency(norm);
      sampleMemory = in_sample;
      sample_len = len - 1; //you cant use sineof on a poniter to fine the size of waht it's pointing to 
      max_rec_len = len - 1;
      start_pos = 0;
      end_mod = len - 1;
      mode = 0;
    }

    void begin(uint16_t *in_sample, uint32_t len) {
      frequency(norm);
      sampleMemory = (unsigned int*)in_sample;
      sample_len = len - 1;
      max_rec_len = len - 1;
      start_pos = 0;
      end_mod = len - 1;
      mode = 0;
    }

    void start_location(uint32_t starts) {
      start_pos = starts;
      if (start_pos >= sample_len) {
        start_pos = sample_len - 255;
      }
    }

    void play_length(uint32_t plength) {
      end_mod = start_pos + plength;
      if (end_mod >= sample_len) {
        end_mod = sample_len;
      }
    }

    void loop(int sl) {
      looping1 = sl;
    }

    void play() {
      index2 = start_pos;
      accumulator = 0;
      mode = 2;
    }

    void record() {
      write_head = 0;
      mode = 1;
    }


    void stop() {
      if (mode == 1) {
        sample_len = write_head;
        end_mod = sample_len;
        Serial.print(" ended ");
        Serial.println(sample_len);
      }
      mode = 0;
      write_head = 0;
      index2 = start_pos;
      accumulator = 0;
      speed_offset = 2.0;
    }

    void reverse(byte sr) {
      rev = sr;
    }

    virtual void update(void);

  private:
    // volatile prevents the compiler optimizing out the frequency function (from wavefrom code)
    volatile uint32_t increment;
    uint32_t accumulator;
    float norm = 86.1679; //normal playback speed
    int speed_offset = 1.0;
    unsigned int *sampleMemory;
    int32_t rev_start, rev_end;
    uint32_t scale;
    audio_block_t *inputQueueArray[1];
    uint32_t sample_len, start_pos, mod_start_pos, end_mod, rev_mod_start_pos, max_rec_len;
    uint32_t index1, index2, prev_index1, end_mod_en;
    int32_t fpos, blocknum;
    uint16_t looping1, playing, rev, bankoff, recording;
    uint32_t write_head;
    uint32_t recorded_length;
    uint32_t print_count;
    byte prev_rev;
    float findex;
    int printcount;
    int prev_mode, mode;
};


#endif
