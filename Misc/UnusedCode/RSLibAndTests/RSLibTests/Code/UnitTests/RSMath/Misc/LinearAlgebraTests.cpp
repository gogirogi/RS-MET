#include "LinearAlgebraTests.h"

bool testLinearAlgebra(std::string &reportString)
{
  std::string testName = "LinearAlgebra";
  bool testResult = true;

  testResult &= testLinearSystem2x2(      reportString);
  testResult &= testLinearSystem3x3(      reportString);
  testResult &= testLinearSystemViaGauss( reportString);
  testResult &= testGaussJordanInversion( reportString);
  testResult &= testTridiagonalSystem(    reportString);
  testResult &= testSquareMatrixTranspose(reportString);
  testResult &= testMatrixVectorMultiply( reportString);
  testResult &= testMatrixMultiply(       reportString);
  testResult &= testChangeOfBasis(        reportString);

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}

bool testLinearSystem2x2(std::string &reportString)
{
  std::string testName = "LinearSystem2x2";
  bool testResult = true;

  double x[2];
  double y[2]    = {17, 39};
  double A[2][2] = {{1, 2},
                    {3, 4}};

  rsSolveLinearSystem2x2(A, x, y);

  testResult &= (x[0] == 5.0);
  testResult &= (x[1] == 6.0);

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}

bool testLinearSystem3x3(std::string &reportString)
{
  std::string testName = "LinearSystem3x3";
  bool testResult = true;

  double x[3];
  double y[3]    = {-4, 15, 11};
  double A[3][3] = {{ 1, 2, -3},
                    {-5, 7,  2},
                    {-2, 2,  3}};

  rsSolveLinearSystem3x3(A, x, y);

  testResult &= (x[0] == 1.0);
  testResult &= (x[1] == 2.0);
  testResult &= (x[2] == 3.0);

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}

bool testLinearSystemViaGauss(std::string &reportString)
{
  std::string testName = "LinearSystemViaGauss";
  bool testResult = true;

  rsComplexDbl x[3];
  rsComplexDbl y[3]    = {rsComplexDbl(-2,-6), rsComplexDbl(-19,+16), rsComplexDbl(+44,-37)};
  rsComplexDbl A[3][3] = {{rsComplexDbl(+1,+2), rsComplexDbl(+2,-1), rsComplexDbl(-3,+1)},
                          {rsComplexDbl(+4,-2), rsComplexDbl(-6,+1), rsComplexDbl(+5,-2)},
                          {rsComplexDbl(-9,+1), rsComplexDbl(+7,-2), rsComplexDbl(+8,+2)}};
  rsComplexDbl *pA[3];
  for(int i = 0; i < 3; i++)
    pA[i] = &A[i][0]; // we need a pointer-to-pointer for the solver

  rsSolveLinearSystem(pA, x, y, 3);

  rsComplexDbl xt[3] = {rsComplexDbl(-2,+1), rsComplexDbl(+3,-2), rsComplexDbl(+1,-1)};
  double tol = 1.e-14;
  testResult &= rsAbs(x[0]-xt[0]) < tol;
  testResult &= rsAbs(x[1]-xt[1]) < tol;
  testResult &= rsAbs(x[2]-xt[2]) < tol;

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}

bool testGaussJordanInversion(std::string &reportString)
{
  std::string testName = "GaussJordanInversion";
  bool testResult = true;

  double A[3][3] = {{2,  1, 4},
                    {3, 10, 3},
                    {1,  5, 1}};
  double *pA[3];
  for(int i = 0; i < 3; i++)
    pA[i] = &A[i][0];

  rsInvertMatrix(pA, 3);

  // create the correct inverse matrix:
  double Ai[3][3] = {{-0.5, +1.9, -3.7},
                     {+0.0, -0.2, +0.6},
                     {+0.5, -0.9, +1.7}};

  // compare - note that we must use the pointer pA instead of A itself because the inversion 
  // routine may exchange rows by just swapping the pointers to the rows:
  double tol = 1.e-14;
  testResult &= rsAbs(pA[0][0]-Ai[0][0]) < tol;
  testResult &= rsAbs(pA[0][1]-Ai[0][1]) < tol;
  testResult &= rsAbs(pA[0][2]-Ai[0][2]) < tol;
  testResult &= rsAbs(pA[1][0]-Ai[1][0]) < tol;
  testResult &= rsAbs(pA[1][1]-Ai[1][1]) < tol;
  testResult &= rsAbs(pA[1][2]-Ai[1][2]) < tol;
  testResult &= rsAbs(pA[2][0]-Ai[2][0]) < tol;
  testResult &= rsAbs(pA[2][1]-Ai[2][1]) < tol;
  testResult &= rsAbs(pA[2][2]-Ai[2][2]) < tol;

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}

