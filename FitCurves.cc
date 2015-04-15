/*
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 * This code is in the public domain.
 */

// #define TESTMODE

/*  fit_cubic.c	*/									
/*	Piecewise cubic fitting code	*/

#include "GraphicsGems.h"					
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

using namespace toad;

/* Forward declarations */
void			FitCurve(TPoint *d, int nPts, double error);
static	void		FitCubic(TPoint *d, int first, int last, TPoint tHat1, TPoint tHat2, double error);
static	void		Reparameterize(TPoint *d, int first, int last, double *u, BezierCurve bezCurve);
static	double		NewtonRaphsonRootFind(BezierCurve Q, TPoint P, double u);
static	TPoint		BezierII(int degree, TPoint *V, double t);
static	double 		B0(double), B1(double), B2(double), B3(double);
static	TPoint		ComputeLeftTangent(TPoint *d, int end);
static	TPoint		ComputeRightTangent(TPoint *d, int end);
static	TPoint		ComputeCenterTangent(TPoint *d, int center);
static	double		ComputeMaxError(TPoint *d, int first, int last, BezierCurve bezCurve, double *u, int *splitPoint);
static	void		ChordLengthParameterize(TPoint *d, int first, int last, double *u);
static	void		GenerateBezier(TPoint *d, int first, int last, double *uPrime, TPoint tHat1, TPoint tHat2, TPoint *curve);
static	TPoint		V2AddII(TPoint a, TPoint b);
static	TPoint		V2ScaleIII(TPoint v, double s);
static	TPoint		V2SubII(TPoint a, TPoint b);

#define MAXPOINTS	1000		/* The most points you can have */

static double epsilon = 1e-12;

extern void DrawBezierCurve(int n, BezierCurve curve);


#ifdef TESTMODE


/*
 *  main:
 *	Example of how to use the curve-fitting code.  Given an array
 *   of points and a tolerance (squared error between points and 
 *	fitted curve), the algorithm will generate a piecewise
 *	cubic Bezier representation that approximates the points.
 *	When a cubic is generated, the routine "DrawBezierCurve"
 *	is called, which outputs the Bezier curve just created
 *	(arguments are the degree and the control points, respectively).
 *	Users will have to implement this function themselves 	
 *   ascii output, etc. 
 *
 */
main()
{
    static TPoint d[7] = {	/*  Digitized points */
	{ 0.0, 0.0 },
	{ 0.0, 0.5 },
	{ 1.1, 1.4 },
	{ 2.1, 1.6 },
	{ 3.2, 1.1 },
	{ 4.0, 0.2 },
	{ 4.0, 0.0 },
    };
    double	error = 4.0;		/*  Squared error */
    FitCurve(d, 7, error);		/*  Fit the Bezier curves */
}
#endif						 /* TESTMODE */

/*
 *  FitCurve :
 *  	Fit a Bezier curve to a set of digitized points 
 *  \param d Array of digitized points
 *  \param nPts Number of digitized points
 *  \param error User-defined error squared
 */
void FitCurve(TPoint *d, int nPts, double error)
{
    TPoint	tHat1, tHat2;	/*  Unit tangent vectors at endpoints */

    tHat1 = ComputeLeftTangent(d, 0);
    tHat2 = ComputeRightTangent(d, nPts - 1);
    FitCubic(d, 0, nPts - 1, tHat1, tHat2, error);
}

/*
 *  FitCubic :
 *  	Fit a Bezier curve to a (sub)set of digitized points
 *  \param d Array of digitized points
 *  \param first, last Indices of first and last pts in region
 *  \param tHat1, tHat2 Unit tangent vectors at endpoints
 *  \param error User-defined error squared
 */
