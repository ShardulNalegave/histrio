
.text
.global coroutine_trampoline
.type coroutine_trampoline, @function

coroutine_trampoline:

    # r12 -> Entry function pointer
    # r13 -> user data

    # Set the first argument of function to provided user data
    movq %r13, %rdi

    # Call entry function
    call *%r12

after_exit:
    # Runs after function returns
    # This should not happen normally, we either switch
    # to a different coroutine or call cleanup.
    # If it does, we call finished handler

    call coroutine_finished

    # Unreachable; crash using a undefined instruction
    ud2

.size coroutine_trampoline, .-coroutine_trampoline
