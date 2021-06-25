global smp_core_init_begin
global smp_core_init_end

smp_core_init_begin:

incbin 'sched/smp.bin'

smp_core_init_end:
