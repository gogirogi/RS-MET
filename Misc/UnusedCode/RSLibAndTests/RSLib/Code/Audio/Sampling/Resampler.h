#ifndef RS_RESAMPLER_H
#define RS_RESAMPLER_H

using namespace std;

namespace RSLib
{

  // todo: this file is messed up and contains a lot of functions classes that have nothing to do 
  // with resampling directly. move all those other functions into appropriate files, maybe for 
  // those which are still under construction, create a special UnderConstructionAudio.h/cpp file


  /** Implements bidirectional filtering, which means the signal is first passed through the filter
  in normal (forward) direction and then passed through the same filter again in opposite 
  (backward) direction. As a result of this forward/backward application of the same filter, the 
  magnitude response of the filter is squared and the phase response cancels to zero. A zero phase 
  response is desirable whenever we want to preserve time domain characteristics of the signal, 
  because certain time-domain features are better preserved that way. The filters may run not only
  once (forward/backward) over the signals, but optionally also multiple times. Running the same 
  filter bidirectionally multiple times changes the shape of the envelope of the impulse-response 
  in a way so as to approach a Gaussian shape with more and more passes. It will also raise the 
  magnitude response to a power (given by the number of passes) and spread the impulse response out 
  in time, but this time-spreading (and to some extent, the magnitude powering) can be compensated 
  by choosing larger bandwidths for the single-pass filter to begin with (this compensation is done
  internally here). The closer the shape approximates the Gaussian, the smaller the product of the 
  filter's time-domain width and frequency-domain width will be. When the shape is an exact 
  Gaussian, the this time-frequency product will be at its theoretical minimum value, determined 
  by the time-frequency uncertainty principle. So, typically, more passes are better, but there 
  will be a point of diminishing returns because the Gaussian shape is approached rather quickly. 
  The sweet spot seems to be somewhere between 10 and 20 passes [maybe elaborate this in terms of 
  decrease of time-frequency product].  

  \todo we really should use a bilinear biquad internally consistently instead of different filter 
  classes for different purposes

  \todo maybe rename into rsBiDirectionalBiquad, maybe derive from rsBiquad class

  */

  class RSLib_API rsBiDirectionalFilter
  {

  public:

    /** Applies a bidirectional 2nd order bandpass to the signal x of length N and stores the 
    result in y. The bandpass has a center frequency fc and bandwidth bw (both in Hz) at a 
    samplerate fs. The gain at fc is always fixed to unity, independently from the bandwidth. With
    the optional numPasses parameter, you can control, how many times the filter will be applied. 
    To compensate for the effects of multiple passes, the bandwidth of the single-pass filter will 
    be adjusted internally. 
    gc: gain at bandedge frequencies (raw amplitude)
    */
    static void applyConstPeakBandpassBwInHz(double *x, double *y, int N, double fc, double bw, 
      double fs, int numPasses = 1, double gc = RS_SQRT2_INV);

    /** Similar to applyConstPeakBandpassBwInHz but uses a Butterworth bandpass with given order
    instead of a 2nd order bandpass. */
    static void applyButterworthBandpassBwInHz(double *x, double *y, int N, double fc, double bw, 
      double fs, int order, int numPasses = 1, double gc = RS_SQRT2_INV);


    static void applyButterworthLowpass(double *x, double *y, int N, double fc, double fs, 
      int order, int numPasses = 1, double gc = RS_SQRT2_INV);

    static void applyButterworthHighpass(double *x, double *y, int N, double fc, double fs, 
      int order, int numPasses = 1, double gc = RS_SQRT2_INV);


    /** Applies a bidirectional 1st order lowpass to the signal x of length N and stores the 
    result in y. The filter has a cutoff frequency fc at a samplerate fs. With the optional 
    numPasses parameter, you can control, how many times the filter will be applied. To compensate 
    for the effects of multiple passes, the cutoff frequency of the single-pass filter will be 
    adjusted internally. */
    static void applyLowpass(double *x, double *y, int N, double fc, double fs, 
      int numPasses = 1, double gc = RS_SQRT2_INV);

  protected:

    /** Computes the number of samples of zero-padding required at the start and end of the signal 
    before applying the filter, to make sure that the filter can ring out properly to minimize edge 
    effects. bw is the bandwidth of the filter (equals the cutoff for lowpass filters) and fs is
    the samplerate. */
    static int getPaddingLength(double bw, double fs);

  };

