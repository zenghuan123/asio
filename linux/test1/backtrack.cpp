#include <stdio.h>
#include <unwind.h>
#include <stdint.h>
#include "test.h"
#include <signal.h>

struct sigaction act_old;


static _Unwind_Reason_Code unwind_backtrace_callback(struct _Unwind_Context* context, void* arg) {

    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        printf("unwind got pc ...0x%x\n", pc);
    }

    return _URC_NO_REASON;
}

ssize_t unwind_backtrace() {

    _Unwind_Reason_Code rc = _Unwind_Backtrace(unwind_backtrace_callback, 0);

    return rc == _URC_END_OF_STACK ? 0 : -1;
}

void func_1() {
    int ret = unwind_backtrace();
    printf("unwind_backtrace return ...%d\n", ret);
}

void func_2() {
   func_1();
}

static void crash_handler_more(int sig, siginfo_t * info, void* buf) {

    unwind_backtrace();

    sigaction(sig, &act_old, 0);
}

void initCrashHandler() {
    struct sigaction act;
    act.sa_sigaction = crash_handler_more;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGKILL, &act, 0);
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGILL, &act, 0);
    sigaction(SIGABRT, &act, 0);
    sigaction(SIGBUS, &act, 0);
    sigaction(SIGSEGV, &act, &act_old);
}

void triggerCrash() {
    char *p = 0;
    p[100] = 'a';
}

int test1() {

    initCrashHandler();

    func_2();
    
    triggerCrash();
    return 0;
}