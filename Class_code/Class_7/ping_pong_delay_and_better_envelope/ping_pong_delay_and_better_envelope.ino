/*
  ping pong delay
  One delay leads to another and the output from the last comes into the first for feedback.
  The times of both delays are pased on one pot, making them rhythically linked
  Each delay outputs to the left and right output mixers, both of which get the dry sigal from the input.

  Also in this code is an example of my shape adjsutable envelope
  the default envelope is linear which doesnt sound very natural. This one can do adjustable curves
  as well as triggered pulses. More info in setup and https://github.com/BleepLabs/adjustable_envelope_example

  when using the GUI wiht these libraries use the regual delay and envelope objects and change:
  AudioEffectDelay to AudioEffectTapeDelay
  AudioEffectEnvelope to AudioEffectEnvelopeAdjustable

*/

#include "effect_tape_delay.h" // these should to be before the other audio code 
#include "envelope_adjustable.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=102.5,331.2500047683716
AudioInputI2S            i2s1;           //xy=104,191
AudioEffectEnvelopeAdjustable      envelope1;      //xy=123.75,276.25
AudioMixer4              mixer1;         //xy=318.25000381469727,210.25000381469727
AudioEffectTapeDelay         delay1;         //xy=351,369
AudioEffectTapeDelay         delay2;         //xy=515,373
AudioMixer4              mixer2;         //xy=663,141
AudioMixer4              mixer3;         //xy=667,223
AudioOutputI2S           i2s2;           //xy=818,199
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(i2s1, 0, mixer1, 0);
AudioConnection          patchCord3(envelope1, 0, mixer1, 1);
AudioConnection          patchCord4(mixer1, delay1);
AudioConnection          patchCord5(mixer1, 0, mixer2, 1);
AudioConnection          patchCord6(mixer1, 0, mixer3, 1);
AudioConnection          patchCord7(delay1, 0, delay2, 0);
AudioConnection          patchCord8(delay1, 0, mixer2, 0);
AudioConnection          patchCord9(delay2, 0, mixer3, 0);
AudioConnection          patchCord10(delay2, 0, mixer1, 3);
AudioConnection          patchCord11(mixer2, 0, i2s2, 0);
AudioConnection          patchCord12(mixer3, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=745.2500152587891,374.7500057220459
// GUItool: end automatically generated code


#define DELAY_SIZE 44100 //size samples. We're sampling at 44100HZ so this is 1 second
int16_t tape_delay_bank[2][DELAY_SIZE]; //int16_t should be used for all of these banks.

//A special library must be used to communicate with the two ws2812 addressable LEDs on the board
// The standard LED libraries, like fastLED and adafruits neopixel, cause problems with the audio code so this version is used
// except for these line:
#define num_of_leds 2 //increase for external LEDs
float max_brightness = .1; //change this to increase the max brightness of the LEDs. 1.0 is crazy bright

//everything else can be left alone
#include <WS2812Serial.h> //include the code from this file in our sketch  https://github.com/PaulStoffregen/WS2812Serial/archive/refs/heads/master.zip
#define led_data_pin 29
byte drawingMemory[num_of_leds * 3];
DMAMEM byte displayMemory[num_of_leds * 12];
WS2812Serial LEDs(num_of_leds, displayMemory, drawingMemory, led_data_pin, WS2812_GRB);

//this reades the buttons and can be left alone
#include <Bounce2.h>
#define NUM_BUTTONS 8
const int BUTTON_PINS[NUM_BUTTONS] = {30, 31, 32, 33, 34, 35, 36, 37};
Bounce * buttons = new Bounce[NUM_BUTTONS];
#define BOUNCE_LOCK_OUT

//this is needed for the envelopes. It's an array of array with exopnential and log curves
PROGMEM const uint16_t lut[256 * 9] = {
  37640, 40342, 42011, 43237, 44213, 45026, 45726, 46340, 46889, 47386, 47840, 48258, 48646, 49007, 49347, 49666, 49968, 50255, 50527, 50787, 51035, 51273, 51502, 51721, 51933, 52137, 52334, 52525, 52710, 52889, 53062, 53231, 53395, 53555, 53710, 53862, 54009, 54154, 54294, 54432, 54567, 54698, 54827, 54953, 55077, 55198, 55317, 55434, 55548, 55660, 55771, 55879, 55986, 56090, 56193, 56295, 56395, 56493, 56589, 56685, 56778, 56871, 56962, 57052, 57140, 57227, 57313, 57398, 57482, 57565, 57647, 57727, 57807, 57886, 57964, 58040, 58116, 58191, 58266, 58339, 58411, 58483, 58554, 58624, 58694, 58762, 58830, 58898, 58964, 59030, 59095, 59160, 59224, 59287, 59350, 59412, 59474, 59535, 59595, 59655, 59715, 59774, 59832, 59890, 59947, 60004, 60060, 60116, 60172, 60227, 60281, 60335, 60389, 60442, 60495, 60547, 60599, 60651, 60702, 60753, 60803, 60853, 60903, 60952, 61001, 61050, 61098, 61146, 61194, 61241, 61288, 61335, 61381, 61427, 61473, 61518, 61563, 61608, 61653, 61697, 61741, 61784, 61828, 61871, 61914, 61956, 61998, 62041, 62082, 62124, 62165, 62206, 62247, 62288, 62328, 62368, 62408, 62447, 62487, 62526, 62565, 62604, 62642, 62681, 62719, 62757, 62794, 62832, 62869, 62906, 62943, 62980, 63016, 63053, 63089, 63125, 63161, 63196, 63232, 63267, 63302, 63337, 63372, 63406, 63440, 63475, 63509, 63543, 63576, 63610, 63643, 63677, 63710, 63743, 63775, 63808, 63840, 63873, 63905, 63937, 63969, 64001, 64032, 64064, 64095, 64126, 64157, 64188, 64219, 64250, 64280, 64311, 64341, 64371, 64401, 64431, 64461, 64490, 64520, 64549, 64579, 64608, 64637, 64666, 64695, 64723, 64752, 64780, 64809, 64837, 64865, 64893, 64921, 64949, 64976, 65004, 65032, 65059, 65086, 65113, 65140, 65167, 65194, 65221, 65248, 65274, 65301, 65327, 65354, 65380, 65406, 65432, 65458, 65484, 65509, 65535,
  24447, 27654, 29721, 31281, 32547, 33620, 34554, 35385, 36134, 36817, 37446, 38030, 38575, 39087, 39570, 40026, 40460, 40874, 41269, 41647, 42010, 42359, 42695, 43019, 43332, 43636, 43930, 44215, 44491, 44760, 45022, 45277, 45526, 45768, 46004, 46235, 46461, 46682, 46898, 47110, 47317, 47520, 47720, 47915, 48107, 48295, 48480, 48662, 48841, 49017, 49190, 49360, 49527, 49692, 49855, 50015, 50172, 50328, 50481, 50632, 50781, 50928, 51073, 51217, 51358, 51498, 51635, 51772, 51906, 52039, 52171, 52301, 52429, 52556, 52682, 52806, 52929, 53050, 53171, 53290, 53408, 53524, 53640, 53754, 53867, 53979, 54091, 54201, 54310, 54418, 54525, 54631, 54736, 54840, 54943, 55046, 55147, 55248, 55348, 55447, 55545, 55642, 55739, 55835, 55930, 56024, 56118, 56211, 56303, 56395, 56485, 56576, 56665, 56754, 56842, 56930, 57017, 57103, 57189, 57274, 57359, 57443, 57526, 57609, 57691, 57773, 57854, 57935, 58015, 58095, 58174, 58253, 58331, 58409, 58486, 58563, 58639, 58715, 58791, 58866, 58940, 59014, 59088, 59161, 59234, 59307, 59379, 59450, 59522, 59592, 59663, 59733, 59803, 59872, 59941, 60009, 60078, 60146, 60213, 60280, 60347, 60414, 60480, 60546, 60611, 60676, 60741, 60806, 60870, 60934, 60997, 61061, 61123, 61186, 61249, 61311, 61372, 61434, 61495, 61556, 61617, 61677, 61737, 61797, 61857, 61916, 61975, 62034, 62093, 62151, 62209, 62267, 62324, 62382, 62439, 62495, 62552, 62608, 62664, 62720, 62776, 62831, 62887, 62942, 62996, 63051, 63105, 63159, 63213, 63267, 63320, 63374, 63427, 63480, 63532, 63585, 63637, 63689, 63741, 63792, 63844, 63895, 63946, 63997, 64048, 64098, 64149, 64199, 64249, 64299, 64348, 64398, 64447, 64496, 64545, 64594, 64642, 64691, 64739, 64787, 64835, 64883, 64930, 64978, 65025, 65072, 65119, 65166, 65213, 65259, 65306, 65352, 65398, 65444, 65489, 65535,
  11348, 14129, 16062, 17592, 18878, 19998, 20997, 21903, 22734, 23504, 24224, 24899, 25538, 26143, 26720, 27271, 27799, 28306, 28794, 29265, 29720, 30160, 30587, 31001, 31404, 31796, 32178, 32550, 32913, 33268, 33615, 33954, 34286, 34611, 34930, 35243, 35549, 35850, 36146, 36437, 36722, 37003, 37279, 37551, 37819, 38083, 38343, 38599, 38852, 39101, 39346, 39588, 39828, 40064, 40297, 40527, 40755, 40979, 41202, 41421, 41638, 41853, 42065, 42275, 42483, 42689, 42892, 43093, 43293, 43490, 43686, 43879, 44071, 44261, 44450, 44636, 44821, 45004, 45186, 45366, 45545, 45722, 45897, 46071, 46244, 46416, 46586, 46754, 46922, 47088, 47252, 47416, 47578, 47740, 47900, 48058, 48216, 48373, 48528, 48683, 48836, 48989, 49140, 49290, 49440, 49588, 49736, 49882, 50028, 50173, 50316, 50459, 50601, 50742, 50883, 51022, 51161, 51299, 51436, 51572, 51708, 51842, 51976, 52110, 52242, 52374, 52505, 52636, 52765, 52894, 53023, 53150, 53277, 53404, 53529, 53654, 53779, 53903, 54026, 54148, 54270, 54392, 54513, 54633, 54753, 54872, 54990, 55108, 55226, 55343, 55459, 55575, 55690, 55805, 55920, 56034, 56147, 56260, 56372, 56484, 56595, 56706, 56817, 56927, 57036, 57145, 57254, 57362, 57470, 57577, 57684, 57791, 57897, 58002, 58107, 58212, 58317, 58421, 58524, 58627, 58730, 58833, 58935, 59036, 59138, 59238, 59339, 59439, 59539, 59638, 59738, 59836, 59935, 60033, 60130, 60228, 60325, 60421, 60518, 60614, 60709, 60805, 60900, 60994, 61089, 61183, 61277, 61370, 61463, 61556, 61649, 61741, 61833, 61924, 62016, 62107, 62198, 62288, 62378, 62468, 62558, 62647, 62736, 62825, 62914, 63002, 63090, 63178, 63265, 63353, 63440, 63526, 63613, 63699, 63785, 63871, 63956, 64041, 64126, 64211, 64296, 64380, 64464, 64548, 64631, 64714, 64797, 64880, 64963, 65045, 65128, 65209, 65291, 65373, 65454, 65535,
  2899, 4281, 5377, 6321, 7166, 7940, 8659, 9334, 9973, 10582, 11164, 11724, 12264, 12786, 13292, 13783, 14261, 14727, 15182, 15626, 16060, 16486, 16903, 17313, 17715, 18110, 18498, 18881, 19257, 19628, 19993, 20353, 20708, 21059, 21405, 21747, 22084, 22418, 22748, 23074, 23397, 23716, 24032, 24345, 24654, 24961, 25264, 25565, 25864, 26159, 26452, 26742, 27030, 27316, 27599, 27880, 28159, 28436, 28711, 28983, 29254, 29523, 29790, 30055, 30318, 30579, 30839, 31097, 31353, 31608, 31861, 32113, 32363, 32611, 32858, 33104, 33348, 33591, 33833, 34073, 34312, 34549, 34785, 35020, 35254, 35487, 35718, 35949, 36178, 36406, 36633, 36859, 37083, 37307, 37530, 37751, 37972, 38192, 38410, 38628, 38845, 39061, 39276, 39490, 39703, 39915, 40126, 40337, 40546, 40755, 40963, 41170, 41376, 41582, 41787, 41990, 42194, 42396, 42598, 42799, 42999, 43198, 43397, 43595, 43792, 43989, 44185, 44380, 44575, 44769, 44962, 45155, 45347, 45539, 45729, 45920, 46109, 46298, 46486, 46674, 46861, 47048, 47234, 47419, 47604, 47789, 47973, 48156, 48338, 48521, 48702, 48883, 49064, 49244, 49424, 49603, 49781, 49959, 50137, 50314, 50491, 50667, 50842, 51017, 51192, 51366, 51540, 51713, 51886, 52059, 52231, 52402, 52573, 52744, 52914, 53084, 53254, 53423, 53591, 53759, 53927, 54094, 54261, 54428, 54594, 54760, 54925, 55090, 55255, 55419, 55583, 55746, 55909, 56072, 56234, 56396, 56558, 56719, 56880, 57041, 57201, 57361, 57520, 57679, 57838, 57997, 58155, 58313, 58470, 58627, 58784, 58941, 59097, 59253, 59408, 59564, 59718, 59873, 60027, 60181, 60335, 60488, 60641, 60794, 60947, 61099, 61251, 61402, 61554, 61705, 61855, 62006, 62156, 62306, 62455, 62605, 62754, 62902, 63051, 63199, 63347, 63495, 63642, 63789, 63936, 64083, 64229, 64375, 64521, 64667, 64812, 64957, 65102, 65247, 65391, 65535,
  256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4096, 4352, 4608, 4864, 5120, 5376, 5632, 5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936, 8192, 8448, 8704, 8960, 9216, 9472, 9728, 9984, 10240, 10496, 10752, 11008, 11264, 11520, 11776, 12032, 12288, 12544, 12800, 13056, 13312, 13568, 13824, 14080, 14336, 14592, 14848, 15104, 15360, 15616, 15872, 16128, 16384, 16640, 16896, 17152, 17408, 17664, 17920, 18176, 18432, 18688, 18944, 19200, 19456, 19712, 19968, 20224, 20480, 20736, 20992, 21248, 21504, 21760, 22016, 22272, 22528, 22784, 23040, 23296, 23552, 23808, 24064, 24320, 24576, 24832, 25088, 25344, 25600, 25856, 26112, 26368, 26624, 26880, 27136, 27392, 27648, 27904, 28160, 28416, 28672, 28928, 29184, 29440, 29696, 29952, 30208, 30464, 30720, 30976, 31232, 31488, 31744, 32000, 32256, 32512, 32768, 33023, 33279, 33535, 33791, 34047, 34303, 34559, 34815, 35071, 35327, 35583, 35839, 36095, 36351, 36607, 36863, 37119, 37375, 37631, 37887, 38143, 38399, 38655, 38911, 39167, 39423, 39679, 39935, 40191, 40447, 40703, 40959, 41215, 41471, 41727, 41983, 42239, 42495, 42751, 43007, 43263, 43519, 43775, 44031, 44287, 44543, 44799, 45055, 45311, 45567, 45823, 46079, 46335, 46591, 46847, 47103, 47359, 47615, 47871, 48127, 48383, 48639, 48895, 49151, 49407, 49663, 49919, 50175, 50431, 50687, 50943, 51199, 51455, 51711, 51967, 52223, 52479, 52735, 52991, 53247, 53503, 53759, 54015, 54271, 54527, 54783, 55039, 55295, 55551, 55807, 56063, 56319, 56575, 56831, 57087, 57343, 57599, 57855, 58111, 58367, 58623, 58879, 59135, 59391, 59647, 59903, 60159, 60415, 60671, 60927, 61183, 61439, 61695, 61951, 62207, 62463, 62719, 62975, 63231, 63487, 63743, 63999, 64255, 64511, 64767, 65023, 65279, 65535,
  3, 12, 24, 40, 60, 83, 109, 138, 170, 205, 243, 284, 327, 373, 422, 473, 527, 584, 643, 704, 768, 834, 903, 974, 1047, 1122, 1200, 1281, 1363, 1448, 1535, 1624, 1715, 1809, 1904, 2002, 2102, 2204, 2308, 2415, 2523, 2634, 2746, 2861, 2977, 3096, 3217, 3339, 3464, 3591, 3720, 3850, 3983, 4117, 4254, 4393, 4533, 4675, 4820, 4966, 5114, 5264, 5416, 5570, 5726, 5883, 6043, 6204, 6367, 6532, 6699, 6868, 7038, 7211, 7385, 7561, 7739, 7918, 8100, 8283, 8468, 8655, 8843, 9033, 9226, 9420, 9615, 9813, 10012, 10213, 10415, 10620, 10826, 11034, 11243, 11455, 11668, 11882, 12099, 12317, 12537, 12759, 12982, 13207, 13433, 13662, 13892, 14124, 14357, 14592, 14829, 15067, 15307, 15549, 15792, 16037, 16284, 16532, 16782, 17034, 17287, 17542, 17799, 18057, 18316, 18578, 18841, 19105, 19372, 19640, 19909, 20180, 20453, 20727, 21003, 21280, 21559, 21840, 22122, 22406, 22691, 22978, 23267, 23557, 23849, 24142, 24437, 24733, 25031, 25331, 25632, 25934, 26239, 26544, 26852, 27161, 27471, 27783, 28096, 28411, 28728, 29046, 29366, 29687, 30009, 30333, 30659, 30986, 31315, 31645, 31977, 32311, 32645, 32982, 33319, 33659, 34000, 34342, 34686, 35031, 35378, 35726, 36076, 36427, 36780, 37135, 37490, 37848, 38206, 38567, 38928, 39291, 39656, 40022, 40390, 40759, 41129, 41501, 41875, 42250, 42626, 43004, 43383, 43764, 44146, 44530, 44915, 45302, 45690, 46079, 46470, 46862, 47256, 47652, 48048, 48446, 48846, 49247, 49649, 50053, 50458, 50865, 51273, 51683, 52094, 52506, 52920, 53336, 53752, 54170, 54590, 55011, 55433, 55857, 56282, 56709, 57137, 57566, 57997, 58429, 58863, 59298, 59734, 60172, 60611, 61052, 61494, 61938, 62382, 62829, 63276, 63725, 64176, 64627, 65080, 65535,
  0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 7, 8, 10, 12, 15, 18, 21, 24, 28, 32, 37, 42, 47, 53, 60, 67, 74, 83, 91, 101, 111, 121, 133, 145, 157, 171, 185, 200, 216, 233, 250, 268, 288, 308, 329, 351, 375, 399, 424, 450, 478, 506, 536, 567, 599, 632, 667, 703, 740, 778, 818, 859, 901, 945, 990, 1037, 1086, 1135, 1187, 1240, 1294, 1350, 1408, 1467, 1529, 1591, 1656, 1722, 1790, 1860, 1932, 2006, 2081, 2159, 2238, 2320, 2403, 2489, 2576, 2666, 2758, 2851, 2947, 3046, 3146, 3249, 3354, 3461, 3570, 3682, 3796, 3913, 4032, 4154, 4278, 4404, 4533, 4665, 4799, 4936, 5075, 5217, 5362, 5510, 5660, 5813, 5969, 6128, 6289, 6454, 6621, 6791, 6965, 7141, 7320, 7503, 7688, 7877, 8069, 8263, 8461, 8663, 8867, 9075, 9286, 9501, 9719, 9940, 10164, 10393, 10624, 10859, 11098, 11340, 11586, 11835, 12088, 12345, 12605, 12869, 13137, 13409, 13684, 13963, 14247, 14534, 14825, 15120, 15419, 15722, 16029, 16340, 16655, 16974, 17298, 17626, 17958, 18294, 18634, 18979, 19328, 19682, 20039, 20402, 20768, 21140, 21515, 21896, 22280, 22670, 23064, 23463, 23866, 24274, 24687, 25105, 25527, 25954, 26387, 26824, 27266, 27712, 28164, 28621, 29083, 29550, 30022, 30500, 30982, 31470, 31963, 32461, 32964, 33473, 33987, 34506, 35031, 35561, 36097, 36638, 37185, 37737, 38295, 38858, 39427, 40002, 40583, 41169, 41761, 42359, 42962, 43572, 44187, 44808, 45435, 46069, 46708, 47353, 48004, 48662, 49325, 49995, 50671, 51353, 52041, 52736, 53437, 54144, 54857, 55578, 56304, 57037, 57776, 58522, 59275, 60034, 60800, 61572, 62351, 63137, 63930, 64729, 65535,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 7, 8, 8, 9, 10, 12, 13, 14, 16, 17, 19, 21, 23, 25, 27, 29, 32, 35, 38, 41, 45, 48, 52, 57, 61, 66, 71, 76, 82, 88, 95, 101, 109, 116, 124, 133, 142, 152, 162, 172, 183, 195, 208, 221, 234, 249, 264, 280, 296, 313, 332, 351, 371, 392, 414, 436, 460, 485, 511, 539, 567, 597, 627, 660, 693, 728, 764, 802, 841, 882, 925, 969, 1015, 1063, 1112, 1163, 1217, 1272, 1329, 1389, 1451, 1514, 1581, 1649, 1720, 1793, 1869, 1948, 2029, 2114, 2200, 2290, 2383, 2479, 2578, 2680, 2786, 2895, 3008, 3124, 3243, 3367, 3494, 3625, 3761, 3900, 4044, 4192, 4344, 4501, 4663, 4829, 5000, 5176, 5357, 5543, 5735, 5932, 6134, 6343, 6557, 6776, 7002, 7234, 7473, 7717, 7969, 8227, 8492, 8763, 9042, 9328, 9622, 9923, 10232, 10548, 10873, 11206, 11547, 11897, 12255, 12622, 12998, 13384, 13778, 14183, 14596, 15020, 15454, 15898, 16353, 16818, 17294, 17781, 18279, 18788, 19310, 19843, 20388, 20945, 21515, 22098, 22693, 23302, 23923, 24559, 25208, 25872, 26549, 27241, 27948, 28670, 29407, 30160, 30929, 31713, 32514, 33331, 34165, 35017, 35885, 36772, 37676, 38598, 39539, 40499, 41477, 42475, 43493, 44531, 45589, 46667, 47767, 48887, 50029, 51193, 52379, 53588, 54820, 56074, 57353, 58655, 59981, 61332, 62707, 64108, 65535,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 7, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 20, 22, 24, 26, 28, 31, 34, 36, 40, 43, 47, 50, 55, 59, 64, 69, 75, 81, 87, 94, 101, 109, 117, 126, 136, 146, 157, 168, 181, 194, 208, 223, 239, 255, 273, 292, 313, 334, 357, 381, 407, 434, 463, 493, 526, 560, 596, 634, 675, 718, 763, 811, 861, 915, 971, 1030, 1093, 1159, 1228, 1302, 1379, 1460, 1546, 1636, 1731, 1831, 1936, 2046, 2162, 2283, 2411, 2546, 2687, 2835, 2990, 3153, 3324, 3503, 3691, 3887, 4093, 4309, 4536, 4772, 5020, 5280, 5551, 5835, 6132, 6442, 6767, 7106, 7460, 7830, 8217, 8621, 9042, 9482, 9941, 10420, 10920, 11441, 11984, 12551, 13141, 13757, 14398, 15066, 15762, 16486, 17241, 18026, 18843, 19694, 20579, 21500, 22457, 23453, 24488, 25564, 26683, 27845, 29053, 30308, 31611, 32965, 34371, 35830, 37345, 38917, 40548, 42241, 43997, 45819, 47708, 49667, 51698, 53804, 55986, 58248, 60591, 63020, 65535
};

//starts at midi note 12, C0 https://newt.phys.unsw.edu.au/jw/notes.html
PROGMEM const static float chromatic[121] = {16.3516, 17.32391673, 18.35405043, 19.44543906, 20.60172504, 21.82676736, 23.12465449, 24.499718, 25.95654704, 27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192, 8869.845359, 9397.273811, 9956.06479, 10548.08321, 11175.30488, 11839.82309, 12543.8556, 13289.75207, 14080.00185, 14917.24233, 15804.26772, 16744.03838};

//then you can declare any variables you want.
unsigned long current_time;
unsigned long prev_time[8]; //an array of 8 variables all named "prev_time"
int delay_time[2];
float feedback;
float wet_dry;

void setup() {

  LEDs.begin(); //must be done in setup for the addressable LEDs to work.
  //here is a basic way of writing to the LEDs.
  LEDs.setPixelColor(0, 0, 0, 0); //(LED number, red level, green level, blue level). All levels are 0-255
  LEDs.setPixelColor(1, 0, 0, 0);
  LEDs.show(); //send these values to the LEDs

  analogReadAveraging(64); //take lots of readings and average them each time we do analogRead. This is one stage of smoothing

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  ); //setup the bounce instance for the current button
    buttons[i].interval(10);  // interval in milliseconds. How long after the first cahnge will ignore noise
  }

  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // The Teensy 4.1 hasalmost 2000 block of memory
  // It's usually the delay and reverb that hog it.
  AudioMemory(100);

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary is you're not using the ins
  sgtl5000_1.lineInLevel(10); //The volume of the input. 0-15 with 15 bing more amplifications
  //sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  //sgtl5000_1.micGain(13); //0 - 63bd of gain.

  //headphone jack output volume. Goes from 0.0 to 1.0 but a 100% signal will clip over .8 or so.
  // For headphones it's pretty loud at .4
  // There are lots of places we can change the final volume level.
  // For now lets set this one once and leave it alone.
  sgtl5000_1.volume(0.25);

  //The line out has a separate level control but it's not meant to be adjusted like the volume function above.
  // If you're not using the line out don't worry about it.
  sgtl5000_1.lineOutLevel(21); //11-32, the smaller the louder. 21 is about 2 Volts peak to peak

  waveform1.begin(1, 0, WAVEFORM_BANDLIMIT_SQUARE); //(amplitude, frequency, shape)

  mixer1.gain(0, 0); //audio in
  mixer1.gain(1, .6); //waveform 1 through envelope 1
  mixer1.gain(3, .3); //feedback

  //left mixer
  mixer2.gain(0, .5); //from delay1
  mixer2.gain(1, .5);//dry

  //right mixer
  mixer3.gain(0, .5); //from delay2
  mixer3.gain(1, .5);//dry


  //(bank select, size of bank, starting delay length, redux, lerp)
  //redux is sample rate reduction. takes integers. 0 is 44100, 1 is 22050, 2 is 11025 etc. It's crude but allows you to double or quaduple the delay length at the expense of sample rate
  //lerp is how fast it moves to the desired length. 0 is as fast as it can. takes integers.
  delay1.begin(tape_delay_bank[0], DELAY_SIZE, 0, 0, 2);
  delay2.begin(tape_delay_bank[1], DELAY_SIZE, 0, 0, 2);

  envelope1.lutSelect(lut); //needed for my dumb library

  //these are jsut the same as the standard library;
  envelope1.attack(2); //times in milliseconds
  envelope1.decay(150);
  envelope1.sustain(.5); //amplitued 0-1.0
  envelope1.release(200);

  //change shape of all stages. -1.0 very exponential, 1.0 very log
  envelope1.shape(-.8);
  // envelope1.attackShape(-.9); //change each one
  // envelope1.decayShape(.3);
  // envelope1.releaseShape(-.3);

} //setup is over


void loop() {
  current_time = millis();

  for (int j = 0; j < NUM_BUTTONS; j++)  {
    buttons[j].update();
  }

  if ( buttons[0].fell() ) {
    int r1 = random(40, 90);
    waveform1.frequency(chromatic[r1]);
    envelope1.trigger(); //just the attack and decay, how long the button down has no effect on the output
  }

  if ( buttons[1].fell() ) {
    int r1 = random(40, 90);
    waveform1.frequency(chromatic[r1]);
    envelope1.noteOn(); //gates work jsut like the standard envelope
  }
  if ( buttons[1].rose() ) {
    envelope1.noteOff();
  }

  if (current_time - prev_time[2] > 0) { //no need to do these fast. slower is less noisy and delaytimes should be v smooth.
    prev_time[2] = current_time;
    //smooth(select,input) each separate thing you smooth needs it's own select value
    //smooth can't take floats so we smooth the reading then divide
    feedback = (1.0 - (smooth(0, analogRead(A10)) / 1023.0)) * 1.5;
    mixer1.gain(3, feedback);

    wet_dry = smooth(1, analogRead(A11)) / 1023.0;
    mixer2.gain(1, wet_dry); //dry waveform1
    mixer2.gain(0, 1.0 - wet_dry); //wet delay1
    mixer3.gain(1, wet_dry); //dry waveform1
    mixer3.gain(0, 1.0 - wet_dry); //wet delay2

    int s1 = smooth(2, analogRead(A12));
    delay_time[0] = map(s1, 0, 1023, 0, DELAY_SIZE);
    delay_time[1] = delay_time[0] * .33;
    delay1.length(delay_time[0]);
    delay2.length(delay_time[1]);
  }


  if (current_time - prev_time[1] > 33) { //33 milliseconds is about 30 Hz, aka 30 fps
    prev_time[1] = current_time;

    //there's another function in this sketch bellow the loop which makes it easier to control the LEDs more info bellow the loop
    //(led to change, hue,saturation,brightness)
    set_LED(0, 0, 1, 0);
    set_LED(1, 0, 1, 0);
    LEDs.show(); //send these values to the LEDs
  }

  if (current_time - prev_time[7] > 50 && 1) { //change to && 0 to not do this code
    prev_time[7] = current_time;
    Serial.print("delay time[0] ");
    Serial.println(delay_time[0]);
    Serial.print("delay time[1] ");
    Serial.println(delay_time[1]);
    Serial.print("feedback ");
    Serial.println(feedback);
    Serial.print("wet dry ");
    Serial.println(wet_dry);
    Serial.println();

  }

  if (current_time - prev_time[0] > 500 && 0) { //change to && 0 to not do this code
    prev_time[0] = current_time;

    //Here we print out the usage of the audio library
    // If we go over 90% processor usage or get near the value of memory blocks we set aside in the setup we'll have issues or crash.
    // If you're using too many block, jut increase the number up top until you're over it by a couple
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();
  }

}// loop is over


////////////////LED function

//This function is a little different than you might see in other libraries but it works pretty similar
// instead of 0-255 you see in other libraries this is all 0-1.0
// you can copy this to the bottom of any code as long as the declarations at the top in "led biz" are done

//set_LED(led to change, hue,saturation,value aka brightness)
// hue - 0 is red, then through the ROYGBIV to 1.0 as red again
// saturation - 0 is fully white, 1 is fully colored.
// value - 0 is off, 1 is the value set by max_brightness
// (it's not called brightness since, unlike in photoshop, we're going from black to fully lit up

//based on https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

void set_LED(int pixel, float fh, float fs, float fv) {
  byte RedLight;
  byte GreenLight;
  byte BlueLight;

  if (fs > 1.0) {
    fs = 1.0;
  }

  if (fv > 1.0) {
    fv = 1.0;
  }

  //wrap the hue around but if it's over 100 or under -100 cap it at that
  if (fh > 100) {
    fh = 100;
  }

  if (fh < -100) {
    fh = -100;
  }
  //keep subtracting or adding 1 untill it's in the range of 0-1.0
  while (fh > 1.0) {
    fh -= 1.0;
  }
  while (fh < 0) {
    fh += 1.0;
  }

  byte h = fh * 255;
  byte s = fs * 255;
  byte v = fv * max_brightness * 255;

  h = (h * 192) / 256;  // 0..191
  unsigned int i = h / 32;   // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;   // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;
  unsigned int fInv = 255 - f;
  byte pv = v * sInv / 256;  // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      RedLight = v;
      GreenLight = tv;
      BlueLight = pv;
      break;
    case 1:
      RedLight = qv;
      GreenLight = v;
      BlueLight = pv;
      break;
    case 2:
      RedLight = pv;
      GreenLight = v;
      BlueLight = tv;
      break;
    case 3:
      RedLight = pv;
      GreenLight = qv;
      BlueLight = v;
      break;
    case 4:
      RedLight = tv;
      GreenLight = pv;
      BlueLight = v;
      break;
    case 5:
      RedLight = v;
      GreenLight = pv;
      BlueLight = qv;
      break;
  }
  LEDs.setPixelColor(pixel, RedLight, GreenLight, BlueLight);
}


////////////smooth function
//based on https://playground.arduino.cc/Main/DigitalSmooth/

#define filterSamples   17   // filterSamples should  be an odd number, no smaller than 3. Incerease for more smoooothness
#define array_num 8 //numer of differnt smooths we can take, one for each pot
int sensSmoothArray[array_num] [filterSamples];   // array for holding raw sensor values for sensor1

int smooth(int array_sel, int input) {
  int j, k, temp, top, bottom;
  long total;
  static int i;
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[array_sel][i] = input;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j = 0; j < filterSamples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[array_sel][j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++) {
      if (sorted[j] > sorted[j + 1]) {    // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j + 1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1);
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j < top; j++) {
    total += sorted[j];  // total remaining indices
    k++;
  }

  return total / k;    // divide by number of samples
}