  //===============================================================================================

  /** A structure to represent a fractional index inside some array of values, for example, for
  reading out an interpolated value. Representing such fractional indices by plain double variables
  has the disadvantage of precision loss for the fractional part as the integer part goes up 
  because more and more of the overall floating point precision is used up for the integer part. 
  This can be avoided by storing integer and fractional parts separately which is the purpose of 
  this structure. */
  struct rsFractionalIndex
  {
    int    intPart;   // integer part of the index
    double fracPart;  // fractional part of the index
  };
  // move somewhere into the RSCore module


  /** A class with a collection of functions to find the zero-crossings in a signal, possibly with 
  subsample precision. The algorithm for subsample precsision location works by looking at the 
  sample values before and after the zero crossing (which actually happens somwhere between the two 
  sample instants) and fitting a polynomial to the sample values around the zero crossings and 
  finding the root of the interpolating polynomial. The order of the polynomial is determined by 
  the precision parameter p which is passed to some of the functions as: order = 2*p+1 and 2*p+2 
  points will be used. If p=0, it will just fit a straight line between the sample before and after
  the zero and solve for the zero of this line. This is the lowest precision estimate. For higher 
  precision, the linear zero is used as initial guess in a Newton-iteration for finding the root of
  the higher order polynomial interpolant. Typically, p=1 corresponding to a cubic interpolant 
  gives adequate results. I also tried to use the cubic spline (which does not necessarily pass 
  through the outer points but instead matches derivatives of neighbouring interpolants at the 
  inner points), but the cubic that goes through the points seemed to give better results.

  \todo: Theoretically, the true zero in continuous time is given by the  zero-crossing of a
  sinc-interpolant rather than a polynomial interpolant. Maybe, it's possible to use
  windowed-sinc interpolation in conjunction with (derivative-free) root-finding, i.e. bisection
  or something (or maybe the derivative of the sinc interpolant can be computed straightforwardly
  together with the interpolant itself). 
   -make a class rsInterpolatingFunction as subclass of rsUnivariateScalarFunction
   -from that, make subclasses rsInterpolatingPolynomial, rsInterpolatingSpline,
    rsInterpolatingSinc, etc.
   -maybe generalize for non-equidistant data
   -maybe optionally return values of a number of derivatives at the crossing

  generalize to findValueCrossings that finds the crossing of an arbitrary value. to this end,
  a constant term should be included in the root-finding process (implement this at the level
  of rsUnivariateScalarFunction::findRoot...
  */

  class RSLib_API rsZeroCrossingFinder
  {

  public:

    /** Returns true, if x[n] < 0.0 and x[n+1] >= 0.0. */
    static bool isUpwardCrossing(double *x, int n);

    /** Returns the number of upward zero crossings in array x of length N. */
    static int numUpwardCrossings(double *x, int N);

    /** Returns the integer parts of the upward zero crossings in signal x of length N. */
    static std::vector<int> upwardCrossingsInt(double *x, int N);

    /** Returns the positions of the upward zero crossings in the array x of length N with 
    subsample precision. */
    static std::vector<rsFractionalIndex> upwardCrossingsIntFrac(double *x, int N,
      int p = 1);

    /** More convenient version, returning a vector double values to represent the zero-crossings.
    May have precision loss for the fractional part of zero crossings later inside the signal. */
    static std::vector<double> upwardCrossings(double *x, int N, int p = 1);

    /** Like upwardCrossings(), but before finding the zeros, this function applies a 
    (bidirectional, multipasss) bandpass filter to the input signal. Parameter fc is the center 
    frequency, bw: bandwidth, fs: samplerate, np: number of passes, p: precision for subsample
    detection. */
    static std::vector<double> bandpassedUpwardCrossings(double *x, int N, double fc, double bw,
      double fs, int np, int p = 1);

  };


  //===============================================================================================

  /** A class for finding the time-instants inside a (quasi)periodic signal where one cycle ends 
  and the next one begins. 
  
  \todo: maybe have a phase-offset parameter
  */

  class RSLib_API rsCycleMarkFinder
  {

  public:

    /** Enumeration of the cycle-mark finder algorithms. */
    enum algorithms
    {
      F0_ZERO_CROSSINGS = 0,  // zero crossings of extracted fundamental
      CYCLE_CORRELATION       // auto-correlation between successive cycles
    };

