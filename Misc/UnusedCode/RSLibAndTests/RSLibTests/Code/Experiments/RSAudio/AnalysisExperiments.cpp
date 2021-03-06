#include "AnalysisExperiments.h"



void autoCorrelation()
{
  static const int bufferSize = 512;
  double sampleRate = 44100;
  double frequency  = 3024.5;
  double startPhase = 0.0;


  int    k = 8;
  double d = 0.5;

  frequency  = sampleRate/(k+d); 

  //frequency  = sampleRate/10;    // exact peak at l=10,20,...
  //frequency  = sampleRate/10.1; 
  //frequency  = sampleRate/10.2; 
  //frequency  = sampleRate/10.5; 
  //frequency  = sampleRate/11;  // exact peak at l=11,22,...

  //frequency  = sampleRate/10.5; 
  //frequency  = sampleRate/10.25; 
  //startPhase = 0.0*PI;
  //startPhase = PI/8; 
  //startPhase = PI/4; 
  //startPhase = PI/3;
  //startPhase = PI/2;
  //startPhase = 1.0*PI;

  double t[bufferSize]; 
  //createTimeAxis(bufferSize, t, sampleRate);
  rsFillWithIndex(t, bufferSize);
  double buffer[bufferSize];  
  int n;
  for(n = 0; n < bufferSize; n++)
    buffer[n] = sin(n*2*PI*frequency/sampleRate + startPhase);

  // obtain sample autocorrelation function (ACF):
  double acf[bufferSize];
  rsAutoCorrelationFFT(buffer, bufferSize, acf);
  rsScale(acf, bufferSize, 1.0/acf[0]);

  // obtain unbiased ACF:
  double uacf[bufferSize];
  rsRemoveCorrelationBias(acf, bufferSize, uacf);

  //for(n = 0; n < bufferSize; n++)
  //  uacf[n] = acf[n]/(bufferSize-n);
  //normalize(uacf, bufferSize, 1.0);


  /*
  // obtain a smoothed acf by a 3-point moving average (MA) filter:
  double sacf[bufferSize];
  copyBuffer(acf, sacf, bufferSize);
  double h[3] = {1, 1, 0}; 
  sacf[0] = h[1]*acf[0] + h[2]*acf[1];
  for(n = 1; n <= bufferSize-2; n++)
    sacf[n] = h[0]*acf[n-1]+h[1]*acf[n]+h[2]*acf[n+1];
  sacf[bufferSize-1] = h[1]*acf[bufferSize-1] + h[2]*acf[bufferSize-2];
  scale(sacf, bufferSize, 1.0/sum(h, 3));
    // A kernel of h[-1] = 1, h[0] = 1, h[+1] = 0 seems to be good to avoid the situation where the 
    // 2nd peak becomes higher than the 1st for sinusoidal signals at f=fs/(k+1/2) (k integer).
    // The estimate of the exact peak location shifts by 0.5 due to this filter - this must be 
    // taken into account when the peak location is computed.
    */

  // Apply a 2-point moving average (MA) filter to the ACF (without the scaling by 1/2). This 
  // avoids octave errors when a sinusoid of f = fs/(k+1/2) (k integer) is applied. Later we will 
  // need to take accout of this by moving the exact location of the maximum by -0.5:  
  double sacf[bufferSize];
  rsCopyBuffer(acf, sacf, bufferSize);
  /*
  sacf[0] = 2*acf[0];
  for(int n = 1; n < bufferSize; n++)
    sacf[n] = acf[n] + acf[n-1];
  scale(sacf, bufferSize, 1.0/2.0); // the missing scaling of the MA
  */

  // find maximum peak index in the smoothed ACF:
  int maxIndex = rsFindHighestPeakIndex(sacf, bufferSize);

  // compute exact value of the index by parabolic interpolation:
  double a[3];
  fitQuadratic_0_1_2(a, &sacf[maxIndex-1]);
  double offset = 0.0;
  if( a[2] != 0.0 )
    offset = -0.5*a[1]/a[2];
  double exactIndex = (maxIndex-1)+offset;
  //exactIndex -= 0.5; // -0.5 takes the MA filter into account

  //double cOpt = acf[2*k+1] / acf[k]; 
  double cOpt = uacf[2*k+1] / uacf[k]; 

  double c;   
  //c = double (bufferSize) / (double) (bufferSize*(1+k));
  //double ks = (double) k / (double) bufferSize;
  c  = 1.0+1.0/(k*k);
  c *= c;


  //plotData(50, t, buffer, acf);
  plotData(bufferSize, t, acf, uacf);
  //plotData(40, t, buffer, acf, sacf);
  int dummy = 0;
}


