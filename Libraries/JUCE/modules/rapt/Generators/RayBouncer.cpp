template<class T>
rsRayBouncer<T>::rsRayBouncer()
{
  reset();
}

// processing:

template<class T>
void rsRayBouncer<T>::getLineEllipseIntersectionPoint(T* xi, T* yi)
{
  // we use xi,yi temporarily for the two solutions for the intersection parameter ti, yi is the
  // relevant solution (the greater value):
  ellipse.lineIntersectionParameter(x, dx, y, dy, xi, yi); // xi <- ti1, yi <- ti2
  *xi = x + *yi * dx;  // yi is still ti2
  *yi = y + *yi * dy;  // ...now not anymore
}

template<class T>
void rsRayBouncer<T>::reflectInTangentAt(T xt, T yt, T* x, T *y)
{
  T A, B, C;
  ellipse.getTangentCoeffs(xt, yt, &A, &B, &C);
  rsLine2D<T>::reflectPointInLine(*x, *y, A, B, C, x, y);
}

template<class T>
void rsRayBouncer<T>::ensurePointIsInEllipse(T* x, T* y)
{
  if(ellipse.isPointOutside(*x, *y, tolerance))
  {
    *x = x0; 
    *y = y0;
  }
}

template<class T>
void rsRayBouncer<T>::updateVelocity(T xi, T yi)
{
  dx = x-xi;
  dy = y-yi;
  T scaler = speed / sqrt(dx*dx + dy*dy);
  dx *= scaler;
  dy *= scaler;
}

template<class T>
void rsRayBouncer<T>::getSampleFrame(T &xOut, T &yOut)
{
  // assign outputs:
  xOut = x;
  yOut = y;

  // Compute new (tentative) x,y coordinates:
  x += dx;
  y += dy;

  // Reflect, if new coordinates are outside elliptic enclosure:
  //T tol = T(1.e-8); // to avoid div-by-almost-zero in velocity update
  //T tol = 0; // for debug - maybe that's not such a good idea with the tolerance
  //while(ellipse.isPointOutside(x+tol*dx, y+tol*dy))
  //if(ellipse.isPointOutside(x-tol, y-tol))
  //while(ellipse.isPointOutside(x, y, tol))
  if(ellipse.isPointOutside(x, y, tolerance))
  {
    T xi, yi;
    getLineEllipseIntersectionPoint(&xi, &yi); // intersection between line segment and ellipse

    T err = ellipse.evaluate(xi, yi);        // for debug - should be 0 up to roundoff
    T val = ellipse.evaluate(x, y);            // also for debug

    reflectInTangentAt(xi, yi, &x, &y);        // reflect new point in tangent at intersection

    ensurePointIsInEllipse(&x, &y);            // because sometimes, we fail numerically

    updateVelocity(xi, yi);                    // points from intersection to reflected point now
  }
  // in certain conditions we get hung up in this loop - may use an "if" or a maximum number
  // of iterations....i guess, it happens when the new point ends up exactly on the ellipse
  // in this case, the reflected point equals the original point and it never gets from the outside
  // to the inside...we need some special treatment for such cases...write unit tests....
  // ..it happens when we get a negative discriminant in the quadratic equation that has to be 
  // solved in getLineEllipseIntersectionPoint - this means that the line in question does not
  // really intersect the ellipse - perhaps the ellipse has moved while the particle was moving
  // out

  // apply nonlinear velocity modifications:
  xyToX = xyToY = 1;
  //nonLinAmount = 0.1;

  T xx = dx*dx;
  T xy = dx*dy;
  T yy = dy*dy;
  dx += nonLinAmount * (xxToX * xx + xyToX * xy + yyToX * yy);
  dy += nonLinAmount * (xxToY * xx + xyToY * xy + yyToY * yy);
  // we need some experimenting - maybe it's sufficient to use xy alone to save computational power
  // ...see if the other terms add something new qualitatively, if not, maybe leave them out

  // experimental: mess with the velocity vector:
  //T k = -1.0;  // coefficient for nonlinear velocity modification
  //T tx, ty;
  //tx = x*(1-x*y);   // bad
  //ty = y*(1-x*y);

  //tx = dy*(dx+dy);  // good
  //ty = dx*(dx+dy);

  //tx = dy*dy;      // also good
  //ty = dx*dx;

  //tx = dy*dy * rsSign(dx);      // also good
  //ty = dx*dx * rsSign(dy);

  //tx = dy;  // works only with extremely small coeff like 0.0001 - but then it tends to drag
  //ty = dx;  // the sound towards periodic/harmonic waveform

  //tx = dx*dx;      // also good
  //ty = dy*dy;

  //tx = ty = dx*dy;   // also good

  // i really think, i should provide user parameters xxToX, xyToX, xyToY, yyToY that scale the 
  // amounts by which various velocity-products are added to the respective velocity components

  // maybe a highpass filter could be interesting. it would disable a constant velocity vector so
  // it would always want to bend away

  ////tx = (1-x*y); tx *= tx; tx *= tx;
  ////ty = tx;
  //dx += k*tx;
  //dy += k*ty;

  //T scaler = speed / sqrt(dx*dx + dy*dy);
  //dx *= scaler;
  //dy *= scaler;
  // renormalization seems not to be critical - maybe leave it out for efficiency because it's
  // expensive
}

template<class T>
void rsRayBouncer<T>::reset()
{
  x  = x0; 
  y  = y0;
  dx = speed * cos(angle);
  dy = speed * sin(angle);
}

//-------------------------------------------------------------------------------------------------

template<class T>
void rsRayBouncerDriver<T>::setFrequencyAndSampleRate(T newFreq, T newRate)
{
  T k = T(1); // proportionality constant - preliminary figure out...
  T speed = k * newFreq / newRate;
  rayBouncer.setSpeed(speed);
}

template<class T>
void rsRayBouncerDriver<T>::setStartX(T newX)
{
  startX = newX;
  rayBouncer.setInitialPosition(startX, startY);
}

template<class T>
void rsRayBouncerDriver<T>::setStartY(T newY)
{
  startY = newY;
  rayBouncer.setInitialPosition(startX, startY);
}

//template<class T>
//void rsRayBouncerDriver<T>::setEllipseSize(T newSize)
//{
//
//}
//
//template<class T>
//void rsRayBouncerDriver<T>::setEllipseAspectRatio(T newRatio)
//{
//
//}
//
//template<class T>
//void rsRayBouncerDriver<T>::setEllipseAngle(T newAngle)
//{
//
//}

template<class T>
void rsRayBouncerDriver<T>::reset()
{
  rayBouncer.reset();
  // todo: reset modulators, when we have some...
}