bool testTridiagonalSystem(std::string &reportString)
{
  std::string testName = "TridiagonalSystem";
  bool testResult = true;

  double xt[5];
  double x[5];
  double y[5]    =  {-4, +3, +1, -5, -2};
  double A[5][5] = {{+2, -1,  0,  0,  0},
                    {+3, +5, +3,  0,  0},
                    { 0, +2, -3, -5,  0},
                    { 0,  0, +5, -2, +6},
                    { 0,  0,  0, -3, -7}};
  double *pA[5];
  for(int i = 0; i < 5; i++)
    pA[i] = &A[i][0];

  // solve via standard procedure to obtain the target solution:
  rsSolveLinearSystem(pA, xt, y, 5);

  // solve via special tridiagonal procedure:
  double lower[4] = {+3, +2, +5, -3};     // lower diagonal of A
  double main[5]  = {+2, +5, -3, -2, -7}; // main  diagonal of A
  double upper[4] = {-1, +3, -5, +6};     // upper diagonal of A
  rsSolveTridiagonalSystem(lower, main, upper, y, x, 5);

  // compare results:  
  double tol = 1.e-14;
  for(int i = 0; i < 5; i++)
    testResult &= fabs(x[i]-xt[i]) < tol;

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}


bool testSquareMatrixTranspose(std::string &reportString)
{
  std::string testName = "SquareMatrixTranspose";
  bool testResult = true;

  // create test-matrix:
  double A[5][5] = {{ 1,  2,  3,  4,  5},
                    { 6,  7,  8,  9, 10},
                    {11, 12, 13, 14, 15},
                    {16, 17, 18, 19, 20},
                    {21, 22, 23, 24, 25}};

  // create transposed test-matrix:
  double T[5][5] = {{ 1,  6, 11, 16, 21},
                    { 2,  7, 12, 17, 22},
                    { 3,  8, 13, 18, 23},
                    { 4,  9, 14, 19, 24},
                    { 5, 10, 15, 20, 25}};

  // test (in-place) transposition of matrix A:
  double *pA[5];
  for(int i = 0; i < 5; i++)
    pA[i] = &A[i][0];


  rsTransposeSquareArray(pA, 5);

  for(int i = 0; i < 5; i++)
  {
    for(int j = 0; j < 5; j++)
      testResult &= A[i][j] == T[i][j];
  }

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}



// todo: Gram-Schmidt orthogonalization of a set of basis vectors


bool testMatrixVectorMultiply(std::string &reportString)
{
  std::string testName = "MatrixVectorMultiply";
  bool testResult = true;

  // create test-matrix and vector, allocate result vector:
  double A[3][3] = {{1, 2, 3},
                    {4, 5, 6},
                    {7, 8, 9}};
  double *pA[3];
  for(int i = 0; i < 3; i++)
    pA[i] = &A[i][0];
  double x[3] =  {1, 2, 3};
  double y[3];

  // 3x3-matrix times 3-vector:
  // |1 2 3| * |1| = |14|
  // |4 5 6|   |2|   |32|
  // |7 8 9|   |3|   |50|
  rsFillWithValue(y, 3, -1.0);
  rsMatrixVectorMultiply(pA, x, y, 3, 3);
  testResult &= y[0] == 14;
  testResult &= y[1] == 32;
  testResult &= y[2] == 50;

  // transposed 3x3-matrix times 3-vector:
  // |1 2 3|^T * |1| = |1 4 7| * |1| = |30|
  // |4 5 6|     |2|   |2 5 8|   |2|   |36|
  // |7 8 9|     |3|   |3 6 9|   |3|   |42|
  rsFillWithValue(y, 3, -1.0);
  rsTransposedMatrixVectorMultiply(pA, x, y, 3, 3);
  testResult &= y[0] == 30;
  testResult &= y[1] == 36;
  testResult &= y[2] == 42;

  // 3x2-matrix times 2-vector:
  // |1 2| * |1| = | 5|
  // |4 5|   |2|   |14|
  // |7 8|         |23|
  rsFillWithValue(y, 3, -1.0);
  rsMatrixVectorMultiply(pA, x, y, 3, 2);
  testResult &= y[0] == 5;
  testResult &= y[1] == 14;
  testResult &= y[2] == 23;

  // transposed 2x3-matrix times 2-vector:
  // |1 2 3|^T * |1| = |1 4| * |1| = | 9|
  // |4 5 6|     |2|   |2 5|   |2|   |12|
  //                   |3 6|         |15|
  rsFillWithValue(y, 3, -1.0);
  rsTransposedMatrixVectorMultiply(pA, x, y, 2, 3);
  testResult &= y[0] == 9;
  testResult &= y[1] == 12;
  testResult &= y[2] == 15;

  // 2x3-matrix times 3-vector:
  // |1 2 3| * |1| = |14|
  // |4 5 6|   |2|   |32|
  //           |3|
  rsFillWithValue(y, 3, -1.0);
  rsMatrixVectorMultiply(pA, x, y, 2, 3);
  testResult &= y[0] == 14;
  testResult &= y[1] == 32;
  testResult &= y[2] == -1;

  // transposed 3x2-matrix times 3-vector:
  // |1 2|^T * |1| = |1 4 7| * |1| = |30|
  // |4 5|     |2|   |2 5 8|   |2|   |36|
  // |7 8|     |3|             |3|
  rsFillWithValue(y, 3, -1.0);
  rsTransposedMatrixVectorMultiply(pA, x, y, 3, 2);
  testResult &= y[0] == 30;
  testResult &= y[1] == 36;
  testResult &= y[2] == -1;

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}