static void FitCubic(TPoint *d, int first, int last, TPoint tHat1, TPoint tHat2, double error)
{
    TPoint      curve[4];	/*Control points of fitted Bezier curve*/
    double	*uPrime;	/*  Improved parameter values */
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
	double dist = V2DistanceBetween2Points(&d[last], &d[first]) / 3.0;
	TPoint curve[4];
	curve[0] = d[first];
	curve[3] = d[last];
	curve[1] = curve[0] + tHat1 * dist;
	curve[2] = curve[3] + tHat2 * dist;
	DrawBezierCurve(3, curve);
	return;
    }

    /*  Parameterize points, and attempt to fit curve */
    double u[last-first+1]; // parameter values for point
    ChordLengthParameterize(d, first, last, u);
    GenerateBezier(d, first, last, u, tHat1, tHat2, curve);

    /*  Find max deviation of points to fitted curve */
    maxError = ComputeMaxError(d, first, last, curve, u, &splitPoint);
    if (maxError < error) {
		DrawBezierCurve(3, curve);
		return;
    }

    /*  If error not too large, try some reparameterization  */
    /*  and iteration */
    if (maxError < iterationError) {
		for (i = 0; i < maxIterations; i++) {
	    	Reparameterize(d, first, last, u, curve);
	    	GenerateBezier(d, first, last, uPrime, tHat1, tHat2, curve);
	    	maxError = ComputeMaxError(d, first, last,
				       curve, uPrime, &splitPoint);
	    	if (maxError < error) {
			DrawBezierCurve(3, curve);
			return;
	    }
	}
    }

    /* Fitting failed -- split at max error point and fit recursively */
    tHatCenter = ComputeCenterTangent(d, splitPoint);
    FitCubic(d, first, splitPoint, tHat1, tHatCenter, error);
    V2Negate(&tHatCenter);
    FitCubic(d, splitPoint, last, tHatCenter, tHat2, error);
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
static void GenerateBezier(TPoint *d, int first, int last, double *uPrime, TPoint tHat1, TPoint tHat2, TPoint *bezCurve)
{
    int 	i;
    TPoint 	A[MAXPOINTS][2];	/* Precomputed rhs for eqn	*/
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

 
    /* Compute the A's	*/
    for (i = 0; i < nPts; i++) {
		TPoint		v1, v2;
		v1 = tHat1;
		v2 = tHat2;
		V2Scale(&v1, B1(uPrime[i]));
		V2Scale(&v2, B2(uPrime[i]));
		A[i][0] = v1;
		A[i][1] = v2;
    }

    /* Create the C and X matrices	*/
    C[0][0] = 0.0;
    C[0][1] = 0.0;
    C[1][0] = 0.0;
    C[1][1] = 0.0;
    X[0]    = 0.0;
    X[1]    = 0.0;

    for (i = 0; i < nPts; i++) {
        C[0][0] += V2Dot(&A[i][0], &A[i][0]);
		C[0][1] += V2Dot(&A[i][0], &A[i][1]);
/*					C[1][0] += V2Dot(&A[i][0], &A[i][1]);*/	
		C[1][0] = C[0][1];
		C[1][1] += V2Dot(&A[i][1], &A[i][1]);

		tmp = V2SubII(d[first + i],
	        V2AddII(
	          V2ScaleIII(d[first], B0(uPrime[i])),
		    	V2AddII(
		      		V2ScaleIII(d[first], B1(uPrime[i])),
		        			V2AddII(
	                  		V2ScaleIII(d[last], B2(uPrime[i])),
	                    		V2ScaleIII(d[last], B3(uPrime[i]))))));
	

	X[0] += V2Dot(&A[i][0], &tmp);
	X[1] += V2Dot(&A[i][1], &tmp);
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
    double segLength = V2DistanceBetween2Points(&d[last], &d[first]);
    epsilon = 1.0e-6 * segLength;
    if (alpha_l < epsilon || alpha_r < epsilon)
    {
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
 *  Reparameterize:
 *   Given set of points and their parameterization, try to find
 *   a better parameterization.
 *   \param d Array of digitized points
 *   \param first, last Indices defining region
 *   \param[inout] u Current parameter values
 *   \param bezCurve Current fitted curve
 */
static void Reparameterize(TPoint *d, int first, int last, double *u, BezierCurve bezCurve)
{
    for (int i = first; i <= last; i++) {
	u[i-first] = NewtonRaphsonRootFind(bezCurve, d[i], u[i-first]);
    }
}



/*
 *  NewtonRaphsonRootFind :
 *	Use Newton-Raphson iteration to find better root.
 *  \param Q Current fitted curve
 *  \param P Digitized point
 *  \param u Parameter value for "P"
 */
static double NewtonRaphsonRootFind(BezierCurve Q, TPoint P, double u)
{
    double 		numerator, denominator;
    TPoint 		Q1[3], Q2[2];	/*  Q' and Q''			*/
    TPoint		Q_u, Q1_u, Q2_u; /*u evaluated at Q, Q', & Q''	*/
    double 		uPrime;		/*  Improved u			*/
    int 		i;
    
    /* Compute Q(u)	*/
    Q_u = BezierII(3, Q, u);
    
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
    Q1_u = BezierII(2, Q1, u);
    Q2_u = BezierII(1, Q2, u);
    
    /* Compute f(u)/f'(u) */
    numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
    denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
		      	  (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
    if (denominator == 0.0f) return u;

    /* u = u - f(u)/f'(u) */
    uPrime = u - (numerator/denominator);
    return (uPrime);
}

	
		       
/*
 *  Bezier :
 *  	Evaluate a Bezier curve at a particular parameter value
 *    \param degree The degree of the bezier curve
 *    \param V      Array of control points
 *    \param t      Parametric value to find point for
 */
static TPoint BezierII(int degree, TPoint *V, double t)
{
    int 	i, j;		
    TPoint 	Q;	        /* Point on curve at parameter t	*/

    /* Copy array	*/
    assert(degree<4);
    TPoint Vtemp[4];     	/* Local copy of control points         */
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

    Q = Vtemp[0];
    return Q;
}


/*
 *  B0, B1, B2, B3 :
 *	Bezier multipliers
 */
static double B0(double u)
{
    double tmp = 1.0 - u;
    return (tmp * tmp * tmp);
}


static double B1(double u)
{
    double tmp = 1.0 - u;
    return (3 * u * (tmp * tmp));
}

static double B2(double u)
{
    double tmp = 1.0 - u;
    return (3 * u * u * tmp);
}

static double B3(double u)
{
    return (u * u * u);
}



/*
 * ComputeLeftTangent, ComputeRightTangent, ComputeCenterTangent :
 * Approximate unit tangents at endpoints and "center" of digitized curve
 * \param d Digitized points
 * \param end Index to "left" end of region
 */
static TPoint ComputeLeftTangent(TPoint *d, int end)
{
    TPoint	tHat1;
    tHat1 = d[end+1] - d[end];
    tHat1 = *V2Normalize(&tHat1);
    return tHat1;
}

/**
 * \param d Digitized points
 * \param end Index to "right" end of region
 */
static TPoint ComputeRightTangent(TPoint *d, int end)
{
    TPoint	tHat2;
    tHat2 = d[end-1] - d[end];
    tHat2 = *V2Normalize(&tHat2);
    return tHat2;
}

/**
 * \param d Digitized points
 * \param center Index to point inside region
 */
static TPoint ComputeCenterTangent(TPoint *d, int center)
{
    TPoint	V1, V2, tHatCenter;

    V1 = d[center-1] - d[center];
    V2 = d[center] - d[center+1];
    tHatCenter.x = (V1.x + V2.x)/2.0;
    tHatCenter.y = (V1.y + V2.y)/2.0;
    tHatCenter = *V2Normalize(&tHatCenter);
    return tHatCenter;
}


/*
 *  ChordLengthParameterize :
 *	Assign parameter values to digitized points 
 *	using relative distances between points.
 *  \param d Array of digitized points
 *  \param first, last Indices defining region
 */
static void ChordLengthParameterize(TPoint *d, int first, int last, double *u)
{
    int		i;	

    u[0] = 0.0;
    for (i = first+1; i <= last; i++) {
		u[i-first] = u[i-first-1] +
	  			V2DistanceBetween2Points(&d[i], &d[i-1]);
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
static double ComputeMaxError(TPoint *d, int first, int last, BezierCurve bezCurve, double *u, int *splitPoint)
{
    int		i;
    double	maxDist;		/*  Maximum error		*/
    double	dist;		/*  Current error		*/
    TPoint	P;			/*  Point on curve		*/
    TPoint	v;			/*  Vector from point to curve	*/

    *splitPoint = (last - first + 1)/2;
    maxDist = 0.0;
    for (i = first + 1; i < last; i++) {
		P = BezierII(3, bezCurve, u[i-first]);
		v = P - d[i];
		dist = V2SquaredLength(&v);
		if (dist >= maxDist) {
	    	maxDist = dist;
	    	*splitPoint = i;
		}
    }
    return (maxDist);
}
static TPoint V2AddII(TPoint a, TPoint b)
{
    TPoint	c;
    c.x = a.x + b.x;  c.y = a.y + b.y;
    return (c);
}
static TPoint V2ScaleIII(TPoint v, double s)
{
    TPoint result;
    result.x = v.x * s; result.y = v.y * s;
    return (result);
}

static TPoint V2SubII(TPoint a, TPoint b)
{
    TPoint	c;
    c.x = a.x - b.x; c.y = a.y - b.y;
    return (c);
}
