/*
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 * This code is in the public domain.
 */

// #define TESTMODE

/*  fit_cubic.c	*/									
/*	Piecewise cubic fitting code	*/

#include <toad/types.hh>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace toad;

/* returns squared length of input vector */	
static inline double squaredLength(const TPoint &a) 
{
  return a.x*a.x+a.y*a.y;
}

/* returns length of input vector */
static inline double length(const TPoint &a) 
{
  return sqrt(squaredLength(a));
}

/* return the distance between two points */
static inline double distance(const TPoint &a, const TPoint &b)
{
  return length(a-b);
}
	
/* normalizes the input vector and returns it */
static inline TPoint normalize(const TPoint &v) 
{
  TPoint result;
  double len = length(v);
  if (len != 0.0) {
    result.x = v.x / len;
    result.y = v.y / len;
  } else {
    result.x = v.x;
    result.y = v.y;
  }
  return(result);
}

/* return the dot product of vectors a and b */
static inline double
dot(const CGPoint &a, const CGPoint &b)
{
  return a.x*b.x+a.y*b.y;
}

/*
 *  B0, B1, B2, B3 :
 *	Bezier multipliers
 */
static inline double
B0(double u)
{
  double tmp = 1.0 - u;
  return (tmp * tmp * tmp);
}

static inline double
B1(double u)
{
  double tmp = 1.0 - u;
  return (3 * u * (tmp * tmp));
}

static inline double
B2(double u)
{
  double tmp = 1.0 - u;
  return (3 * u * u * tmp);
}

static inline double
B3(double u)
{
  return (u * u * u);
}

/*
 *  GenerateBezier :
 *  Use least-squares method to find Bezier control points for region.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 *  \param uPrime Parameter values for region
 *  \param tHat1, tHat2 Unit tangents at endpoints
 *
 */
static void
generateBezier(const TPoint *d, int first, int last, const double *uPrime, TPoint tHat1, TPoint tHat2, TPoint *bezCurve)
{
  int 	i;
  int 	nPts;			/* Number of pts in sub-curve 	*/
  double 	C[2][2];		/* Matrix C			*/
  double 	X[2];			/* Matrix X			*/
  double 	det_C0_C1,		/* Determinants of matrices	*/
    	   	det_C0_X,
	   	det_X_C1;
  double 	alpha_l,		/* Alpha values, left and right	*/
    	   	alpha_r;
  TPoint 	tmp;			/* Utility variable		*/
  nPts = last - first + 1;
  CGPoint      A[nPts][2];		/* Precomputed rhs for eqn      */
 
  /* Compute the A's	*/
  for (i = 0; i < nPts; i++) {
    A[i][0] = tHat1 * B1(uPrime[i]);
    A[i][1] = tHat2 * B2(uPrime[i]);
  }

  /* Create the C and X matrices */
  C[0][0] = 0.0;
  C[0][1] = 0.0;
  C[1][0] = 0.0;
  C[1][1] = 0.0;
  X[0]    = 0.0;
  X[1]    = 0.0;

  for (i = 0; i < nPts; i++) {
    C[0][0] += dot(A[i][0], A[i][0]);
    C[0][1] += dot(A[i][0], A[i][1]);
    // C[1][0] += dot(A[i][0], A[i][1]);
    C[1][0] = C[0][1];
    C[1][1] += dot(A[i][1], A[i][1]);
    tmp = d[first + i]
          - d[first]* B0(uPrime[i])
          - d[first]* B1(uPrime[i])
          - d[last] * B2(uPrime[i])
          - d[last] * B3(uPrime[i]);
    X[0] += dot(A[i][0], tmp);
    X[1] += dot(A[i][1], tmp);
  }

  /* Compute the determinants of C and X	*/
  det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
  det_C0_X  = C[0][0] * X[1]    - C[1][0] * X[0];
  det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];

  /* Finally, derive alpha values	*/
  alpha_l = (det_C0_C1 == 0) ? 0.0 : det_X_C1 / det_C0_C1;
  alpha_r = (det_C0_C1 == 0) ? 0.0 : det_C0_X / det_C0_C1;

  /* If alpha negative, use the Wu/Barsky heuristic (see text) */
  /* (if alpha is 0, you get coincident control points that lead to
   * divide by zero in any subsequent NewtonRaphsonRootFind() call. */
  double segLength = distance(d[last], d[first]);
  double epsilon = 1.0e-6 * segLength;
  if (alpha_l < epsilon || alpha_r < epsilon) {
    /* fall back on standard (probably inaccurate) formula, and subdivide further if needed. */
    double dist = segLength / 3.0;
    bezCurve[0] = d[first];
    bezCurve[3] = d[last];
    bezCurve[1] = bezCurve[0] + tHat1 * dist;
    bezCurve[2] = bezCurve[3] + tHat2 * dist;
    return;
  }

  /*  First and last control points of the Bezier curve are */
  /*  positioned exactly at the first and last data points */
  /*  Control points 1 and 2 are positioned an alpha distance out */
  /*  on the tangent vectors, left and right, respectively */
  bezCurve[0] = d[first];
  bezCurve[3] = d[last];
  bezCurve[1] = bezCurve[0] + tHat1 * alpha_l;
  bezCurve[2] = bezCurve[3] + tHat2 * alpha_r;
}

/*
 *  Bezier :
 *  	Evaluate a Bezier curve at a particular parameter value
 *    \param degree The degree of the bezier curve
 *    \param V      Array of control points
 *    \param t      Parametric value to find point for
 */
static TPoint
evaluateBezier(int degree, const TPoint *V, double t)
{
  int i, j;		

  /* Copy array	*/
  CGPoint Vtemp[degree+1];     	/* Local copy of control points         */
  for (i = 0; i <= degree; i++) {
    Vtemp[i] = V[i];
  }

  /* Triangle computation	*/
  for (i = 1; i <= degree; i++) {	
    for (j = 0; j <= degree-i; j++) {
      Vtemp[j].x = (1.0 - t) * Vtemp[j].x + t * Vtemp[j+1].x;
      Vtemp[j].y = (1.0 - t) * Vtemp[j].y + t * Vtemp[j+1].y;
    }
  }

  return TPoint(Vtemp->x, Vtemp->y); // Point on curve at parameter t
}

/*
 *  NewtonRaphsonRootFind :
 *	Use Newton-Raphson iteration to find better root.
 *  \param Q Current fitted curve
 *  \param P Digitized point
 *  \param u Parameter value for "P"
 */
static double
newtonRaphsonRootFind(const TPoint* Q, TPoint P, double u)
{
  double numerator, denominator;
  TPoint Q1[3], Q2[2];    // Q' and Q''
  TPoint Q_u, Q1_u, Q2_u; // u evaluated at Q, Q', & Q''
  int i;
    
  /* Compute Q(u)	*/
  Q_u = evaluateBezier(3, Q, u);
    
  /* Generate control vertices for Q'	*/
  for (i = 0; i <= 2; i++) {
    Q1[i].x = (Q[i+1].x - Q[i].x) * 3.0;
    Q1[i].y = (Q[i+1].y - Q[i].y) * 3.0;
  }
    
  /* Generate control vertices for Q'' */
  for (i = 0; i <= 1; i++) {
    Q2[i].x = (Q1[i+1].x - Q1[i].x) * 2.0;
    Q2[i].y = (Q1[i+1].y - Q1[i].y) * 2.0;
  }
    
  /* Compute Q'(u) and Q''(u)	*/
  Q1_u = evaluateBezier(2, Q1, u);
  Q2_u = evaluateBezier(1, Q2, u);
    
  /* Compute f(u)/f'(u) */
  numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
  denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
		      	  (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
  if (denominator == 0.0f) return u;

  /* u = u - f(u)/f'(u) */
  return u - (numerator/denominator);
}

/*
 *  Reparameterize:
 *   Given set of points and their parameterization, try to find
 *   a better parameterization.
 *   \param d Array of digitized points
 *   \param first, last Indices defining region
 *   \param[inout] u Current parameter values
 *   \param bezCurve Current fitted curve
 */
static void
reparameterize(const TPoint *d, int first, int last, double *u, const TPoint* bezCurve)
{
  for (int i = first; i <= last; i++) {
    u[i-first] = newtonRaphsonRootFind(bezCurve, d[i], u[i-first]);
  }
}

/*
 * ComputeLeftTangent, ComputeRightTangent, ComputeCenterTangent :
 * Approximate unit tangents at endpoints and "center" of digitized curve
 * \param d Digitized points
 * \param end Index to "left" end of region
 */
static inline TPoint
computeLeftTangent(const TPoint *d, int end)
{
  return normalize(d[end+1] - d[end]);
}

/**
 * \param d Digitized points
 * \param end Index to "right" end of region
 */
static inline TPoint
computeRightTangent(const TPoint *d, int end)
{
  return normalize(d[end-1] - d[end]);
}

/**
 * \param d Digitized points
 * \param center Index to point inside region
 */
static TPoint
computeCenterTangent(const TPoint *d, int center)
{
  TPoint V1, V2, tHatCenter;

  V1 = d[center-1] - d[center];
  V2 = d[center] - d[center+1];
  tHatCenter.x = (V1.x + V2.x)/2.0;
  tHatCenter.y = (V1.y + V2.y)/2.0;
  return normalize(tHatCenter);
}


/*
 *  ChordLengthParameterize :
 *	Assign parameter values to digitized points 
 *	using relative distances between points.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 */
static void
chordLengthParameterize(const TPoint *d, int first, int last, double *u)
{
  int i;	

  u[0] = 0.0;
  for (i = first+1; i <= last; i++) {
    u[i-first] = u[i-first-1] +	distance(d[i], d[i-1]);
  }

  for (i = first + 1; i <= last; i++) {
    u[i-first] = u[i-first] / u[last-first];
  }
}

/*
 *  ComputeMaxError :
 *	Find the maximum squared distance of digitized points
 *	to fitted curve.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 *  \param bezCurve Fitted Bezier curve
 *  \param u Parameterization of points
 *  \param splitPoint Point of maximum error
 */
static double
computeMaxError(const TPoint *d, int first, int last, const TPoint* bezCurve, const double *u, int *splitPoint)
{
  int		i;
  double	maxDist;	/*  Maximum error		*/
  double	dist;		/*  Current error		*/
  TPoint	P;		/*  Point on curve		*/
  TPoint	v;		/*  Vector from point to curve	*/

  *splitPoint = (last - first + 1)/2;
  maxDist = 0.0;
  for (i = first + 1; i < last; i++) {
    P = evaluateBezier(3, bezCurve, u[i-first]);
    v = P - d[i];
    dist = squaredLength(v);
    if (dist >= maxDist) {
      maxDist = dist;
      *splitPoint = i;
    }
  }
  return maxDist;
}

extern void DrawBezierCurve(int n, const TPoint *curve);

/*
 *  FitCubic :
 *  	Fit a Bezier curve to a (sub)set of digitized points
 *  \param d Array of digitized points
 *  \param first, last Indices of first and last pts in region
 *  \param tHat1, tHat2 Unit tangent vectors at endpoints
 *  \param error User-defined error squared
 */
static void
fitCubic(const TPoint *d, int first, int last, TPoint tHat1, TPoint tHat2, double error)
{
  TPoint        curve[4];	/*Control points of fitted Bezier curve*/
  double	maxError;	/*  Maximum fitting error	 */
  int		splitPoint;	/*  Point to split point set at	 */
  int		nPts;		/*  Number of points in subset  */
  double	iterationError; /*Error below which you try iterating  */
  int		maxIterations = 4; /*  Max times to try iterating  */
  TPoint	tHatCenter;   	/* Unit tangent vector at splitPoint */
  int		i;		

  iterationError = error * error;
  nPts = last - first + 1;

  /*  Use heuristic if region only has two points in it */
  if (nPts == 2) {
    double dist = distance(d[last], d[first]) / 3.0;
    curve[0] = d[first];
    curve[3] = d[last];
    curve[1] = curve[0] + tHat1 * dist;
    curve[2] = curve[3] + tHat2 * dist;
    DrawBezierCurve(3, curve);
    return;
  }

  /*  Parameterize points, and attempt to fit curve */
  double u[last-first+1]; // parameter values for point
  chordLengthParameterize(d, first, last, u);
  generateBezier(d, first, last, u, tHat1, tHat2, curve);

  /*  Find max deviation of points to fitted curve */
  maxError = computeMaxError(d, first, last, curve, u, &splitPoint);
  if (maxError < error) {
    DrawBezierCurve(3, curve);
    return;
  }

  /*  If error not too large, try some reparameterization  */
  /*  and iteration */
  if (maxError < iterationError) {
    for (i = 0; i < maxIterations; i++) {
      reparameterize(d, first, last, u, curve);
      generateBezier(d, first, last, u, tHat1, tHat2, curve);
      maxError = computeMaxError(d, first, last, curve, u, &splitPoint);
      if (maxError < error) {
        DrawBezierCurve(3, curve);
        return;
      }
    }
  }

  /* Fitting failed -- split at max error point and fit recursively */
  tHatCenter = computeCenterTangent(d, splitPoint);
  fitCubic(d, first, splitPoint, tHat1, tHatCenter, error);
  fitCubic(d, splitPoint, last, -tHatCenter, tHat2, error);
}

/*
 *  FitCurve :
 *  	Fit a Bezier curve to a set of digitized points 
 *  \param d Array of digitized points
 *  \param nPts Number of digitized points
 *  \param error User-defined error squared
 */
void FitCurve(const TPoint *d, int nPts, double error)
{
  TPoint	tHat1, tHat2;	/*  Unit tangent vectors at endpoints */

  tHat1 = computeLeftTangent(d, 0);
  tHat2 = computeRightTangent(d, nPts - 1);
  fitCubic(d, 0, nPts - 1, tHat1, tHat2, error);
}