bool testMatrixMultiply3x3()
{
  // This function tests only some special cases where P either equals N or M using matrices where
  // no index-range exceeds 3. It is called from the more general test function
  // testMatrixMultiply3x3 internally.

  bool testResult = true;

  double A[3][3] = {{1, 2, 3},
                    {4, 5, 6},
                    {7, 8, 9}};
  double B[3][3] = {{9, 8, 7},
                    {6, 5, 4},
                    {3, 2, 1}};
  double C[3][3];
  double *pA[3], *pB[3], *pC[3];
  for(int i = 0; i < 3; i++)
  {
    pA[i] = &A[i][0];
    pB[i] = &B[i][0];
    pC[i] = &C[i][0];
  }

  // 2x3-matrix times 3x2 matrix (unused fields of result matrix should retain init-value -1):
  // |1 2 3| * |9 8| = |30 24|
  // |4 5 6|   |6 5|   |84 69|
  //           |3 2|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsMatrixMultiply(pA, pB, pC, 2, 3, 2);
  testResult &= C[0][0] == 30 && C[0][1] == 24 && C[0][2] == -1;
  testResult &= C[1][0] == 84 && C[1][1] == 69 && C[1][2] == -1;
  testResult &= C[2][0] == -1 && C[2][1] == -1 && C[2][2] == -1;

  // 3x2-matrix times 2x3 matrix:
  // |1 2| * |9 8 7| = | 21 18 15|
  // |4 5|   |6 5 4|   | 66 57 48|
  // |7 8|             |111 96 81|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsMatrixMultiply(pA, pB, pC, 3, 2, 3);
  testResult &= C[0][0] ==  21 && C[0][1] == 18 && C[0][2] == 15;
  testResult &= C[1][0] ==  66 && C[1][1] == 57 && C[1][2] == 48;
  testResult &= C[2][0] == 111 && C[2][1] == 96 && C[2][2] == 81;

  // transposed 3x2-matrix times 3x2 matrix:
  // |1 2|^T * |9 8| = |1 4 7| * |9 8| = |54 42|
  // |4 5|     |6 5|   |2 5 8|   |6 5|   |72 57|
  // |7 8|     |3 2|             |3 2|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsMatrixMultiplyFirstTransposed(pA, pB, pC, 3, 2, 2);
  testResult &= C[0][0] == 54 && C[0][1] == 42 && C[0][2] == -1;
  testResult &= C[1][0] == 72 && C[1][1] == 57 && C[1][2] == -1;
  testResult &= C[2][0] == -1 && C[2][1] == -1 && C[2][2] == -1;

  // transposed 2x3-matrix times 2x3 matrix:
  // |1 2 3|^T * |9 8 7| = |1 4| * |9 8 7| = |33 28 23|
  // |4 5 6|     |6 5 4|   |2 5|   |6 5 4|   |48 41 34|
  //                       |3 6|             |63 54 45|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsMatrixMultiplyFirstTransposed(pA, pB, pC, 2, 3, 3);
  testResult &= C[0][0] == 33 && C[0][1] == 28 && C[0][2] == 23;
  testResult &= C[1][0] == 48 && C[1][1] == 41 && C[1][2] == 34;
  testResult &= C[2][0] == 63 && C[2][1] == 54 && C[2][2] == 45;

  // 2x3-matrix times transposed 2x3 matrix:
  // |1 2 3| * |9 8 7|^T = |1 2 3| * |9 6| = | 46 28|
  // |4 5 6|   |6 5 4|     |4 5 6|   |8 5|   |118 73|
  //                                 |7 4|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsMatrixMultiplySecondTransposed(pA, pB, pC, 2, 3, 2);
  testResult &= C[0][0] ==  46 && C[0][1] == 28 && C[0][2] == -1;
  testResult &= C[1][0] == 118 && C[1][1] == 73 && C[1][2] == -1;
  testResult &= C[2][0] ==  -1 && C[2][1] == -1 && C[2][2] == -1;

  // 3x2-matrix times transposed 3x2 matrix:
  // |1 2| * |9 8|^T = |1 2| * |9 6 3| = | 25 16  7|
  // |4 5|   |6 5|     |4 5|   |8 5 2|   | 76 49 22|
  // |7 8|   |3 2|     |7 8|             |127 82 37|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsMatrixMultiplySecondTransposed(pA, pB, pC, 3, 2, 3);
  testResult &= C[0][0] ==  25 && C[0][1] == 16 && C[0][2] ==  7;
  testResult &= C[1][0] ==  76 && C[1][1] == 49 && C[1][2] == 22;
  testResult &= C[2][0] == 127 && C[2][1] == 82 && C[2][2] == 37;

  // transposed 2x3-matrix times transposed 3x2 matrix:
  // |1 2 3|^T * |9 8|^T = |1 4| * |9 6 3| = |41 26 11|
  // |4 5 6|     |6 5|     |2 5|   |8 5 2|   |58 37 16|
  //             |3 2|     |3 6|             |75 48 21|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsMatrixMultiplyBothTransposed(pA, pB, pC, 2, 3, 3);
  testResult &= C[0][0] == 41 && C[0][1] == 26 && C[0][2] == 11;
  testResult &= C[1][0] == 58 && C[1][1] == 37 && C[1][2] == 16;
  testResult &= C[2][0] == 75 && C[2][1] == 48 && C[2][2] == 21;

  // transposed 3x2-matrix times transposed 2x3 matrix:
  // |1 2|^T * |9 8 7|^T = |1 4 7| * |9 6| = | 90 54|
  // |4 5|     |6 5 4|     |2 5 8|   |8 5|   |114 69|
  // |7 8|                           |7 4|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsMatrixMultiplyBothTransposed(pA, pB, pC, 3, 2, 2);
  testResult &= C[0][0] ==  90 && C[0][1] == 54 && C[0][2] == -1;
  testResult &= C[1][0] == 114 && C[1][1] == 69 && C[1][2] == -1;
  testResult &= C[2][0] ==  -1 && C[2][1] == -1 && C[2][2] == -1;

  // 3x3-matrix times 3x3-matrix in place multiplication:
  // |1 2 3| * |9 8 7| = | 30  24 18|
  // |4 5 6|   |6 5 4|   | 84  69 54|
  // |7 8 9|   |3 2 1|   |138 114 90|
  rsCopyMatrix(pA, pC, 3, 3);
  rsMatrixInPlaceMultiply(pC, pB, 3, 3);
  testResult &= C[0][0] ==  30 && C[0][1] ==  24 && C[0][2] == 18;
  testResult &= C[1][0] ==  84 && C[1][1] ==  69 && C[1][2] == 54;
  testResult &= C[2][0] == 138 && C[2][1] == 114 && C[2][2] == 90;

  // 2x3-matrix times 3x3-matrix in place multiplication:
  // |1 2 3| * |9 8 7| = | 30  24 18|
  // |4 5 6|   |6 5 4|   | 84  69 54|
  //           |3 2 1|
  rsInitMatrix(pC, 3, 3, -1.0);
  rsCopyMatrix(pA, pC, 2, 3);
  rsMatrixInPlaceMultiply(pC, pB, 2, 3);
  testResult &= C[0][0] ==  30 && C[0][1] ==  24 && C[0][2] == 18;
  testResult &= C[1][0] ==  84 && C[1][1] ==  69 && C[1][2] == 54;
  testResult &= C[2][0] ==  -1 && C[2][1] == -1 && C[2][2] == -1;

  return testResult;
}

