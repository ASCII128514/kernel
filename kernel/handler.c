#include "handler.h"

#include <stdint.h>
#include <stddef.h>

#include "kprint.h"
#include "util.h"
#include "port.h"
#include "pic.h"
#include "char.h"

// Do we need this?
__attribute__((interrupt)) void example_handler(interrupt_context_t* ctx)
{
  kprintf("example interrupt handler\n");
  halt();
}

// Do we need this?
__attribute__((interrupt)) void reserve_handler(interrupt_context_t *ctx)
{
  kprintf("reserved interrupt handler\n");
  halt();
};

// Handler for a divide error fault
__attribute__((interrupt)) void divide_error_handler(interrupt_context_t* ctx)
{
  kprintf("divide error handler\n");
  halt();
};

// Handler for a debug exception (fault/trap)
__attribute__((interrupt)) void debug_exception_handler(interrupt_context_t* ctx)
{
  kprintf("debug exception handler\n");
  halt();
}

// Handler for an NMI interrupt
__attribute__((interrupt)) void nmi_interrupt_handler(interrupt_context_t* ctx)
{
  kprintf("nmi interrupt handler\n");
  halt();
}

// Handler for a breakpoint trap
__attribute__((interrupt)) void breakpoint_handler(interrupt_context_t* ctx)
{
  kprintf("breakpoint handler\n");
  halt();
};

// Handler for an overflow trap
__attribute__((interrupt)) void overflow_handler(interrupt_context_t* ctx)
{
    kprintf("overflow handler\n");
  halt();
};

// Handler for a BOUND range exceeded fault
__attribute__((interrupt)) void bound_range_exceeded_handler(interrupt_context_t *ctx)
{
  kprintf("bound range exceeded handler\n");
  halt();
};

// Handler for an invalid opcode fault
__attribute__((interrupt)) void invalid_opcode_handler(interrupt_context_t *ctx)
{
  kprintf("invalid opcode handler\n");
  halt();
}

// Handler for a devide not available fault
__attribute__((interrupt)) void device_not_available_handler(interrupt_context_t *ctx)
{
  kprintf("device not available handler\n");
  halt();
}

// Handler for a double fault abort
__attribute__((interrupt)) void double_fault_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
  kprintf("double fault handler (ec=%d)\n", ec);
  halt();
}

// Handler for a coprocessor segment overrun fault
__attribute__((interrupt)) void coprocessor_segment_overrun_handler(interrupt_context_t *ctx)
{
  kprintf("coprocessor segment overrun fault handler\n");
  halt();
}

// Handler for an invalid TSS fault
__attribute__((interrupt)) void invalid_tss_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
  kprintf("invalid TSS handler (ec=%d)\n", ec);
  halt();
}

// Handler for a segment not present fault
__attribute__((interrupt)) void segment_not_present_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
  kprintf("segment not present handler (ec=%d)\n", ec);
  halt();
}

// Handler for a stack-segment fault
__attribute__((interrupt)) void stack_segment_fault_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
  kprintf("stack segment fault handler (ec=%d)\n", ec);
  halt();
}

// Handler for a general protection fault
__attribute__((interrupt)) void general_protection_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
  kprintf("general protection handler (ec=%d)\n", ec);
  halt();
}

// Handler for a page fault
__attribute__((interrupt)) void page_fault_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
  kprintf("page fault handler (ec=%d)\n", ec);
  halt();
}

// Handler for an x87 FPU floating-point error (math fault)
__attribute__((interrupt)) void x87_fpu_floating_point_error_handler(interrupt_context_t *ctx)
{
  kprintf("x87 fpu floating point error handler\n");
  halt();
}

// Handler for an alignment check fault
__attribute__((interrupt)) void alignment_check_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
  kprintf("alignment check handler (ec=%d)\n", ec);
  halt();
}

// Handler for a machine check abort
__attribute__((interrupt)) void machine_check_handler(interrupt_context_t *ctx)
{
  kprintf("machine check handler\n");
  halt();
}

// Handler for an SIMD floating-point exception (fault)
__attribute__((interrupt)) void simd_floating_point_exception_handler(interrupt_context_t *ctx)
{
  kprintf("simd floating point exception handler\n");
  halt();
}

// Handler for a virtualization exception (fault)
__attribute__((interrupt)) void virtualization_exception_handler(interrupt_context_t *ctx)
{
  kprintf("virtualization exception handler\n");
  halt();
}

// Handler for a control protection exception (fault)
__attribute__((interrupt)) void control_protection_exception_handler_ec(interrupt_context_t *ctx, uint64_t ec)
{
  kprintf("control protection exception handler (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt)) void irq1_interrupt_handler(interrupt_context_t *ctx)
{
    kprintf("%c\n", getchar(inb(0x60)));
    outb(PIC1_COMMAND, PIC_EOI);
}
