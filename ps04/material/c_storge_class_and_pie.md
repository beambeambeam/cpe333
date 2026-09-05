# C Storage Class and PIE

## Programming API Perspective: C Storage Classes

- A Storage Class (SC) defines the scope, life-time, and physical location of a variable and precedes the variable name, e.g. `static int count`
  - *Auto:* the default storage class for all local variables
  - *Static:* used to keep a local variable in existence during the life-time of the program (instead of creating and destroying it each time the function is called and exits)
  - *Extern:* used to give a reference of a global variable that is visible to ALL the program files
  - *Register:* used to define local variables that **should** be stored in a register instead of RAM

## Auto and Static Demonstration

### Code

```c
void example_func() {
    static int static_v = 5;
    int auto_v = 3;

    static_v++;
    auto_v++;
    printf("static variable = %d, auto variable = %d\n\n",
        static_v, auto_v);
}

int main() {
    example_func();
    example_func();
    return 0;
}
```

### Output

```text
static variable = 6, auto variable = 4

static variable = 7, auto variable = 4
```

Note:

- static variable start at 5
- auto variable start at 3

## Auto and Extern Demonstration

### Code

```c
int extern_v = 10;
int v = 5;

void main() {
    int v;
    extern int extern_v;

    printf("v = %d, extern_v = %d\n", v, extern_v);
}
```

### Output

```text
v = 25864, extern_v = 10
```

Note:

- V is random since we declare it without initial value.
- extern_v is become the same as the one initialized before.

## Position Independent Executable (PIE)

- **PIE allows an executable to be loaded at a different virtual address each time it runs.**
- Instead of relying heavily on **fixed absolute addresses**, PIE uses **Position-independent / relative addressing** using offset from pointer.
