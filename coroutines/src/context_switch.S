
.text
.global histrio_coroutine_context_switch
.type histrio_coroutine_context_switch, @function

histrio_coroutine_context_switch:

    # Save the current CPU state into `from`

    leaq  resume_here(%rip), %rax   # rax = address of 'resume_here' label
    movq  %rax,   0(%rdi)           # from->rip = address of resume point

    movq  %rsp,   8(%rdi)           # from->rsp = current stack pointer
    movq  %rbp,  16(%rdi)           # from->rbp = current base pointer
    movq  %rbx,  24(%rdi)           # from->rbx
    movq  %r12,  32(%rdi)           # from->r12
    movq  %r13,  40(%rdi)           # from->r13
    movq  %r14,  48(%rdi)           # from->r14
    movq  %r15,  56(%rdi)           # from->r15

    # Load the new CPU state from `to`

    movq  56(%rsi), %r15            # r15 = to->r15
    movq  48(%rsi), %r14            # r14 = to->r14
    movq  40(%rsi), %r13            # r13 = to->r13
    movq  32(%rsi), %r12            # r12 = to->r12
    movq  24(%rsi), %rbx            # rbx = to->rbx
    movq  16(%rsi), %rbp            # rbp = to->rbp

    # Shift the stack
    movq  8(%rsi), %rsp             # rsp = to->rsp

    # Final jump
    jmpq  *0(%rsi)                  # jump to to->rip

resume_point:
    ret

.size histrio_coroutine_context_switch, .-histrio_coroutine_context_switch