bool testMatrixMultiply(std::string &reportString)
{
  std::string testName = "MatrixMultiply";
  bool testResult = testMatrixMultiply3x3();

  double A[4][4] = {{  2,   3,   5,   7},
                    { 11,  13,  17,  19},
                    { 23,  29,  31,  37},
                    { 41,  43,  47,  53}};
  double B[4][4] = {{ 59,  61,  67,  71},
                    { 73,  79,  83,  89},
                    { 97, 101, 103, 107},
                    {109, 113, 127, 131}};
  double C[4][4];
  double *pA[4], *pB[4], *pC[4];
  for(int i = 0; i < 4; i++)
  {
    pA[i] = &A[i][0];
    pB[i] = &B[i][0];
    pC[i] = &C[i][0];
  }

  // 2x3 matrix times 3x4 matrix:
  // | 2  3  5| * |59  61  67  71| = | 822  864  898  944|
  // |11 13 17|   |73  79  83  89|   |3247 3415 3567 3757|
  //              |97 101 103 107|
  rsInitMatrix(pC, 4, 4, -1.0);
  rsMatrixMultiply(pA, pB, pC, 2, 3, 4);
  testResult &= C[0][0] ==  822 && C[0][1] ==  864 && C[0][2] ==  898 && C[0][3] ==  944;
  testResult &= C[1][0] == 3247 && C[1][1] == 3415 && C[1][2] == 3567 && C[1][3] == 3757;
  testResult &= C[2][0] ==   -1 && C[2][1] ==   -1 && C[2][2] ==   -1 && C[2][3] ==   -1;
  testResult &= C[3][0] ==   -1 && C[3][1] ==   -1 && C[3][2] ==   -1 && C[3][3] ==   -1;

  // 2x3 matrix times transposed 3x4 matrix:
  // | 2  3  5| * | 59  61  67|^T = | 636  798 1012 1192|
  // |11 13 17|   | 73  79  83|     |2581 3241 4131 4827|
  //              | 97 101 103|
  //              |109 113 127|
  rsInitMatrix(pC, 4, 4, -1.0);
  rsMatrixMultiplySecondTransposed(pA, pB, pC, 2, 3, 4);
  testResult &= C[0][0] ==  636 && C[0][1] ==  798 && C[0][2] == 1012 && C[0][3] == 1192;
  testResult &= C[1][0] == 2581 && C[1][1] == 3241 && C[1][2] == 4131 && C[1][3] == 4827;
  testResult &= C[2][0] ==   -1 && C[2][1] ==   -1 && C[2][2] ==   -1 && C[2][3] ==   -1;
  testResult &= C[3][0] ==   -1 && C[3][1] ==   -1 && C[3][2] ==   -1 && C[3][3] ==   -1;

  // transposed 3x2 matrix times 3x4 matrix:
  // | 2  3|^T * |59  61  67  71| = |3152 3314 3416 3582|
  // |11 13|     |73  79  83  89|   |3939 4139 4267 4473|
  // |23 29|     |97 101 103 107|
  rsInitMatrix(pC, 4, 4, -1.0);
  rsMatrixMultiplyFirstTransposed(pA, pB, pC, 3, 2, 4);
  testResult &= C[0][0] == 3152 && C[0][1] == 3314 && C[0][2] == 3416 && C[0][3] == 3582;
  testResult &= C[1][0] == 3939 && C[1][1] == 4139 && C[1][2] == 4267 && C[1][3] == 4473;
  testResult &= C[2][0] ==   -1 && C[2][1] ==   -1 && C[2][2] ==   -1 && C[2][3] ==   -1;
  testResult &= C[3][0] ==   -1 && C[3][1] ==   -1 && C[3][2] ==   -1 && C[3][3] ==   -1;

  // transposed 3x2 matrix times transposed 4x3 matrix:
  // | 2  3|^T * | 59  61  67|^T = |2330 2924 3647 4382|
  // |11 13|     | 73  79  83|     |2913 3653 4691 5479|
  // |23 29|     | 97 101 103|
  //             |109 113 127|
  rsInitMatrix(pC, 4, 4, -1.0);
  rsMatrixMultiplyBothTransposed(pA, pB, pC, 3, 2, 4);
  testResult &= C[0][0] == 2330 && C[0][1] == 2924 && C[0][2] == 3674 && C[0][3] == 4382;
  testResult &= C[1][0] == 2913 && C[1][1] == 3653 && C[1][2] == 4591 && C[1][3] == 5479;
  testResult &= C[2][0] ==   -1 && C[2][1] ==   -1 && C[2][2] ==   -1 && C[2][3] ==   -1;
  testResult &= C[3][0] ==   -1 && C[3][1] ==   -1 && C[3][2] ==   -1 && C[3][3] ==   -1;

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}

