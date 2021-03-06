//#include "AutomaticTests.h"
//#include "InteractiveTests.h"
//#include "PerformanceTests.h"

#include "../romos.h"

bool runUnitTests()
{
  romos::UnitTestRunner testRunner;
  bool testsPassed = testRunner.runAllTestsAndPrintResultsToConsole();


  if( testsPassed == false )
    printf("%s", "At least one unit test FAILED !!! FAILED !!! FAILED !!! FAILED !!! FAILED !!!\n");
  else
    printf("%s", "All unit tests passed\n");
  printf("%s", "\n");


  return testsPassed;
}

void runPerformanceTests(bool createLogFile)
{
  romos::PerformanceTestRunner testRunner;
  testRunner.runAllTestsAndPrintResultsToConsole(createLogFile);
}

/*
void runAutomatedCorrectnessTests()
{
  bool verboseOutput    = false;
  int  numVoicesToCheck = 3;

  runUnitTests();
  //runPerformanceTests(false);

  //double *leak = new double[100];
  //String *leakString = new String("LeakString"); // uncomment this when you want to trigger the leak-detection deliberately
}
*/



void testCodeGenerator()
{
  // here we choose what kind of module to create (and genreate code for):
  //romos::Module *testModule = createSumDiffModule(NULL);
  //romos::Module *testModule = createWrappedSumDiffModule(NULL);
  romos::Module *testModule = romos::TestModuleBuilder::createTestFilter1("TestFilter1", 0, 0, false);

  rosic::rsString codeForModule = romos::ModuleBuildCodeGenerator::getCodeForModule(testModule);
  codeForModule.printToStandardOutput();
  romos::ModuleFactory::deleteModule(testModule);
}

void runInteractiveTests()
{
  romos::InteractiveTestRunner testRunner;
  testRunner.runTests();


  //testCodeGenerator();  // move this into the testrunner, create a class for the codegenerator test etc.
}

/*
void runPerformanceTests()
{
  printModuleByteSizes();
  //printf("%s %.3f %s", "Performance hit for biquad: ", measureFrameworkOverheadWithBiquad(), "\n" );
}
*/

void runTests()
{
  /*
  // move this into a test-class of it own - maybe we need some reliable, invertible and pretty 
  // double->string->double roundtrip implementations in romos
  rosic::rsString s01 = rosic::rsString(6467325487632587632.0234);
  rosic::rsString s02 = rosic::rsString(0.2);
  rosic::rsString s03 = rosic::rsString(0.7);
  rosic::rsString s04 = rosic::rsString(0.9);
  rosic::rsString s05 = rosic::rsString(0.0002);
  rosic::rsString s06 = rosic::rsString(0.9999);
  rosic::rsString s07 = rosic::rsString(1.2);
  rosic::rsString s08 = rosic::rsString(1.9);
  rosic::rsString s09 = rosic::rsString(10.2);
  rosic::rsString s10 = rosic::rsString(10.9);
  rosic::rsString s11 = rosic::rsString(10.79);
  rosic::rsString s12 = rosic::rsString(10.99);
  rosic::rsString s13 = rosic::rsString(11.99);
  rosic::rsString s14 = rosic::rsString(117.99);
  rosic::rsString s15 = rosic::rsString(9.9999999999999999);
  */

  double someDouble = -0.4;
  double absOfSomeDouble = fabs(someDouble);
  double abs2 = rAbs(someDouble);
  //unsigned long long intAbsValue = *((unsigned long long*) &someDouble) & 0x7FFFFFFFFFFFFFFFULL;
  //abs2 = *((double*) &intAbsValue);


  //runUnitTests();
  runInteractiveTests();
  //runPerformanceTests(false);

  romos::ModuleTypeRegistry::deleteSoleInstance(); // deletes the singleton object
  romos::BlitIntegratorInitialStates::deleteStateValueTables();
}





int main(int argc, char** argv)
{


  runTests();

  if( detectMemoryLeaks() )
    printf("%s", "Memory leaks detected\n");

  printf("%s", "Tests done.");
  //getchar();
  return 0;
}
