# parts/

One function body per file (a few files hold two or three functions
that must stay adjacent).  None of these files is compiled on its own:
each is `#include`d by exactly one unity unit -- `stx_u1.c` .. `stx_u4.c`,
`games.c`, `vdistx.c` or `midi_seq.c` -- which supplies the headers.

The ORDER of the `#include` lines in each unit is the function order of
the corresponding object in the original program, and the compiled
bytes depend on it (call distances decide between short and long
branches).  A file whose body must sit next to a particular function
says so in its own comment.

To find which unit uses a file: `grep -n 'parts/NAME.c' *.c`.