// Find the deviation between the peak of an autocorrelation sequence for an input sinusoid
// of frequency f = fs/k and f = fs / (k+1/2). In the former case, there's an exact peak at 
// lag k, in the latter case, there are two equal values at lag k and k+1 which imply a peak
// halfway in between. We want to find out the difference (or ratio) between these cases.
void autocorrelationPeakVariation()
{
  static const int N      = 512;
  static const int kMin   = 3;
  static const int kMax   = 50;
  static const int kRange = kMax-kMin;

  double t[N]; 
  rsFillWithIndex(t, N);
  double x1[N];  
  double x2[N];  

  double d[kRange+1];
  double r[kRange+1];

  // the ratio depends on k, so we create a loop over k in the hope to "see" the function:
  int k;
  for(k = kMin; k <= kMax; k++)
  {
    // create sinusoid x1 with acf-peak at lag k:
    double fs = 44100;
    double f  = fs/k; 
    int n;
    for(n = 0; n < N; n++)
      x1[n] = sin(n*2*PI*f/fs);

    // create sinusoid x2 with acf-peak at lag k+1/2:
    f = fs/(k+0.5); 
    for(n = 0; n < N; n++)
      x2[n] = sin(n*2*PI*f/fs);

    // obtain (scaled, unbiased) sample autocorrelation of x1 and x2
    double acf1[N];
    rsAutoCorrelationFFT(x1, N, acf1);
    rsRemoveCorrelationBias(acf1, N, acf1);
    rsScale(acf1, N, 1.0/acf1[0]);
    double acf2[N];
    rsAutoCorrelationFFT(x2, N, acf2);
    rsRemoveCorrelationBias(acf2, N, acf2);
    rsScale(acf2, N, 1.0/acf2[0]);

    d[k-kMin] = acf1[k]-acf2[k]; // difference
    r[k-kMin] = acf1[k]/acf2[k]; // ratio

    //Plotter::plotData(3*k, t, x1, x2);
    //Plotter::plotData(5*k, t, acf1, acf2);
  }

  // plot the difference and ratio as function of k:
  double kValues[kRange+1];
  rsFillWithRangeLinear(kValues, kRange+1, (double)kMin, (double)kMax);
  //Plotter::plotData(kRange, kValues, d, rLog);
  //Plotter::plotData(kRange, kValues, r);
  //Plotter::plotData(kRange, kValues, rLog);

  // model the dependence of the difference d on the peak-location k with a rational function:
  double dModel[kRange+1];
  for(k = kMin; k <= kMax; k++)
    dModel[k-kMin] = 4.5/(3.0+k*k);

  // plot difference and its model as function of k:
  plotData(kRange, kValues, d, dModel);
}

void autoCorrelationPitchDetector()
{
  // parameters:
  static const int blockSize  = 256;   // size of the successive input blocks
  //static const int blockSize  = 290;   // size of the successive input blocks
  static const int numBlocks  = 500;   // number of blocks to process
  static const int bufferSize = 1024;  // internal buffersize of the pitch-detector
  //static const int bufferSize = 512;
  //static const int bufferSize = 2048;


  double sampleRate = 44100;
  double frequency  = 160.0; // frequency of the sinuosid


  frequency = 3520.0;
  //frequency = 3620.0;

  // comments about what happens are at blockSize = 256, bufferSize = 1024:
  //frequency = 3000.0; 
  //frequency = 3024.0;      // works well
  //frequency = 3024.5;      // jumps back and forth between correct and sub-octave
  //frequency = 3025.0;      // jumps into sub-octave
  //frequency = 3030.0;    // jumps into sub-octave
  //frequency = 3040.0;
  //frequency = sampleRate / 14.5;  // 3041.379...
  //frequency = 3056.0;    // jumps into sub-octave
  //frequency = 3057.0;    // jumps back and forth between correct and sub-octave
  //frequency = 3058.0;    // jumps back and forth between correct and sub-octave
  //frequency = 3059.0;    // works well
    // with a smaller bufferSize (512) it doesn't jump into the sub-octave

  //frequency = sampleRate/11;  // no bias

  //frequency = 4400.0;    // bias about +0.33, with sqrt about -0.21
  //frequency = 4410.0;    // == fs/10 -> no bias
  //frequency = 4420.0;    // bias about -0.33 Hz, with sqrt about +0.21
    // speculative conclusion: the estimate leans towards the frequencies whicha ere a divider of 
    // the sample-rate, the parabolic fit to the autocorrelation function has a maximum that leans
    // towards then inside

  //frequency = sampleRate/9;  // no bias
  //frequency = sampleRate / 9.7;

  rsAutoCorrelationPitchDetector pd;
  pd.setBufferSize(bufferSize);
  pd.setUpdateInterval(blockSize);
  pd.setSampleRate(sampleRate);
  pd.setMaxFundamental(8000.0);

  double x[blockSize];  // temporarily holds a block of input data

  double t[numBlocks];  // time-axis (in units of the input blocksize)
  double ft[numBlocks]; // true frequency
  double fe[numBlocks]; // estimated frequency
  double fm[numBlocks]; // running mean of estimated frequency
  double fs[numBlocks]; // exponentially smoothed estimate
  rsFillWithIndex(t,  numBlocks);
  rsFillWithValue(ft, numBlocks, frequency);

  double w    = 2*PI*frequency/sampleRate;
  double fSum = 0.0;
  int b, n;
  for(b = 0; b < numBlocks; b++)
  {
    for(n = 0; n < blockSize; n++)
      x[n] = sin( w * (b*blockSize+n) );
    fe[b]  = pd.processBlock(x, blockSize);
    fSum  += fe[b];
    fm[b]  = fSum / (b+1);
  }

  // obtain an exponentially smoothed estimated frequency:
  double y     = ft[0];
  double coeff = 0.95;
  for(b = 0; b < numBlocks; b++)
  {
    y     = coeff*y + (1-coeff)*fe[b];
    fs[b] = y;
  }

  // get the maximum error between true and estimated frequency:
  //double maxError = maxDeviation(ft, fe, numBlocks);
  double maxError = rsMaxDeviation(&ft[10], &fe[10], numBlocks-10);
  double bias     = rsMean(&fs[100], numBlocks-100) - ft[0];

  // plot the true frequency, the instantaneous estimate and the running mean of the current 
  // estimate:
  plotData(numBlocks, t, ft, fe, fs);
  //plotData(numBlocks, t, ft, fe);
}








