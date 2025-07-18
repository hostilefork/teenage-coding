/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/12/10/load-balancing-teenage-coding/
 */


/*
    MESHTEST.C: Revision 2.0
        Program file for testing the efficiency of the diffusion algorithm
        on a five by five mesh.

    DESCRIPTION:
        This file will display the network on the screen, as well as keep a
        running list of the generation, the standard deviation, and the
        communication time in an external data file.
*/

#include "node.h"
#include "graph.h"
#include "automata.h"
#include "mesh.h"
#include <conio.h>

int main()
{
    mesh m;
    float time = 0.0;
    FILE *datafile = fopen( "LOG.TXT", "wt" );
    int steps = 0;

    graphicson();

    makemesh( m );
    setconstant( 0 );
    m[2][2]->work = 2500.0;

    do
    {
        traversemesh( m, drawnet );
        traversemesh( m, drawnode );
        traversemesh( m, calcmaxdleta );
        traversemesh( m, calcaverage );
        traversemesh( m, calcstdev );

        fprintf( datafile, "%d %f %f\n", steps, time, getstdev());
        time += getmaxdelta();
        steps++;

        traversemesh( m, calcwork );
    } while( getch() != 27 );

    killmesh( m );

    fclose( datafile );
    graphicsoff();
    return( 0 );
}
