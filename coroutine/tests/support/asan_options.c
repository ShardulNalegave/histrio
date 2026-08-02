//   - detect_stack_use_after_return relocates address-taken locals onto a
//     heap-backed "fake stack" instead of the real stack, which breaks
//     test_execution.c's stack-isolation assertion (it checks a
//     coroutine-local variable's address against the coroutine's actual
//     stack buffer) even though nothing is actually wrong.
//   - handle_segv defaults on, so ASan intercepts the deliberate SIGSEGVs
//     in test_misuse.c and turns them into its own report + internal
//     _exit/abort instead of letting the real signal propagate, breaking
//     Criterion's .signal = SIGSEGV expectation. Those tests are checking
//     for a real, deterministic crash on purpose; ASan's diagnostics add
//     nothing there.
//   - detect_leaks flags a small fixed allocation inside Criterion's own
//     per-test timeout bookkeeping (bxfi_push_timeout), not this library.
const char *__asan_default_options(void) {
    return "detect_stack_use_after_return=0:handle_segv=0:detect_leaks=0";
}
