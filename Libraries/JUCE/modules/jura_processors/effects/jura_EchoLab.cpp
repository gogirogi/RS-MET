
// construction/destruction:

EchoLabDelayLineAudioModule::EchoLabDelayLineAudioModule(CriticalSection *newPlugInLock, 
  rosic::EchoLabDelayLine *echoLabDelayLineToWrap) : AudioModule(newPlugInLock)
{
  jassert( echoLabDelayLineToWrap != NULL ); // you must pass a valid rosic-object to the constructor
  wrappedEchoLabDelayLine = echoLabDelayLineToWrap;
  moduleName = juce::String("DelayLine");
  setActiveDirectory(getApplicationDirectory() + juce::String("/Presets/EchoLab/DelayLine") );
  initializeAutomatableParameters();

  inputEqualizerModule = new EqualizerAudioModule(lock, &echoLabDelayLineToWrap->inputEqualizer);
  inputEqualizerModule->setModuleName(juce::String("InputFilter"));
  inputEqualizerModule->setActiveDirectory(getApplicationDirectory() 
    + juce::String("/Presets/EchoLab/InputFilter") );
  addChildAudioModule(inputEqualizerModule);

  feedbackEqualizerModule = new EqualizerAudioModule(lock, &echoLabDelayLineToWrap->feedbackEqualizer);
  feedbackEqualizerModule->setModuleName(juce::String("FeedbackFilter"));
  feedbackEqualizerModule->setActiveDirectory(getApplicationDirectory() 
    + juce::String("/Presets/EchoLab/FeedbackFilter") );
  addChildAudioModule(feedbackEqualizerModule);
}

// setup:

void EchoLabDelayLineAudioModule::parameterChanged(Parameter* parameterThatHasChanged)
{
  if( wrappedEchoLabDelayLine == NULL )
    return;

  double value = parameterThatHasChanged->getValue();
  switch( getIndexOfParameter(parameterThatHasChanged) )
  {
  case 0: wrappedEchoLabDelayLine->setDelayTime(        value         );   break;
  case 1: wrappedEchoLabDelayLine->setGlobalGainFactor( value         );   break;
  case 2: wrappedEchoLabDelayLine->setFeedbackInPercent(value         );   break;
  case 3: wrappedEchoLabDelayLine->setPan(              value         );   break;
  case 4: wrappedEchoLabDelayLine->setPingPongMode(     value >= 0.5  );   break;
  case 5: wrappedEchoLabDelayLine->setMute(             value >= 0.5  );   break;
  } 

  //sendChangeMessage();
  markStateAsDirty();
}

// audio processing:

/*
void EchoLabDelayLineAudioModule::getSampleFrameStereo(double* inOutL, double* inOutR)
{ 
  *inOutL = 0.0;
  *inOutR = 0.0;
  jassertfalse; // this function is not supposed to be used
}

void EchoLabDelayLineAudioModule::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages) 
{ 
  buffer.clear();
  jassertfalse; // this function is not supposed to be used
} 
*/

// others:

void EchoLabDelayLineAudioModule::initializeAutomatableParameters()
{
  std::vector<double> defaultValues;
  AutomatableParameter* p;

  // #00:
  p = new AutomatableParameter(lock, "DelayTime", 0.0001, 4.25, 0.0001, 1.0, Parameter::LINEAR);
  defaultValues.clear();
  defaultValues.push_back(0.125);
  defaultValues.push_back(0.25);
  defaultValues.push_back(0.5);
  defaultValues.push_back(1.0);
  defaultValues.push_back(2.0);
  defaultValues.push_back(4.0);
  p->setDefaultValues(defaultValues);
  addObservedParameter(p);

  addObservedParameter( new AutomatableParameter(lock, "Amplitude",  -1.0,  1.0,  0.001, 0.5, Parameter::LINEAR) );  // #01
  addObservedParameter( new AutomatableParameter(lock, "Feedback",  -99.0, 99.0,  0.1,   0.0, Parameter::LINEAR) );  // #02
  addObservedParameter( new AutomatableParameter(lock, "Pan",        -1.0,  1.0,  0.01,  0.0, Parameter::LINEAR) );  // #03
  addObservedParameter( new AutomatableParameter(lock, "PingPong",    0.0,  1.0,  1.0,   0.0, Parameter::BOOLEAN));  // #04
  addObservedParameter( new AutomatableParameter(lock, "Mute",        0.0,  1.0,  1.0,   0.0, Parameter::BOOLEAN));  // #05

  //addObservedParameter( new AutomatableParameter(plugInLock, "Solo",        0.0,  1.0,  1.0,   0.0, Parameter::BOOLEAN));  // #05
    // nah - "Solo" is not a parameter - it's a GUI feature - when slo is on, alway the currently selected delayline will play solo

  for(int i=0; i < (int) parameters.size(); i++ )
    parameterChanged(parameters[i]);
}

/*
void EchoLabDelayLineAudioModule::setStateFromXml(const XmlElement& xmlState, 
const juce::String& stateName, bool markAsClean)
{
  AudioModule::setStateFromXml(xmlState, stateName, markAsClean);


  int dummy = 0;

}
*/

void EchoLabDelayLineAudioModule::reset()
{
  //wrappedEchoLabDelayLine->reset();  // maybe we should indeed implement this rest-function...
}

//=================================================================================================


EchoLabDelayLineModuleEditor::EchoLabDelayLineModuleEditor(CriticalSection *newPlugInLock, 
  EchoLabDelayLineAudioModule* newEchoLabDelayLineAudioModule) 
  : AudioModuleEditor(newEchoLabDelayLineAudioModule)
{
  setHeadlineStyle(NO_HEADLINE);
  setPresetSectionPosition(INVISIBLE);

  delayLineModuleToEdit = NULL;

  inputEqualizerEditor = new EqualizerModuleEditor(newPlugInLock, NULL);
  inputEqualizerEditor->setLayout(EqualizerModuleEditor::SLIDERS_ABOVE);
  //inputEqualizerEditor->setPresetSectionPosition(AudioModuleEditor::INVISIBLE);
  inputEqualizerEditor->setLinkPosition(AudioModuleEditor::INVISIBLE);
  //inputEqualizerEditor->setHeadlineStyle(SUB_HEADLINE);
  inputEqualizerEditor->setHeadlineText(juce::String("Input Filter"));
  inputEqualizerEditor->setUseShortSliderNames(true);
  inputEqualizerEditor->setUseSmallComboBox(true);
  addChildEditor(inputEqualizerEditor);
  inputEqualizerEditor->setDescriptionField(infoField, true);

  feedbackEqualizerEditor = new EqualizerModuleEditor(newPlugInLock, NULL);
  feedbackEqualizerEditor->setLayout(EqualizerModuleEditor::SLIDERS_ABOVE);
  //feedbackEqualizerEditor->setPresetSectionPosition(AudioModuleEditor::INVISIBLE);
  feedbackEqualizerEditor->setLinkPosition(AudioModuleEditor::INVISIBLE);
  feedbackEqualizerEditor->setUseShortSliderNames(true);
  feedbackEqualizerEditor->setUseSmallComboBox(true);
  feedbackEqualizerEditor->setHeadlineText(juce::String("Feedback Filter"));
  addChildEditor(feedbackEqualizerEditor);
  feedbackEqualizerEditor->setDescriptionField(infoField, true);

  addWidget( timeSlider = new RSlider("TimeSlider") );
  timeSlider->setSliderName(juce::String("Time"));
  timeSlider->setDescription( juce::String("Delaytime for the selected delayline (in seconds or beats)") );
  timeSlider->setRange(0.01, 4.25, 0.01, 0.25);
  timeSlider->setDescriptionField(infoField);
  timeSlider->setStringConversionFunction(secondsToStringWithUnit3);
  //timeSlider->addListener(this);

  addWidget( gainSlider = new RSlider("GainSlider") );
  gainSlider->setSliderName(juce::String("Gain"));
  gainSlider->setDescription( juce::String("Gain for the selected delayline (raw amplitude)") );
  gainSlider->setRange(-1.0, 1.0, 0.01, 0.0);
  gainSlider->setScaling(Parameter::LINEAR_BIPOLAR);
  gainSlider->setDescriptionField(infoField);
  gainSlider->setStringConversionFunction(&valueToString3);
  //gainSlider->addListener(this);

  addWidget( panSlider = new RSlider("PanSlider") );
  panSlider->setSliderName(juce::String("Pan"));
  panSlider->setDescription( juce::String("Panorama position for the selected delayline (-1...+1)") );
  panSlider->setRange(-1.0, 1.0, 0.01, 0.0);
  panSlider->setScaling(Parameter::LINEAR_BIPOLAR);
  panSlider->setDescriptionField(infoField);
  panSlider->setStringConversionFunction(&valueToString2);
  //panSlider->addListener(this);

  addWidget( feedbackSlider = new RSlider("FeedbackSlider") );
  feedbackSlider->setSliderName(juce::String("Feedback"));
  feedbackSlider->setDescription( juce::String("Feedback for the selected delayline in %") );
  feedbackSlider->setRange(-99.0, 99.0, 0.1, 0.0);
  feedbackSlider->setScaling(Parameter::LINEAR_BIPOLAR);
  feedbackSlider->setDescriptionField(infoField);
  feedbackSlider->setStringConversionFunction(&percentToStringWithUnit2);
  feedbackSlider->setLayout(RSlider::NAME_ABOVE);
  //feedbackSlider->addListener(this);

  addWidget( pingPongButton = new RButton(juce::String("Ping Pong")) );
  pingPongButton->setDescription(
    juce::String("Switch delayline into ping-pong mode (alternating pan-positions for successive echos)"));
  pingPongButton->setDescriptionField(infoField);
  pingPongButton->addRButtonListener(this);

  addWidget( muteButton = new RButton(juce::String("Mute")) );
  muteButton->setDescription(
    juce::String("Mute the delayline"));
  muteButton->setDescriptionField(infoField);
  muteButton->addRButtonListener(this);

  addWidget( soloButton = new RButton(juce::String("Solo")) );
  soloButton->setDescription(juce::String("Listen to the delayline in solo-mode (mute all others)"));
  soloButton->setDescriptionField(infoField);
  soloButton->addRButtonListener(this);

  addWidget( flushButton = new RButton(juce::String("Flush")) );
  flushButton->setDescription(juce::String("Flush/clear the content of the delaylines"));
  flushButton->setDescriptionField(infoField); 
  flushButton->setClickingTogglesState(false);
  flushButton->addRButtonListener(this);

  guiLayoutRectangles.add(middleRectangle);

  updateWidgetsAccordingToState();
}

//-------------------------------------------------------------------------------------------------
// setup:

void EchoLabDelayLineModuleEditor::setDelayLineModuleToEdit(
  EchoLabDelayLineAudioModule* newEchoLabDelayLineModuleToEdit)
{
  ScopedLock scopedLock(*lock);

  removeWatchedAudioModule(delayLineModuleToEdit);

  delayLineModuleToEdit = newEchoLabDelayLineModuleToEdit;
  if( delayLineModuleToEdit != NULL )
  {
    addWatchedAudioModule(delayLineModuleToEdit);

    inputEqualizerEditor   ->setEqualizerModuleToEdit(delayLineModuleToEdit->getInputEqualizerModule());
    feedbackEqualizerEditor->setEqualizerModuleToEdit(delayLineModuleToEdit->getFeedbackEqualizerModule());

    timeSlider    ->assignParameter(delayLineModuleToEdit->getParameterByName(juce::String("DelayTime")));
    gainSlider    ->assignParameter(delayLineModuleToEdit->getParameterByName(juce::String("Amplitude")));
    panSlider     ->assignParameter(delayLineModuleToEdit->getParameterByName(juce::String("Pan")));
    feedbackSlider->assignParameter(delayLineModuleToEdit->getParameterByName(juce::String("Feedback")));
    pingPongButton->assignParameter(delayLineModuleToEdit->getParameterByName(juce::String("PingPong")));
    muteButton    ->assignParameter(delayLineModuleToEdit->getParameterByName(juce::String("Mute")));
    soloButton    ->assignParameter(delayLineModuleToEdit->getParameterByName(juce::String("Solo")));

    timeSlider->setSliderName(juce::String("Time"));
    gainSlider->setSliderName(juce::String("Gain"));
  }
  else
  {
    inputEqualizerEditor   ->setEqualizerModuleToEdit(NULL);
    feedbackEqualizerEditor->setEqualizerModuleToEdit(NULL);

    timeSlider    ->assignParameter(NULL);
    gainSlider    ->assignParameter(NULL);
    panSlider     ->assignParameter(NULL);
    feedbackSlider->assignParameter(NULL);
    pingPongButton->assignParameter(NULL);
    muteButton    ->assignParameter(NULL);
    soloButton    ->assignParameter(NULL);
  }

  //updateWidgetVisibility();
  updateWidgetsAccordingToState();
}

void EchoLabDelayLineModuleEditor::setHueOffsetForFilterEditors(float hueOffset)
{
  ScopedLock scopedLock(*lock);
  inputEqualizerEditor->setCentralHue(editorColourScheme.getCentralHue() 
    + editorColourScheme.getHueOffset(0));
  feedbackEqualizerEditor->setCentralHue(editorColourScheme.getCentralHue() 
    + editorColourScheme.getHueOffset(0));
}

//-------------------------------------------------------------------------------------------------
// callbacks:

/*
void EchoLabDelayLineModuleEditor::rSliderValueChanged(RSlider *rSliderThatHasChanged)
{
sendChangeMessage(); // triggers update of the plot
}
*/

void EchoLabDelayLineModuleEditor::updateWidgetsAccordingToState()
{
  ScopedLock scopedLock(*lock);

  AudioModuleEditor::updateWidgetsAccordingToState();
  updateWidgetVisibility();

  if( delayLineModuleToEdit == NULL )
    return;

  if( delayLineModuleToEdit->wrappedEchoLabDelayLine->isInSyncMode() )
    timeSlider->setStringConversionFunction(&beatsToStringWithUnit4);
  else
    timeSlider->setStringConversionFunction(&secondsToStringWithUnitTotal4);
}

void EchoLabDelayLineModuleEditor::audioModuleWillBeDeleted(AudioModule *moduleToBeDeleted)
{
  ScopedLock scopedLock(*lock);
  if( moduleToBeDeleted == delayLineModuleToEdit && delayLineModuleToEdit != NULL )
    setDelayLineModuleToEdit(NULL);
}

void EchoLabDelayLineModuleEditor::paint(Graphics &g)
{
  //g.fillAll(Colours::lavender);

  AudioModuleEditor::paint(g);


  int x = middleRectangle.getX(); // + middleRectangle.getWidth()/2;
  int y = middleRectangle.getY();
  //drawBitmapFontText(g, x+4, y+4, juce::String("Delay-Setup"), &boldFont16px, editorColourScheme.headline);
  drawBitmapFontText(g, x+4, y+4, juce::String("Delay-Setup"), 
    &BitmapFontRoundedBoldA10D0::instance, editorColourScheme.headline);
}


