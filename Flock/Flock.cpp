#include "SC_PlugIn.hpp"
#include "BoidFlock.hpp"
#include "Boid.hpp"
#include "Pvector.hpp"

// InterfaceTable contains pointers to functions in the host (server).
static InterfaceTable *ft;

// A macro that is similar to SuperCollider's GET_BUF but runs in the Ctor instead
#define CTOR_GET_BUF                                      \
  float fbufnum  = ZIN0(0);                               \
  fbufnum = sc_max(0.f, fbufnum);                         \
  uint32 bufnum = (int)fbufnum;                           \
  World *world = unit->mWorld;                            \
  SndBuf *buf;                                            \
  if (bufnum >= world->mNumSndBufs) {                     \
    int localBufNum = bufnum - world->mNumSndBufs;        \
    Graph *parent = unit->mParent;                        \
    if(localBufNum <= parent->localBufNum) {              \
      buf = parent->mLocalSndBufs + localBufNum;          \
    } else {                                              \
      bufnum = 0;                                         \
      buf = world->mSndBufs + bufnum;                     \
    }                                                     \
  } else {                                                \
    buf = world->mSndBufs + bufnum;                       \
  }                                                       \
  float* bufData __attribute__((__unused__)) = buf->data; \
  uint32 bufSamples __attribute__((__unused__)) = buf->samples;

// A struct to hold attributes of each generated sine wave value
struct SinWave {
  int32 m_phase;
  int32 phase;
  int32 temp_freq;
  int32 phaseinc;
  float sin_val;
};

// declare struct to hold unit generator state
struct Flock : public SCUnit{

  // Constructor usually does 3 things.
  // 1. set the calculation function.
  // 2. initialize the unit generator state variables.
  // 3. calculate one sample of output.
  // For this UGen, we do steps 1 and 2 in the reverse order.
public:
  Flock() {
    CTOR_GET_BUF;
    m_seed = IN0(3);
    m_num_boids = IN0(4);
    rand_buf = bufData;
    rand_buf_size = bufSamples;
    for (int i = 0; i < m_num_boids; i++) {
      float vx = cos(buf_rand(i)) / 8.0;
      float vy = sin(buf_rand(i + 1)) / 8.0;
      Boid b((WIDTH / 2.0), (HEIGHT / 2.0), vx, vy);
      flock.addBoid(b);
    }

    /*
      1. Initialize the unit generator state variables.
      Unlike some of the example UGens, here we need to do this
      before setting the calculation function.
    */
    int tableSize2 = ft->mSineSize;
    m_radtoinc = tableSize2 * rtwopi * 65536.0;
    m_cpstoinc = tableSize2 * SAMPLEDUR * 65536.0;
    m_lomask = (tableSize2 - 1) << 3;
    m_phasein = IN0(2);
    waves = (SinWave*)RTAlloc(unit->mWorld, m_num_boids * sizeof(SinWave));
    for (int i = 0; i < m_num_boids; i++) {
      waves[i].m_phase = (int32)(m_phasein * m_radtoinc);
    }

     // 2. set the calculation function.
    set_calc_function<Flock,&Flock::next_k>();
 
    /*
      3. calculate one sample of output.
      (The next time next_k is called it will be asked to calculate
      an entire control block's worth of samples, typically 64).
    */
    next_k(1);
  }

  ~Flock(){
    RTFree(unit->mWorld, waves);
  }

private:
  const float WIDTH = 1000.0;
  const float HEIGHT = 1000.0;
  BoidFlock flock;
  float m_phasein;
  double m_radtoinc;
  double m_cpstoinc;
  int32 m_lomask;
  int m_seed;
  int m_num_boids;
  float *rand_buf;
  int rand_buf_size;
  const Unit* unit = this;
  SinWave *waves;
  //////////////////////////////////////////////////////////////////

  // The calculation function executes once per control period
  // which is typically 64 samples.

  // calculation function for an audio rate frequency argument

  //////////////////////////////////////////////////////////////////

  // calculation function for a control rate frequency argument
  void next_k(int inNumSamples)
  {
    float *table0 = ft->mSineWavetable;
    float *table1 = table0 + 1;
    // get the pointer to the output buffer
    float *outBuf = OUT(0);

    // freq is control rate, so calculate it once.
    float freqin = IN0(1);

    // get phase from struct and store it in a local variable.
    // The optimizer will cause it to be loaded it into a register.
    float phasein = IN0(2);

    for(int i = 0; i < m_num_boids; i++){
      waves[i].phase = waves[i].m_phase;
    }
    int32 lomask = m_lomask;

    flock.flocking();

    for (int i = 0; i < inNumSamples; ++i) {
      float out_val = 0.0;

      for (int j = 0; j < m_num_boids; j++) {
        if (i == 0) {
          waves[j].temp_freq = (int32)(m_cpstoinc * (freqin + (flock.getBoid(j).angle(flock.getBoid(j).velocity))));
          waves[j].phaseinc = waves[j].temp_freq + (int32)(CALCSLOPE(phasein, m_phasein) * m_radtoinc);
          m_phasein = phasein;
        }
        waves[j].sin_val = lookupi1(table0, table1, waves[j].phase, lomask);
        out_val += (waves[j].sin_val * sc_reciprocal((float)m_num_boids));
        waves[j].phase += waves[j].phaseinc;
      }
      outBuf[i] = out_val;
    }

    for(int j = 0; j < m_num_boids; j++){
      waves[j].m_phase = waves[j].phase;
    }
  }

  float buf_rand(int idx) {
    return rand_buf[(idx + m_seed) % rand_buf_size];
  }
};

// the entry point is called by the host when the plug-in is loaded
PluginLoad(FlockUGens)
{
  // InterfaceTable *inTable implicitly given as argument to the load function
  ft = inTable; // store pointer to InterfaceTable

  // registerUnit takes the place of the Define*Unit functions. It automatically checks for the presence of a
  // destructor function.
  // However, it does not seem to be possible to disable buffer aliasing with the C++ header.
  registerUnit<Flock>(ft, "Flock");
}