    /** Constructor. You should pass a sample-rate and the minimum and maximum expected values
    for the fundamental frequency. */
    rsCycleMarkFinder(double sampleRate, double minFundamental = 20, double maxFundamental = 5000);


    /** \name Setup */

    /** Sets the sample-rate of the signal to be analyzed. */
    inline void setSampleRate(double newSampleRate) { fs = newSampleRate; }

    /** Sets the expected range for the fundamental frequency. This sets some thresholds in the 
    algorithm. */
    inline void setFundamentalRange(double newMin, double newMax)
    {
      fMin = newMin;
      fMax = newMax;
    }

    /** Selects the algorithm by which this object will determine where one cycle ends and the next 
    cycle starts. @see algorithms. */
    inline void setAlgorithm(int newAlgorithm)
    {
      algo = newAlgorithm;
    }

    /** Sets the precision by which we try to approximate the subsample location of the 
    cycle-marks. Currently, this affects only the F0_ZERO_CROSSINGS algorithm in which case it 
    determines the order of the polynomial that we fit to the samples around the zero-crossing and 
    whose zero-crossing we use. 0: linear, 1: cubic, in general, the polynomial order is 2*k+1 
    where k ist the value passed as "newPrecision". */
    inline void setSubSampleApproximationPrecision(int newPrecision)
    {
      precision = newPrecision;
    }

    /** This sets the length of the correlation that should be used in the CYCLE_CORRELATION 
    algorithm as fraction of the cycle length. Should be at most 1. The larger it is, the more of 
    the neighbourhood of the (old, estimated) cycle-mark will be taken into account to find the 
    new, refined cycle-mark. */
    inline void setRelativeCorrelationLength(double newLength)
    {
      correlationLength = newLength;
    }

    /** In the CYCLE_CORRELATION algorithm, there's an optional highpass that is used on the signal 
    before doing the correlations. Here you can set its frequency as fraction of the estimated 
    fundamental frequency. A value of 0 turns the highpass off (which is the default setting). */
    inline void setRelativeCorrelationHighpassFreq(double newFreq)
    {
      correlationHighpass = newFreq;
    }

    /** Sets the bandwidth of the bandpass that is used to extract the fundamental as fraction of 
    the estimated fundamental frequency. So, if this value is 1 and the (estimated) fundamental is 
    100Hz, a bandpass from 50 to 150 Hz will be used to extract the fundamental. If it's 0.1, the 
    bandpass will go from from 95 to 105 Hz.  */
    inline void setRelativeBandpassWidth(double newWidth)
    {
      bandPassWidth = newWidth;
    }

    /** Sets the number of bidirectional passes of the fundamental extraction bandpass. The basic 
    bandpass is a biquad with 6dB/oct at each side, bi-directionally applied that makes 12, so in 
    the end, you'll get a 12*n dB/oct, where n is the value passed as "newSteepness". Note that the 
    bandwidth will be scaled according to the steepness value to get comparable magnitude responses
    for various steepness settings. */
    inline void setBandpassSteepness(int newSteepness)
    {
      bandpassSteepness = newSteepness;
    }


    /** \name Processing */

    /** Returns an array of cycle-marks for the given input signal of length N. */
    std::vector<double> findCycleMarks(double *x, int N);


  protected:

    /** Refines a given vector of initial estimates of the cycle-marks given in cm by correlating 
    successive (estimated) cycles and placing the new cycle border at the instant of maximum 
    correlation. */
    void refineCycleMarksByCorrelation(double *x, int N, std::vector<double>& cm, double f0);

    /** \name Data */
    double fs;                    /**< sample rate */
    double fMin;                  /**< minimum expected fundamental */
    double fMax;                  /**< maximum expected fundamental */
    int    algo = 0;              /**< algorithm to use */
    int    precision = 3;
    double correlationLength   = 1.0;
    double correlationHighpass = 0.0;
    double bandPassWidth       = 1.0;
    int    bandpassSteepness   = 3;

  };

  //===============================================================================================