void EchoLabDelayLineModuleEditor::resized()
{
  AudioModuleEditor::resized();

  int middleWidth = 112;
  int x = 0;
  int y = 0;
  int w = (getWidth() - middleWidth) / 2;
  int h = getHeight();

  inputEqualizerEditor   ->setBounds(x,               y, w, h);
  feedbackEqualizerEditor->setBounds(x+w+middleWidth, y, w, h);

  guiLayoutRectangles.clear();
  middleRectangle.setBounds(getWidth()/2-middleWidth/2, y, middleWidth, h);
  guiLayoutRectangles.add(middleRectangle);

  x = middleRectangle.getX();
  y = middleRectangle.getY()+32;
  w = middleRectangle.getWidth();

  timeSlider->setBounds(x+4, y, w-8, 16);
  y += 20;
  gainSlider->setBounds(x+4, y, w-8, 16);
  y += 20;
  panSlider->setBounds(x+4, y, w-8, 16);
  y += 24;
  feedbackSlider->setBounds(x+4, y, w-8, 32);
  y += 48;

  pingPongButton->setBounds(x+16, y, w-32, 16);
  y += 32;
  muteButton->setBounds(x+4, y, w/2-8, 16);
  soloButton->setBounds(x+w/2+4, y, w/2-8, 16);
  y += 20;
  x = muteButton->getX()+muteButton->getWidth()/2;
  w = soloButton->getX()+soloButton->getWidth()/2 - x;
  flushButton->setBounds(x+4, y, w-8, 16);
}

void EchoLabDelayLineModuleEditor::updateWidgetVisibility()
{
  ScopedLock scopedLock(*lock);

  bool visible = true;
  if( delayLineModuleToEdit == NULL )
    visible = false;

  timeSlider    ->setVisible(visible);
  gainSlider    ->setVisible(visible);
  panSlider     ->setVisible(visible);
  feedbackSlider->setVisible(visible);
  pingPongButton->setVisible(visible);
  muteButton    ->setVisible(visible);
  soloButton    ->setVisible(visible);
  flushButton   ->setVisible(visible);
}

//=================================================================================================

// construction/destruction:

EchoLabAudioModule::EchoLabAudioModule(CriticalSection *newPlugInLock, 
  rosic::EchoLab *echoLabToWrap) : AudioModule(newPlugInLock)
{
  //jassert(echoLabToWrap != NULL); // you must pass a valid rosic-object to the constructor

  if(echoLabToWrap != nullptr)
    wrappedEchoLab = echoLabToWrap;
  else
  {
    wrappedEchoLab = new rosic::EchoLab;
    wrappedEchoLabIsOwned = true;
  }

  moduleName = juce::String("EchoLab");
  setActiveDirectory(getApplicationDirectory() + juce::String("/Presets/EchoLab") );

  //inputFilterModule = new EqualizerAudioModule(NULL);
  //inputFilterModule->setModuleName(juce::String(T("InputFilter")));

  //feedbackFilterModule = new EqualizerAudioModule(NULL);
  //feedbackFilterModule->setModuleName(juce::String(T("FeedbackFilter")));

  initializeAutomatableParameters();
}

EchoLabAudioModule::~EchoLabAudioModule()
{
  if(wrappedEchoLabIsOwned)
    delete wrappedEchoLab;
}

AudioModuleEditor* EchoLabAudioModule::createEditor()
{
  return new jura::EchoLabModuleEditor(lock, this); // get rid of passing the lock
}

// setup:

void EchoLabAudioModule::parameterChanged(Parameter* parameterThatHasChanged)
{
  ScopedLock scopedLock(*lock);
  //if( wrappedEchoLab == NULL )
  //  return;

  double value = parameterThatHasChanged->getValue();
  switch( getIndexOfParameter(parameterThatHasChanged) )
  {
  case   0: wrappedEchoLab->setDryWet(  value); break;
  case   1: wrappedEchoLab->setWetLevel(value); break;
  } // end of switch( parameterIndex )
}

XmlElement* equalizerStateToXml(Equalizer* equalizer, XmlElement* xmlElementToStartFrom)
{
  // the XmlElement which stores all the releveant state-information:
  XmlElement* xmlState;
  if( xmlElementToStartFrom == NULL )
    xmlState = new XmlElement(juce::String("Equalizer")); 
  else
    xmlState = xmlElementToStartFrom;

  xmlState->setAttribute("GlobalGain", equalizer->getGlobalGain());

  // create an XmlElement for each band and add it as child-XmlElement:
  for(int i=0; i<equalizer->getNumBands(); i++)
  {
    XmlElement* bandState = new XmlElement(juce::String("Band"));

    bandState->setAttribute(juce::String("Frequency"), equalizer->getBandFrequency(i));
    bandState->setAttribute(juce::String("Gain"),      equalizer->getBandGain(i));
    bandState->setAttribute(juce::String("Bandwidth"), equalizer->getBandBandwidth(i));

    juce::String modeString;
    int mode = equalizer->getBandMode(i);
    switch( mode )
    {
    case TwoPoleFilter::PEAK:       modeString = juce::String("Peak/Dip");           break;
    case TwoPoleFilter::LOW_SHELF:  modeString = juce::String("Low Shelving");       break;
    case TwoPoleFilter::HIGH_SHELF: modeString = juce::String("High Shelving");      break;
    case TwoPoleFilter::LOWPASS6:   modeString = juce::String("Lowpass 6 dB/oct");   break;
    case TwoPoleFilter::LOWPASS12:  modeString = juce::String("Lowpass 12 dB/oct");  break;
    case TwoPoleFilter::HIGHPASS6:  modeString = juce::String("Highpass 6 dB/oct");  break;
    case TwoPoleFilter::HIGHPASS12: modeString = juce::String("Highpass 12 dB/oct"); break;
    case TwoPoleFilter::BANDREJECT: modeString = juce::String("Notch 2*6 dB/oct");   break;
    }
    bandState->setAttribute("Mode", modeString);

    xmlState->addChildElement(bandState);
  } 

  return xmlState;
}

XmlElement* echoLabDelayLineStateToXml(EchoLabDelayLine* delayLine, XmlElement* xmlElementToStartFrom)
{
  XmlElement* xmlState;
  if( xmlElementToStartFrom == NULL )
    xmlState = new XmlElement(juce::String("DelayLine")); 
  else
    xmlState = xmlElementToStartFrom;

  // todo: store value only if different from default values (save space)
  xmlState->setAttribute("DelayTime", delayLine->getDelayTime()              );
  xmlState->setAttribute("Amplitude", delayLine->getGlobalGainFactor()       );
  xmlState->setAttribute("Feedback",  delayLine->getFeedbackInPercent()      );
  xmlState->setAttribute("Pan",       delayLine->getPan()                    );
  xmlState->setAttribute("PingPong",  delayLine->isInPingPongMode()          );
  xmlState->setAttribute("Mute",      delayLine->isMuted()                   );

  // store the embedded equalizer's state as child element (unless it's neutral):
  if( delayLine->feedbackEqualizer.getNumBands(0) > 0 
    || delayLine->feedbackEqualizer.getGlobalGain() != 0.0 )
  {
    XmlElement* feedbackEqState = new XmlElement(juce::String("FeedbackFilter"));
    feedbackEqState 
      = equalizerStateToXml(&(delayLine->feedbackEqualizer.equalizers[0]), feedbackEqState);
    xmlState->addChildElement(feedbackEqState);
  }
  if( delayLine->inputEqualizer.getNumBands(0) > 0 
    || delayLine->inputEqualizer.getGlobalGain() != 0.0 )
  {
    XmlElement* inputEqState = new XmlElement(juce::String("InputFilter"));
    inputEqState = equalizerStateToXml(&(delayLine->inputEqualizer.equalizers[0]), inputEqState);
    xmlState->addChildElement(inputEqState);
  }

  return xmlState;
}

