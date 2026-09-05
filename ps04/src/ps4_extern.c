/* PS04 Part 2 - Step 1: Extern Storage Class (original).
 * Global `x = 20`, both display() and main() refer to it via `extern int x;`.
 * Build: default PIE and -no-pie via Makefile.
 * Run 3 times, record values + addresses from main and display.
 * Matches problem-session.md section 2 verbatim (void main kept as handed out).
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
    extern int x;
    printf(" Print value of x: %d\n", x);
    printf(" Adress of x (in main function): %p\n\n", &x);

    display();
}