void autoCorrelationPitchDetectorOffline()
{
  // We create a linear sine-sweep with known instantaneous frequency at each sample and try to 
  // retrieve that instantaneous frequency by measurement as accurate as possible

  int    N  = 2000;   // number of samples in input signal
  double fs = 44100.0; // sample rate
  double f1 = 440.0;   // start value for input signal frequency
  double f2 = 440.0;   // end value for input signal frequency
  double a  =   0.5;   // signal amplitude

  double *f  = new double[N]; // instantaneous frequency of input signal
  double *x  = new double[N]; // input signal
  double *fm = new double[N]; // measured instantaneous frequency

  // create the array with instantaneous frequencies of input signal:
  rsFillWithRangeLinear(f, N, f1, f2);

  // create input signal:
  createSineWave(x, N, f, a, fs);

  // measure the instantaneous frequency (preliminary):
  rsFillWithZeros(fm, N);
  int hopSize = 1;
  int n;
  for(n = 0; n < N; n += hopSize)
    fm[n] = rsInstantaneousFundamentalEstimator::estimateFundamentalAt(x, N, n, fs, 400.0, 5000.0);

  // OK - this is way off in terms of accuracy - we need the zero-crossing approach

  double *t = new double[N]; // time axis for plot
  createTimeAxis(N, t, 1);
  plotData(N, t, f, fm);
  //plotData(N, t, x);
  delete[] t;

  delete[] f;
  delete[] x;
  delete[] fm;
}

void crossCorrelationBestMatch()
{
  // we create two sinusoids at the same frequency with different start-phases and find the amount 
  // of shift required to align them for the best match

  static const int N1 = 2000;  // length of signal 1
  static const int N2 = 2100;  // length of signal 2
  double fs = 44100.0;         // samplerate
  double f  = 300.0;           // frequency of the sinusoids
  double p1 = 1.0;             // start-phase of signal 1
  double p2 = 4.0;             // start-phase of signal 2
  bool deBias = false;         // switch to use unbiased sample cross-correlation function

  // create the input signals:
  double x1[N1], x2[N2];
  double w = 2*PI*f/fs;
  int n;
  for(n = 0; n < N1; n++)
    x1[n] = sin(p1 + n*w);
  for(n = 0; n < N2; n++)
    x2[n] = sin(p2 + n*w);

  // find the maximum of the cross-correlation sequence with subsample precision:
  int N = rsMin(N1, N2);
  double lag     = rsGetShiftForBestMatch(x1, x2, N, deBias);
  double lagTrue = (p2-p1)/w;
  double error   = (lag-lagTrue)/lagTrue;

  // shift the 2nd signal so as to match the 1st:
  double y2[N2];
  rsResampler::shiftSinc(x2, y2, N2, lag, 64);

  // plot:
  plotData(N, 0, 1, x1, y2);     // original 1st and shifted 2nd signal

  // Observations: It turns out that the de-biasing may increase the correlation at later lags by 
  // amounts that let them be higher than the actually desired maximum which occurs earlier in the
  // sequence. On the other hand, not de-biasing makes the shift values inaccurate. 
  // Generally, the precision increases with increasing signal length and decreases with decreasing
  // frequency. Put another way: lower frequency require longer lengths for a given amount of
  // precision hwich is intuitive

  int dummy = 0;
}


