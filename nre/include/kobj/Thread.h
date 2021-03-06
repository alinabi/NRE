/*
 * Copyright (C) 2012, Nils Asmussen <nils@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of NRE (NOVA runtime environment).
 *
 * NRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NRE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#pragma once

#include <arch/ExecEnv.h>
#include <kobj/Ec.h>
#include <collection/SList.h>
#include <util/Atomic.h>
#include <Syscalls.h>

namespace nre {

class Pd;
class RCU;
class RCULock;
class Utcb;

/**
 * Represents a thread, i.e. an Ec that has a stack and a Utcb. It is the base class for the two
 * supported thread variants, LocalThread and GlobalThread. This class can't be used directly.
 * Note that each Thread contains a few slots for thread local storage (TLS). The index
 * Thread::TLS_PARAM is always available, e.g. to pass a parameter to a Thread. You may create
 * additional ones by Thread::create_tls().
 */
class Thread : public Ec, public SListItem {
    friend class RCU;
    friend class RCULock;

    static const size_t TLS_SIZE    = 4;

public:
    // the slot 0 is reserved for putting a ec-parameter in it
    static const size_t TLS_PARAM   = 0;
    enum Flags {
        HAS_OWN_STACK   = 1,
        HAS_OWN_UTCB    = 2,
    };

    /**
     * @return the current execution context
     */
    static Thread *current() {
        return ExecEnv::get_current_thread();
    }

protected:
    /**
     * Constructor to create a new Ec
     *
     * @param pd the protection domain the Thread should run in
     * @param type the type of Thread
     * @param start the startup function
     * @param ret the return address
     * @param cpu the logical cpu to bind the Thread to
     * @param evb the offset for the event-portals
     * @param cap the capability (INVALID if a new one should be used)
     * @param stack the stack address (0 = create one automatically)
     * @param uaddr the utcb address (0 = create one automatically)
     */
    explicit Thread(Pd *pd, Syscalls::ECType type, ExecEnv::startup_func start, uintptr_t ret,
                    cpu_t cpu, capsel_t evb, uintptr_t stack = 0, uintptr_t uaddr = 0);

    /**
     * Constructor for the first global thread
     *
     * @param cpu the logical cpu to bind the Thread to
     * @param evb the offset for the event-portals
     * @param cap the capability
     * @param stack the stack address
     * @param uaddr the utcb address
     */
    explicit Thread(cpu_t cpu, capsel_t evb, capsel_t cap, uintptr_t stack, uintptr_t uaddr);

public:
    /**
     * Destructor
     */
    virtual ~Thread();

    /**
     * @return the flags
     */
    uint flags() const {
        return _flags;
    }
    /**
     * @return the stack-address
     */
    uintptr_t stack() const {
        return _stack_addr;
    }
    /**
     * @return the utcb
     */
    Utcb *utcb() {
        return reinterpret_cast<Utcb*>(_utcb_addr);
    }

    /**
     * Creates a new TLS slot for all Threads
     *
     * @return the TLS index
     */
    size_t create_tls() {
        size_t next = Atomic::add(&_tls_idx, +1);
        assert(next < TLS_SIZE);
        return next;
    }
    /**
     * @param idx the TLS index
     * @return the value at given index
     */
    template<typename T>
    T get_tls(size_t idx) const {
        assert(idx < TLS_SIZE);
        return reinterpret_cast<T>(_tls[idx]);
    }
    /**
     * @param idx the TLS index
     * @param val the new value
     */
    template<typename T>
    void set_tls(size_t idx, T val) {
        assert(idx < TLS_SIZE);
        _tls[idx] = reinterpret_cast<void*>(val);
    }

private:
    Thread(const Thread&);
    Thread& operator=(const Thread&);

    static capsel_t create(Thread *t, Pd *pd, Syscalls::ECType type, cpu_t cpu, capsel_t evb,
                           ExecEnv::startup_func start, uintptr_t ret, uintptr_t &uaddr,
                           uintptr_t &stack, uint &flags);

    uint32_t _rcu_counter;
    uintptr_t _utcb_addr;
    uintptr_t _stack_addr;
    uint _flags;
    void *_tls[TLS_SIZE];
    static size_t _tls_idx;
};

}
