#ifndef APIC_HPP_
#define APIC_HPP_

#include <cpu.hpp>
#include <acpi/madt.hpp>

namespace apic {

constexpr size_t msr_lapic_base = 0x1b;

class ioapic {
public:
    ioapic(madt1 madt1_ptr) : madt1_ptr(madt1_ptr) { }
    ioapic() = default;

    uint32_t read(uint32_t reg);
    void write(uint32_t reg, uint32_t data);

    size_t read_redirection(uint32_t gsi);
    void write_redirection(uint32_t gsi, size_t data);

    void mask_gsi(uint32_t gsi);
    void unmask_gsi(uint32_t gsi);
    void mask_all_gsi();
    void redirect_gsi(uint8_t vector, uint32_t gsi, uint16_t flags);

    uint32_t max_gsi();
private:
    friend ioapic ioapic_validate(uint32_t gsi);

    madt1 madt1_ptr;
};

struct xxapic {
    virtual void write(size_t reg, size_t data) const = 0;
    virtual size_t read(size_t reg) const = 0;
    virtual void send_ipi(size_t ap, size_t ipi) const = 0;

    virtual const size_t &id_reg() const = 0;
    virtual const size_t &version_reg() const = 0;
    virtual const size_t &tpr() const = 0;
    virtual const size_t &ppr() const = 0;
    virtual const size_t &eoi() const = 0;
    virtual const size_t &ldr() const = 0;
    virtual const size_t &sint() const = 0;
    virtual const size_t &isr0() const = 0;
    virtual const size_t &isr1() const = 0;
    virtual const size_t &isr2() const = 0;
    virtual const size_t &isr3() const = 0;
    virtual const size_t &isr4() const = 0;
    virtual const size_t &isr5() const = 0;
    virtual const size_t &isr6() const = 0;
    virtual const size_t &isr7() const = 0;
    virtual const size_t &tmr0() const = 0;
    virtual const size_t &tmr1() const = 0;
    virtual const size_t &tmr2() const = 0;
    virtual const size_t &tmr3() const = 0;
    virtual const size_t &tmr4() const = 0;
    virtual const size_t &tmr5() const = 0;
    virtual const size_t &tmr6() const = 0;
    virtual const size_t &tmr7() const = 0;
    virtual const size_t &irr0() const = 0;
    virtual const size_t &irr1() const = 0;
    virtual const size_t &irr2() const = 0;
    virtual const size_t &irr3() const = 0;
    virtual const size_t &irr4() const = 0;
    virtual const size_t &irr5() const = 0;
    virtual const size_t &irr6() const = 0;
    virtual const size_t &irr7() const = 0;
    virtual const size_t &esr() const = 0;
    virtual const size_t &icr() const = 0;
    virtual const size_t &timer_lvt() const = 0;
    virtual const size_t &thermal_lvt() const = 0;
    virtual const size_t &performance_lvt() const = 0;
    virtual const size_t &lint0() const = 0;
    virtual const size_t &lint1() const = 0;
    virtual const size_t &evt() const = 0;
    virtual const size_t &timer_inital_count() const = 0;
    virtual const size_t &timer_current_count() const = 0;
    virtual const size_t &timer_divide_conf() const = 0;
};

class x2apic : public xxapic {
public:
    x2apic();

    size_t read(size_t reg) const override {
        return rdmsr(reg);
    }

    void write(size_t reg, size_t data) const override {
        wrmsr(reg, data);
    }

    void send_ipi(size_t ap, size_t ipi) const override {
        write(icr(), (ap << 24) << 32 | ipi);
    }

