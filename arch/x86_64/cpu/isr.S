.section .rodata, "a", @progbits

error_msg:
  .asciz "[!] Unhandled interrupt %d\n"

.section .text, "ax"
.code64

#define ISRERR(n)  \
  .global isr##n  ;\
  isr##n:         ;\
  pushq $n        ;\
  jmp isr_stub

#define ISRNOERR(n)  \
  .global isr##n    ;\
  isr##n:           ;\
  pushq $0          ;\
  pushq $n          ;\
  jmp isr_stub

#define PUSHREGS()  \
  pushq %rbp       ;\
  pushq %rax       ;\
  pushq %rbx       ;\
  pushq %rcx       ;\
  pushq %rdx       ;\
  pushq %rdi       ;\
  pushq %rsi       ;\
  pushq %r8        ;\
  pushq %r9        ;\
  pushq %r10       ;\
  pushq %r11       ;\
  pushq %r12       ;\
  pushq %r13       ;\
  pushq %r14       ;\
  pushq %r15

#define POPREGS()  \
  popq %r15       ;\
  popq %r14       ;\
  popq %r13       ;\
  popq %r12       ;\
  popq %r11       ;\
  popq %r10       ;\
  popq %r9        ;\
  popq %r8        ;\
  popq %rsi       ;\
  popq %rdi       ;\
  popq %rdx       ;\
  popq %rcx       ;\
  popq %rbx       ;\
  popq %rax       ;\
  popq %rbp

isr_stub:
  PUSHREGS()
  /* store pointer to isr_frame struct into %rdi */
  movq %rsp, %rdi
  call isr_common
  POPREGS()
  addq $16, %rsp
  iretq

/* exceptions */
ISRNOERR(0)   /* #DE */
ISRNOERR(1)   /* #DB */
ISRNOERR(2)   /* --- */
ISRNOERR(3)   /* #BP */
ISRNOERR(4)   /* #OF */
ISRNOERR(5)   /* #BR */
ISRNOERR(6)   /* #UD */
ISRNOERR(7)   /* #NM */
ISRERR(8)     /* #DF */
ISRNOERR(9)   /* --- */
ISRERR(10)    /* #TS */
ISRERR(11)    /* #NP */
ISRERR(12)    /* #SS */
ISRERR(13)    /* #GP */
ISRERR(14)    /* #PF */
ISRNOERR(15)  /* --- */
ISRNOERR(16)  /* #MF */
ISRERR(17)    /* #AC */
ISRNOERR(18)  /* #MC */
ISRNOERR(19)  /* #XF */
ISRNOERR(20)  /* #VE */
ISRERR(21)    /* #CP */
ISRNOERR(22)  /* --- */
ISRNOERR(23)  /* --- */
ISRNOERR(24)  /* --- */
ISRNOERR(25)  /* --- */
ISRNOERR(26)  /* --- */
ISRNOERR(27)  /* --- */
ISRNOERR(28)  /* #HV */
ISRERR(29)    /* #VC */
ISRERR(30)    /* #SX */
ISRNOERR(31)  /* --- */

/* pic interrupts */
ISRNOERR(32)  /* PIT */
ISRNOERR(33)  /* --- */
ISRNOERR(34)  /* --- */
ISRNOERR(35)  /* --- */
ISRNOERR(36)  /* --- */
ISRNOERR(37)  /* --- */
ISRNOERR(38)  /* --- */
ISRNOERR(39)  /* --- */
ISRNOERR(40)  /* --- */
ISRNOERR(41)  /* --- */
ISRNOERR(42)  /* --- */
ISRNOERR(43)  /* --- */
ISRNOERR(44)  /* --- */
ISRNOERR(45)  /* --- */
ISRNOERR(46)  /* --- */
ISRNOERR(47)  /* --- */
