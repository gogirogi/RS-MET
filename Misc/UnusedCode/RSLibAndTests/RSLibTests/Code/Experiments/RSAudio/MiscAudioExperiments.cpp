#include "MiscAudioExperiments.h"

void centroid()
{
  double x[10] ={ 0,0,1,2,2,1,0,0,0,0 };

  double c; 
  c = rsCentroid(x, 10);         // should be 3.5
  c = rsCentroidOfEnergy(x, 10); // should still be 3.5

  int dummy = 0;
}

double cubicFadeIn(double x)
{
  return x*(x*((PI/2-2)*x+(3-PI))+(PI/2));
}
double cubicFadeOut(double x)
{
  return x*x*((2-PI/2)*x+(PI/2-3))+1;
}
void cubicCrossfade()
{
  // code needs update to use GNUPlotter:
  //Plotter p;
  ////p.plotTwoFunctions(rsCubicFadeIn, rsCubicFadeOut, -0.5, 1.5);
}

void recursiveSineSweep()
{
  // We create a linearly sweeeping complex exponential from which sine and cosine sweeps can be 
  // obtained by taking real and imaginary parts. We note that a complex eponential with constant
  // frequency can be obtained by the recursion z[n] = a * z[n-1] where a is a complex number of unit
  // magnitude and the angle of a determines the rotation speed. If a = exp(j*w), the angular 
  // frequency w gives the phase increment per sample for the output sine/cosine pair. If we now let
  // a itself be time varying by a[n] = b * a[n-1] where b is another unit magnitude complex number
  // of the form b = exp(j*dw), then dw gives the frequency increment per sample and z[n] will be
  // a sine sweep with linearly increasing frequency. This can be useful in the context of the
  // Bluestein FFT and chirp-Z transform where such linear chirp/sweep signals are used to modulate 
  // the input- and output sequences.

  double fs = 10000.0;   // sample rate
  double f0 = 10.0;      // start frequency
  double df = 30.0;      // frequency increment per second

  static const int N = 10000;
  double y[N];

  double w0 = 2*PI*f0/fs;         // initial normalized radian rotation frequency
  double dw = 2*PI*df/(fs*fs);    // increment for rotation frequency
  rsComplexDbl z(1, 0);           // initial value vor z
  rsComplexDbl j(0, 1);           // imaginary unit
  rsComplexDbl a = rsExpC(j*w0);  // initial rotation factor
  rsComplexDbl b = rsExpC(j*dw);  // multiplier for rotation factor

  for(int n = 0; n < N; n++)
  {
    y[n] = z.re;  // as output, we take the real part
    z *= a;       // update our rotating phasor (multiply by rotation factor)
    a *= b;       // update our rotation factor
  }

  plotData(N, 0.0, 1/fs, y);
  int dummy = 0;

  // Observations:
  // We see a linearly sweeping sinusoid that starts at 10 Hz and ends (after 1 second) at 
  // f0 + df = 10 + 30 = 40 Hz. Periods are 0.1s at the start and 0.025s at the end.

  // todo: generalize the idea to obtain a recursive frequency modulation - the factor b should be 
  // such that b.re = cos(u), b.im = sin(u) for some u that is determined by the modulation index.
  // that way, b would always have unit magnitude and its angle would oscillate with zero mean.
}

void ringModNoise()
{
  // We generate a kind of pseudo-noise by a generalized amplitude-/ringmodulation of sinusoids. Our
  // signal will be generated as:
  // y(t) = product_{k=1}^K (a * s_k + b) where s_k = sin(w_k * t + p_k)
  // for a=1,b=0 we have a pure ringmodulation, for a=b=0.5 we have a pure amplitude modulation.

  // user parameters:
  static const int K = 9;          // number of sinusoids
  static const int N = 88200;      // number of samples to generate
  double f0 = 1000.0;              // anchor frequency
  double fs = 44100.0;             // samplerate
  double a  = 1.0;                 // multiplier for sinusoid 
  double b  = 0.5;                 // constant term
  double r  = 5.0;                 // frequency ratio between f[0] and f[K] (f[K] is not actually
                                   // produced, though - we only go up to f[K-1])

  // algorithm variables:
  int k, n;                        // index for sinusoid and sample
  double f[K], w[K], p[K];         // frequencies, normalized radian frequencies, phases
  double x[N];                     // output signal

  // fill arrays with sine frequencies and phases:
  for(k = 0; k < K; k++)
  {
    f[k] = f0 * pow(r, double(k)/K);
    w[k] = 2*PI*f[k]/fs;
    p[k] = 0.0;
  }

  // generate signal:
  for(n = 0; n < N; n++)
  {
    x[n] = 1.0;
    for(k = 0; k < K; k++)
      x[n] *= a*sin(w[k]*n + p[k]) + b;

    //x[n] = (x[n]-b)/b;   // DC adjust and normalize (not sure yet about the formula)
  }

  // we have a DC offset of..

  // normalize and write to file:
  rsNormalize(x, N, 1.0, true);
  writeToMonoWaveFile("RingModNoise.wav",  x, N, (int) fs, 16);

  // plot signal:
  plotData(8000, 0.0, 1/fs, x);

  // Observations:
  // with a=1.0, b=0.5 all frequencies in the output signal have the same amplitude
  // with exponential frequency spacing of the sinusoids, the signal becomes more spikey as K goes 
  // up, to counteract, one can increase r
  // K=9,f0=1000,a=1,b=0.5,r=5.0: it sounds a bit like vinyl crackling noise
  // ...it sounds a lot like crackling anyway.

  int dummy = 0;
}

