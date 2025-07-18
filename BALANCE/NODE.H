/*
 * Archival code written when I was 17, see blog about it:
 *     http://hostilefork.com/2013/12/10/load-balancing-teenage-coding/
 */


/*
    NODE.H: Revision 3.2
        Header file for parallel network abstraction routines, used
        in simulations of dynamic load balancing.

    DESCRIPTION:
        These data structures are not specific to a particular
        architecture, nodes may be connected in an arbitrary manner.
        Specific architectures are implemented in external files.
        Routines are included for displaying the state of the network
        on a graphics display.
*/

#ifndef __NODE_H__
#define __NODE_H__

#include <stdio.h>

typedef struct node;
typedef struct node *nodeptr;
    /* Definition of a single processor on the network. */

typedef struct link;
typedef struct link *linkptr;
    /* Definition of a list of links between nodes in the network. */

typedef struct node
{
    int x;                /* X location of node on the screen. */
    int y;                /* Y location of node on the screen. */
    float work;           /* Number of processes at this node. */
    float keep;           /* Amount of data which the processor keeps. */
    int numconn;          /* Number of neighboring nodes. */
    linkptr list;         /* Links to other nodes */
};

typedef struct link
{
    nodeptr data;          /* The pointer to the neighboring processor. */
    float give;            /* Amount of work to give to that processor. */
    linkptr next;          /* Pointer to the next processor in the list. */
};

typedef int rule();        /* Generic type for a cellular automaton rule. */

nodeptr makenode();        /* Make a disconnected node. */
nodeptr killnode();        /* Kill a node and release its memory */

int makeconn();            /* Make a connection between two nodes. */
int isconn();              /* Check to see if two nodes are connected. */
int killconn();            /* Kill a connection between two nodes */
nodeptr neighbor();        /* Returns a pointer to a particular neighbor. */

linkptr getlink();         /* Get a link to a particular neighbor. */
linkptr looklink();        /* Look at neighbor's link to yourself. */

#endif
