#ifndef jura_FuncShaper_h
#define jura_FuncShaper_h

/** This class wraps rosic::FuncShaper into a jura::AudioModule to facilitate its use as plugIn. */

class FuncShaperAudioModule : public ModulatableAudioModule
{

  friend class FuncShaperModuleEditor;

public:

  //---------------------------------------------------------------------------------------------
  // construction/destruction:

  FuncShaperAudioModule(CriticalSection *newPlugInLock, rosic::FuncShaper *funcShaperToWrap, 
    MetaParameterManager* metaManagerToUse = nullptr, 
    ModulationManager* modManagerToUse = nullptr);

  FuncShaperAudioModule(CriticalSection *newPlugInLock, 
    MetaParameterManager* metaManagerToUse = nullptr,
    ModulationManager* modManagerToUse = nullptr);

  virtual ~FuncShaperAudioModule();

  AudioModuleEditor* createEditor() override;

  //---------------------------------------------------------------------------------------------
  // automation and state management:

  virtual void parameterChanged(Parameter* parameterThatHasChanged) override;

  virtual void setStateFromXml(const XmlElement& xmlState, const juce::String& stateName,
    bool markAsClean) override;

  virtual XmlElement* getStateAsXml(const juce::String& stateName, bool markAsClean) override;

  //---------------------------------------------------------------------------------------------
  // parameter settings:

  virtual void setSampleRate(double newSampleRate) override
  {
    wrappedFuncShaper->setSampleRate(newSampleRate);
  }

  // formula parameter range settings: 
  void setFormulaParameterMinValue(const juce::String& augmentedName, double newMinValue);
  void setFormulaParameterMaxValue(const juce::String& augmentedName, double newMaxValue);
  void setFormulaParameterRange(   const juce::String& augmentedName, double newMinValue, 
    double newMaxValue);

  // callback targets:
  void setA(double newA) { wrappedFuncShaper->setA(newA, true); }
  void setB(double newB) { wrappedFuncShaper->setB(newB, true); }
  void setC(double newC) { wrappedFuncShaper->setC(newC, true); }
  void setD(double newD) { wrappedFuncShaper->setD(newD, true); }

  void setMinA(double newMin) { setFormulaParameterMinValue("aMin", newMin); } // correct to pass "aMin" - or just "a"?
  void setMinB(double newMin) { setFormulaParameterMinValue("bMin", newMin); }
  void setMinC(double newMin) { setFormulaParameterMinValue("cMin", newMin); }
  void setMinD(double newMin) { setFormulaParameterMinValue("dMin", newMin); }

  void setMaxA(double newMax) { setFormulaParameterMaxValue("aMax", newMax); }
  void setMaxB(double newMax) { setFormulaParameterMaxValue("bMax", newMax); }
  void setMaxC(double newMax) { setFormulaParameterMaxValue("cMax", newMax); }
  void setMaxD(double newMax) { setFormulaParameterMaxValue("dMax", newMax); }


  //---------------------------------------------------------------------------------------------
  // audio processing:

  virtual void getSampleFrameStereo(double* inOutL, double* inOutR)
  {
    wrappedFuncShaper->getSampleFrameStereo(inOutL, inOutR, inOutL, inOutR);
  }

  virtual void processBlockStereo(float *left, float *right, int numSamples)
  {
    for(int n = 0; n < numSamples; n++)
    {
      double dL = left[n];
      double dR = right[n];
      wrappedFuncShaper->getSampleFrameStereo(&dL, &dR, &dL, &dR);
      left[n] = (float)dL;
      right[n] = (float)dR;
    }
  }

  virtual void processBlock(double **inOutBuffer, int numChannels, int numSamples) override
  {
    for(int n = 0; n < numSamples; n++)
      wrappedFuncShaper->getSampleFrameStereo(
        &inOutBuffer[0][n], &inOutBuffer[1][n],   // inputs
        &inOutBuffer[0][n], &inOutBuffer[1][n]);  // outputs
    // provide a function that we cann call like:
    // wrappedFuncShaper->processBlockStereo(inOutBuffer[0], inOutBuffer[1], numSamples);
  }

  virtual void processStereoFrame(double *left, double *right) override
  {
    wrappedFuncShaper->getSampleFrameStereo(left, right, left, right);
  }


protected:

  void createParameters();


  rosic::FuncShaper *wrappedFuncShaper;
  bool wrappedFuncShaperIsOwned = false;

  juce_UseDebuggingNewOperator;

};

//=================================================================================================

class FuncShaperModuleEditor : public AudioModuleEditor, public RTextEntryFieldObserver, 
  public ParameterObserver //public RSliderListener, 
{

public:

  //---------------------------------------------------------------------------------------------
  // construction/destruction:

  /** Constructor. */
  FuncShaperModuleEditor(CriticalSection *newPlugInLock, FuncShaperAudioModule* newFuncShaperAudioModule);

  /** Destructor. */
  virtual ~FuncShaperModuleEditor();

  //---------------------------------------------------------------------------------------------
  // callbacks:

  virtual void parameterIsGoingToBeDeleted(Parameter* parameterThatWillBeDeleted);
  virtual void parameterChanged(Parameter* parameterThatHasChanged);

  /** Implements the purely virtual method inherited from RTextEntryFieldObserver. */
  virtual void textChanged(RTextEntryField *rTextEntryFieldThatHasChanged);

  /** Overrides paint(). */   
  virtual void paint(Graphics &g);

  /** Overrides resized(). */    
  virtual void resized();

protected:

  /** Overrides the method inherited from RPolyphonicInstrumentEditor. */
  virtual void updateWidgetsAccordingToState();

  /** This is the actual plugin engine which does all the dsp and automation handling. */
  FuncShaperAudioModule *funcShaperAudioModule;

  // some rectangles to subdivide the GUI:
  juce::Rectangle<int> formulaRectangle, inputRectangle, outputRectangle;

  // the widgets:
  RTextField *formulaLabel, *inputLabel, *outputLabel; //, *aLabel, *bLabel, *cLabel, *dLabel;

  RTextEntryField *formulaField, *aMinField, *aMaxField, *bMinField, *bMaxField, 
    *cMinField, *cMaxField, *dMinField, *dMaxField;

  RSlider *aSlider, *bSlider, *cSlider, *dSlider, *driveSlider, *dcSlider, *oversamplingSlider, 
    *dryWetSlider, *outVolumeSlider, *inHighpassSlider, *inLowpassSlider, *outHighpassSlider,
    *outLowpassSlider;
  RButton *preFilterButton, *postFilterButton;

  // the function plot which plots the characteristic line:
  CurveFamilyPlotOld* shaperPlot;
  double *xValues;
  double **yValues;

  juce_UseDebuggingNewOperator;
};



#endif 
