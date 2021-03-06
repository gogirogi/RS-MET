#ifndef RS_POLYNOMIALTESTS_H
#define RS_POLYNOMIALTESTS_H

#include "../../UnitTestUtilities.h"

bool testPolynomial(std::string &reportString);

// polynomial algorithms:

bool testConvolution(std::string &reportString);

bool testCubicCoeffsFourPoints(std::string &reportString);
bool testCubicCoeffsTwoPointsAndDerivatives(std::string &reportString);
bool testPolynomialEvaluation(std::string &reportString);
bool testPolynomialDivision(std::string &reportString);
bool testPolynomialArgumentShift(std::string &reportString);
bool testPolynomialDiffAndInt(std::string &reportString);
bool testPolynomialFiniteDifference(std::string &reportString);
bool testPolynomialComposition(std::string &reportString);
bool testPolynomialWeightedSum(std::string &reportString);
bool testPolynomialIntegrationWithPolynomialLimits(std::string &reportString);
bool testPolynomialInterpolation(std::string &reportString);
bool testPolynomialRootFinder(std::string &reportString);
bool testPartialFractionExpansion(std::string &reportString);
bool testPolynomialBaseChange(std::string &reportString);
bool testPolynomialRecursion(std::string &reportString);
bool testJacobiPolynomials(std::string &reportString);

bool testPowersChebychevExpansionConversion(std::string &reportString);





// polynomial class:

bool testPolynomialOperators(std::string &reportString);


#endif
