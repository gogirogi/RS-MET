#ifndef RAPT_RAYBOUNCER_H_INCLUDED
#define RAPT_RAYBOUNCER_H_INCLUDED


/** A sound generator based on the idea of a particle/ray that bounces around inside an elliptic 
enclosure. 
maybe rename this to RayBouncerCore
*/

template<class T>
class rsRayBouncer
{

public:

  /** \name Construction/Destruction */

  /** Constructor.  */
  rsRayBouncer();


  /** \name Setup */

  /** Sets the initial position of the particle. */
  inline void setInitialPosition(T x, T y) { x0 = x; y0 = y; }

  /** Sets the angle at which our particle is launched from its initial position (in radians). */
  inline void setLaunchAngle(T newAngle) { angle = newAngle; }

  /** Sets the speed by which our particle moves around, i.e. the magnitude of the velocity, 
  which is a vector. */
  inline void setSpeed(T newSpeed) { speed = newSpeed; }
  // later replace this with setFrequency, setSampleRate...the speed is then proportional to
  // frequency/sampleRate - but maybe move this to some outside "driver" class
  // setting the speed variable is not enough - we also need to re-normalize the dx,dy

  /** Sets the parameters of the enclosing ellipse. Note that we do not include a shear 
  transformation because that would be redundant, because a sheared ellipse is still an ellipse
  (affine transformations map conic sections into conic sections of the same type) */
  inline void setEllipseParameters(T newScale = 1, T newAspectRatio = 1, T newAngle = 0,
    T newCenterX = 0, T newCenterY = 0)
  {
    ellipse.setParameters(newScale, newAspectRatio, newAngle, newCenterX, newCenterY);
  }


  /** \name Processing */

  /** Computes one x,y-pair of output values at a time. */
  void getSampleFrame(T &x, T &y);
  // use pointers for output variables

  /** Resets the internal state (position and velocity). */
  void reset();


protected:

  /** \name Internal Functions */

  /** Computes the intersection point between the current line segment along 
  xc + t*dx, yc + t*dy and the ellipse. */
  inline void getLineEllipseIntersectionPoint(T* xi, T* yi);

  /** Given a point xt,yt assumed to be on the ellipse, this function reflects the point
  x,y about the tangent at that point. */
  inline void reflectInTangentAt(T xt, T yt, T* x, T *y);

  /** Updates dx, dy by taking the direction vector that points from the given line/ellipse 
  intersection point xi,yi to the current point as new direction and adjusting its length according 
  to the desired speed. */
  inline void updateVelocity(T xi, T yi);


  /** \name Data */

  rsEllipse<T> ellipse; // enclosing ellipse

  // particle state:
  T x0 = 0, y0 = 0;     // initial position of particle
  T x , y, dx, dy;      // current position and velocity (as increment per sample):

  // user parameters: 
  T speed = T(0.2);     // speed (i.e. magnitude of velocity)
  T angle = T(0.0);     // launching angle

};

//=================================================================================================

/** A class that encapsulates an rsRayBouncer object to let the user set it up in terms of musical
parameters (such as a pseudo frequency, etc.) and also some modulators that animate certain
parameters of the bouncer such as the parameters of the ellipse, an output transformation, etc. */

template<class T>
class rsRayBouncerDriver
{

public:

  /** \name Setup */

  /** Sets the frequency and sample rate in Hz */
  void setFrequencyAndSampleRate(T newFreq, T newRate);


  /** \name Processing */

  /** Computes one x,y-pair of output values at a time. */
  inline void getSampleFrame(T &x, T &y)
  {
    rayBouncer.getSampleFrame(x, y);
  }

  /** Resets the internal state (position and velocity). */
  void reset();


protected:

  rsRayBouncer<T> rayBouncer;

  // rsSineOscillator lfoSize, lfoRatio, lfoX, lfoY;
  // rsNaiveSawOscillator lfoAngle;
  // maybe, instead of letting the center just x,y bob up and down, move the ellipse center in a 
  // circle or ellipse as well...or maybe do both...lots of movement in the ellipse should lead
  // to complex animated shape and sound
  // maybe when the ellipse size is animated, we should compensate for any frequency change that
  // is caused by this - or maybe compensate only partially according to a user parameter

};

#endif