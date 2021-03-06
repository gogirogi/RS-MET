﻿#ifndef RS_LINEARALGEBRA_INL
#define RS_LINEARALGEBRA_INL

namespace RSLib
{

  template<class T>
  void rsSolveLinearSystem2x2(const T A[2][2], T x[2], const T y[2])
  {
    T s  = T(1) / ( A[0][0]*A[1][1] - A[0][1]*A[1][0]);
    x[0] = s * (A[1][1]*y[0] - A[0][1]*y[1]);
    x[1] = s * (A[0][0]*y[1] - A[1][0]*y[0]);
  }

  template<class T>
  void rsSolveLinearSystem3x3(const T A[3][3], T x[3], const T y[3])
  {
    T k1 = A[1][1]*A[2][0] - A[1][0]*A[2][1];
    T k2 = A[1][2]*A[2][1] - A[1][1]*A[2][2];
    T k3 = A[2][2]*y[1]    - A[1][2]*y[2];
    T k4 = A[1][0]*y[2]    - A[2][0]*y[1];
    T c  = T(1) / (A[0][0]*k2 + A[0][1]*(A[1][0]*A[2][2] - A[1][2]*A[2][0]) + A[0][2]*k1);
    x[0] = +c*(A[0][1]*k3 + A[0][2]*(A[1][1]*y[2] - A[2][1]*y[1]) + k2*y[0]);
    x[1] = -c*(A[0][0]*k3 + A[0][2]*k4 + (A[1][2]*A[2][0] - A[1][0]*A[2][2])*y[0]);
    x[2] = +c*(A[0][0]*(A[2][1]*y[1] - A[1][1]*y[2]) + A[0][1]*k4 + k1*y[0]);

    /*
    // un-optimized maxima output - measure performance against optimized version above:
    T s = T(1) / (A[0][0]*(A[1][2]*A[2][1]-A[1][1]*A[2][2])+A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])+A[0][2]*(A[1][1]*A[2][0]-A[1][0]*A[2][1]));
    x[0] = s*(A[0][1]*(A[2][2]*y[1]-A[1][2]*y[2])+A[0][2]*(A[1][1]*y[2]-A[2][1]*y[1])+(A[1][2]*A[2][1]-A[1][1]*A[2][2])*y[0]);
    x[1] =-s*(A[0][0]*(A[2][2]*y[1]-A[1][2]*y[2])+A[0][2]*(A[1][0]*y[2]-A[2][0]*y[1])+(A[1][2]*A[2][0]-A[1][0]*A[2][2])*y[0]);
    x[2] = s*(A[0][0]*(A[2][1]*y[1]-A[1][1]*y[2])+A[0][1]*(A[1][0]*y[2]-A[2][0]*y[1])+(A[1][1]*A[2][0]-A[1][0]*A[2][1])*y[0]);
    */
  }