  /**

  This class implements resampling by means of windowed-sinc interpolation of the time-domain 
  signal.

  \todo: maybe implement different interpolation strategies, such as linear, polynomial, with or 
  without (bidirectional) IIR prefiltering, etc.

  \todo: let the user select the window function

  For the choice of the window function, we must make a tradeoff between the width of the mainlobe
  and the height of the sidelobes in the frequency domain. Narrower mainlobes give rise to a faster
  transition from passband to stopband - so, narrower is better. Lower sidelobes will produce less 
  ripple in the passband (passband ripple leads to undesired boosting or attenuation of 
  frequencies) and also less ripple in the stopband (stopband ripple leads to leaking through of 
  frequencies which we actually want to block) - so, lower sidelobes are better. However, a 
  narrower mainlobe will typically give rise to higher sidelobes which is why there has to be a 
  tradeoff. 
  ....explain this stuff further - how it relates to aliasing when downsampling and what it means 
  for upsampling

  \todo maybe rename to rsSincInterpolator

  It is recommended to use sinc kernel length of at least 8 samples, otherwise amplitude errors 
  will occur (i.e. the output signal is to quiet)...maybe we can compensate this effect


  \todo: make the signal type a template parameter such that the class can be used for multichannel
  signals

  maybe, it would be more convenient, if these functions would deal with std::vector or rsArray, so
  client code doesn't have to bother with signal lengths

  reorganize the RSAudio part of the library to have folders Processes, Realtime and Tools where
  Tools contains the general stuff (math-formulas, filter design, etc.), Realtime contains all 
  realtime objects and Processes contains all nonrealtime processes which access the whole signal
  at once

  make a file Preliminary.h/cpp which contains classes and functions that are currently under 
  construction - currently we have a lot of such code here in this file which should eventually
  be moved into appropriate places in the library

  */

  class RSLib_API rsResampler
  {

  public:

    /** \name Audio Processing */

    /** Allows for random access to signal values at noninteger time-instants t using windowed sinc 
    interpolation. 

    \todo move suitable parts of the comments for transposeSinc to here.
    
    */
    static double signalValueViaSincAt(double *x, int N, double t, double sincLength, 
      double stretch);

    /** Transposes a signal x of length xN by the given factor using linear interpolation and 
    stores the result in y which is assumed to an array of length yN. When the end of the input 
    signal x is reached before the end of the output signal y, the tail of y will be filled with 
    zeros. */
    static void transposeLinear(double *x, int xN, double *y, int yN, double factor);

    /** Like transposeLinear, but uses windowed sinc interpolation. You may pass the length of the
    sinc filter kernel to be used. This length is of type "double" because its interpretation is
    not the number of samples used but rather the domain on the continuous time-axis for which the 
    continuous time function is nonzero. The length can actually be any number >= 2, but for values
    < 10, noticable amplitude errors may occur in the output (in the sense that the output signal 
    is too quiet - todo: maybe compensate for that). In practice, it's recommended to use an even 
    integer >= 16. If antiAlias is true and the transposition factor is greater than unity, the 
    sinc function will be time-scaled by the transposition factor inside its window, thereby 
    changing its effective cutoff frequency to (sampleRate/2)/factor.

    The actual number of samples M used for interpolation is given by: 
    M = 2 * ( ((int)floor(sincLength)) / 2) + 1  [verify this formula] */
    static void transposeSinc(double *x, int xN, double *y, int yN, double factor,                        
      double sincLength = 64.0, bool antiAlias = true);

    /** Shifts an input signal x of length N by an arbitrary (noninteger) amount of samples using 
    sinc-interpolation and stores the result in y (also of length N). x and y may point to the same
    array in which case a temporary buffer will be used internally. */
    static void shiftSinc(double *x, double *y, int N, double amount, double sincLength = 64.0);

  };



  //===============================================================================================

  /** Implements time-warping of a signal by means of windowed-sinc interpolation.


  \todo move the class into a separate file (maybe)
  ....
  
  */


  class RSLib_API rsTimeWarper
  {

  public:


    /** Time warps a signal x of length xN to a signal y of length yN using the time warping map w
    which defines for every output sample y[n] a (noninteger) readout time instant in the input 
    signal, such that: y[n] = x[w[n]], n = 0,...,yN-1. So, the length of the warping map w must 
    also be yN. If you pass "true" for the antiAlias parameter, the function will for each output 
    sample look at the instantaneous readout speed (which equals the difference of successive 
    values in w) and adapt the cutoff frequency of the sinc-interpolator accordingly. That means, 
    the sinc-function will be stretched on the time axis by min(1, readSpeed). If 
    maxLengthScaler = 1, the sinc will be just stretched inside its window, the length of which 
    remains fixed. However, for values > 1, the length of the window itself will stretch along 
    with the sinc, up to some maximum stretch factor. It makes sense to use a window length 
    proportional to the stretch factor in order to retain the quality of the anti-aliasing also 
    for larger upward pitch transpositions. 

    Note: if you have a readout-speed for each output sample index n instead of a time-instant 
    where to read the input signal, you can convert your values into time-instants using 
    rsCumulativeSum. */
    static void timeWarpSinc(double *x, int xN, double *y, double *w, int yN,                        
      double minSincLength = 64.0, double maxLengthScaler = 1.0, bool antiAlias = true);
      // \todo provide a function to compute the nonzero length of the y-signal beforehand



    /** Given a (monotonuously increasing) time warping map w of length N, this function computes 
    the inverse warping map of w and stores it in wi which is of length ceil(w[N-1]), so the caller
    has to take care that the array wi is at least that long. */
    static void invertMonotonousWarpMap(double *w, int N, double *wi);

    /** Computes the resulting length of a signal when it is read with an instantaneous read-speed
    given by the array values in r which is of length N. */
    static int getPitchModulatedLength(double *r, int N);

    /** Modulates the pitch the pitch of the signal x (of length N) using the array of 
    instantaneous readout speeds r and stores the result in y. The length of y is can be determined
    beforehand using getPitchModulatedLength(). */
    static void applyPitchModulation(double *x, double *r, int N, double *y, 
      double minSincLength = 64.0, double maxLengthScaler = 1.0, bool antiAlias = true);

    /** Given an array of instantaneous (fundamental) frequencies of length N and a desired target
    frequency ft, this function computes the length of the output signal that will be created */
    static int getPitchDemodulatedLength(double *f, int N, double ft);


    //static void removePitchModulation(double *x, double *fx, int N, double *y, double fy,
    //  double minSincLength = 64.0, double maxLengthScaler = 1.0, bool antiAlias = true);

  };


  //===============================================================================================

  /** A class for variable speed readout of a signal.  */

  class RSLib_API rsVariableSpeedPlayer
  {

  public:

    /** \name Construction/Destruction */

    /** Standard Constructor. After calling it, you need to manually call setup. */
    rsVariableSpeedPlayer(); 

    /** Destructor. */
    ~rsVariableSpeedPlayer();  


    /** \name Setup */

    /** Sets up the input signal and the corresponding array of instantaneous read-speeds, both 
    assumed to be of the given length.*/
    void setInputAndSpeed(double *input, double *speeds, int length);


    /** \name Time Conversion */

    /** From a time index in the original signal tx (given in samples), compute the corresponding 
    time index in the warped signal.  */
    double warpTime(double tx);

    /** From a time index in the warped signal ty (given in samples), compute the corresponding 
    time index in the original signal. */
    double unwarpTime(double ty);


    /** \name Output Computation */

    /** Returns the length of the output signal. */
    int getOutputLength() { return Ny; }

    /** Computes the pitch-flattened output signal using sinc-interpolation and stored it in y. The
    passed pointer to the output array y must be of length given by getOutputLength(). The 
    remaining parameters determine the behavior of the sinc-interpolation - 
    @see rsTimeWarper::timeWarpSinc for their meaning. */
    void getOutput(double *y, double minSincLength = 64.0, double maxLengthScaler = 1.0, 
      bool antiAlias = true);

    /** Returns output as std::vector (more convenient). */
    vector<double> getOutput(double minSincLength = 64.0, double maxLengthScaler = 1.0, 
      bool antiAlias = true);


    /** \name Misc */

    /** Returns the warping map that assigns for each original time-index in input signal x the
    corresponding target time-index in output signal y. The length of the map equals the length
    of the input signal x. */
    vector<double> getTimeWarpMapXY();

    /** Returns the warping map that assigns for each time-index in the output signal y the 
    corresponding source time-index in imput signal x. The length of the map equals the length
    of the output signal y. */
    vector<double> getTimeWarpMapYX();


    /** Given a vector of read-speeds to be applied to an input signal x to give output signal y, 
    this function creates the corresponding vector of speeds to apply to y to get back x. */
    static vector<double> invertSpeeds(vector<double>& speeds);


