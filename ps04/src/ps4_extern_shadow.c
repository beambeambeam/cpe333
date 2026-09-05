/* PS04 Part 2 - Steps 2-3: Extern Storage Class with `extern` removed in main().
 * Line 15 of the handout (`extern int x;` inside main) becomes `int x;`:
 * an uninitialized auto local that shadows the global, so its value is
 * indeterminate while display() still sees the global via extern.
 * Build: default PIE and -no-pie via Makefile.
 */
#include <stdio.h>
#include <stdlib.h>

int x = 20;

void display()
{
    extern int x;
    printf(" Display value of x: %d\n", x);
    printf(" Adress of x (in display function): %p\n\n", &x);
}

void main()
{
    int x;
    printf(" Print value of x: %d\n", x);
    printf(" Adress of x (in main function): %p\n\n", &x);

    display();
}