XmlElement* echoLabStateToXml(EchoLab *echoLab, XmlElement* xmlElementToStartFrom)
{
  XmlElement* xmlState;
  if( xmlElementToStartFrom == NULL )
    xmlState = new XmlElement(juce::String("EchoLabState")); 
  else
    xmlState = xmlElementToStartFrom;

  // create an XmlElement for each delayline and add it as child-XmlElement:
  echoLab->acquireLock();
  xmlState->setAttribute("SyncDelayTimes", echoLab->isDelayTimeSynced());
  xmlState->setAttribute("DryWet",         echoLab->getDryWet() );
  xmlState->setAttribute("WetLevel",       echoLab->getWetLevel() );
  xmlState->setAttribute("Solo",           echoLab->getSoloedDelayLineIndex() );
  for(int i=0; i<echoLab->getNumDelayLines(); i++)
  {
    rosic::EchoLabDelayLine* delayLine = echoLab->getDelayLine(i);
    XmlElement* delayLineState = echoLabDelayLineStateToXml(delayLine, NULL);
    xmlState->addChildElement(delayLineState);
  } 
  echoLab->releaseLock();

  return xmlState;
}

XmlElement* EchoLabAudioModule::getStateAsXml(const juce::String& stateName, bool markAsClean)
{
  ScopedLock scopedLock(*lock);
  XmlElement *xmlState = AudioModule::getStateAsXml(stateName, markAsClean);
  if( wrappedEchoLab != NULL )
  {
    wrappedEchoLab->acquireLock();
    xmlState = echoLabStateToXml(wrappedEchoLab, xmlState);
    wrappedEchoLab->releaseLock();
  }

  // mmm...actually, we may get away with the baseclass implementation, i think

  return xmlState;
}

void EchoLabAudioModule::setStateFromXml(const XmlElement& xmlState, 
  const juce::String& stateName, bool markAsClean)
{
  jassertfalse;

  // this function must be re-implemented
  // we have to do more here - we need to create and set up all the required child-modules
  // ahh - and quite possibly we need legacy-preset conversion (but maybe not)

  // perhaps we just need to look at echoLabStateFromXml, define the right static parameters 
  // inside EchoLsbDelayLineAudioModule, create the child-modules here and then rely on the 
  // recursion to recall all the parameters for the individual delaylines

  // we perhaps need to drag over the EchoLabStateFromXml function from the old codebase 
  // first

  ScopedLock scopedLock(*lock);

  removeAllDelayLines();

  // create the audiomodules for the delaylines:
  for(int i = 0; i < xmlState.getNumChildElements(); i++)
  {
    if( xmlState.getChildElement(i)->hasTagName(juce::String("DelayLine")) )
    {
      juce::XmlElement *delayLineState = xmlState.getChildElement(i);
      double delayTime = delayLineState->getDoubleAttribute(juce::String("DelayTime"), 1.0);
      double amplitude = delayLineState->getDoubleAttribute(juce::String("Amplitude"), 0.5);
      addDelayLine(delayTime, amplitude);
    }
  }

  // at this point, the delaytimes are still correct

  AudioModule::setStateFromXml(xmlState, stateName, markAsClean);
  // should set up the internal states of the individual delaylines (which are now child-modules)
  // -> we must perhaps override setStateFromXml in EchoLabDelayLineAudioModule
  // ...this seems not yet to work....


  /*
  // this is the old implementation:
  ScopedLock scopedLock(*plugInLock);
  AudioModule::setStateFromXml(xmlState, stateName, markAsClean);
  if( wrappedEchoLab != NULL )
  {
  wrappedEchoLab->acquireLock();
  echoLabStateFromXml(wrappedEchoLab, xmlState);
  wrappedEchoLab->releaseLock();
  }
  */
}

/*
XmlElement EchoLabAudioModule::convertXmlStateIfNecessary(const XmlElement& xmlState)
{
ScopedLock scopedLock(*plugInLock);

DEBUG_BREAK; 
// \todo: implement this function - should take care of updating the sub-states for the two filters (the new EqualizerAudioModule
// admits 2 channels
// hmmmm....may it's better to just define a mono version and stereo version of EqualizerAudioModule
// ...or maybe we can adapt the state save/recall in the EqualizerAudioModule so as to admit for both versions
// one with and one without the "Channel" child element

int xmlPatchFormatIndex = xmlState.getIntAttribute(T("PatchFormat"), patchFormatIndex);

return AudioModule::convertXmlStateIfNecessary(xmlState);
}
*/

void EchoLabAudioModule::setSampleRate(double newSampleRate)   
{ 
  ScopedLock scopedLock(*lock);
  wrappedEchoLab->setSampleRate(newSampleRate); 
}

void EchoLabAudioModule::setBeatsPerMinute(double newBpm)   
{ 
  ScopedLock scopedLock(*lock);
  wrappedEchoLab->setTempoInBPM(newBpm); 
}

int EchoLabAudioModule::addDelayLine(double newDelayTime, double newGainFactor)
{
  ScopedLock scopedLock(*lock);

  int index = wrappedEchoLab->addDelayLine(newDelayTime, newGainFactor);

  if( index != -1 )
  {
    EchoLabDelayLineAudioModule *newDelayLineModule 
      = new EchoLabDelayLineAudioModule(lock, wrappedEchoLab->getDelayLine(index));
    newDelayLineModule->getParameterByName(
      juce::String("DelayTime"))->setValue(newDelayTime,  true, true);
    newDelayLineModule->getParameterByName(
      juce::String("Amplitude"))->setValue(newGainFactor, true, true);
    delayLineModules.add(newDelayLineModule);
    addChildAudioModule(newDelayLineModule);
  }

  return index;
}

bool EchoLabAudioModule::removeDelayLine(int index)
{
  ScopedLock scopedLock(*lock);

  bool success = false;

  //if( index < 0 || index >= delayLineModules.size() )  // huh? shouldn't it be index >= 0 && index < size?
  if( index >= 0 && index < delayLineModules.size() )
  {
    EchoLabDelayLineAudioModule *delayLineModule = delayLineModules[index];
    delayLineModules.remove(index);
    removeChildAudioModule(delayLineModule, true);

    success = wrappedEchoLab->removeDelayLine(index);
    jassert( success  == true   ); 
    // we successfully removed the EqualizerAudioModules but not the underlying rosic-delayline object - something must have gone wrong
  }
  else
    jassertfalse; // invalid index

  return success;
}

void EchoLabAudioModule::removeAllDelayLines()
{
  ScopedLock scopedLock(*lock);
  while( delayLineModules.size() > 0 )
    removeDelayLine(delayLineModules.size()-1);
}

// inquiry:

EchoLabDelayLineAudioModule* EchoLabAudioModule::getDelayLineModule(int index) const
{
  jassert( index >= 0  && index < delayLineModules.size() );
  return delayLineModules[index];
}

// audio processing:
/*
void EchoLabAudioModule::getSampleFrameStereo(double* inOutL, double* inOutR)
{ 
  jassertfalse; // nor good idea to use this function - performance hog
  ScopedLock scopedLock(*lock);
  wrappedEchoLab->getSampleFrameStereo(inOutL, inOutR); 
}

void EchoLabAudioModule::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages) 
{ 
  ScopedLock scopedLock(*lock);

  if( wrappedEchoLab == NULL || buffer.getNumChannels() < 1 )
  {
    jassertfalse;
    return;
  }

  float *left, *right;  
  left  = buffer.getWritePointer(0, 0);
  if( buffer.getNumChannels() < 2 )
    right = buffer.getWritePointer(0, 0);
  else
    right = buffer.getWritePointer(1, 0);
  wrappedEchoLab->processBlock(left, right, buffer.getNumSamples());
} 
*/
void EchoLabAudioModule::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
  // something to do...
}

// others:

void EchoLabAudioModule::reset() 
{ 
  ScopedLock scopedLock(*lock);
  wrappedEchoLab->resetDelayLines(); 
}