// maybe move to RSMath experiments:
void combineFFTs()
{
  static const int N = 8;
  rsComplexDbl x[N], X[N], X1[N/2], X2[N/2], XX[N];

  // create test signal:
  rsRandomUniform(-1.0, 1.0, 0);  // to init the seed
  for(int n = 0; n < N; n++)
  {
    x[n].re = rsRandomUniform(-1.0, 1.0);
    x[n].im = rsRandomUniform(-1.0, 1.0);
  }

  // compute an N-point FFT:
  rsCopyBuffer(x, X, N);
  rsFFT(X, N);

  // compute two N/2-point FFTs (of the 1st and 2nd half of the buffer):
  rsCopyBuffer(x, X1, N/2);
  rsFFT(X1, N/2);
  rsCopyBuffer(&x[N/2], X2, N/2);
  rsFFT(X2, N/2);

  // combine the two N/2 point FFTs into one N-point FFT:
  rsComplexDbl wN2 = rsExpC( -rsComplexDbl(0, 1)*rsComplexDbl(PI/N) );
  rsComplexDbl w;
  for(int k = 0; k < N/2; k++)
  {
    w = rsPowC(wN2, rsComplexDbl(k+1));

    XX[2*k]   = X1[k] + X2[k];
    XX[2*k+1] = w*XX[2*k];       // ...the odd-numbered bins are still wrong
                                 // i think, it doesn't work out anyway
  }
}


void instantaneousFrequency()
{
  // we create a linear frequency sweep of a sinusoidal waveform and measure the instantaneous
  // frequency with rsSineFrequencyAt

  static const int N = 10000;  // number of samples
  double fs = 44100;           // sample rate
  double a  = 0.5;             // amplitude (maybe use a time-varying amplitude later)
  double f1 = 2000;            // start frequency
  double f2 = 1000;            // end frequency

  double small = 1.e-8;

  double f[N];                 // true instantaneous frequencies
  double fm1[N];               // measured instantaneous frequencies with core algorithm
  double fm2[N];               // ... with robustified algorithm
  double fm3[N];               // ... with robustified, refined algorithm
  double e1[N], e2[N], e3[N];  // relative measurement errors
  double x[N];                 // signal

  // create frequency array and signal:
  rsFillWithRangeLinear(f, N, f1, f2);
  createSineWave(x, N, f, a, fs);

  // measure instantaneous frequency and obtain error:
  for(int n = 0; n < N; n++)
  {
    fm1[n] = rsSineFrequencyAtCore(x, N, n, small); // normalized radian frequency
    fm2[n] = rsSineFrequencyAt(x, N, n, false);
    fm3[n] = rsSineFrequencyAt(x, N, n, true);

    fm1[n] = fs*fm1[n] / (2*PI);                    // frequency in Hz
    fm2[n] = fs*fm2[n] / (2*PI);
    fm3[n] = fs*fm3[n] / (2*PI);

    e1[n]  = (f[n] - fm1[n]) / f[n];
    e2[n]  = (f[n] - fm2[n]) / f[n];
    e3[n]  = (f[n] - fm3[n]) / f[n];
  }

  double maxError = rsMaxAbs(e3, N);
    // c = 0.76: 3.6285047484070877e-005
    // c = 0.75: 3.4832150523579422e-005
    // c = 0.72: 3.0473459643206141e-005

  double meanError = rsMean(e3, N);
    // c = 0.66: -1.2387131128786400e-006
    // c = 0.666: -2.4751832800983252e-007

  //plotData(N, 0, 1, f, fm1); // true and measured frequency with core algorithm
  //plotData(N, 0, 1, f, fm2); // true and measured frequency with robust algorithm
  //plotData(N, 0, 1, x, e1, e2);    // signal and measurement errors
  //plotData(N, 0, 1, x, e2);    // signal and measurement error of robust algorithm
  plotData(N, 0, 1, e2, e3);    // measurement error of robust algorithm

  // Observations:
  // with core algorithm (fm1):
  // When the sinusoid is not stable (i.e. it sweeps), there are errors in the measurement. These 
  // errors are largest at the zero crossings of the signal (and there, they may become excessive 
  // indeed) but are small at the peaks (positive or negative)
  // solution: for any sample, find the peaks before and above that sample, measure there 
  // and take the average, this is the fm2 meausrement. In this case, the measurements are musch 
  // more stable over time but there is a little bias: the freqeuncies are slightly underestimated
  // in case of an upward sweep and slightly overestimated in case of a downward sweep. 

  // more ideas:
  // (we may use an arithmetic or geometric mean - the choice will determine
  // whether we will be more exact in case of linear or exponential sweeps - maybe we can even
  // use a kind of mixed mean (geometric/arithmetic) with a user parameter to adjust the mix)
  // the average should be a weighted average, weighted by the distances to the peaks (maybe 
  // we can even use subsample-precision peak locations)

  int dummy = 0;
}

