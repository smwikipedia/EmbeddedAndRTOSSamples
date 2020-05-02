.text
.code 32
.global reset_handler, vectors_start, vectors_end, lock, unlock

reset_handler:
    ldr sp, =svc_stack_top // set SVC mode stack
    bl copy_vectors        // copy vector table to address 0
    msr cpsr, #0x92        // change mode to IRQ mode so we can set the IRQ stack
    ldr sp, =irq_stack_top // set IRQ mode stack
    msr cpsr, #0x13        // go back to SVC mode with IRQ on
    bl main
    b .

/*
If this one is used, comment out the "irq_handler()" in kernel.c
*/
/*
irq_handler:
    sub lr, lr, #4            // when IRQ happens, LR_irq = PC, the next instruction should be PC-4, so we substract 4 from LR here.
    stmfd sp!, {r0-r12, lr}   // stack r0-r12 and lr
    bl IRQ_handler            // call IRQ_handler() in C
    ldmfd sp!, {r0-r12, pc}^  // return
*/

undef_handler:
swi_handler:
prefetch_abort_handler:
data_abort_handler:
fiq_handler:
    b .

lock:
    mrs r0, cpsr
    orr r0, r0, #0x80 // set the I bit to disable IRQ
    msr cpsr, r0
    mov pc, lr // return, by ARM C calling convention, the lr=pc-4, i.e. the next instruction

unlock:
    mrs r0, cpsr
    bic r0, r0, #0x80 // clear the I bit to enable IRQ
    msr cpsr, r0
    mov pc, lr


vectors_start:
    ldr pc, reset_handler_addr
    ldr pc, undef_handler_addr
    ldr pc, swi_handler_addr
    ldr pc, prefetch_abort_handler_addr
    ldr pc, data_abort_handler_addr
    b .
    ldr pc, irq_handler_addr
    ldr pc, fiq_handler_addr

reset_handler_addr:              .word reset_handler
undef_handler_addr:              .word undef_handler
swi_handler_addr:                .word swi_handler
prefetch_abort_handler_addr:     .word prefetch_abort_handler
data_abort_handler_addr:         .word data_abort_handler
irq_handler_addr:                .word irq_handler // it can be in the reset.S or the kernel.c
fiq_handler_addr:                .word fiq_handler
vectors_end:
