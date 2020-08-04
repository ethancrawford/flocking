// This is an alternate way to write the BoringMixer plugin using a newer C++ header (introduced in 2010).

// Using SC_PlugIn.hpp instead of SC_PlugIn.h. SC_PlugIn.hpp includes SC_PlugIn.h. C++ and old-fashioned ugens can
// coexist in the same file.
#include "SC_PlugIn.hpp"
#include "BoidFlock.hpp"
#include "Boid.hpp"
#include "Pvector.hpp"

static InterfaceTable *ft;

#define CTOR_GET_BUF                                \
  float fbufnum  = ZIN0(0);                         \
  fbufnum = sc_max(0.f, fbufnum);                   \
  uint32 bufnum = (int)fbufnum;                     \
  World *world = unit->mWorld;                      \
  SndBuf *buf;                                      \
  if (bufnum >= world->mNumSndBufs) {               \
    int localBufNum = bufnum - world->mNumSndBufs;  \
    Graph *parent = unit->mParent;                  \
    if(localBufNum <= parent->localBufNum) {        \
      buf = parent->mLocalSndBufs + localBufNum;    \
    } else {                                        \
      bufnum = 0;                                   \
      buf = world->mSndBufs + bufnum;               \
    }                                               \
  } else {                                          \
    buf = world->mSndBufs + bufnum;                 \
        }                                           \
  float* bufData __attribute__((__unused__)) = buf->data;

// Inherits from SCUnit, not Unit.
// Also note that the constructor, destructor, and calc functions are methods of the SCUnit.
struct Flock : public SCUnit {
public:
  // Constructor function
  Flock() {
    m_idx = 0;
    m_seed = 0;
    const Unit* unit = this;
    CTOR_GET_BUF;

   

    for (int i = 0; i < 20; i++) {
      float x = bufData[i];
      float y = bufData[i + 1];
      Boid b((WIDTH / 2.0) + x, (HEIGHT / 2.0) + y); // Starts all boids in the center of the screen
      // Adding the boid to the flock
      flock.addBoid(b);
    }

    // New way of setting calc function.
    if (isAudioRateIn(1)) {
      set_calc_function<Flock,&Flock::next_a>();
    } else {
      set_calc_function<Flock,&Flock::next_k>();
    }
    mFreqMul = 2.0 * sampleDur();
    mPhase = 0.0;

    if (isAudioRateIn(1)) {
      next_a(1);
    } else {
      next_k(1);
    }
  }

  // If you want a destructor, you would declare "~BoringMixer2() { ... }" here.
  ~Flock() {

  }
private:
  // You can declare state variables here.
  float m_fbufnum;
  SndBuf *m_buf;
  float m_num_boids;
  int m_idx;
  int m_seed;
  const float WIDTH = 1000.0;
  const float HEIGHT = 1000.0;
  BoidFlock flock;
  double mPhase; // phase of the oscillator, from -1 to 1.
  float mFreqMul; // a constant for multiplying frequency

  // Calc function
  void next_a(int inNumSamples) {
    // Note, there is no "unit" variable here, so you can't use a lot of the traditional helper macros. That's why
    // the C++ header offers replacements.
    const float *freq = in(1);
    float *out_left = out(0);
    float *out_right = out(1);
    float freqmul = mFreqMul;
    double phase = mPhase;

    flock.flocking();

    for (int i = 0; i < inNumSamples; i++) {

      float z = phase;
      phase += freq[i] * freqmul;

      // these if statements wrap the phase a +1 or -1.
      if (phase >= 1.f) phase -= 2.f;
      else if (phase <= -1.f) phase += 2.f;
      //out_left[i] = freq[i] * 0.5;
      //out_right[i] = freq[i] * 0.5;
      out_left[i] = z * 0.5;
      out_right[i] = z * 0.5;
    }
    mPhase = phase;
  }

  // Calc function
  void next_k(int inNumSamples) {
    // Note, there is no "unit" variable here, so you can't use a lot of the traditional helper macros. That's why
    // the C++ header offers replacements.
    float freq = in0(1);
    float *out_left = out(0);
    float *out_right = out(1);

    for (int i = 0; i < inNumSamples; i++) {
      out_left[i] = freq * 0.5;
      out_right[i] = freq * 0.5;
    }
  }
};



PluginLoad(FlockUGens) {
  ft = inTable;
  // registerUnit takes the place of the Define*Unit functions. It automatically checks for the presence of a
  // destructor function.
  // However, it does not seem to be possible to disable buffer aliasing with the C++ header.
  registerUnit<Flock>(ft, "Flock");
}