bool testChangeOfBasis(std::string &reportString)
{
  std::string testName = "ChangeOfBasis";
  bool testResult = true;

  // basis A and B (the columns represent the basis vectors)
  double A[3][3] = {{-2,  3,  4},
                    {-3,  2, -5},
                    { 2, -3,  2}};

  double B[3][3] = {{ 2,  1, -2},
                    { 1, -3,  2},
                    {-4, -2,  3}};
  double AT[3][3], BT[3][3]; // transposed bases (rows are basis vectors)
  double C[3][3],  CT[3][3]; // change-of-base matrices (column-, and row-case)

  // row-pointers:
  double *pA[3], *pB[3], *pC[3], *pAT[3], *pBT[3], *pCT[3];
  for(int i = 0; i < 3; i++)
  {
    pA[i]  = &A[i][0];
    pB[i]  = &B[i][0];
    pC[i]  = &C[i][0];
    pAT[i] = &AT[i][0];
    pBT[i] = &BT[i][0];
    pCT[i] = &CT[i][0];
  }

  // obtain the row-wise bases:
  rsTransposeSquareArray(pA, pAT, 3);
  rsTransposeSquareArray(pB, pBT, 3);

  // coordinates of vector v in basis A:
  double va[3] = {1, -2, 3};

  // compute coordinates of v in basis B:
  double vb[3];
  rsChangeOfBasisColumnWise(pA, pB, va, vb, 3);

  // compute canonical coordinates of v from its coordinates in va and vb and compare:
  double vea[3], veb[3];
  rsMatrixVectorMultiply(pA, va, vea, 3, 3);
  rsMatrixVectorMultiply(pB, vb, veb, 3, 3);
  testResult &= rsAreBuffersEqual(vea, veb, 3);

  // compute change-of-base matrix to go from basis A to B:
  rsChangeOfBasisMatrixColumnWise(pA, pB, pC, 3);

  // compute coordinates of v in B again, this time using the change-of-base matrix, compare to
  // previously computed coordinates:
  double vb2[3];
  rsMatrixVectorMultiply(pC, va, vb2, 3, 3);
  testResult &= rsAreBuffersApproximatelyEqual(vb, vb2, 3, 1.e-14);

  // tests for row-based representations of bases A and B:
  rsChangeOfBasisRowWise(pAT, pBT, va, vb, 3);
  testResult &= rsAreBuffersApproximatelyEqual(vb, vb2, 3, 1.e-14);

  rsChangeOfBasisMatrixRowWise(pAT, pBT, pCT, 3);
  rsMatrixVectorMultiply(pCT, va, vb2, 3, 3);
  testResult &= rsAreBuffersApproximatelyEqual(vb, vb2, 3, 1.e-14);

  testResult &= rsAreMatricesApproximatelyEqual(pC, pCT, 3, 3, 1.e-14);
   // the change-of-base matrix doesn't care about, how the base-vectors are represented in some 
   // matrix-representation, so it should be the same in both cases 

  appendTestResultToReport(reportString, testName, testResult);
  return testResult;
}

// i think, i found a rule: iff C = A^-1 * B, then C^-1 = B^-1 * A - is this a known theorem?
// it can easily be found from A*x = B*y (where x,y and either A or B are assumed to be known) and
// the solving for x and y respectively and defining C as conversion matrix from x to y and C-^1
// the inverse conversion, can also be verified by E = C * C^-1 = A^-1 * B * B^-1 * A
