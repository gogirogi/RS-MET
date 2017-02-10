
ModuleChainer::ModuleChainer(CriticalSection *lockToUse) : AudioModuleWithMidiIn(lockToUse)
{
  ScopedLock scopedLock(*plugInLock);
  moduleName = "ModuleChainer";
  setActiveDirectory(getApplicationDirectory() + "/ModuleChainerPresets");

  addModule("None"); // always have at least one dummy module in the chain
}

ModuleChainer::~ModuleChainer()
{
  for(int i = 0; i < modules.size(); i++)
    delete modules[i];
}

AudioModule* ModuleChainer::createModule(const String& type)
{
  if(type == "None")         return new DummyModule( plugInLock);
  if(type == "PhaseScope")   return new PhaseScope(  plugInLock);
  if(type == "Enveloper")    return new Enveloper(   plugInLock);
  if(type == "Ladder")       return new Ladder(      plugInLock);
  if(type == "PhasorFilter") return new PhasorFilter(plugInLock);
  // add more module types here...

  jassertfalse;  // unknown module type requested
  return nullptr;
}

void ModuleChainer::addModule(const String& type)
{
  AudioModule *m = createModule(type);
  modules.add(m);
}

// overrides:

AudioModuleEditor* ModuleChainer::createEditor()
{
  return new ModuleChainerEditor(this);
}

void ModuleChainer::processBlock(double **inOutBuffer, int numChannels, int numSamples)
{
  jassert(numChannels == 2);
  for(int i = 0; i < modules.size(); i++)
    modules[i]->processBlock(inOutBuffer, numChannels, numSamples);
}

void ModuleChainer::setSampleRate(double newSampleRate)
{
  for(int i = 0; i < modules.size(); i++)
    modules[i]->setSampleRate(newSampleRate);
}

void ModuleChainer::noteOn(int noteNumber, int velocity)
{
  for(int i = 0; i < modules.size(); i++)
  {
    AudioModuleWithMidiIn *m = dynamic_cast<AudioModuleWithMidiIn*> (modules[i]);
    if(m != nullptr)
      m->noteOn(noteNumber, velocity);
  }
}

void ModuleChainer::noteOff(int noteNumber)
{
  for(int i = 0; i < modules.size(); i++)
  {
    AudioModuleWithMidiIn *m = dynamic_cast<AudioModuleWithMidiIn*> (modules[i]);
    if(m != nullptr)
      m->noteOff(noteNumber);
  }
}

void ModuleChainer::reset()
{
  for(int i = 0; i < modules.size(); i++)
    modules[i]->reset();
}

//=================================================================================================

AudioModuleSelector::AudioModuleSelector() : RComboBox("ModuleSelector") 
{
  // add menu items for the different module types:
  int i = 0;
  addItem(i, "None");         i++;
  addItem(i, "Ladder");       i++;
  addItem(i, "PhasorFilter"); i++;
  addItem(i, "PhaseScope");   i++;
  addItem(i, "Enveloper");    i++;
}

//=================================================================================================

ModuleChainerEditor::ModuleChainerEditor(jura::ModuleChainer *moduleChainerToEdit)
  : AudioModuleEditor(moduleChainerToEdit)
{
  ScopedLock scopedLock(*plugInLock);
  chainer = moduleChainerToEdit;
  createWidgets();
  setSize(200, 100);
}

void ModuleChainerEditor::createWidgets()
{
  // create combo-boxes for selecting modules...
}

void ModuleChainerEditor::resized()
{
  // We should arrange the GUI in a way such that the plugin-chain and global preset all go into 
  // the left column (maybe around 150..200 pixels wide). To the right of that, the editor for the
  // currently active plugin is shown

  // maybe, we could have bypass switches for each plugin

  // arrange combo-boxes...
}