void EchoLabAudioModule::initializeAutomatableParameters()
{
  ScopedLock scopedLock(*lock);

  // create the automatable parameters and add them to the list - note that the order of the adds
  // is important because in parameterChanged(), the index (position in the array) will be used to
  // identify which particlua parameter has changed.

  juce::Array<double> defaultValues;

  // this pointer will be used to temporarily store the addresses of the created Parameter-objects:
  AutomatableParameter* p;

  // #00:
  p = new AutomatableParameter(lock, "DryWet", 0.0, 1.0, 0.01, 0.5, Parameter::LINEAR); 
  addObservedParameter(p);

  // #01:
  p = new AutomatableParameter(lock, "WetLevel", -36.0, 6.0, 0.01, 0.0, Parameter::LINEAR); 
  addObservedParameter(p);

  // make a call to setValue for each parameter in order to set up all the slave voices:
  for(int i=0; i < (int) parameters.size(); i++ )
    parameterChanged(parameters[i]);
}

//=================================================================================================


EchoLabPlotEditor::EchoLabPlotEditor(CriticalSection *newPlugInLock, 
  EchoLabAudioModule* newEchoLabModuleToEdit) 
  : CurveFamilyPlotOld(juce::String("EchoLabPlot"))
  , InteractiveCoordinateSystemOld(juce::String("EchoLabPlot"))
{
  setDescription("Left: insert or grab band-handle, right: remove band");

  //ParameterObserver::isGuiElement = true;

  jassert( newEchoLabModuleToEdit != NULL ); // you must pass a valid pointer here

  plugInLock          = newPlugInLock;
  echoLabModuleToEdit = newEchoLabModuleToEdit;

  // set up the plot range:
  setAutoReRendering(false);
  setMaximumRange(-0.125, 4.25, -1.5, 1.5);
  setCurrentRange(-0.125, 4.25, -1.5, 1.5);
  setHorizontalFineGrid(1.0,   true);
  setVerticalFineGrid(  0.125, false);
  setAxisLabelX(juce::String("t"));
  setAxisLabelY(juce::String(""));
  //setHorizontalFineGrid(  1.0, false);
  //setVerticalCoarseGridVisible( true);
  //setVerticalFineGridVisible(   false);
  //CoordinateSystem::setAxisValuesPositionX(CoordinateSystem::ABOVE_AXIS);
  //CoordinateSystem::setAxisValuesPositionY(CoordinateSystem::RIGHT_TO_AXIS);
  //setSnapToFineGridX(true);
  //setSnapToFineGridY(true);
  setAutoReRendering(true);


  currentMouseCursor = MouseCursor(MouseCursor::NormalCursor);
  setMouseCursor(currentMouseCursor);

  // this stuff will be (re-) assigned in resized():
  numSamplesInPlot = 0;
  timeAxis         = NULL;
  impulseResponse  = NULL;

  selectedIndex          = -1;
  selectedDelayLine      = NULL;
  currentlyDraggedHandle = NONE;
  delayLineEditor        = NULL;

  ParameterObserver::setLocalAutomationSwitch(true);
}

EchoLabPlotEditor::~EchoLabPlotEditor(void)
{
  deRegisterFromObservedParameters();
  deleteAndZero(timeAxis);
  deleteAndZero(impulseResponse);
}

//-------------------------------------------------------------------------------------------------
// parameter-settings:

void EchoLabPlotEditor::setDelayLineModuleEditor(
  EchoLabDelayLineModuleEditor *delayLineEditorToUse)
{
  deSelectDelayLine();
  delayLineEditor = delayLineEditorToUse;
  //selectDelayLine(selectedIndex);
}

bool EchoLabPlotEditor::selectDelayLine(int indexToSelect)
{
  ScopedLock scopedLock(*plugInLock);

  removeWatchedAudioModule(selectedDelayLine);
  deRegisterFromObservedParameters();

  if( indexToSelect < 0 
    || indexToSelect >= echoLabModuleToEdit->wrappedEchoLab->getNumDelayLines() )
  {
    selectedIndex = -1;
    selectedDelayLine = NULL;
    if( delayLineEditor != NULL )
      delayLineEditor->setDelayLineModuleToEdit(NULL);
    return false;
  }

  selectedIndex     = indexToSelect;
  selectedDelayLine = echoLabModuleToEdit->getDelayLineModule(indexToSelect);
  if( selectedDelayLine != NULL )
  {
    addWatchedAudioModule(selectedDelayLine);
    selectedDelayLine->getParameterByName("DelayTime")->registerParameterObserver(this);
    selectedDelayLine->getParameterByName("Amplitude")->registerParameterObserver(this);
    selectedDelayLine->getParameterByName("Feedback") ->registerParameterObserver(this);
    selectedDelayLine->getParameterByName("Pan")      ->registerParameterObserver(this);
    selectedDelayLine->getParameterByName("PingPong") ->registerParameterObserver(this);
    selectedDelayLine->getParameterByName("Mute")     ->registerParameterObserver(this);
  }

  // switch always the currently selected delayline to solo, if solo is desired:
  if( echoLabModuleToEdit->wrappedEchoLab->getSoloedDelayLineIndex() != -1 )
    echoLabModuleToEdit->wrappedEchoLab->setDelayLineSolo(selectedIndex);  

  updatePlot();

  if( delayLineEditor != NULL )
    delayLineEditor->setDelayLineModuleToEdit(selectedDelayLine);

  sendChangeMessage();
  return true;
}

void EchoLabPlotEditor::deSelectDelayLine()
{
  selectDelayLine(-1);
}

bool EchoLabPlotEditor::removeDelayLine(int indexToRemove)
{
  ScopedLock scopedLock(*plugInLock);

  if( indexToRemove < 0 
    || indexToRemove >= echoLabModuleToEdit->wrappedEchoLab->getNumDelayLines() )
    return false;

  // new:
  deSelectDelayLine();
  echoLabModuleToEdit->removeDelayLine(indexToRemove);
  updatePlot();  // perhaps superfluous - check that
  return true;


  /*
  // old:
  if( delayLineEditor != NULL )
  delayLineEditor->setDelayLineModuleToEdit(NULL);

  echoLabModuleToEdit->removeDelayLine(indexToRemove);
  selectedIndex = -1;
  updatePlot();

  return true;
  */
}

//-------------------------------------------------------------------------------------------------
// inquiry:

int EchoLabPlotEditor::getIndexAtPixelPosition(int x, int y)
{
  ScopedLock scopedLock(*plugInLock);

  double dotRadius = 5.0;
  double xd = (double) x;
  double yd = (double) y;
  for(int i=0; i<echoLabModuleToEdit->wrappedEchoLab->getNumDelayLines(); i++)
  {
    double xi = echoLabModuleToEdit->wrappedEchoLab->getDelayTime(i);
    double yi = echoLabModuleToEdit->wrappedEchoLab->getGainFactor(i);
    transformToComponentsCoordinates(xi, yi);
    double d = sqrt( (xi-xd)*(xi-xd) + (yi-yd)*(yi-yd) );
    if( d <= dotRadius )
      return i;
  }

  return -1;
}

//-------------------------------------------------------------------------------------------------
// callbacks:

void EchoLabPlotEditor::audioModuleWillBeDeleted(AudioModule *moduleToBeDeleted)
{  
  ScopedLock scopedLock(*plugInLock);
  if( moduleToBeDeleted == selectedDelayLine && selectedDelayLine != NULL )
    selectDelayLine(-1);
}

void EchoLabPlotEditor::parameterChanged(Parameter* parameterThatHasChanged)
{

  ScopedLock scopedLock(*plugInLock);
  updatePlot();
}

