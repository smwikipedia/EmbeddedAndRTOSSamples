    .global start, stack_top
start:
    ldr sp, =stack_top
    bl main
    b .
