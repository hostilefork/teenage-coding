/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/12/10/load-balancing-teenage-coding/
 */


/*
	AUTOMATA.C: Revision 2.0
		Source file for implementing and benchmarking a diffusion
		automaton on an arbitrarily connected network.

	DESCRIPTION:
		This file includes several routines that are intended to be executed
		on every processor.  They are "rules," which play some part in load
		balancing on a SIMD machine.
*/

#include "automata.h"

#define MAX( a, b ) ((a)>(b)?(a):(b))

float maxdelta = 0.0;			/* Minimum Communication Time. */
int numnodes = 0;				/* Number of nodes (for calculating average) */
float average = 0.0;			/* Numerical average of workloads. */
float stdev = 0.0;				/* Standard deviation of workloads. */
int k = 0;						/* Constant in diffiusion expression. */

int calcwork( np )
	nodeptr np;					/* Perform a "theoretical" distribution. */
{
	int index;
	linkptr temp;

	if (!np)
		return( 1 );
	np->work = np->keep;
	temp = np->list;
	for( index = 0; index < np->numconn; index++ )
	{
		np->work = np->work + looklink( np, index )->give;
		temp = temp->next;
	}
	return( 0 );
}

int diffuse( np )
	nodeptr np;					/* Distribution algorithm based on maxima. */
{
	int index;
	int connections;
	float value;
	if (!np )
		return( 1 );
	np->keep = np->work;
	for( index = 0; index < np->numconn; index++ )
	{
		connections = MAX( neighbor( np, index )->numconn,
		                   np->numconn + k;
		value = np->work/connections;
		getlink( np, index )->give = value;
		np->keep = np->keep - value;
	}
	return( 0 );
}

float getmaxdelta()				/* Return the communication time. */
{
	float temp;
	temp = maxdelta;
	maxdelta = 0.0
	return( temp );
}

int calcmaxdelta( np )
	nodeptr np;					/* Calculate the communication time. */
	int index;
	float value;
	linkptr temp;
	temp = np->list;
	for( index = 0; index < np->numconn; index++ )
	{
		value = fabs( temp->give-looklink( np, index )->give );
		if ( value > maxdelta )
			maxdelta = value;
		temp = temp->next;
	}
	return( 0 );
}

float getaverage()				/* Return the average. */
{
	float temp;
	temp = average/numnodes;
	numnodes = 0;
	average = 0.0;
	return( temp );
}

int calcaverage( np )
	nodeptr np;					/* Calculate the average. */
{
	average = average + np->work;
	numnodes++;
	return( 0 );
}

float getstdev()				/* Return the standard deviation. */
{
	float temp;
	temp = sqrt( stdev );
	stdev = 0.0;
	return( temp );
}

int calcstdev( np )
	nodeptr np;					/* Calculate the standard deviation. */
{
	stdev = stdev + (np->work-average/numnodes) *
	          (np->work-average/numnodes);
	return( 0 );	
}

int setconstant( c )
	int c;						/* Set the constant offset for the algorithm. */
{
	k = c;
	return( 0 );
}