  template<class T>
  bool rsSolveLinearSystemInPlace(T **A, T *x, T *b, int N)
  {
    bool   matrixIsSingular = false;
    int    i, j, k, p;
    double biggest;
    T      multiplier;
    T      tmpSum;

    // outermost loop over the rows to be scaled and subtracted from the rows below them:
    for(i = 0; i < N; i++)
    {

      // search for largest pivot in the i-th column from the i-th row downward:
      p       = i;
      biggest = 0.0;
      for(j = i; j < N; j++)
      {
        if( rsAbs(A[j][i]) > biggest )
        {
          biggest = rsAbs(A[j][i]);
          p       = j;
        }
      }
      if( rsIsCloseTo(biggest, 0.0, 1.e-12) )
      {
        matrixIsSingular = true;
        rsError("Matrix close to singular.");
      }

      // swap current row with pivot row (a pointer switch in the first index of A and an exchange 
      // of two values in b):
      if( p != i )
      {
        rsSwap(A[i], A[p]);
        rsSwap(b[i], b[p]);
        p = i;
      }

      // subtract a scaled version of the pivot-row p (==i) from all rows below it (j=i+1...N-1),
      // the multiplier being the i-th column element of the current row divided by the i-th column
      // element of the pivot-row - but we do the subtraction only for those columns which were not
      // already zeroed out by a previous iteration, that is: only from the i-th column rightward
      // (k = i...N-1):
      for(j = i+1; j < N; j++)
      {
        multiplier = A[j][i] / A[p][i];
        b[j] -= multiplier * b[p];
        for(k = i; k < N; k++)
          A[j][k] -= multiplier * A[p][k];
      }
    }

    // matrix A is now in upper triangular form - solve for x[0...N-1] by backsubstitution
    // (maybe factor out):
    x[N-1] = b[N-1] / A[N-1][N-1];
    for(i = N-2; i >= 0; i--)
    {
      // multiply the already obtained x-values by their coefficients from the current row and
      // accumulate them:
      tmpSum = 0.0;
      for(j = i+1; j < N; j++)
        tmpSum += A[i][j] * x[j];

      // this accumulated sum is to be subtracted from the target value, and the result of that
      // subtraction must be divided diagonal-element which corresponds to the index of the new
      // x-value which is to be computed:
      x[i] = (b[i] - tmpSum) / A[i][i];
    }

    // done: the vector x now contains the solution to the system A*x=b (unless the matrix was
    // singular in which case it contains meaningless numbers or not-a-numbers)

    return !matrixIsSingular;
  }

  template<class T>
  //bool rsSolveLinearSystem(const T **A, T *x, const T *b, int N)
  bool rsSolveLinearSystem(T **A, T *x, T *b, int N)
  {
    int i, j;

    // allocate memory for temporary copies of the coefficient matrix A and target vector b:
    T*  tmpB  = new T[N];
    T*  tmpA  = new T[N*N];
    T** tmpAP = new T*[N];

    // assign the pointer array for the matrix (tmpAP) to to the beginnings of the rows:
    for(i = 0; i < N; i++)
      tmpAP[i] = &(tmpA[i*N]);

    // copy the data into the temporary arrays:
    for(i = 0; i < N; i++)
    {
      tmpB[i] = b[i];
      for(j = 0; j < N; j++)
        tmpAP[i][j] = A[i][j];
    }

    // solve the linear system in place with the temporary arrays:
    bool success = rsSolveLinearSystemInPlace(tmpAP, x, tmpB, N);

    // free allocated memory:
    delete[] tmpB;
    delete[] tmpA;
    delete[] tmpAP;

    return success;
  }

  template<class T>
  bool rsInvertMatrix(T **A, int N)
  {
    bool   matrixIsSingular = false;

    int i, j, k, p;
    double biggest;
    T      multiplier;

    T*  tmpA  = new T[N*N];
    T** tmpAP = new T*[N];

    // assign the pointer array for the matrix (tmpAP) to to the beginnings of the rows:
    for(i = 0; i < N; i++)
      tmpAP[i] = &(tmpA[i*N]);

    // copy the data from A into the temporary matrix and initialize the matrix A with the unit
    // matrix:
    for(i = 0; i < N; i++)
    {
      for(j = 0; j < N; j++)
      {
        tmpAP[i][j] = A[i][j];
        A[i][j]     = 0.0;
      }
      A[i][i] = 1.0;
    }

    // outermost loop over the rows to be scaled and subtracted from the rows below them:
    for(i = 0; i < N; i++)
    {

      // search for largest pivot in the i-th column from the i-th row downward:
      p       = i;
      biggest = 0.0;
      for(j = i; j < N; j++)
      {
        if( fabs(tmpAP[j][i]) > biggest )
        {
          biggest = fabs(tmpAP[j][i]);
          p       = j;
        }
      }
      if( rsIsCloseTo(biggest, 0.0, 1.e-12) )
      {
        matrixIsSingular = true;
        rsError("Matrix close to singular.");
      }

      // swap the current row with pivot row (a pointer switch in the first index of A and ...?):
      if( p != i )
      {
        rsSwap(tmpAP[i], tmpAP[p]);
        rsSwap(A[i],     A[p]);
        p = i;
      }

      // divide the pivot-row row by the pivot element to get a 1 on the diagonal:
      multiplier = 1.0 / tmpAP[i][i];
      for(k = 0; k < N; k++)
      {
        tmpAP[p][k] *= multiplier;
        A[p][k]     *= multiplier;
      }

      // subtract a properly scaled version of the pivot-row from all other rows to get zeros in 
      // this column (this is different from Gaussian eleimination where it is subtracted only 
      // from the rows below):
      for(j = 0; j < N; j++)
      {
        multiplier = tmpAP[j][i];
        if( j != i )
        {
          for(k = 0; k < N; k++)
          {
            tmpAP[j][k] -= multiplier * tmpAP[p][k];
            A[j][k]     -= multiplier * A[p][k];
          }
        }
      }

    }

    // free temporarily allocated memory:
    delete[] tmpA;
    delete[] tmpAP;

    return !matrixIsSingular;
  }