void slewRateLimiterLinear()
{
  double fs      = 10;   // samplerate in Hz
  double attack  = 1000; // attack time in ms
  double release = 2000; // release time in ms

  // Attack- and release times are 1 and 2 seconds respectively. With a sample-rate of 10 Hz, that
  // translates to 10 and 20 samples to ramp up and down between 0 and 1, respectively

  rsSlewRateLimiterLinear slewRateLimiter;

  slewRateLimiter.setSampleRate(fs);
  slewRateLimiter.setAttackTime(attack);
  slewRateLimiter.setReleaseTime(release);

  static const int N = 300;
  double t[N];  // time axis
  double x[N];  // input signal
  double y[N];  // output signal

  rsFillWithIndex(t, N);
  rsScale(t, N, 1.0/fs);

  rsFillWithZeros(x, N);
  rsFillWithZeros(y, N);

  int n;
  for(n = N/3; n < 2*N/3; n++)
    x[n] = 1.0;

  for(n = 0; n < N; n++)
    y[n] = slewRateLimiter.getSample(x[n]);

  plotData(N, t, x, y);
}

void stretchedCorrelation()
{
  // Compute a crosscorrelation value between two signals of different length, where the shorter 
  // one is first stretched (by linear interpolation) to the length of the longer one. This kind 
  // of correlation might be a good measure of waveform similarity regardless of the lengths, 
  // i.e. an array with a waveform and another array with a time compressed version of that 
  // waveform should have a "stretched" correlation value of 1 (up to error due to linear 
  // interpolation)

  static const int N1 = 120;  // length of array 1
  static const int N2 = 100;  // length of array 2
  double x1[N1], x2[N2];

  // fill x1, x2 with sinuosids such that one cycle fits exactly into the respective array:
  double w1 = 2*PI/N1;
  double w2 = 2*PI/N2;
  int n;
  for(n = 0; n < N1; n++)
    x1[n] = sin(w1*n);
  for(n = 0; n < N2; n++)
    x2[n] = sin(w2*n);

  // compute regular and stretched cross-correlation (the 2nd also with swapped arguments)
  double c, cs12, cs21;
  c    = rsCrossCorrelation(x1, N1, x2, N2);           // depends on the length ratio N1/N2
  cs12 = rsStretchedCrossCorrelation(x1, N1, x2, N2);  // should be close to 1
  cs21 = rsStretchedCrossCorrelation(x2, N2, x1, N1);  // should equal cs12

  plotData(N2, 0, 1, x1, x2);
}

void taperedFourierSeries()
{
  static const int numHarmonics = 20;
  static const int numSamples   = 5000;
  double tMin = -PI;
  double tMax = 3*PI;

  double a[numHarmonics+1];   // harmonic amplitudes without tapering
  double aL[numHarmonics+1];  // Lanczos sigma tapering factors
  double aF[numHarmonics+1];  // Fejer tapering factors
  double p[numHarmonics+1];   // phases
  double t[numSamples];
  double x[numSamples];       // signal without tapering
  double xL[numSamples];      // signal with Lanczos tapering
  double xF[numSamples];      // signal with Fejer tapering
  double f[numHarmonics+1];   // normalized frequencies for plot

  rsFillWithRangeLinear(f, numHarmonics+1, 0.0, (double) numHarmonics);

  int n, k;
  a[0]  = 0.0;
  p[0]  = 0.0;
  aL[0] = 1.0;
  for(k = 1; k <= numHarmonics; k++)
  {
    a[k] = (2/PI) / k;
    aL[k] = sin(k*PI/numHarmonics) / (k*PI/numHarmonics); // Lanczos sigma factors (sinc-function?)
    p[k] = 0.0;
  }
  for(k = 0; k <= numHarmonics; k++)
    aF[k] = 1.0 - (double) k / (double) (numHarmonics+1);


  // experimental - juggle the phases:
  //p[2] = 0.25*PI;


  rsFillWithRangeLinear(t, numSamples, tMin, tMax);
  double tmp;
  for(n = 0; n < numSamples; n++)
  {
    x[n]  = 0.0; 
    xL[n] = 0.0;
    xF[n] = 0.0;
    for(k = 0; k <= numHarmonics; k++)
    {
      tmp    = a[k] * sin(k*t[n] + p[k]);
      x[n]  += tmp;
      xL[n] += aL[k] * tmp;
      xF[n] += aF[k] * tmp;
    }
  }

  double peak = rsMaxAbs(x, numSamples);

  //plotData(numHarmonics+1, f, aL, aF);
  plotData(numSamples, t, x, xL, xF);
}

