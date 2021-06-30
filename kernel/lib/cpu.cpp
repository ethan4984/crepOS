#include <sched/smp.hpp>
#include <sched/scheduler.hpp>
#include <cpu.hpp>

extern "C" void syscall_main();

void cpu_init_features() {
    wrmsr(msr_efer, rdmsr(msr_efer) | 1); // set SCE

    uint64_t cr0 = 0;
    asm volatile ( "mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2);
    cr0 |= (1 << 1);
    asm volatile ( "mov %0, %%cr0" :: "r"(cr0));

    uint64_t cr4;
    asm volatile ( "mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 7) | (1 << 9) | (1 << 10);
    asm volatile ( "mov %0, %%cr4" :: "r"(cr4));

    wrmsr(msr_star, 0x0013000800000000);
    wrmsr(msr_lstar, (uint64_t)syscall_main);
    wrmsr(msr_sfmask, (uint64_t)~((uint32_t)0x002));
}

extern "C" void syscall_set_fs_base(regs *regs_cur) {
    smp::cpu &core = smp::core_local();
    sched::task &current_task = sched::task_list[core.pid];
    sched::thread &current_thread = current_task.threads[core.tid];

    set_user_fs(regs_cur->rdi);

    current_thread.user_fs_base = regs_cur->rdi;
}

extern "C" void syscall_set_gs_base(regs *regs_cur) {
    smp::cpu &core = smp::core_local();
    sched::task &current_task = sched::task_list[core.pid];
    sched::thread &current_thread = current_task.threads[core.tid];

    set_user_gs(regs_cur->rdi);

    current_thread.user_gs_base = regs_cur->rdi;
}

extern "C" void syscall_get_fs_base(regs *regs_cur) {
    regs_cur->rax = get_user_fs();
}

extern "C" void syscall_get_gs_base(regs *regs_cur) {
    regs_cur->rax = get_user_gs();
}

void set_errno(uint64_t errno) {
    smp::cpu &core = smp::core_local();
    core.errno = errno;
}

size_t get_errno() {
    smp::cpu &core = smp::core_local();
    return core.errno;
}
