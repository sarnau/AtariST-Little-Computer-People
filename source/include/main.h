/* main.h -- extern declarations for main.c. */

#ifndef MAIN_H
#define MAIN_H


extern void gameLoop();
#ifdef HOST
extern int lcp_main();          /* see parts/main.c */
#else
extern int main();
#endif

#endif /* MAIN_H */