void instantaneousPhase()
{
  static const int N = 10000;  // number of samples
  double fs = 44100;           // sample rate
  double a1 = 0.5;             // start amplitude
  double a2 = 0.5;            // end amplitude
  double f1 = 1000;            // start frequency
  double f2 = 2000;            // end frequency
  double p0 = 0.0;            // start phase

  double w[N];                 // (true) instantaneous normalized radian frequencies
  double wm[N];                // measured values
  double we[N];                // relative measurement error
  double a[N];                 // (true) instantaneous amplitudes
  //double am[N];                // measured values
  //double ae[N];                // relative measurement error
  double p[N];                 // (true) instantaneous phases
  double pm1[N];               // measured values with algorithm based on euqations
  double pe1[N];               // relative measurement error
  double pm2[N];               // measured values with algorithm based on zeros crossings
  double pe2[N];               // relative measurement error
  double x[N];                 // signal

  int precision = 2;           // precision parameter for zero-crossing based measurement

  // fill arrays with true instantaneous (normalized radian) frequencies, phases and amplitudes:
  int n;
  rsFillWithRangeLinear(w, N, f1, f2);
  for(n = 0; n < N; n++)
    w[n] *= 2*PI/fs;
  rsFillWithRangeLinear(a, N, a1, a2);
  p[0] = p0;
  for(n = 1; n < N; n++)
    p[n] = rsWrapToInterval(p[n-1] + w[n], -PI, PI);

  // create signal:
  for(n = 0; n < N; n++)
    x[n] = a[n] * sin(p[n]);

  // measure instantaneous parameters:
  for(n = 0; n < N; n++)
  {
    wm[n] = rsSineFrequencyAt(x, N, n, true);
    pm1[n] = rsSinePhaseAt(x, N, n, wm[n]);
    pm2[n] = rsSinePhaseAtViaZeros(x, N, n, precision);
    //am[n] = rsSineAmplitudeAt(x, N, n, wm[n]); //we don't have this function yet
  }

  // compute errors
  for(n = 0; n < N; n++)
  {
    we[n] = (w[n] - wm[n]) / w[n];  // relative error
    pe1[n] = (p[n] - pm1[n]);       // absolute error
    pe2[n] = (p[n] - pm2[n]); 
  }
  

  //plotData(N, 0, 1/fs, x);
  //plotData(N, 0, 1, w, wm);  // true and measured normalized radian frequency
  //plotData(N, 0, 1, p, pm1, pm2);  // true and measured instantaneous phase
  //plotData(N, 0, 1, pe1, pe2);       // phase error
  plotData(N-3, 0, 1, pe1, pe2);   // last 3 are outliers - need to be cosidered as special case later


  int dummy = 0;
}


