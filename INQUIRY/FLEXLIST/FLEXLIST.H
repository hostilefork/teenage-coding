    /*

        flexlist.h

        Copyright 1989
        John W. Small
        All rights reserved

        PSW / Power SoftWare
        P.O. Box 10072
        McLean, Virginia 22102 8072
        (703) 759-3838

        1/14/89

    */

#ifndef __FLEXLIST_H__
#define __FLEXLIST_H__

    #include    <stddef.h>      /*  size_t  */

    typedef int FlistData;      /* force host alignment */



    typedef struct FlistNode  *FlistN;

    struct  FlistNode {
      FlistN        next, prev;
      FlistData     data;
    };



    struct FlistHeader  {
      FlistN    front,  current, rear;
      unsigned  ncurrent, nodes;
      size_t    ndlen, hdlen;
      FlistData data;
    };

    typedef struct  FlistHeader *Flist;

    /* housekeeping primitives */

    Flist       mkFlist     (size_t  ndsize, size_t  hdsize);
    int         clrFlist    (Flist lptr);
    int         rmFlist     (Flist *lptrAddr);
    unsigned    nempty      (Flist lptr);
    void       *Flistdptr   (Flist lptr);

    /* stack and queue primitives */

    void       *pushdptr    (Flist lptr, void *bufAddr);
    int         pushn       (Flist lptr, FlistN nptr);
    int         popd        (Flist lptr, void *bufAddr);
    FlistN      popn        (Flist lptr);
    void       *topdptr     (Flist lptr, void *bufAddr);
    void       *iquedptr    (Flist lptr, void *bufAddr);
    int         iquen       (Flist lptr, FlistN nptr);

    /* current node primitives */

    unsigned    ncur        (Flist lptr);
    void       *curdptr     (Flist lptr);
    void       *mkcdptr     (Flist lptr, unsigned loc);

    /* list primitives */

    void       *insdptr     (Flist lptr, void *bufAddr);
    int         insn        (Flist lptr, FlistN nptr);
    int         deld        (Flist lptr, void *bufAddr);
    FlistN      deln        (Flist lptr);
    void       *nextdptr    (Flist lptr, void *bufAddr);
    void       *prevdptr    (Flist lptr, void *bufAddr);
    int         getd        (Flist lptr, void *bufAddr);
    int         putd        (Flist lptr, void *bufAddr);

    /* array primitives */

    int   stod  (Flist lptr, void *bufAddr, unsigned loc);
    int   rcld  (Flist lptr, void *bufAddr, unsigned loc);

#endif
