
.text
.global histrio_coroutine_trampoline
.type histrio_coroutine_trampoline, @function

histrio_coroutine_trampoline:

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

    call histrio_coroutine_finished

    # Unreachable; crash using a undefined instruction
    undefined

.size histrio_coroutine_trampoline, .-histrio_coroutine_trampoline
