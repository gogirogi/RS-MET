using namespace RSLib;

// Construction/Destruction:

rsOnePoleFilter::rsOnePoleFilter()
{
  shelvingGain = 1.0;
  setSampleRate(44100.0);  // sampleRate = 44100 Hz by default
  setMode      (0);        // bypass by default
  setCutoff    (20000.0);  // cutoff = 20000 Hz by default
  reset();                 // reset memorized samples to zero
}

// Setup:

void rsOnePoleFilter::setSampleRate(double newSampleRate)
{
  if( newSampleRate > 0.0 )
    sampleRate = newSampleRate;
  sampleRateRec = 1.0 / sampleRate;

  calcCoeffs();
  return;
}

void rsOnePoleFilter::setMode(int newMode)
{
  mode = newMode; // 0:bypass, 1:Low Pass, 2:High Pass
  calcCoeffs();
}

void rsOnePoleFilter::setCutoff(double newCutoff)
{
  if( (newCutoff > 0.0) && (newCutoff <= 20000.0) )
    cutoff = newCutoff;
  else
    cutoff = 20000.0;

  calcCoeffs();
  return;
}

void rsOnePoleFilter::setShelvingGain(double newGain)
{
  if( newGain > 0.0 )
  {
    shelvingGain = newGain;
    calcCoeffs();
  }
  else
    RS_DEBUG_BREAK; // this is a linear gain factor and must be >= 0.0
}

void rsOnePoleFilter::setShelvingGainInDecibels(double newGain)
{
  setShelvingGain(rsDB2amp(newGain));
}

void rsOnePoleFilter::setCoefficients(double newB0, double newB1, double newA1)
{
  b0 = newB0;
  b1 = newB1;
  a1 = newA1;
}

void rsOnePoleFilter::setInternalState(double newX1, double newY1)
{
  x1 = newX1;
  y1 = newY1;
}

// Inquiry:

double rsOnePoleFilter::getMagnitudeAt(double f)
{
  return onePoleMagnitudeAt(b0, b1, -a1, 2*PI*f*sampleRateRec);
    // we use a different sign-convention for the a1 coefficient here ...maybe fix this someday
}

// Misc:

void rsOnePoleFilter::calcCoeffs()
{
  switch(mode)
  {
  case LOWPASS: 
    {
      // formula from dspguide (impulse invariant):
      double x = exp(-2.0 * PI * cutoff * sampleRateRec); 
      b0 = 1-x;
      b1 = 0.0;
      a1 = x;
    }
    break;
  case HIGHPASS:  
    {
      // formula from dspguide (impulse invariant):
      double x = exp(-2.0 * PI * cutoff * sampleRateRec);
      b0 =  0.5*(1+x);
      b1 = -0.5*(1+x);  // = -b0 -> optimize
      a1 = x;
    }
    break;
  case ALLPASS:  
    {
      // formula from DAFX (bilinear):
      double t = tan(PI*cutoff*sampleRateRec);
      double x = (t-1.0) / (t+1.0);

      b0 = x;
      b1 = 1.0;
      a1 = -x;
    }
    break;
  case LOWSHELV:
    {
      // formula derived as special case of the Orfanidis equalizers:
      double g    = rsDB2amp(shelvingGain);
      double wc   = 2*PI*cutoff*sampleRateRec;
      double wa   = 2*sampleRate*tan(wc/2);
      double gb   = rsSqrt(g);
      double beta = rsSqrt( (gb*gb-1)/(g*g-gb*gb) );
      double pa   = -beta*wa;
      double za   = -g*beta*wa;
      double s    = 0.5*sampleRateRec;
      double p    = (1+s*pa)/(1-s*pa);
      double z    = (1+s*za)/(1-s*za);
      b1          = -z;
      a1          = -p;
      double n    = rsSqrt((1+a1*a1-2*a1) / (1+b1*b1-2*b1));
      b0          = n;
      b1         *= n;
    }
    break;
  case HIGHSHELV:
    {
      // formula derived as special case of the Orfanidis equalizers:
      double g    = rsDB2amp(shelvingGain);
      double wc   = 2*PI*cutoff*sampleRateRec;
      double wa   = 2*sampleRate*tan(wc/2);
      double gb   = rsSqrt(g);
      double beta = rsSqrt( (gb*gb-1)/(g*g-gb*gb) );
      double pa   = -beta*wa;
      double za   = -g*beta*wa;
      double s    = 0.5*sampleRateRec;
      double p    = (1+s*pa)/(1-s*pa);
      double z    = (1+s*za)/(1-s*za);
      b1          = -p;
      a1          = -z;
      double n    = rsSqrt((1+a1*a1+2*a1) / (1+b1*b1+2*b1));
      b0          = n;
      b1         *= n;
      // \todo get rid of the code duplication
    }
    break;
  case LOWSHELV_DAFX:
    {
      // formula from DAFX:
      double c = 0.5*(shelvingGain-1.0);
      double t = tan(PI*cutoff*sampleRateRec);
      double a;
      if( shelvingGain >= 1.0 )
        a = (t-1.0)/(t+1.0);
      else
        a = (t-shelvingGain)/(t+shelvingGain);

      b0 = 1.0 + c + c*a;
      b1 = c + c*a + a;
      a1 = -a;
    }
    break;
  case HIGHSHELV_DAFX:
    {
      // formula from DAFX:
      double c = 0.5*(shelvingGain-1.0);
      double t = tan(PI*cutoff*sampleRateRec);
      double a;
      if( shelvingGain >= 1.0 )
        a = (t-1.0)/(t+1.0);
      else
        a = (shelvingGain*t-1.0)/(shelvingGain*t+1.0);

      b0 = 1.0 + c - c*a;
      b1 = a + c*a - c;
      a1 = -a;
    }
    break;
  default: // bypass
    {
      b0 = 1.0;
      b1 = 0.0;
      a1 = 0.0;
    }
    break;
  }
  // \todo provide IIT, BLT, and magnitude-matched (MMT) transform versions
  // get rid of these strange names DAFX, etc.
}

void rsOnePoleFilter::reset()
{
  x1 = 0.0;
  y1 = 0.0;
}