    static vector<double> applyPlaybackSpeed(vector<double>& input, vector<double>& speeds);
      // rename to applyPlaybackSpeed


  protected:

    /** Initializes internal variables */
    void init();

    /** Clears the internal buffers */
    void clear();

    double *x;           // input signal
    double *w, *wi;      // warp-map and its inverse
    int Nx;              // length of x, wi
    int Ny;              // length of output signal y and warp map w

    // maybe exchange naming of w and wi - w should be the map that maps from original time to
    // target time (see DAFX book, which convention is used there) - or use wxy, wyx for warp
    // x-to-y, warp y-to-x

  };
  // todo: maybe factor out w, wi, Nx, Ny, x and the conversion and output processing functions 
  // into the rsTimeWarper class


  //===============================================================================================

  /** A class for flattening the pitch of an audio signal. It is assumed that the instantaneous 
  frequency is known at each sample instant. */

  class RSLib_API rsPitchFlattener : public rsVariableSpeedPlayer
  {

  public:

    /** \name Setup */

    /** Sets up the input signal, instantaneous frequency array and target frequency. */
    void setInput(double *input, double *frequencies, int length, double targetFrequency);

  };


  //===============================================================================================

  /** A class for crossfading between two signals in a way such that at each sample instant during
  the crossfade, the phases of both signals are matched. This is achieved by applying time varying 
  resampling to both signals. To make it work, you have to know the instantaneous frequencies of 
  both signals at each sample instant. It is used as follows: 
  (1) Pass the two signals along with their instantaneous frequency arrays via setInputs
  (2) Retrieve the two pitch-flattened versions of both signals via getFlattenedSignal1/2
  (3) Set up the crossfade start- and end time as well as a time-shift of the 2nd with respect
      to the first signal - all values are in samples with respect to the time domain of the 
      flattened signal x1. Here, you should take care that the phases match inside the flattnened
      signals - if this is the case, they will also be matched in the output during crossfade. You 
      may set those values manually or use whatever algorithm seems appropriate for that task.
  (4) Retrieve the output via getOutput  */

  class RSLib_API rsPhaseLockedCrossfader
  {

  public:

    /** \name Setup */

    /** Sets up the 2 input signals with corresponding instantaneous frequency arrays and a target
    value for the flattened frequency. If you pass 0.0 or nothing for the target frequency, the 
    mean value of both instananeous frequency arrays will be used. */
    void setInputs(double *input1, double *frequencies1, int length1, double *input2, 
      double *frequencies2, int length2, double targetFrequency = 0.0);

    /** Sets the start and end points of the crossfade in the time domain of the pitch-flattened 
    signal x1 (as returned by getFlattenedSignal1). The optional shift-parameter is a time shift
    of the 2nd (flattened) signal with respect to the first along the (pitch-flattened) time 
    axis. */
    void setFlattenedCrossfade(double start, double end, double shift = 0.0);


    /** \name Inquiry */

    /** Returns the length of the crossfade in the output signal (in samples). 
    At the momemnt, this is the (rounded) harmonic mean of the crossfade-lengths with respect to
    x1 and x2. */
    int getCrossfadeOutputLength();


    /** \name Processing */

    /** Returns pitch flattened version of input 1. */
    vector<double> getFlattenedSignal1();

    /** Returns pitch flattened version of input 2. */
    vector<double> getFlattenedSignal2();

    /** Returns crossfaded output signal. */
    vector<double> getOutput();


    /** \name Misc */

    vector<double> getTimeWarpMapXY1();
    vector<double> getTimeWarpMapYX1();
    vector<double> getTimeWarpMapXY2();
    vector<double> getTimeWarpMapYX2();
    // Return the warping maps and their inverses for inputs x1, x2. See comments in 
    // rsVariableSpeedPlayer getTimeWarpMapXY/YX

  protected:

    /** Computes and returns the crossfade section. */
    vector<double> getCrossfadeSection();

    /** Computes the time instants at which to read out x1 and x2 during the crossfade (and stores
    them in our t1, t2 members. */
    void computeReadoutTimes();