void EchoLabPlotEditor::mouseMove(const MouseEvent &e)
{
  ScopedLock scopedLock(*plugInLock);

  int index = getIndexAtPixelPosition(e.x, e.y);
  if( index != -1 )
  {
    double t    = echoLabModuleToEdit->wrappedEchoLab->getDelayTime(index);
    double g    = echoLabModuleToEdit->wrappedEchoLab->getGainFactor(index);
    juce::String tStr = juce::String("Time: ");
    if( !echoLabModuleToEdit->wrappedEchoLab->isDelayTimeSynced() )
      tStr += secondsToStringWithUnitTotal4(t) + juce::String(", ");
    else
      tStr += beatsToStringWithUnit4(t) + juce::String(", ");

    juce::String gStr = juce::String("Gain: ") + valueToString3(g);
    // maybe display some more parameters .....
    setDescription(tStr + gStr);
  }
  else
    setDescription(juce::String(
      "Left: insert new delayline or grab handle, right: remove delayline"));

  int dragHandle = getDragHandleAt(e.x, e.y);
  if( dragHandle == NONE )
  {
    CoordinateSystemOld::currentMouseCursor = MouseCursor::NormalCursor; // in case there is a zoomer
    setMouseCursor(MouseCursor::NormalCursor);
  }
  else if( dragHandle == TIME_AND_GAIN )
  {
    CoordinateSystemOld::currentMouseCursor = MouseCursor::PointingHandCursor; 
    setMouseCursor(MouseCursor::PointingHandCursor);
  }
  //else if( dragHandle == BANDWIDTH_AND_GAIN_LEFT || dragHandle == BANDWIDTH_AND_GAIN_RIGHT )
  //  setMouseCursor(MouseCursor::LeftRightResizeCursor);
}

void EchoLabPlotEditor::mouseDown(const MouseEvent &e)
{
  ScopedLock scopedLock(*plugInLock);

  int tmpIndex = getIndexAtPixelPosition(e.x, e.y);
  if( tmpIndex == -1 )
  {
    if( e.mods.isLeftButtonDown() )
    {
      // create a new delayline and mark it selected:
      double t = e.x;
      double g = e.y;
      transformFromComponentsCoordinates(t, g);
      echoLabModuleToEdit->addDelayLine(t, g);
      selectDelayLine(echoLabModuleToEdit->wrappedEchoLab->getNumDelayLines()-1);
      currentlyDraggedHandle = TIME_AND_GAIN;
    }
  }
  else
  {
    if( e.mods.isRightButtonDown() )
    {
      removeDelayLine(tmpIndex);
      currentlyDraggedHandle = NONE;
    }
    else
    {
      selectDelayLine(tmpIndex);
      currentlyDraggedHandle = TIME_AND_GAIN;
    }
  }

  sendChangeMessage();
}

void EchoLabPlotEditor::mouseDrag(const juce::MouseEvent &e)
{
  if( e.mods.isRightButtonDown() || e.mouseWasClicked() )
    return;   // ignore right-drags because the band was just removed

  ScopedLock scopedLock(*plugInLock);

  // get the position of the event in components coordinates:
  double x = e.getMouseDownX() + e.getDistanceFromDragStartX();
  double y = e.getMouseDownY() + e.getDistanceFromDragStartY();

  x = rosic::clip(x, 0.0, (double) getWidth());
  y = rosic::clip(y, 0.0, (double) getHeight());      
  transformFromComponentsCoordinates(x, y);

  snapToGrid(x,y); // will snap only when activated

  if( currentlyDraggedHandle == TIME_AND_GAIN )
  {
    if( !e.mods.isCtrlDown() )
    {
      y = clip(y, -1.0, 1.0);
      selectedDelayLine->getParameterByName(juce::String("DelayTime"))->setValue(x, true, true);
      selectedDelayLine->getParameterByName(juce::String("Amplitude"))->setValue(y, true, true);
    }
    else
    {
      x = clip( 0.02*e.getDistanceFromDragStartX(),  -1.0,  1.0);
      y = clip(-2.0 *e.getDistanceFromDragStartY(), -99.0, 99.0);
      selectedDelayLine->getParameterByName(juce::String("Pan"))     ->setValue(x, true, true);
      selectedDelayLine->getParameterByName(juce::String("Feedback"))->setValue(y, true, true);
    }
  }

  mouseMove(e);
  updatePlot();

  sendChangeMessage();
}

void EchoLabPlotEditor::mouseUp(const juce::MouseEvent &e)
{
  currentlyDraggedHandle = NONE;
}

void EchoLabPlotEditor::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel)
{
  ScopedLock scopedLock(*plugInLock);

  int index = getIndexAtPixelPosition(e.x, e.y);
  if( index != -1 )
  {
    double g = echoLabModuleToEdit->wrappedEchoLab->getGainFactor(index);
    //g  = linToLin(g, -24.0, 0.0, 0.0, 1.0);
    g += 0.0625*wheel.deltaY;
    //g  = linToLin(g, 0.0, 1.0, -24.0, 0.0);
    echoLabModuleToEdit->wrappedEchoLab->setGainFactor(index, g);
    updatePlot();
    sendChangeMessage();
  }

}

int EchoLabPlotEditor::getDragHandleAt(int x, int y)
{
  ScopedLock scopedLock(*plugInLock);

  double xd = (double) x;
  double yd = (double) y;
  double xt, yt;             // target coordinates for matching

                             // check if x,y is over the time/gain handle of some delayline:
  for(int i=0; i<echoLabModuleToEdit->wrappedEchoLab->getNumDelayLines(); i++)
  {
    xt = echoLabModuleToEdit->wrappedEchoLab->getDelayTime(i);
    yt = echoLabModuleToEdit->wrappedEchoLab->getGainFactor(i);
    transformToComponentsCoordinates(xt, yt);
    if( euclideanDistance(xt, yt, xd, yd) < 4.0 )
      return TIME_AND_GAIN;
  }

  return NONE;
}

//-------------------------------------------------------------------------------------------------
// drawing:

void EchoLabPlotEditor::resized()
{
  CurveFamilyPlotOld::resized();

  // (re) allocate and fill the arrays for the impulse-response plot
  numSamplesInPlot = getWidth();
  if( timeAxis == NULL )
    delete[] timeAxis;
  if( impulseResponse == NULL )
    delete[] impulseResponse;
  timeAxis        = new double[numSamplesInPlot];
  impulseResponse = new double[numSamplesInPlot];

  int k;
  for(k=0; k<numSamplesInPlot; k++)
  {
    timeAxis[k]        = 0.01 * k;
    impulseResponse[k] = 0.0;
  }

  // preliminary - actually the time-axis does not need to be passed - this can be handled
  // by the default x-axis drawing of CoordinateSystem

  updatePlot();


  /*
  if( echoLabModuleToEdit != NULL )
  { 
  delayDesignerForPlot.acquireLock();
  //delayDesignerForPlot.getImpulseResponse(timeAxis, impulseResponse, numSamplesInPlot);  
  delayDesignerForPlot.releaseLock();
  // \todo: setup sample-rate and stuff ....see BreakpointModulatorEditor....
  }
  */
  //setCurveValues(numSamplesInPlot, timeAxis, impulseResponse); 
}

void EchoLabPlotEditor::updatePlot()
{
  ScopedLock scopedLock(*plugInLock);

  // \todo: setup sample-rate and stuff ....see BreakpointModulatorEditor....
  /*
  double plotRange      = getCurrentRangeMaxX(); - getCurrentRangeMinX();
  double plotSampleRate = (double) numSamplesInPlot / plotRange;
  delayDesingerForPlot.setSampleRate(plotSampleRate);
  delayDesignerForPlot.getImpulseResponseLeft(impulseResponse, numSamplesInPlot);  
  */

  // we probably need a switch here whether to retrieve the left or right channel impulse
  // response

  setCurveValues(numSamplesInPlot, timeAxis, impulseResponse); 
}

