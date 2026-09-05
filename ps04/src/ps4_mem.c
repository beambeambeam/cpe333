/* PS04 Part 3 - Steps 1-3: memory allocation (original, pointer b commented out).
 * Run WITHOUT -no-pie (default PIE build).
 * Record Address of Pointer, Effective Address, array a after malloc(),
 * Array c, and After realloc sections.
 * Matches problem-session.md section 3 verbatim (void main kept as handed out).
 */
#include <stdio.h>
#include <stdlib.h>

void main()
{
    int *a;
    float *b;
    int c[10];

    printf("\nAddress of Pointer\n");
    printf(">>%p\n", &a);
    printf(">>%p\n", &b);

    printf("\nEffective Address\n");
    printf(">>%p\n", a);
    printf(">>%p\n", b);

    a = (int*) malloc(10*sizeof(int));
    printf("\nAfter malloc Pointer a\n");
    printf(">>%p\n", a);
    printf(">>%p\n", &a[0]);
    printf(">>%p\n", &a[9]);

    printf("\nArray c\n");
    printf(">>%p\n", c);
    printf(">>%p\n", &c[0]);
    printf(">>%p\n", &c[9]);

/*  b = (float*) malloc(10*sizeof(float));
    printf("\nAfter malloc Pointer b\n");
    printf(">>%p\n", b);
    printf(">>%p\n", &b[0]);
    printf(">>%p\n", &b[9]);
*/
    a = (int*) realloc(a, 1000*sizeof(int));
    printf("\nAfter realloc Pointer a\n");
    printf(">>%p\n", a);
    printf(">>%p\n", &a[0]);
    printf(">>%p\n", &a[9]);
    printf(">>%p\n", &a[999]);

    free(a);
//  free(b);
}
