#include <stdio.h>
    
main()
{
    printf("compliled %s.\n", __DATE__);
    printf("sizeof int: %d.\n", sizeof(int));
    printf("sizeof int*: %d.\n", sizeof(int*));
    printf("sizeof void*: %d.\n", sizeof(void*));
}
    
