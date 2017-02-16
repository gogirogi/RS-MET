#ifndef RAPT_IMAGEPAINTER_H_INCLUDED
#define RAPT_IMAGEPAINTER_H_INCLUDED

/** A class for painting on an Image object. It is based on an "alpha-mask" that is used as 
prototype "dot". Whenever a dot is painted onto the image at a particular location, the mask will
be used to blend the existing colors at these pixels with a new target color.  */

template<class TPix, class TWgt, class TCor>  // pixel, weight, coordinate types
class ImagePainter
{

public:

  /** \name Construction/Destruction */

  /** Constructor. */
  ImagePainter(Image<TPix> *imageToPaintOn = nullptr, AlphaMask<TWgt> *maskToUse = nullptr);


  /** \name Setup */

  /** Sets the image on which we paint.  */
  void setImageToPaintOn(Image<TPix> *imageToPaintOn);

  /** Sets the alpha mask that we use as prototye "dot". It is basically a matrix of weights. */
  void setAlphaMaskForDot(AlphaMask<TWgt> *maskToUse);

  /** Sets the weights that are used in the simple (non alpha mask based) dot drawing mode. */
  void setNeighbourWeightsForSimpleDot(TWgt straight, TWgt diagonal);

  /** Switches anti-aliasing on/off. */
  void setAntiAlias(bool shouldAntiAlias);

  /** Switches between using the alpha-mask and the simple dot algorithm. */
  void setUseAlphaMask(bool shouldUseMask);


  /** \name Inquiry */

  /** Returns a pointer to the target image onto which we paint. */
  Image<TPix>* getImage() { return image; }

  /** Returns a pointer to the alpha mask that we use for painting. */
  AlphaMask<TWgt>* getAlphaMask() { return mask; }


  /** \name Painting */

  /** Paints a dot at the given position. This function dispatches between the various versions of
  the dot painting (using alpha-mask or not, anti-alias or not) according to the settings of the 
  object. */
  void paintDot(TCor x, TCor y, TPix color);

  /** Function for painting a simple 3x3 dot at given integer position. */
  void paintDot3x3(int x, int y, TPix color, TWgt weightStraight = 0, TWgt weightDiagonal = 0);  

  /** Function for painting a simple 3x3 dot at given noninteger position. */
  void paintDot3x3(TCor x, TCor y, TPix color, TWgt weightStraight = 0, TWgt weightDiagonal = 0);

  /** Paints a dot at an integer position using out stored alpha mask (which represents a prototype 
  dot). */
  void paintDotViaMask(int x, int y, TPix color);

  /** Anti-aliased version of alpha mask dot painting.  */
  void paintDotViaMask(TCor x, TCor y, TPix color);

  /** Draws a line by inserting a number of dots along the line. The number is proportional to the 
  given density parameter and to the Euclidean distance between the two endpoints (i.e. the length 
  of the line). The color will be scaled inversely proportional to the length, such that the total
  amount of color added to the picture is independent of the length. The maxNumDots parameter
  is for restricting the number of dots that are used which maight be important in realtime 
  situations. scaleByNumDots ...
  \todo: maybe make this color scaling optional  */
  void drawDottedLine(TCor x1, TCor y1, TCor x2, TCor y2, TPix color, TCor density = 1, 
    int maxNumDots = 0, bool scaleByNumDots = false);
    


protected:

  /** Internal functions. */

  /** Accumulates the given value into the accumulator accu. We use a rather peculiar accumulation
  function here: newAccu = (oldAccu + value) / (1 + value). When accu starts out a zero and all 
  accumulated values are >= 0, this function will ensure that accu is always < 1 and it will go
  into saturation smoothly. */
  inline void accumulate(TPix &accu, TPix value)
  {
    //rsAssert(value <= TPix(1));
    //rsAssert(value >= TPix(0));

    accu = (accu + value) / (TPix(1) + value);

    //accu = accu+value; // just for testing
    //accu = rsMin(TPix(1), accu+value);
  }
  // rename to addAndSaturate

  /** Blends the pixel in the image at given coordinates with a new color according to some weight.
  If the weight is 0, the pixel's color is unchanged, if it's 1, the new color has the biggest 
  impact (\todo: what exactly happens should depend on a blend-mode setting). */
  inline void blend(int x, int y, TPix color, TWgt weight)
  {
    blend(x, y, TPix(weight) * color);
    //accumulate((*image)(x, y), TPix(weight) * color);
    // todo: use blend modes here
  }

  /** Same as 4-argument blend() with weight = 1. */
  inline void blend(int x, int y, TPix color)
  {
    accumulate((*image)(x, y), color);
  }
  // rename the blend() functions to plot()

  // data members:

  Image<TPix> *image;
  AlphaMask<TWgt> *mask;

  bool antiAlias, useMask;
  TWgt straightNeighbourWeight, diagonalNeighbourWeight;
};

#endif