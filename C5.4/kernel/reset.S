.text
.code 32
.global reset_handler
.global vectors_start, vectors_end
.global kernel_init, main, proc, procsize
.global tswitch, scheduler, running
.global lock, unlock // disable/enable interrupt

// in KC Wang's book, below 2 functions are named int_off, int_on
// but it has nothing to do with interrupt
// so I changed the names and implementation
.global get_cpsr, set_cpsr//int_off, int_on // get and set the CPSR

reset_handler:
    // set SVC sp to proc[0] high end
    // ming: because we set the SVC stack to proc, so no need to use the "svc_stack_top" defined in the myos.ld
    LDR r0, =proc
    LDR r1, =procsize
    LDR r2, [r1, #0]
    ADD r0, r0, r2
    MOV sp, r0
    // copy vector table to address 0
    BL copy_vectors
    // go in IRQ mode to set IRQ stack
    MSR cpsr, #0x92
    LDR sp, =irq_stack_top
    // go back to SVC mode with IRQ enabled
    MSR cpsr, #0x13
    // call main() in SVC mode
    BL main
    B .

tswitch:
    mrs r0, cpsr // SVC mode, IRQ interrupts off
    orr r0, r0, #0x80
    msr cpsr, r0

    stmfd sp!, {r0-r12, lr}

    LDR r0, =running // r0=&running, running is a PROC *, so r0 here is a PROC **
    LDR r1, [r0, #0] // r1=running, r1 is a PROC *
    str sp, [r1, #4] // running->ksp = sp

    // change the running
    // note that the "task switch" and "task scheduling" are 2 different things.
    // so decoupled.
    bl scheduler

    LDR r0, =running
    LDR r1, [r0, #0] // r1=running, the new running
    lDR sp, [r1, #4] // get the sp from the new running PROC, stack switch happens here! the stack of the new PROC should be ready for *resume*!

    mrs r0, cpsr // SVC mode, IRQ interrupts on
    bic r0, r0, #0x80
    msr cpsr, r0

    /*
    IRQ can happen here. And the task switch will be interrupted.
    But it should be OK as long as the IRQ handling can return to here.
    */

    // resume the execution of the new PROC by restoring the context from the new PROC's stack
    ldmfd sp!, {r0-r12, pc}


irq_handler:
    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    bl IRQ_handler
    ldmfd sp!, {r0-r12, pc}^

lock: // disable IRQ interrupts
    MRS r0, cpsr
    ORR r0, r0, #0x80 // set IRQ mask bit
    MSR cpsr, r0
    MOV pc, lr

unlock: // enable IRQ interrupts
    MRS r0, cpsr
    BIC r0, r0, #0x80 // clear IRQ mask bit
    MSR cpsr, r0
    MOV pc, lr

// in KC Wang's book, below function is named int_off
// but it has nothing to do with interrupt
// so I changed the name and implementation
get_cpsr: // only return CPSR
    MRS r1, cpsr // load cpsr into r1
    MOV r0, r1 // store cpsr in r0 as the return value
    //ORR r1, r1, #0x80 // set IRQ mask bit  // maybe we don't need to care about interrupt here.
    //MSR cpsr, r1 // disable interrupt
    MOV pc, lr  // return by put lr into pc, lr (= old pc-4) is the next instruction to execute

// in KC Wang's book, below function is named int_on
// but it has nothing to do with interrupt
// so I changed the name and implementation
set_cpsr: //restore original CPSR, void int_on(u32 CPSR)
    MSR cpsr, r0 // parameter 1 is passed through r0
    MOV pc, lr


undef_handler:
swi_handler:
prefetch_abort_handler:
data_abort_handler:
fiq_handler:
    b .

vectors_start: // vector table
    LDR PC, reset_handler_addr
    LDR PC, undef_handler_addr
    LDR PC, swi_handler_addr
    LDR PC, prefetch_abort_handler_addr
    LDR PC, data_abort_handler_addr
    B .
    LDR PC, irq_handler_addr
    LDR PC, fiq_handler_addr


    reset_handler_addr: .word reset_handler
    undef_handler_addr: .word undef_handler
    swi_handler_addr: .word swi_handler
    prefetch_abort_handler_addr: .word prefetch_abort_handler
    data_abort_handler_addr: .word data_abort_handler
    irq_handler_addr: .word irq_handler
    fiq_handler_addr: .word fiq_handler
vectors_end:
