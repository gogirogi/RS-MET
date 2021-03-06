#ifndef RS_STATISTICS_INL
#define RS_STATISTICS_INL

namespace RSLib
{

  template<class T>
  void rsCrossCorrelationDirect(T x[], T y[], int N, T r[])
  {
    for(int k = 0; k < N; k++)
    {
      r[k] = 0;
      for(int n = 0; n < N-k; n++)
        r[k] += x[n+k] * y[n]; // for complex sequences, y must be conjugated?
    }
  }

  template<class T>
  void rsCrossCorrelationFFT(T x[], T y[], int N, T r[])
  {
    // create zero-padded sequences such that no foldover occurs (use a power of 2 for the FFT) and
    // obtain the two FFT spectra:
    int Np = 2*rsNextPowerOfTwo(N);
    rsComplex<T> *X = new rsComplex<T>[Np];
    rsComplex<T> *Y = new rsComplex<T>[Np];
    rsConvertBuffer(x, X, N);
    rsConvertBuffer(y, Y, N);
    rsFFT(X, Np);
    rsFFT(Y, Np);

    // compute cross power spectrum, store it in X (see text below Eq.12.0.11 in: Numerical
    // Recipies in C, 2nd Ed.):
    int n;
    X[0] = X[0] * Y[0];
    for(n = 1; n < Np; n++)
      X[n] = X[n] * Y[Np-n];

    // obtain cross correlation by IFFT, copy to output and cleanup:
    rsIFFT(X, Np);
    for(n = 0; n < N; n++)
      r[n] = X[n].re;
    delete[] X;
    delete[] Y;
  }

  template<class T>
  void rsCrossCorrelation(T x[], T y[], int N, T r[], bool removeBias = false)
  {
    if( N < 64 ) // ad hoc - measure at which point FFT becomes more efficient
      rsCrossCorrelationDirect(x, y, N, r);
    else
      rsCrossCorrelationFFT(x, y, N, r);

    if(removeBias == true)
      rsRemoveCorrelationBias(r, N, r);
  }

  template<class T>
  void rsAutoCorrelationFFT(T x[], int N, T r[])
  {
    int Np = 2*rsNextPowerOfTwo(N);
    rsComplex<T> *X = new rsComplex<T>[Np];
    rsConvertBuffer(x, X, N);
    rsFFT(X, Np);
    int n;
    X[0] = X[0] * X[0];
    for(n = 1; n < Np; n++)
      X[n] = X[n] * X[n].getConjugate();
    // We can't use X[n]*X[Np-n] because the X-array is getting messed inside the loop itself, so
    // we use Eq.12.0.11 as is.
    rsIFFT(X, Np);
    for(n = 0; n < N; n++)
      r[n] = X[n].re;
    delete[] X;
  }

  template<class T>
  void rsRemoveCorrelationBias(T x[], int N, T r[])
  {
    for(int k = 0; k < N; k++)
      r[k] = N*x[k]/(N-k);
  }

  template<class T>
  T rsCrossCorrelation(T *x, int Nx, T *y, int Ny)
  {
    T xx = rsSumOfSquares(x, Nx);
    T yy = rsSumOfSquares(y, Ny);
    T xy = rsSumOfProducts(x, y, rsMin(Nx, Ny));
    if(xx == 0 || yy == 0)
      return 0;
    return xy / sqrt(xx*yy);
    // Assuming that we would divide each sum by the same value, namely max(Nx,Ny), for computing 
    // a mean, the divisor cancels out in the cross-correlation formula
  }

  template<class T>
  T rsStretchedCrossCorrelation(T *x, int Nx, T *y, int Ny)
  {
    // x should be the longer than y:
    if(Nx < Ny)
      return rsStretchedCrossCorrelation(y, Ny, x, Nx); // recursive call with swapped arguments
    if(Nx == Ny)
      return rsCrossCorrelation(x, Nx, y, Ny);          // function for same-length arrays

    double a = double(Ny) / double(Nx);  // readout speed for y-array
    int    nx;                           // index into the x-array
    double xn;                           // value of x at nx
    T yn;                                // (interpolated) value of y at a*nx
    T xx = 0;                            // sum of squares of x-values   
    T yy = 0;                            // sum of squares of (interpolated) y-values
    T xy = 0;                            // sum of x*y products
    for(nx = 0; nx < Nx; nx++)
    {
      xn  = x[nx];
      yn  = rsInterpolatedValueAt(y, Ny, a*nx);
      xx += xn*xn;
      yy += yn*yn;
      xy += xn*yn;
    }
    if(xx == 0 || yy == 0)
      return 0;
    return xy / sqrt(xx*yy);
  }


  // \todo explicit template instantiations for float and double
  //....

}

#endif