    const size_t &id_reg() const override { return id_reg_msr; }
    const size_t &version_reg() const override { return version_reg_msr; }
    const size_t &tpr() const override { return tpr_msr; }
    const size_t &ppr() const override { return ppr_msr; }
    const size_t &eoi() const override { return eoi_msr; }
    const size_t &ldr() const override { return ldr_msr; }
    const size_t &sint() const override { return sint_msr; }
    const size_t &isr0() const override { return isr0_msr; }
    const size_t &isr1() const override { return isr1_msr; }
    const size_t &isr2() const override { return isr2_msr; }
    const size_t &isr3() const override { return isr3_msr; }
    const size_t &isr4() const override { return isr4_msr; }
    const size_t &isr5() const override { return isr5_msr; }
    const size_t &isr6() const override { return isr6_msr; }
    const size_t &isr7() const override { return isr7_msr; }
    const size_t &tmr0() const override { return tmr0_msr; }
    const size_t &tmr1() const override { return tmr1_msr; }
    const size_t &tmr2() const override { return tmr2_msr; }
    const size_t &tmr3() const override { return tmr3_msr; }
    const size_t &tmr4() const override { return tmr4_msr; }
    const size_t &tmr5() const override { return tmr5_msr; }
    const size_t &tmr6() const override { return tmr6_msr; }
    const size_t &tmr7() const override { return tmr7_msr; }
    const size_t &irr0() const override { return irr0_msr; }
    const size_t &irr1() const override { return irr1_msr; }
    const size_t &irr2() const override { return irr2_msr; }
    const size_t &irr3() const override { return irr3_msr; }
    const size_t &irr4() const override { return irr4_msr; }
    const size_t &irr5() const override { return irr5_msr; }
    const size_t &irr6() const override { return irr6_msr; }
    const size_t &irr7() const override { return irr7_msr; }
    const size_t &esr() const override { return esr_msr; }
    const size_t &icr() const override { return icr_msr; }
    const size_t &timer_lvt() const override { return timer_lvt_msr; }
    const size_t &thermal_lvt() const override { return thermal_lvt_msr; }
    const size_t &performance_lvt() const override { return performance_lvt_msr; }
    const size_t &lint0() const override { return lint0_msr; }
    const size_t &lint1() const override { return lint1_msr; }
    const size_t &evt() const override { return lint1_msr; }
    const size_t &timer_inital_count() const override { return timer_inital_count_msr; }
    const size_t &timer_current_count() const override { return timer_current_count_msr; }
    const size_t &timer_divide_conf() const override { return timer_divide_conf_msr; }
    const size_t &sipi() const { return sipi_msr; }
private:
    static constexpr size_t id_reg_msr = 0x802;
    static constexpr size_t version_reg_msr = 0x803;
    static constexpr size_t tpr_msr = 0x808;
    static constexpr size_t ppr_msr = 0x80a;
    static constexpr size_t eoi_msr = 0x80b;
    static constexpr size_t ldr_msr = 0x80d;
    static constexpr size_t sint_msr = 0x80f;
    static constexpr size_t isr0_msr = 0x810;
    static constexpr size_t isr1_msr = 0x811;
    static constexpr size_t isr2_msr = 0x812;
    static constexpr size_t isr3_msr = 0x813;
    static constexpr size_t isr4_msr = 0x814;
    static constexpr size_t isr5_msr = 0x815;
    static constexpr size_t isr6_msr = 0x816;
    static constexpr size_t isr7_msr = 0x817;
    static constexpr size_t tmr0_msr = 0x818;
    static constexpr size_t tmr1_msr = 0x819;
    static constexpr size_t tmr2_msr = 0x81a;
    static constexpr size_t tmr3_msr = 0x81b;
    static constexpr size_t tmr4_msr = 0x81c;
    static constexpr size_t tmr5_msr = 0x81d;
    static constexpr size_t tmr6_msr = 0x81e;
    static constexpr size_t tmr7_msr = 0x81f;
    static constexpr size_t irr0_msr = 0x820;
    static constexpr size_t irr1_msr = 0x821;
    static constexpr size_t irr2_msr = 0x822;
    static constexpr size_t irr3_msr = 0x823;
    static constexpr size_t irr4_msr = 0x824;
    static constexpr size_t irr5_msr = 0x825;
    static constexpr size_t irr6_msr = 0x826;
    static constexpr size_t irr7_msr = 0x827;
    static constexpr size_t esr_msr = 0x828;
    static constexpr size_t lvt_cmci_msr = 0x82f;
    static constexpr size_t icr_msr = 0x830;
    static constexpr size_t timer_lvt_msr = 0x832;
    static constexpr size_t thermal_lvt_msr = 0x833;
    static constexpr size_t performance_lvt_msr = 0x834;
    static constexpr size_t lint0_msr = 0x835;
    static constexpr size_t lint1_msr = 0x836;
    static constexpr size_t evt_msr = 0x837;
    static constexpr size_t timer_inital_count_msr = 0x838;
    static constexpr size_t timer_current_count_msr = 0x839;
    static constexpr size_t timer_divide_conf_msr = 0x83e;
    static constexpr size_t sipi_msr = 0x83f;
};

class xapic : public xxapic {
public:
    size_t read(size_t reg) const override {
        size_t ret = *reinterpret_cast<volatile uint32_t*>((rdmsr(msr_lapic_base) & 0xfffff000) + vmm::high_vma + reg);
        if(reg == id_reg_off)
            return ret >> 24;
        return ret;
    }

    void write(size_t reg, size_t data) const override {
        *reinterpret_cast<volatile uint32_t*>((rdmsr(msr_lapic_base) & 0xfffff000) + vmm::high_vma + reg) = data;
    }

    void send_ipi(size_t ap, size_t ipi) const override {
        write(icr() + 0x10, (ap << 24));
        write(icr(), ipi);
    }

