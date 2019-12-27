    .global start, stack_top, label2
start:
    ldr sp, =stack_top // sp = &stack_top, as soon as we have the stack ready, we can call C function
    bl main
    b .
