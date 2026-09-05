/*
 * obdefs1.h -- include <obdefs.h> at most once.
 *
 * The DK's obdefs.h has no include guard of its own, so a unity
 * translation unit that pulls in two sources which both include it
 * gets "redeclaration: object".  Port sources include this instead.
 */
#ifndef OBDEFS1_H
#define OBDEFS1_H
#ifdef HOST
#include "hostgem.h"
#else
#include <obdefs.h>
#endif
#endif