// convenience function to make the zero-crossing finding work for plain arrays (as required for 
// plotting)
void upwardZeroCrossings(double *x, int N, double *z, int Nz, int p)
{
  std::vector<double> zv = rsZeroCrossingFinder::upwardCrossings(x, N, p);
  for(int n = 0; n < rsMin(Nz, (int)zv.size()); n++)
    z[n] = zv[n];
}
void zeroCrossingFinder()
{
  // We create a linear sine-sweep for which the true time-instants of the zero crossings can be 
  // computed analytically and compare the measured values with these true values.

  static const int N  = 10000; // number of samples
  static const int Nh = 5;     // number of harmonics
  double fs = 44100;           // samplerate in Hz
  double f1 = 2000.0;          // signal frequency at start
  double f2 = 4000.0;          // signal frequency at end
  double p0 = 1.0;             // start phase
  double A[Nh];                // amplitudes of harmonics
  double *x = new double[N];   // input signal

  // fill array with harmonic amplitudes:
  A[0] = 1.0;
  A[1] = 0.0;
  A[2] = 0.3;
  A[3] = 0.0;
  A[4] = 0.0;

  // normalized radian frequencies:
  double w1 = 2*PI*f1/fs;
  double w2 = 2*PI*f2/fs;

  // parameters of linear function w(t) = a*t + b
  double b  = w1;              // offset
  double a  = (w2-w1)/(N-1);   // slope

  // create signal:
  int n;
  for(n = 0; n < N; n++)
    x[n] = sineSum(a*n*n + b*n + p0, A, Nh);

  // allcate memory for true and measured zero crossings:
  int Nz = rsZeroCrossingFinder::numUpwardCrossings(x, N);
  double *zt = new double[Nz]; // true zero crossing values
  double *z0 = new double[Nz]; // measurement with precision 0 (linear)
  double *z1 = new double[Nz]; // measurement with precision 1 (cubic)
  double *z2 = new double[Nz]; // measurement with precision 2 (quintic)
  double *z3 = new double[Nz]; // measurement with precision 3 (septic)

  // compute true zero-crossings: 
  double p = b/a;
  double q;
  int k = 0;
  n = 0;
  while( k < Nz )
  {
    //q = (p0-k*PI)/a;  // finds all zero crossings
    q = (p0-2*k*PI)/a; // finds only upward zero crossings
    if( q < 0.0 )
    {
      zt[n] = -p/2 + sqrt(p*p/4 - q);
      n++;
    }
    k++;
  }
  Nz = n; // ...why?

  // do the measurements:
  upwardZeroCrossings(x, N, z0, Nz, 0);
  upwardZeroCrossings(x, N, z1, Nz, 1);
  upwardZeroCrossings(x, N, z2, Nz, 2);
  upwardZeroCrossings(x, N, z3, Nz, 3);

  // compute measurement errors
  double *e0 = new double[Nz]; // error for precision 0 (linear)
  double *e1 = new double[Nz]; // error for precision 1 (cubic)
  double *e2 = new double[Nz]; // error for precision 2 (quintic)
  double *e3 = new double[Nz]; // error for precision 3 (heptic)
  rsSubtract(z0, zt, e0, Nz);
  rsSubtract(z1, zt, e1, Nz);
  rsSubtract(z2, zt, e2, Nz);
  rsSubtract(z3, zt, e3, Nz);

  // find maximum errors:
  double eMax0 = rsMaxAbs(e0, Nz);
  double eMax1 = rsMaxAbs(e1, Nz);
  double eMax2 = rsMaxAbs(e2, Nz);
  double eMax3 = rsMaxAbs(e3, Nz);

  // compute error ratios - these are the precision improvement factors by which choosing a higher 
  // precision value actually affects the precision
  double e10 = eMax0/eMax1;
  double e21 = eMax1/eMax2;
  double e32 = eMax2/eMax3;

  // plot:
  //plotData(N, 0, 1, x);
  plotData(Nz, 0, 1, e0, e1, e2, e3);

  // Observations: 
  // For a purely sinusoidal sweep (A[0]=1, A[n]=0 for n>0), the error increases linearly with 
  // frequency. Using higher precision values leads to a frequency dependent precision improvement. 
  // For a sweep between 200 and 400 Hz, the improvement from linear to cubic e10 is around 559, 
  // the improvent from cubic to quintic e21 is around 516 and the improvement from quintic to 
  // heptic is around 149. For a sweep from 2000 to 4000 Hz: e10=7.0, e21=6.6, e32=6.5 - so the 
  // precision improves more slowly at higher frequencies. Generally, it can be said that 
  // incrementing the precision setting by 1 will typically increase the actual precision by some 
  // factor which will be higher for lower frequencies. It's around of the order of 5 for 
  // frequencies around 4kHz and a few hundreds for frequencies around 400Hz. I tend to think, for 
  // typical input frequencies around a few hundreds to a few thousands of Hertz, the quintic 
  // (p=2) precision setting should be adequate.
  // If there are overtones present, the precision is generally much worse and also does increase
  // more slowly when going higher with the polynomial order. Also, the error increase with respect to
  // frequency looks not like a linear function anymore but more curved. If only a single overtone
  // is present, the error increases with the amplitude of that overtone.

  delete[] x;
  delete[] zt;
  delete[] z0;
  delete[] z1;
  delete[] z2;
  delete[] e0;
  delete[] e1;
  delete[] e2;
}

void zeroCrossingFinder2()
{
  // We just create a small array of -1, +1 values and see, if it finds the correct locations
  // (maybe, instead this could be put into a unit test and maybe use a somewhat longer signal with
  // more zero crossings in the signal (so that we don't just have some at the borders where the
  // detector is forced use a linear interpolant only)

  int p = 3; // precision

  static const int N = 10;
  double x[N];
  rsFillWithValue(x,       N/2, +1.0);
  rsFillWithValue(&x[N/2], N/2, -1.0);
  x[0]   = -1.0;
  x[N-1] = +1.0;

  // find zero crossings:
  std::vector<double> z = rsZeroCrossingFinder::upwardCrossings(x, N, p);
   // z should be: 0.5, 8.5
  int dummy = 0;
}

