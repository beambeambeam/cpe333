/* PS04 Part 1 - Steps 2-3: Static Storage Class with `static` removed.
 * Same program as ps4_static.c except `static int y = 5;` -> `int y = 5;`
 * so y becomes an auto variable re-created each loop iteration.
 * Build: default PIE and -no-pie via Makefile.
 */
#include <stdio.h>
#include <stdlib.h>

void main()
{
    auto int x = 3;

    while (x > 0)
    {
        int y = 5;
        y++;

        printf("The value of y is %d\n", y);
        printf("The address of y is %p\n\n", &y);

        x--;
    }
}