    double *x1, *x2;              // input signals
    int    N1, N2;                // lengths of x1, x2
    int    cs1, ce2;              // crossfade start in x1 and end in x2
    double cs2, ce1;              // crossfade start in x2 and end in x1
    double shift;                 // time-shift of x2 with respect to x1
    rsPitchFlattener pf1, pf2;    // pitch-flatteners for x1, x2
    vector<double> t1, t2;        // readout times for crossfade

  };

  //===============================================================================================

  /** A class for measurement of the instantaneous fundamental frequency of a signal. The algorithm
  assumes a stable pitch throughout the whole signal, i.e. it is not made for time-varying pitches
  although it may tolerate some moderate vibrato and glide (in fact, it was inititially written
  to analyze vibrato for the purpose of removing it). It is based on taking a preliminary estimate 
  of the fundamental by taking an autocorrelation from somewhere in the middle of the signal, 
  tuning a bandpass to that preliminary estimate, bandpassing the whole signal with that bandpass
  and determining the zero crossings of the bandpassed signal with subsample precision.

  \todo
  maybe rename to rsPitchDetector ...but there's already that realtime pitch detector class with
  this name - maybe add some tag like Realtime/Offline/Task/Process/whatever to one of them
  turn it into a real class with member variables (rather than a collection of static functions
  which take an insane number of parameters), let the user set up more of the now internally
  hardcoded parameters such as the number of passes for the bandpass, zero-crossing detection
  precision, bandwidth of bandpass, etc.), make more algorithms available (time-varying 
  autocorrelation based measurement, maybe spectral methods, etc.), also the pitchmarks need
  not the zero-crossings but could be maximum peaks or energy-centers of the cycles, etc.

  let the class have some member variables such that we don't always have to pass a large number of
  parameters into a static function
  */

  class RSLib_API rsInstantaneousFundamentalEstimator
  {

  public:




    /** Given an input signal x of length N, this function measures the instantaneous fundamental 
    frequency of the signal at each sample instant and stores the values in f (which must be also 
    of length N). fs is the samplerate, fMin and fMax are the minimum and maximum expected values 
    for the fundamental (these are needed to tune some internal parameters of the algorithm). The 
    algorithm is suitable only for signals whose pitch is more or less constant over the whole 
    length but it allows for some vibrato and also some moderate pitch glide but not for multiple 
    subsequent pitches as in a melody. The optional reliability parameter is also an array of 
    length N which will be filled with a reliability measure for the measured frequency values at 
    each sample. The relaibility measure is between 0 and 1. If you don't need this reliability 
    measure, just pass a nullpointer. The cycleMarkAlgo parameter determines, how we find the cycle
    starts/ends and can be any of the values in enum cycleMarkAlgorithms. */    
    static void measureInstantaneousFundamental(double *x, double *f, int N, double fs, 
      double fMin, double fMax, double *reliability = nullptr, 
      int cycleMarkAlgo = rsCycleMarkFinder::F0_ZERO_CROSSINGS); 

    /** Given an input signal x of length N, this function estimates the fundamental frequency of
    the signal at sample instant n using an autocorrelation based approach. Here, this estimate is
    only used as a rough inital estimate to tune the filters used inside the actual high-precision
    measurement algorithm. */
    static double estimateFundamentalAt(double *x, int N, int n, double fs, double fMin, 
      double fMax);

  protected:

    /** Given an array x of length N representing as our input signal and a vector z with the time 
    instants of the zero-crossings of x, this function computes a reliability measure for each 
    sample instant n (from 0...N-1) and stores it in the array r (of length N). The function is 
    used internally inside measureInstantaneousFundamental(). The reliability measure is based on 
    the cross-correlation of successive cycles in the signal x. */
    static void estimateReliability(double *x, int N, const std::vector<double>& z, double *r);

  };


  //===============================================================================================


  /** Given a sinusoidal signal x of length N with frequency f at samplerate fs, this function will
  produce a 90� phase-shifted "quadrature" component corresponding to the sine and store it in y by
  means of applying a 1st order allpass to x, with its 90� frequency adjusted to f. As there might 
  be transient/edge effects at the start of the signal, you may want to choose to apply the filter 
  backwards instead (considering that at the end, there's often a slow fade-out, edge effects tend 
  to be less annoying there). */
  void RSLib_API rsSineQuadraturePart(double *x, double *y, int N, double f, double fs, 
    bool backward = false);

