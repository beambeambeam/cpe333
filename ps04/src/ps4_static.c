/* PS04 Part 1 - Step 1: Static Storage Class (original, with `static`).
 * Build: default PIE and -no-pie via Makefile.
 * Run 3 times, record value + address of y each run.
 * Matches problem-session.md section 1 verbatim (void main kept as handed out).
 */
#include <stdio.h>
#include <stdlib.h>

void main()
{
    auto int x = 3;

    while (x > 0)
    {
        static int y = 5;
        y++;

        printf("The value of y is %d\n", y);
        printf("The address of y is %p\n\n", &y);

        x--;
    }
}