void EchoLabPlotEditor::plotCurveFamily(Graphics &g, juce::Image* targetImage, XmlElement *targetSVG)
{
  /*
  Colour defaultColour  = Colours::black;
  //Colour selectedColour = CurveFamilyPlotOld::colourScheme.plotColours[0];
  //Colour selectedColour = CurveFamilyPlotOld::colourScheme.curves; // preliminary
  Colour selectedColour = Colours::red;
  //Colour graphColour = colourScheme.getCurveColour(index);  
  //g.setColour(graphColour); 
  */

  ScopedLock scopedLock(*plugInLock);

  Colour defaultColour  = plotColourScheme.getCurveColour(0);
  Colour selectedColour = plotColourScheme.getCurveColour(1);

  float  dotRadius = 4.f;
  for(int i=0; i<echoLabModuleToEdit->wrappedEchoLab->getNumDelayLines(); i++)
  {
    Colour currentColour;
    if( i == selectedIndex )
      currentColour = selectedColour;
    else
      currentColour = defaultColour;
    g.setColour(currentColour);

    double fb = echoLabModuleToEdit->wrappedEchoLab->getDelayLineParameterThreadSafe(rosic::EchoLab::FEEDBACK_FACTOR, i);
    double gn = echoLabModuleToEdit->wrappedEchoLab->getDelayLineParameterThreadSafe(rosic::EchoLab::GAIN_FACTOR, i);
    double dt = echoLabModuleToEdit->wrappedEchoLab->getDelayTime(i);
    double pn = echoLabModuleToEdit->wrappedEchoLab->getDelayLineParameterThreadSafe(rosic::EchoLab::PAN, i);
    bool   pp = echoLabModuleToEdit->wrappedEchoLab->getDelayLineParameterThreadSafe(rosic::EchoLab::PING_PONG, i) != 0.0;
    double signChanger = 1.0;
    if( pp == true )
      signChanger = -1.0;

    double x = echoLabModuleToEdit->wrappedEchoLab->getDelayTime(i);
    double y = echoLabModuleToEdit->wrappedEchoLab->getGainFactor(i);
    transformToImageCoordinates(x, y, targetImage);

    if( !echoLabModuleToEdit->wrappedEchoLab->isDelayLineActive(i) )
    {
      // draw cross:
      g.drawLine((float)x-4.f, (float)y-4.f, (float)x+4.f, (float)y+4.f, 2.f);
      g.drawLine((float)x-4.f, (float)y+4.f, (float)x+4.f, (float)y-4.f, 2.f);

      x  = echoLabModuleToEdit->wrappedEchoLab->getDelayTime(i);
      double y0 = 0.0;
      transformToImageCoordinates(x, y0, targetImage);
      g.drawLine((float) x, (float) y, (float) x, (float) y0, 2.f);
    }
    else
    {
      g.setColour(currentColour.withMultipliedAlpha(0.5f));

      double panIndicatorSize = 32.0;
      double gL, gR;
      rosic::equalPowerGainFactors(pn, &gL, &gR, -1.0, 1.0);
      float  topLength        = (float) (fabs(gn)*gL*panIndicatorSize);
      float  bottomLength     = (float) (fabs(gn)*gR*panIndicatorSize);
      g.drawLine((float) x, 0.f,                (float) x, topLength,                         2.f);
      g.drawLine((float) x, (float)getHeight(), (float) x, (float)(getHeight()-bottomLength), 2.f);

      g.setColour(currentColour);
      x  = echoLabModuleToEdit->wrappedEchoLab->getDelayTime(i);
      double y0 = 0.0;
      transformToImageCoordinates(x, y0, targetImage);
      g.drawLine((float) x, (float) y, (float) x, (float) y0, 2.f);

      //g.setColour( curveColour );
      g.fillEllipse((float) (x-dotRadius), (float) (y-dotRadius), 
        (float) (2*dotRadius), (float) (2*dotRadius) );
      //g.drawLine((float)x, (float)y, (float)(x+pn*20.f), (float)y, 2.f);

      // draw the feedback delays:
      double x2, y2, x0;
      double alternator = signChanger;
      double xAccu = 2*dt;
      double yAccu = fb*gn;
      x2           = xAccu;  
      y2           = yAccu;
      while( xAccu <= getCurrentRangeMaxX() && fabs(yAccu) > 0.000001 )
      {
        x2 = xAccu;
        y2 = yAccu;
        y0 = 0.0;
        x0 = x2; 
        transformToImageCoordinates(x2, y2, targetImage);
        transformToImageCoordinates(x0, y0, targetImage);

        g.setColour(currentColour.withMultipliedAlpha(0.75f));

        double pn2 = alternator * pn;
        rosic::equalPowerGainFactors(pn2, &gL, &gR, -1.0, 1.0);

        panIndicatorSize = 40.0;
        topLength        = (float) (fabs(yAccu)*gL*panIndicatorSize);
        bottomLength     = (float) (fabs(yAccu)*gR*panIndicatorSize);
        g.drawLine((float) x0, 0.f,                (float) x0, topLength,                         2.f);
        g.drawLine((float) x0, (float)getHeight(), (float) x0, (float)(getHeight()-bottomLength), 2.f);

        g.setColour(currentColour);
        g.drawLine((float) x2, (float) y2, (float) x0, (float) y0, 2.f);          

        alternator *= signChanger; // changes sign - or not
        xAccu      += dt;
        yAccu      *= fb;
      }
    }

    if( i == selectedIndex )
    {
      g.setColour(currentColour.withMultipliedAlpha(0.5f));
      g.drawLine((float) x,        0.f, (float)          x, (float) getHeight(), 1.f);
      g.drawLine(      0.f,  (float) y, (float) getWidth(), (float) y          , 1.f);
      g.fillEllipse((float) (x-dotRadius-2), (float) (y-dotRadius-2), 
        (float) (2*dotRadius+4), (float) (2*dotRadius+4) );
    }
  }
}

void EchoLabPlotEditor::deRegisterFromObservedParameters()
{
  if( selectedDelayLine != NULL )
  {
    selectedDelayLine->getParameterByName("DelayTime")->deRegisterParameterObserver(this);
    selectedDelayLine->getParameterByName("Amplitude")->deRegisterParameterObserver(this);
    selectedDelayLine->getParameterByName("Feedback") ->deRegisterParameterObserver(this);
    selectedDelayLine->getParameterByName("Pan")      ->deRegisterParameterObserver(this);
    selectedDelayLine->getParameterByName("PingPong") ->deRegisterParameterObserver(this);
    selectedDelayLine->getParameterByName("Mute")     ->deRegisterParameterObserver(this);
  }
}

void EchoLabPlotEditor::refreshCompletely()
{
  // this function is called on total recall

  ScopedLock scopedLock(*plugInLock);

  /*
  // old:
  selectedIndex = -1;  
  if( delayLineEditor != NULL )
  delayLineEditor->setDelayLineModuleToEdit(NULL);
  */

  deSelectDelayLine();

  updatePlot();
}

//=================================================================================================


