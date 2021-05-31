//may 25 21 changed all uint32_t to int32_t just in case

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"
#include "spi_interrupt.h"

class AudioEffectTapeDelay :
  public AudioStream
{
  public:
    AudioEffectTapeDelay(void):
      AudioStream(1, inputQueueArray) {
    }
    void begin(short *delayline, int32_t max_len, int32_t dly_len, short redux, short lerp);
    int32_t length(int32_t dly_len);
    int32_t length_no_lerp(int32_t dly_len);
    void sampleRate(byte redux);
    void interpolation(byte lerp);
    virtual void update(void);

  private:
    int32_t dlyd, dlyt;
    audio_block_t *inputQueueArray[1];
    short *l_delayline;
    int32_t delay_length, desired_delay_length;
    int32_t inv_delay_length;
    int32_t max_dly_len;
    int32_t write_head;
    int32_t delay_depth;
    int32_t rate_redux;
    int32_t delay_offset_idx;
    int32_t   delay_rate_incr;
    int32_t read_head, feedback;
    short SIMPLE_SMOOTH, lerp_len;
    int32_t l_delay_rate_index;
    int pc;

    short sync_out_latch;
    short sync_out_count;
};