void cycleMarkFinder()
{
  // create input:
  static const int N  = 1000;  // number of samples
  double fs = 44100;           // samplerate in Hz
  double f  = 3000.0;           // signal frequency
  vector<double> x(N);         // input signal
  createSineWave(&x[0], N, f, 1.0, fs);

  // find cycle marks by different algorithms:
  rsCycleMarkFinder cmf(fs, 20, 5000);
  vector<double> cm1, cm2;
  cmf.setAlgorithm(rsCycleMarkFinder::F0_ZERO_CROSSINGS); 
  cm1 = cmf.findCycleMarks(&x[0], N);
  cmf.setAlgorithm(rsCycleMarkFinder::CYCLE_CORRELATION); 
  cm2 = cmf.findCycleMarks(&x[0], N);

  vector<double> deltas(cm1.size());
  rsSubtract(&cm1[0], &cm2[0], &deltas[0], cm1.size());

  // plot signal and cycle marks:
  int Nz = cm1.size();       // # cycle marks
  vector<double> cmy(Nz);    // y values for plotting (all zero)
  GNUPlotter plt;


  
  //plt.addDataArrays(deltas.size(), &deltas[0]); // test
  //plt.plot();

  plt.addDataArrays(N, &x[0]);
  plt.addDataArrays(Nz, &cm1[0], &cmy[0]);
  plt.addDataArrays(Nz, &cm2[0], &cmy[0]);
  plt.setGraphStyles("lines", "points", "points");
  plt.setPixelSize(1000, 300);
  plt.plot();
}

