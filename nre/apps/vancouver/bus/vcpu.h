/** @file
 * External Virtual CPU interface.
 *
 * Copyright (C) 2010, Bernhard Kauer <bk@vmmon.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of Vancouver.
 *
 * Vancouver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Vancouver is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */
#pragma once

#include "bus.h"
#include "../executor/cpustate.h"
#include "message.h"

struct CpuMessage {
    enum Type {
        TYPE_CPUID_WRITE,
        TYPE_CPUID,
        TYPE_RDTSC,
        TYPE_RDMSR,
        TYPE_WRMSR,
        TYPE_IOIN,
        TYPE_IOOUT,
        TYPE_TRIPLE,
        TYPE_INIT,
        TYPE_HLT,
        TYPE_INVD,
        TYPE_WBINVD,
        TYPE_CHECK_IRQ,
        TYPE_CALC_IRQWINDOW,
        TYPE_SINGLE_STEP
    } type;
    union {
        struct {
            CpuState *cpu;
            union {
                unsigned cpuid_index;
                struct {
                    unsigned io_order;
                    unsigned short port;
                    void *dst;
                };
            };
        };
        struct {
            unsigned nr;
            unsigned reg;
            unsigned mask;
            unsigned value;
        };
    };
    unsigned mtr_in;
    unsigned mtr_out;
    unsigned consumed; //info whether a model consumed this event

    CpuMessage(Type _type, CpuState *_cpu, unsigned _mtr_in)
        : type(_type), cpu(_cpu), mtr_in(_mtr_in), mtr_out(0), consumed(0) {
        if(type == TYPE_CPUID)
            cpuid_index = cpu->rax;
    }
    CpuMessage(unsigned _nr, unsigned _reg, unsigned _mask, unsigned _value)
        : type(TYPE_CPUID_WRITE), nr(_nr), reg(_reg), mask(_mask), value(_value), consumed(0) {
    }
    CpuMessage(bool is_in, CpuState *_cpu, unsigned _io_order, unsigned _port, void *_dst,
               unsigned _mtr_in)
        : type(is_in ? TYPE_IOIN : TYPE_IOOUT), cpu(_cpu), io_order(_io_order), port(_port), dst(
              _dst), mtr_in(_mtr_in), mtr_out(0), consumed(0) {
    }
};

struct CpuEvent {
    unsigned value;

    CpuEvent(unsigned _value) : value(_value) {
    }
};

struct LapicEvent {
    enum Type {
        INTA, RESET, INIT
    } type;
    unsigned value;

    LapicEvent(Type _type) : type(_type) {
        if(type == INTA)
            value = ~0u;
    }
};

class VCVCpu {
    VCVCpu *_last;
public:
    DBus<CpuMessage> executor;
    DBus<CpuEvent> bus_event;
    DBus<LapicEvent> bus_lapic;
    DBus<MessageMem> mem;
    DBus<MessageMemRegion> memregion;
    uint64_t inj_count;

    VCVCpu *get_last() {
        return _last;
    }
    bool is_ap() {
        return _last;
    }

    bool set_cpuid(unsigned nr, unsigned reg, unsigned value, unsigned mask = ~0) {
        CpuMessage msg(nr, reg, ~mask, value & mask);
        return executor.send(msg);
    }
    enum {
        EVENT_INTR = 1 << 0,
        EVENT_FIXED = 1 << 0,
        EVENT_LOWEST = 1 << 1,
        EVENT_SMI = 1 << 2,
        EVENT_RRD = 1 << 3,
        EVENT_RESET = 1 << 3,
        EVENT_NMI = 1 << 4,
        EVENT_INIT = 1 << 5,
        EVENT_SIPI = 1 << 6,
        EVENT_EXTINT = 1 << 7,
        EVENT_MASK = 0x0ff,
        // SIPI vector: bits 8-15
        DEASS_INTR = 1 << 16,
        EVENT_DEBUG = 1 << 17,
        STATE_BLOCK = 1 << 18,
        STATE_WAKEUP = 1 << 19,
        EVENT_HOST = 1 << 20
    };

    VCVCpu(VCVCpu *last) : _last(last), executor(), bus_event(), bus_lapic(), mem(), memregion(),
                           inj_count(0) {
    }
};
