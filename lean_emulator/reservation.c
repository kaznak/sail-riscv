#include <lean/lean.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Reservation implementation for LR/SC (Load-Reserved / Store-Conditional).
 *
 * These functions are called via @[extern] from Lean. Using @[extern] rather
 * than @[implemented_by] is critical: the Lean compiler will dead-code-eliminate
 * @[implemented_by] functions that return Unit or constant Bool, because their
 * safe stubs have known values. @[extern] foreign calls cannot be eliminated.
 */

/* Global reservation state */
static int    g_reservation_valid = 0;
static size_t g_reservation_addr  = 0;

/* Align to 8-byte boundaries (matching C++ emulator) */
#define RESERVATION_ADDR_MASK ((size_t)0xFFFFFFFFFFFFFFF8ULL)

static inline size_t addr_to_nat(b_lean_obj_arg addr_nat) {
    if (lean_is_scalar(addr_nat)) {
        return lean_unbox(addr_nat);
    } else {
        return (size_t)lean_uint64_of_nat(addr_nat);
    }
}

/*
 * load_reservation_st(addr : @& Nat, width : @& Nat, st : @& RegisterState) : Unit
 * Called by LR instruction. Sets the reservation for the given address.
 * Takes RegisterState as borrowed arg to prevent lean_obj_once caching.
 */
LEAN_EXPORT lean_obj_res lean_sail_load_reservation_st(
    b_lean_obj_arg addr_nat, b_lean_obj_arg width_nat, b_lean_obj_arg st) {
    (void)st; /* unused — only here to prevent constant-folding */
    g_reservation_addr  = addr_to_nat(addr_nat) & RESERVATION_ADDR_MASK;
    g_reservation_valid = 1;
    /* fprintf(stderr, "[RESV] load_reservation addr=0x%lx\n", g_reservation_addr); */
    return lean_box(0); /* Unit */
}

/*
 * match_reservation(addr : Nat) : Bool
 * Called by SC instruction. Returns true if the given address matches the reservation.
 */
LEAN_EXPORT uint8_t lean_sail_match_reservation(b_lean_obj_arg addr_nat) {
    size_t addr = addr_to_nat(addr_nat) & RESERVATION_ADDR_MASK;
    uint8_t result = g_reservation_valid && (g_reservation_addr == addr);
    /* fprintf(stderr, "[RESV] match_reservation ...\n"); */
    return result;
}

/*
 * cancel_reservation_st(st : RegisterState) : RegisterState
 * Called after SC (success or fail) and on context switches.
 * Takes and returns RegisterState (owned) to prevent lean_obj_once caching.
 */
LEAN_EXPORT lean_obj_res lean_sail_cancel_reservation_st(lean_obj_arg st) {
    /* fprintf(stderr, "[RESV] cancel_reservation ...\n"); */
    g_reservation_valid = 0;
    return st; /* pass state through unchanged */
}

/*
 * valid_reservation() : Bool
 * Returns whether any reservation is currently active.
 */
LEAN_EXPORT uint8_t lean_sail_valid_reservation(lean_obj_arg unused) {
    return g_reservation_valid;
}
