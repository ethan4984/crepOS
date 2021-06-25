#ifndef RSDT_HPP_
#define RSDT_HPP_

#include <acpi/tables.hpp>
#include <mm/vmm.hpp>
#include <memutils.hpp>

inline rsdp *rsdp_ptr = NULL;
inline rsdt *rsdt_ptr = NULL;
inline xsdt *xsdt_ptr = NULL;

template <typename T>
T *find_SDT(const char *signature) {
    if(xsdt_ptr != NULL) {
        for(size_t i = 0; i < (xsdt_ptr->acpihdr_ptr.length - sizeof(acpihdr)); i++) {
            acpihdr *acpihdr_ptr = reinterpret_cast<acpihdr*>(xsdt_ptr->acpiptr[i] + vmm::high_vma);
            if(strncmp(acpihdr_ptr->signature, signature, 4) == 0) {
                print("[ACPI] {s} found\n", signature);
                return reinterpret_cast<T*>(acpihdr_ptr);
            }
        }
    } 

    if(rsdt_ptr != NULL) {
        for(size_t i = 0; i < (rsdt_ptr->acpihdr_ptr.length - sizeof(acpihdr)); i++) {
            acpihdr *acpihdr_ptr = reinterpret_cast<acpihdr*>(rsdt_ptr->acpiptr[i] + vmm::high_vma);
            if(strncmp(acpihdr_ptr->signature, signature, 4) == 0) {
                print("[ACPI] {s} found\n", signature);
                return reinterpret_cast<T*>(acpihdr_ptr);
            }
        }
    }

    print("[ACPI] {s} not found\n", signature);

    return NULL;
}

#endif