// move to filterAnalyzer
void directFormImpulseResponse(double *a, int Na, double *b, int Nb, double *h, int Nh)
{
  double x0 = 1.0;
  rsFilter(&x0, 1, h, Nh, b, Nb, a, Na);
}

void transientModeling()
{
  // We try to model a transient by an impulse response of an IIR filter. Consider the simple impulse
  // response h[n] = {0.8, 0.2, 0.1}. As model filter, we use a filter of the form
  // y[n] = b0*x[n] + b2*x[n-2] - a1*y[n-1]
  // we can solve for the coefficients by means of a linear system
  // y[0] = h[0] = 0.8 = b0*0.8 + b2*0 - a1*0    -> b0 =  0.8
  // y[1] = h[1] = 0.2 = 0.8*0 + b2*0 - a1*0.8   -> a1 = -0.25
  // y[2] = h[2] = 0.1 = 0.8*0 + b2*1 + 0.25*0.2 -> b2 =  0.05

  // In general, when the impulse response to be modeled is of length N, we use feedforward 
  // coefficients 1...N/2, feedforward coeffs (N+1)/2...N-1 ->check these formulas

  double h[10] ={ 0.8, 0.2, 0.1, -0.2, -0.4, 0.2, 0.1, 0.6, -0.5, -0.1 };
  double a[5], b[5];  

  static const int Ny = 100;  // length of filter output signal
  double y[Ny];

  // model 3-term impulse response:
  a[0] =  1.0;
  a[1] = -0.25;
  b[0] =  0.8;
  b[1] =  0.0;
  b[2] =  0.05;

  directFormImpulseResponse(a, 1, b, 2, y, Ny);

  plotData(10, 0, 1, y);

  // Observations: well, yeah, the first 3 samples match, as constructed, however, i think,
  // the idea will not scale up well.
  // Maybe it's a better idea to create a pole/zero model
  // for the transient and implement that model as cascade of biquads. that way, we would have
  // control over the frequency distribution of the transient (we could scale all frequencies
  // of all poles, zeros) and the time time duration by scaling the Q-values of all poles zeros.
  // we could also broaden or narrow down the transient spectrum by contracting pole/zero 
  // frequencies from a center. So, with such a model, we could edit the transient in a macro
  // oriented way. user parameters could be: duration, frequency-scale, spectral-contraction, 
  // contraction center frequency
  // perhaps the best way to apply transformations is to convert to the s-domain first, transform
  // there and transform back to the z-domain. the filter cofficient values will depend on the
  // samplerate at which the to-be-modeled signal was sampled. it would be nice to be able to
  // store and transform them in a sample-rate independent way. using oversampling in the 
  // estimation in order to obtain more "analog" coefficients might fail because a lot of coeffs
  // might then be used to model the bandlimitation of the oversampled signal.



  int dummy = 0;
}

void windowFunctionsContinuous()
{
  // plots the windows generated by our functions which for continuous time-domain windows

  static const int N = 5000;   // number of values for plot
  double xMin   = -10.0;       // minimum value for x-axis
  double xMax   = +10.0;       // maximum value for x-axis
  double length =  16.0;       // window length

  double x[N];                 // input values
  double wHann[N];             // Hann window values
  double wHamming[N];          // Hamming window values
  double wExactBlackman[N];    // "exact" Blackman window values

  // generate the window functions:
  rsFillWithRangeLinear(x, N, xMin, xMax);
  for(int n = 0; n < N; n++)
  {
    wHann[n]          = rsRaisedCosineWindow( x[n], length, 0.0);
    wHamming[n]       = rsRaisedCosineWindow( x[n], length, 0.08);
    wExactBlackman[n] = rsExactBlackmanWindow(     x[n], length);
  }

  plotData(N, x, wHann, wHamming, wExactBlackman);
}


void windowedSinc()
{
  static const int N = 5000;   // number of values
  double xMin    = -10.0;
  double xMax    = +10.0;
  double length  = 16.0;       // window length
  double stretch = 1.2;        // stretch factor of the sinc function

  double x[N];                 // input values
  double s[N];                 // normalized sinc values
  double w[N];                 // window values
  double y[N];                 // windowed sinc values
  rsFillWithRangeLinear(x, N, xMin, xMax);
  for(int n = 0; n < N; n++)
  {
    s[n] = rsNormalizedSinc(x[n]/stretch);
    w[n] = rsCosineSquaredWindow(x[n], length);
    y[n] = rsWindowedSinc(x[n], length, stretch);
  }

  plotData(N, x, y, s, w);
}