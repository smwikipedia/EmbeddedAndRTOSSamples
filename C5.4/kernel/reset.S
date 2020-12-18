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
    // The ARM processor enters SVC mode when turned on, reset or executing SWI instruction.
    // Set SVC stack sp to proc[0] high end
    // ming: because we set the SVC stack to locaton within PROC struct, no need to use the "svc_stack_top" defined in the myos.ld
    LDR r0, =proc       // r0 = &proc[0], proc[NPROC] is defined in kernel.c
    LDR r1, =procsize   // r1 = &procsize, procsize is defined in kernel.c as "u32 procsize = sizeof(PROC);"
    LDR r2, [r1, #0]    // r2 = procsize
    ADD r0, r0, r2      // r0 points to the last byte of proc[0], i.e. &(proc[0].kstack[SSIZE - 1]);
    MOV sp, r0          // set up the SVC mode sp as proc[0] high end

    // copy vector table to address 0
    BL copy_vectors     // defined in kernel.c

    // go in IRQ mode to set IRQ stack
    MSR cpsr, #0x92        // 1001 0010, bit[7]=1, interrupt disabled, IRQ mode = 10010 
    LDR sp, =irq_stack_top // set the sp_irq to irq_stack_top, which is defined in myos.ld
    // go back to SVC mode with IRQ enabled
    MSR cpsr, #0x13        // 0001 0011, SVC mode = 10011, bit[7]=0, interrupt enabled
    // call main() in SVC mode
    BL main
    B .

tswitch:
    mrs r0, cpsr      // SVC mode, IRQ interrupts off
    orr r0, r0, #0x80 // 1000 0000, bit[7]=1 to disable IRQ
    msr cpsr, r0      // IRQ disabled

    /*
    Save the context in the stack of current task.
    */
    stmfd sp!, {r0-r12, lr} // totally 14 registers

    /*
    Save current task's sp in current task's PROC structure.
    */
    LDR r0, =running // r0=&running, running is a PROC *, so r0 here is a PROC **
    LDR r1, [r0, #0] // r1=running, r1 is a PROC *
    str sp, [r1, #4] // running->ksp = sp

    // change the global "running"
    // note that the "task switch" and "task scheduling" are 2 different things.
    // thus decoupled.
    bl scheduler
    // now a new running task has been selected, it can still the last one.

    /*
    Get the sp from the new task's PROC structure.
    */
    LDR r0, =running // r0=&running, running is a PROC *, so r0 here is a PROC **
    LDR r1, [r0, #0] // r1=running, r1 is a PROC *, the new running
    lDR sp, [r1, #4] // get the sp from the new running PROC, "stack switch" happens here! the stack of the new PROC must already be ready for *resume*!

    mrs r0, cpsr      // SVC mode, IRQ interrupts on
    bic r0, r0, #0x80 // 1000 0000, bit[7] cleared to enable IRQ.
    msr cpsr, r0      // IRQ enabled

    /*
    IRQ can happen here. And the task switch will be interrupted.
    But it should be OK as long as the IRQ handling can return to here.
    */

    /*
     Resume the execution of the new PROC by restoring the context from the new PROC's stack
     The new task's PROC.ksp field doesn't need to be updated here.
     It will be updated when the task is scheduled out.
    */
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