void applyBellFilter(double *x, double *y, int N, double f, double fs, double b, double g)
{
  // create and set up bell filter:
  rsStateVariableFilter flt; // we may use a biquad later as well (c/p legacy code to RSLib)
  flt.setMode(rsStateVariableFilter::BELL);
  flt.setFrequency(f);
  flt.setSampleRate(fs);
  flt.setBandwidth(b);
  flt.setGain(rsDB2amp(g));
  for(int n = 0; n < N; n++)
    y[n] = flt.getSample(x[n]);
}
void zeroCrossingPitchDetector()
{
  // we create a test signal with a given frequency (pulse-wave with 2 formants) and try to 
  // measure the instantaneous frequency at each sample with different algorithms

  static const int N = 30000;  // number of samples
  double fs  = 44100;          // samplerate in Hz
  double f   = 100.0;          // signal frequency
  double *x  = new double[N];  // signal
  double *f1 = new double[N];  // measured instantaneous frequency by realtime algo
  double *f2 = new double[N];  // ...realtime algo on preprocessed signal  
  double *f3 = new double[N];  // new algorithm - interpolate before convert
  int n;

  // synthesize input signal - a pulse-wave with 2 formants:
  synthesizePulseWave(x, N, f, 0.25, fs, 0.0, false);
  //synthesizePulseWave(x, N, f, 0.5, fs, 0.0, true);
  applyBellFilter(x, x, N,  750, fs, 0.2, 15);
  applyBellFilter(x, x, N, 1500, fs, 0.2, 15);
  rsScale(x, N, 0.25);
  //writeToMonoWaveFile("PitchDetectorInput.wav",  x, N, (int) fs, 16);

  // get initial estimate of fundamental by using an autocorrelation based algorithm at the center
  // of the input signal:
  double fi = rsInstantaneousFundamentalEstimator::estimateFundamentalAt(x, N, N/2, fs, 20.0, 
    5000.0);

  // detect the instantaneous frequency with the realtime algorithm, intialized with our initial
  // estimate - this gives our f1 array:
  rsZeroCrossingPitchDetector pd;
  pd.reset(fi);
  for(n = 0; n < N; n++)
    f1[n] = pd.estimateFundamentalFrequency(x[n]);

  // apply bidirectional bandpass filter to the signal, tuned to the estimated fundamental:
  double *y = new double[N];

  rsBiDirectionalFilter::applyConstPeakBandpassBwInHz(x, y, N, fi, fi, fs, 3);
    // bw and np chosen by a bit of trial and error - experiment more to find sweet spot

  // use realtime algo again, but this time on the filtered signal:
  pd.reset(fi);
  for(n = 0; n < N; n++)
    f2[n] = pd.estimateFundamentalFrequency(y[n]);

  // use the new nonrealtime algo:
  rsInstantaneousFundamentalEstimator::measureInstantaneousFundamental(x, f3, N, fs, 20.0, 5000.0); 


  double *ft = new double[N];  // target value
  rsFillWithValue(ft, N, f);
  //plotData(N, 0, 1/fs, x); // plot original signal
  //plotData(N, 0, 1/fs, y); // plot filtered signal
  //plotData(N, 0, 1/fs, x, y); // plot original and filtered signal
  plotData(N, 0, 1/fs, ft, f1, f2, f3);  // plot actual frequency and measurements
  //plotData(N, 0, 1/fs, ft, f3);  // plot frequency and estimate
  delete[] ft;

  // Observations: 

  // f1 - the raw frequency etsimate from the realtime algo:
  // depending on the input frequency, we may see a periodic error in the raw 
  // frequency estimate f1 obtained by the realtime algorithm - for example, if f=200, the estimate 
  // oscillates between 199.94 and 200.06 in a squarewave-like fashion, around the correct value of
  // 200. For f=220 or f=250, the pattern is more complicated. It seems to be a good idea to apply 
  // a moving-average filter adapted to the length of the period of this oscillation - but how do 
  // we determine this period? i think, it is a multiple of the signal's period - but what 
  // multiple? Or, maybe we should just apply (non-ringing) lowpass smoothing filter to the raw 
  // pitch-detector output.

  // f2 - the filtered input signal passed to the realtime algo:
  // for bw=0.3, np=5, f=220: the oscillations are greatly reduced, but we have problems at the 
  // edges of the signal - possibly due to not letting the bandpass filter ring out appropriately
  // in the bidirectional filtering process

  // f3: at f=100 Hz, it oscillates around the true value and is actually worse that the realtime
  // algorithm - there must be some bug - or maybe not?
  // Ahhh! i think, the two halfcycles have different lengths. maybe, we should only use the upward 
  // zero crossings instead of both. The oscillation gets worse, when the pulsewave becomes more
  // asymmetric

  // Notes on the offline algorithm:
  // There are "edge-effects", i.e. the measured pitch is (sometimes) slightly off at the very 
  // start and end of the signal. This could be traced to a slight mistuning of the bandpass filter
  // by which the signal is filtered first. It is tuned to a preliminary estimate of the 
  // fundamental given by an autocorrelation algorithm taken in the middle of the signal. If this
  // preliminary estimate is a bit off, the final measurement will be off in the same direction at
  // the edges. This is because at the edges, we see more of the transient response of the bandpass
  // and less of the actual signal content near the bandpass'es center frequency. A possible way to
  // counteract would be to apply a second pass of the whole algorithm, but this time with a 
  // refined bandpass center frequency taken as the average of the actual measured pitch (or some 
  // middle portion thereof, in order to not take the poorly measured edges into account in the 
  // averaging).

  // Further ideas:
  // In case of a missing (or weak) fundamental, we could apply a steep (elliptic?) bandpass to 
  // isolate the 2nd and 3rd harmonic, square this signal (this creates the fundamental as 
  // intermodulation product) and then use this algorithm on the resulting signal.


  delete[] x;
  delete[] f1;
  delete[] f2;
  delete[] f3;
  delete[] ft;
  delete[] y;
}

void zeroCrossingPitchDetectorTwoTones()
{
  // Creates 2 sinusoids with different frequencies and tries to detect the pitch of the sum of 
  // those signals.

  static const int N = 22050;  // number of samples

  double fs = 44100.0;  // sample rate
  double f1 = 95;       // frequency of 1st sinusoid
  double f2 = 105;      // frequency of 2nd sinusoid
  double a1 = 1.0;      // amplitude of 1st sinusoid
  double a2 = 1.0;      // amplitude of 2nd sinusoid
  double p1 = 0.0;      // phase of 1st sinusoid
  double p2 = 0.0;      // phase of 2nd sinusoid     

  double x[N], f[N], r[N]; // input signal, detected frequencies and reliabilities

  // create the signal:
  double w1 = 2*PI*f1/fs;
  double w2 = 2*PI*f2/fs;
  for(int n = 0; n < N; n++)
    x[n] = a1*sin(w1*n+p1) + a2*sin(w2*n+p2);


  // todo: refactor the function, such that we can use the zero-crossing based pitch detection
  // also on the unfiltered signal.

  rsInstantaneousFundamentalEstimator::measureInstantaneousFundamental(
    x, f, N, fs, 20.0, 5000.0, r);



  // plot:
  //plotData(N, 0.0, 1.0/fs, x);  // input signal
  plotData(N, 0.0, 1.0/fs, f);  // measured frequency
  //plotData(N, 0.0, 1.0/fs, r);  // reliability


  int dummy = 0;
}