EchoLabModuleEditor::EchoLabModuleEditor(CriticalSection *newPlugInLock, 
  EchoLabAudioModule* newEchoLabAudioModule) 
  : AudioModuleEditor(newEchoLabAudioModule)
{
  setHeadlineStyle(MAIN_HEADLINE);

  jassert( newEchoLabAudioModule != NULL ); // you must pass a valid module here
  echoLabModuleToEdit = newEchoLabAudioModule;

  addWidget( dryWetSlider = new RSlider("DryWetSlider") );
  dryWetSlider->setSliderName(juce::String("Dry/Wet"));
  dryWetSlider->assignParameter( echoLabModuleToEdit->getParameterByName("DryWet") );
  dryWetSlider->setSliderName(juce::String("Dry/Wet"));
  dryWetSlider->setDefaultValue(0.5);
  dryWetSlider->setDescription( juce::String("Ratio between dry and wet signal") );
  dryWetSlider->setDescriptionField(infoField);
  dryWetSlider->setStringConversionFunction(&ratioToString0);

  addWidget( wetLevelSlider = new RSlider("WetLevelSlider") );
  wetLevelSlider->assignParameter( echoLabModuleToEdit->getParameterByName("WetLevel") );
  wetLevelSlider->setDescription( juce::String("Level of the wet signal in dB") );
  wetLevelSlider->setSliderName(juce::String("Wet Level"));
  wetLevelSlider->setDescriptionField(infoField);
  wetLevelSlider->setStringConversionFunction(decibelsToStringWithUnit2);

  addWidget( delaySyncButton = new RButton(juce::String("Sync")) );
  delaySyncButton->setDescription(
    juce::String("Toggle sync for delaytime on/off. Time unit is beats in sync-mode, seconds otherwise"));
  delaySyncButton->setDescriptionField(infoField);
  delaySyncButton->addRButtonListener(this);

  addWidget( snapToTimeGridButton = new RButton(juce::String("Snap:")) );
  snapToTimeGridButton->setDescription(juce::String("Toggle magnetic time-grid on/off."));
  snapToTimeGridButton->setDescriptionField(infoField);
  snapToTimeGridButton->addRButtonListener(this);

  addWidget( timeGridComboBox = new RTimeGridComboBox(juce::String("TimeGridComboBox")) );
  timeGridComboBox->setDescriptionField(infoField);
  timeGridComboBox->registerComboBoxObserver(this);

  delayPlotEditor = new EchoLabPlotEditor(lock, echoLabModuleToEdit);
  delayPlotEditor->setDescriptionField(infoField);
  delayPlotEditor->addChangeListener(this);
  addPlot(delayPlotEditor);

  snapToTimeGridButton->setToggleState(delayPlotEditor->isSnappingToFineGridX(), false);

  delayPlotZoomer = new CoordinateSystemZoomerOld();
  //delayPlotZoomer->setRelativeMargins(5.0, 5.0, 10.0, 10.0);
  delayPlotZoomer->setDescriptionField(infoField, true);
  delayPlotZoomer->setVerticalMouseWheelMode(CoordinateSystemZoomerOld::horizontalZoomViaVerticalMouseWheel);
  addChildColourSchemeComponent(delayPlotZoomer);
  delayPlotZoomer->setCoordinateSystem(delayPlotEditor);

  delayLineModuleEditor = new EchoLabDelayLineModuleEditor(newPlugInLock, nullptr);
    // i think, we really need to pass non-null pointer here. maybe a good place to apply the
    // Null Object pattern
  delayLineModuleEditor->setDescriptionField(infoField, true);
  addChildEditor(delayLineModuleEditor);

  delayPlotEditor->setDelayLineModuleEditor(delayLineModuleEditor);

  isTopLevelEditor = true;

  numHueOffsets = 1; // for filter-sections

  initializeColourScheme();
  updateWidgetsAccordingToState();
  setSize(600, 300);
}

//-------------------------------------------------------------------------------------------------
// setup:

void EchoLabModuleEditor::initializeColourScheme()
{
  updateSubEditorColourSchemes();
}

void EchoLabModuleEditor::updateSubEditorColourSchemes()
{
  ScopedLock scopedLock(*lock);
  delayLineModuleEditor->copyColourSettingsFrom(this);
  delayLineModuleEditor->setHueOffsetForFilterEditors(editorColourScheme.getHueOffset(0));
}

//-------------------------------------------------------------------------------------------------
// callbacks:

void EchoLabModuleEditor::copyColourSettingsFrom(const ColourSchemeComponent *componentToCopyFrom)
{
  AudioModuleEditor::copyColourSettingsFrom(componentToCopyFrom);
  delayLineModuleEditor->setHueOffsetForFilterEditors(editorColourScheme.getHueOffset(0));
}

void EchoLabModuleEditor::rButtonClicked(RButton *buttonThatWasClicked)
{
  ScopedLock scopedLock(*lock);

  if( buttonThatWasClicked == snapToTimeGridButton )
  {
    echoLabModuleToEdit->wrappedEchoLab->setSnapToTimeGrid( 
      snapToTimeGridButton->getToggleState() );
    delayPlotEditor->setSnapToFineGridX( snapToTimeGridButton->getToggleState() );
    delayPlotEditor->setVerticalFineGridVisible(snapToTimeGridButton->getToggleState());
    return; // shall not trigger markStateAsDirty()
  }
  else if( buttonThatWasClicked == delaySyncButton )
  {
    echoLabModuleToEdit->wrappedEchoLab->setSyncForDelayTimes(delaySyncButton->getToggleState());
    delayLineModuleEditor->updateWidgetsAccordingToState();
  }    
  else
    AudioModuleEditor::rButtonClicked(buttonThatWasClicked);


  echoLabModuleToEdit->markStateAsDirty();  // hmmm - -do we need this?
}

void EchoLabModuleEditor::rComboBoxChanged(RComboBox *rComboBoxThatHasChanged)
{
  if( rComboBoxThatHasChanged == timeGridComboBox )
    delayPlotEditor->setVerticalFineGridInterval(timeGridComboBox->getValue());
}

void EchoLabModuleEditor::changeListenerCallback(ChangeBroadcaster *objectThatHasChanged)
{
  if( objectThatHasChanged == delayPlotEditor )
  {
    if( echoLabModuleToEdit != NULL )
      echoLabModuleToEdit->markStateAsDirty();

    //equalizerEditor->setEqualizerCoreToEdit(delayPlotEditor->getSelectedDelayLineEqualizer());
    //echoLabModuleToEdit->inputFilterModule->setEqualizerToWrap(
  }
  else
  {
    // the call must have been due to preset recall - deselect band in this case:
    //delayPlotEditor->deSelectDelayLine();
    delayPlotEditor->refreshCompletely();
    AudioModuleEditor::changeListenerCallback(objectThatHasChanged);
  }
}

void EchoLabModuleEditor::updateWidgetsAccordingToState()
{
  if( echoLabModuleToEdit == NULL )
    return;
  if( echoLabModuleToEdit->wrappedEchoLab == NULL )
    return;

  AudioModuleEditor::updateWidgetsAccordingToState();
  delayPlotEditor->updatePlot();

  delaySyncButton->setToggleState(echoLabModuleToEdit->wrappedEchoLab->isDelayTimeSynced(), false);

  bool snap = echoLabModuleToEdit->wrappedEchoLab->isSnappingToTimeGrid();
  snapToTimeGridButton->setToggleState(snap, false );
  delayPlotEditor->setSnapToFineGridX( snap );
  delayPlotEditor->setVerticalFineGridVisible(snap);
}

void EchoLabModuleEditor::resized()
{
  AudioModuleEditor::resized();
  int x = 0;
  int y = getHeadlineBottom();
  int w = getWidth();
  int h = getHeight();

  stateWidgetSet->setBounds(x, y+4, stateWidgetSet->getWidth()/2, stateWidgetSet->getHeight());

  int equalizerHeight = 220;

  y = getPresetSectionBottom();
  h = infoField->getY()-y-equalizerHeight;
  delayPlotEditor->setBounds(4, y+4, 
    w-delayPlotZoomer->getZoomerSize()-8+RWidget::outlineThickness, 
    h-delayPlotZoomer->getZoomerSize()-4);
  delayPlotZoomer->alignWidgetsToCoordinateSystem();

  x = w - 44;
  y = delayPlotZoomer->getY() - 16 + RWidget::outlineThickness;
  snapToTimeGridButton->setBounds(x-48, y, 44, 16);
  timeGridComboBox->setBounds(    x,    y, 40, 16);
  delaySyncButton->setBounds(snapToTimeGridButton->getX()-48,  y, 40, 16);

  x = stateWidgetSet->getRight();
  y = stateWidgetSet->getY();
  w = delaySyncButton->getX()-x;
  dryWetSlider->setBounds(x+4,       y, w/2-8, 16);
  wetLevelSlider->setBounds(x+w/2+4, y, w/2-8, 16);

  x = 4; 
  y = delayPlotZoomer->getBottom() - RWidget::outlineThickness;
  w = getWidth() - 8;
  h = infoField->getY() - y; 
  delayLineModuleEditor->setBounds(x, y, w, h);
}
