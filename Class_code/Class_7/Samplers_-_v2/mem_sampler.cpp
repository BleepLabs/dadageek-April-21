#include "mem_sampler.h"
#include "arm_math.h"
#include "utility/dspinst.h"



void MemSampler::update(void)
{
  audio_block_t *out_block1;
  audio_block_t *in_block1;
  short *outp;
  short *inp;

  if (sampleMemory == NULL)return;

  in_block1 = receiveReadOnly(0);

  //if (playing == 0 && recording == 0) return;
  out_block1 = allocate();

  if (out_block1) {
    outp = out_block1->data;
  }

  if (in_block1) {
    inp = in_block1->data;
  }
  for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {

    if (mode == 1) {  //recording
      if (in_block1) {
        //l_delayline[write_head] = *bp ;
        sampleMemory[write_head] = *inp++;
        write_head++;
        if (write_head > max_rec_len) {
          sample_len = max_rec_len;
          end_mod = sample_len;
          mode = 0;

          Serial.print(" max ");
          Serial.println(sample_len);
        }
      }
    }

    if (mode == 0) { //idle
      out_block1->data[i] = 0;
    }

    if (mode == 2) { //playing
      prev_index1 = index1;
      index1 = accumulator >> (23);

      if (prev_index1 > 128 && index1 < 128) {
        index2 += 256;
      }

      if (index2 + index1 >= end_mod) {
        index2 = 0;
        accumulator = 0;
        if (looping1 == 0) {
          mode = 0;
        }
      }

      uint32_t loc = index2 + index1;

      int next = 1;
      if (rev == 1) {
        loc = (sample_len - loc);
        next = -1;

      }

      scale = (accumulator >> 7) & 0xFFFF;

      int16_t val1t = sampleMemory[loc];
      int32_t lerp_next = loc + next;
      if (lerp_next < 1) {
        lerp_next = 1;
      }
      if (lerp_next > end_mod) {
        lerp_next = end_mod;
      }
      int16_t val2t = sampleMemory[lerp_next];
      int32_t val2 =  val2t * scale;
      int32_t val1 = val1t * (0xFFFF - scale);
      int32_t val3 = (val1 + val2) >> 16;
      *outp++ = short(val3);

      accumulator += increment;
      accumulator &= 0x7fffffff;

      printcount++;
      if (printcount > 40 && 0) {
        printcount = 0;
        Serial.print(val3);
        Serial.print(" ");
        Serial.println(val1t);
      }
    }

  }

  transmit(out_block1, 0);
  release(out_block1);
  if (in_block1) {
    release(in_block1);

  }

}