  template<class T>
  bool rsSolveTridiagonalSystem(T *lower, T *main, T *upper, T *rhs, T *solution, int N)
  {
    if( main[0] == 0.0 )
    {
      rsError("Division by zero.");
      return false;
    }

    double divisor = main[0];
    double *tmp    = new double[N];
    solution[0]    = rhs[0] / divisor;
    for(int n = 1; n < N; n++)
    {
      tmp[n]  = upper[n-1] / divisor;
      divisor = main[n] - lower[n-1]*tmp[n];
      if( divisor == 0.0 )
      {
        rsError("Division by zero.");
        delete[] tmp;
        return false;
      }
      solution[n] = (rhs[n]-lower[n-1]*solution[n-1]) / divisor;
    }
    for(int n = N-2; n >= 0; n--)
      solution[n] -= tmp[n+1] * solution[n+1];

    delete[] tmp;
    return true;
  }

  template<class T>
  bool rsChangeOfBasisColumnWise(T **A, T **B, T *va, T *vb, int N)
  {
    // coordinates of v in canonical basis:
    T *ve = new T[N]; 
    rsMatrixVectorMultiply(A, va, ve, N, N); 

    // coordinates of v in basis B: A * va = ve = B * vb
    bool result = rsSolveLinearSystem(B, vb, ve, N);

    delete[] ve;
    return result;
  }

  template<class T>
  bool rsChangeOfBasisRowWise(T **A, T **B, T *va, T *vb, int N)
  {
    T *ve = new T[N]; 
    rsTransposedMatrixVectorMultiply(A, va, ve, N, N); 
    rsTransposeSquareArray(B, N); 
    bool result = rsSolveLinearSystem(B, vb, ve, N);
    rsTransposeSquareArray(B, N);
    delete[] ve;
    return result;
  }

  template<class T>
  bool rsChangeOfBasisMatrixColumnWise(T **A, T **B, T **C, int N)
  {
    T **Bi;
    rsAllocateMatrix(Bi, N, N);
    rsCopyMatrix(B, Bi, N, N);
    bool result = rsInvertMatrix(Bi, N);  // Bi = B^-1
    rsMatrixMultiply(Bi, A, C, N, N, N);  // C  = B^-1 * A
    rsDeAllocateMatrix(Bi, N, N);
    return result;
  }

  template<class T>
  bool rsChangeOfBasisMatrixRowWise(T **A, T **B, T **C, int N)
  {
    T **Bi;
    rsAllocateMatrix(Bi, N, N);
    rsTransposeSquareArray(B, Bi, N);
    bool result = rsInvertMatrix(Bi, N);                 // Bi = B^-T
    rsMatrixMultiplySecondTransposed(Bi, A, C, N, N, N); // C  = B^-T * A^T
    rsDeAllocateMatrix(Bi, N, N);
    return result;

    // alternative algorithm:
    // rsTransposeSquareArray(A, N);
    // rsTransposeSquareArray(B, N);
    // bool result = rsChangeOfBasisMatrixColumnWise(A, B, C, N);
    // rsTransposeSquareArray(A, N);
    // rsTransposeSquareArray(B, N);
    // return result;
  }

}

#endif
