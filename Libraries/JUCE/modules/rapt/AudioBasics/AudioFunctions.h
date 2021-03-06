#ifndef RAPT_AUDIOFUNCTIONS_H_INCLUDED
#define RAPT_AUDIOFUNCTIONS_H_INCLUDED

/** Functions that are specific to audio-processing and musical applications such as 
conversion between amplitudes and decibels, frequencies and midi-pitches, beats and seconds, 
etc. */

/** Converts a raw amplitude value/factor to a value in decibels. */
template<class T>
inline T rsAmpToDb(T amp)
{
  return T(8.6858896380650365530225783783321) * log(amp);
}

/** Converts a raw amplitude value/factor to a value in decibels with a check, if the amplitude
is close to zero (to avoid log-of-zero
and related errors). */
template<class T>
inline T rsAmpToDbWithCheck(T amp, T lowAmplitude)
{
  if( amp >= lowAmplitude )
    return rsAmpToDb(amp);
  else
    return rsAmpToDb(lowAmplitude);
}

/** Converts a time-stamp given in beats into seconds acording to a tempo measured in beats per
minute (bpm). */
template<class T>
inline T rsBeatsToSeconds(T beat, T bpm)
{
  return (60 / bpm)*beat;
}

/** Converts a value in decibels to a raw amplitude value/factor. */
template<class T>
inline T rsDbToAmp(T dB)
{
  return exp(dB * T(0.11512925464970228420089957273422));
}

/** Given a value x between 0 and 1, this function returns a value of a cubic polynomial that 
can be used as fade-in function. The polynomial satisfies: y(0)=0, y'(0)=pi/2, y(1)=1, y'(1)=0.
Together with rsCubicFadeOut, the curve approximates a constant power (cross)fade. */
template<class T>
inline T rsCubicFadeIn(T x)
{
  //return x*(1+x*(1-x)); // slope = 1 at x = 0
  return x*(x*((PI/2-2)*x+(3-PI))+(PI/2));
}

/** Given a value x between 0 and 1, this function returns a value of a cubic polynomial that 
can be used as fade-out function. The polynomial satisfies: y(0)=1, y'(0)=0, y(1)=0, y'(1)=-pi/2.
Together with rsCubicFadeIn, the curve approximates a constant power (cross)fade. */
template<class T>
inline T rsCubicFadeOut(T x)
{
  //return x*x*(x-2)+1;   // slope = -1 at x = 1
  return x*x*((2-PI/2)*x+(PI/2-3))+1;
}

/** Converts a frequency in Hz into a MIDI-note value. It can be used also for tunings different 
than the default the 440 Hz. */
template<class T>
inline T rsFreqToPitch(T freq, T masterTuneA4)
{
  return 12.0 * rsLog2(freq / masterTuneA4) + 69.0;
}

/** Converts a pitch-offset in semitones value into a frequency multiplication factor. */
template<class T>
inline T rsPitchOffsetToFreqFactor(T pitchOffset)
{
  return exp(0.057762265046662109118102676788181 * pitchOffset);
  //return pow(2.0, pitchOffset/12.0); // naive, slower but numerically more precise
}

/** Converts a MIDI-note value into a frequency in Hz assuming A4 = 440 Hz. */
template<class T>
inline T rsPitchToFreq(T pitch)
{
  return 8.1757989156437073336828122976033 * exp(0.057762265046662109118102676788181 * pitch);
  //return 440.0*( pow(2.0, (pitch-69.0)/12.0) ); // naive, slower but numerically more precise
}

/** Converts a MIDI-note value into a frequency in Hz for arbitrary master-tunings of A4. */
template<class T>
inline T rsPitchToFreq(T pitch, T masterTuneA4)
{
  return masterTuneA4 * 0.018581361171917516667460937040007
    * exp(0.057762265046662109118102676788181 * pitch);
}

/** Converts a time value in seconds into a time value measured in beats. */
template<class T>
inline T rsSecondsToBeats(T timeInSeconds, T bpm)
{
  return timeInSeconds * (bpm/60.0);
}

/** Converts a time-constant (typically denoted as "tau") of an exponential decay function to
the time-instant at which the decay reaches the given level in decibels. For example
tauToDecayTime(tau, -60.0) returns the time required to fall to -60 dB for the given value of
tau. */
template<class T>
inline T rsTauToDecayTime(T tau, T decayLevel)
{
  static const T k = LN10 / 20.0;
  return -k*tau*decayLevel;
}

/** Converts a time-stamp given in whole notes into seconds according to a tempo measured
in beats per minute (bpm). */
template<class T>
inline T rsWholeNotesToSeconds(T noteValue, T bpm)
{
  return (240.0/bpm) * noteValue;
}

#endif