  /** Exctracts the envelope of a signal x that is supposed to be an enveloped sinusoid at 
  frequency f and writes the result into y. N is the length and fs is the samplerate and s is a 
  smoothing parameter. The algorithm first obtains a 90� phase shifted "quadrature" sinusoid q by 
  allpass filtering the input sinusoid appropriately and then computes sqrt(x*x + q*q) as 
  instantaneous envelope. This, in turn, is optionally smoothed by running a first order lowpass 
  bi-directionally over the instantaneous envelope, the cutoff of this lowpass is set to f/s, such 
  that when s = 4 (a reasonable value), the cutoff will be one fourth of the sinusoids frequency. A
  zero value will avoid the smoothing entirely. */
  void RSLib_API rsSineEnvelopeViaQuadrature(double *x, double *y, int N, double f, double fs, 
    double smooth = 0.0);

  /** Similar to rsSineEnvelopeViaQuadrature, but uses internally the formula for computing an
  instantaneous amplitude as implemented in rsSineAmplitudeAndPhase.  */
  void RSLib_API rsSineEnvelopeViaAmpFormula(double *x, double *y, int N, double f, double fs, 
    double smooth = 0.0);

  /** Fills the y-array of length N with a sine-wave of frequency f at samplerate fs with initial 
  phase p and an amplitude envelope given by the a-array (wich must be also of length N. The 
  a-array may be the same array as y, in which case it will be overwritten.
  \todo: make the a-array optional - if NULL, create a unit amplitude sine */
  void RSLib_API rsEnvelopedSine(double *y, int N, double f, double fs, double p, double *a);


  /** Fills the signal y of length N with a linear sine sweep that reaches a frequency wN at sample
  index N (this sample doesn't exist anymore, but the function is made for appending a fixed 
  frequency sine of wN from index N onwards), starting at some frequency w0 chosen to be suitable, 
  such that at sample index 0 and sample index N, we will see instantaneous phases of p0 and pN, 
  respectively. The optional sweepDirection parameter leads to a downward sweep, if it's -1, to an 
  upward sweep if it's +1 and to an automatic decision according to the smallest frequency 
  deviation if it's 0. 
  \todo: check, if the criterion is really the smallest frequency deviation (we actually look
  at phase differences in the decision, but that seems to lead to the smallest frequency deviation 
  as well - but maybe not always? check the math...)
  \todo: maybe factor out a function for the instantaneous phases such that it can also be used for
  creating sweeps with other waveforms.  
  \todo: maybe make a function that creates a sweep with a parabolic shape instead of linear
  */
  void RSLib_API rsPhaseCatchSweep(double *y, int N, double p0, double pN, double wN,
    int sweepDirection = 0);

  /** Same as rsPhaseCatchSweep(double *y, int N, double p0, double pN, double wN,
  int sweepDirection = 0) but with an additional "a" array as parameter whichshould  contain 
  the amplitude envelope that should be used. "a" and "y" may point to the same arrays. */
  void RSLib_API rsEnvelopedPhaseCatchSweep(double *y, int N, double p0, double pN, double wN, 
    double *a, int sweepDirection = 0);

  /** Similar to rsCreateEnvelopedSine, but it additionally specifies a target-phase pk at some 
  sample index k that should be caught up there by sweeping the sine's frequency between sample 0 
  and sample k accordingly. The "y" and "a" arrays must be distinct.  */
  void RSLib_API rsEnvelopedPhaseCatchSine(double *y, int N, double f, double fs, double p0, 
    double pk, int k, double *a, int sweepDirection = 0);

  /** Given a sinusoidal input signal x of length N, having a frequency fx, this function extracts
  the amplitude envelope of the input sinusoid x and synthesizes a new sinusoid with frequency fy 
  with the same amplitude envelope and writes it into the y-array. Paramaters: x: input sine, 
  y: output sine, N: length, fx: input sine frequency, fy: output sine frequency, fs: sample-rate, 
  p0: initial phase for output, smooth: smoothing parameter for envelope extraction. */
  void RSLib_API rsRecreateSine(double *x, double *y, int N, double fx, double fy, double fs, 
    double p0, double smooth = 0.0);

  void RSLib_API rsRecreateSineWithPhaseCatch(double *x, double *y, int N, double fx, double fy, 
    double fs, double p0, double pk, int k, double smooth = 0.0, int sweepDirection = 0);




}

#endif
