#ifndef RS_ANALYSISEXPERIMENTS_H
#define RS_ANALYSISEXPERIMENTS_H

#include "../ExperimentsUtilities.h"

void autoCorrelation();
void autocorrelationPeakVariation();

void autoCorrelationPitchDetector();
void autoCorrelationPitchDetectorOffline();

void crossCorrelationBestMatch();

void combineFFTs(); // move to RSMath experiments

//void zeroCrossingPitchDetector();

void instantaneousFrequency();
void instantaneousPhase();

void zeroCrossingFinder();
void zeroCrossingFinder2();
void cycleMarkFinder();
void zeroCrossingPitchDetector();
void zeroCrossingPitchDetectorTwoTones();




#endif