    const size_t &id_reg() const override { return id_reg_off; }
    const size_t &version_reg() const override { return version_reg_off; }
    const size_t &tpr() const override { return tpr_off; }
    const size_t &ppr() const override { return ppr_off; }
    const size_t &eoi() const override { return eoi_off; }
    const size_t &ldr() const override { return ldr_off; }
    const size_t &sint() const override { return sint_off; }
    const size_t &isr0() const override { return isr0_off; }
    const size_t &isr1() const override { return isr1_off; }
    const size_t &isr2() const override { return isr2_off; }
    const size_t &isr3() const override { return isr3_off; }
    const size_t &isr4() const override { return isr4_off; }
    const size_t &isr5() const override { return isr5_off; }
    const size_t &isr6() const override { return isr6_off; }
    const size_t &isr7() const override { return isr7_off; }
    const size_t &tmr0() const override { return tmr0_off; }
    const size_t &tmr1() const override { return tmr1_off; }
    const size_t &tmr2() const override { return tmr2_off; }
    const size_t &tmr3() const override { return tmr3_off; }
    const size_t &tmr4() const override { return tmr4_off; }
    const size_t &tmr5() const override { return tmr5_off; }
    const size_t &tmr6() const override { return tmr6_off; }
    const size_t &tmr7() const override { return tmr7_off; }
    const size_t &irr0() const override { return irr0_off; }
    const size_t &irr1() const override { return irr1_off; }
    const size_t &irr2() const override { return irr2_off; }
    const size_t &irr3() const override { return irr3_off; }
    const size_t &irr4() const override { return irr4_off; }
    const size_t &irr5() const override { return irr5_off; }
    const size_t &irr6() const override { return irr6_off; }
    const size_t &irr7() const override { return irr7_off; }
    const size_t &esr() const override { return esr_off; }
    const size_t &icr() const override { return icr_off; }
    const size_t &timer_lvt() const override { return timer_lvt_off; }
    const size_t &thermal_lvt() const override { return thermal_lvt_off; }
    const size_t &performance_lvt() const override { return performance_lvt_off; }
    const size_t &lint0() const override { return lint0_off; }
    const size_t &lint1() const override { return lint1_off; }
    const size_t &evt() const override { return lint1_off; }
    const size_t &timer_inital_count() const override { return timer_inital_count_off; }
    const size_t &timer_current_count() const override { return timer_current_count_off; }
    const size_t &timer_divide_conf() const override { return timer_divide_conf_off; }
private:
    static constexpr size_t id_reg_off = 0x20;
    static constexpr size_t version_reg_off = 0x30;
    static constexpr size_t tpr_off = 0x80;
    static constexpr size_t ppr_off = 0xa0;
    static constexpr size_t eoi_off = 0xb0;
    static constexpr size_t ldr_off = 0xd0;
    static constexpr size_t sint_off = 0xf0;
    static constexpr size_t isr0_off = 0x100;
    static constexpr size_t isr1_off = 0x110;
    static constexpr size_t isr2_off = 0x120;
    static constexpr size_t isr3_off = 0x130;
    static constexpr size_t isr4_off = 0x140;
    static constexpr size_t isr5_off = 0x150;
    static constexpr size_t isr6_off = 0x160;
    static constexpr size_t isr7_off = 0x170;
    static constexpr size_t tmr0_off = 0x180;
    static constexpr size_t tmr1_off = 0x190;
    static constexpr size_t tmr2_off = 0x1a0;
    static constexpr size_t tmr3_off = 0x1b0;
    static constexpr size_t tmr4_off = 0x1c0;
    static constexpr size_t tmr5_off = 0x1d0;
    static constexpr size_t tmr6_off = 0x1e0;
    static constexpr size_t tmr7_off = 0x1f0;
    static constexpr size_t irr0_off = 0x200;
    static constexpr size_t irr1_off = 0x210;
    static constexpr size_t irr2_off = 0x220;
    static constexpr size_t irr3_off = 0x230;
    static constexpr size_t irr4_off = 0x240;
    static constexpr size_t irr5_off = 0x250;
    static constexpr size_t irr6_off = 0x260;
    static constexpr size_t irr7_off = 0x270;
    static constexpr size_t esr_off = 0x280;
    static constexpr size_t icr_off = 0x300;
    static constexpr size_t timer_lvt_off = 0x320;
    static constexpr size_t thermal_lvt_off = 0x330;
    static constexpr size_t performance_lvt_off = 0x340;
    static constexpr size_t lint0_off = 0x350;
    static constexpr size_t lint1_off = 0x360;
    static constexpr size_t evt_off = 0x370;
    static constexpr size_t timer_inital_count_off = 0x380;
    static constexpr size_t timer_current_count_off = 0x390;
    static constexpr size_t timer_divide_conf_off = 0x3e0;
};

inline xxapic *lapic = NULL;

void timer_calibrate(uint64_t ms);
void init();

}

#endif
