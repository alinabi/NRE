/*
 * TODO comment me
 *
 * Copyright (C) 2012, Nils Asmussen <nils@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of NUL.
 *
 * NUL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NUL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#pragma once

namespace nul {

class Ports {
public:
	template<typename T>
	static inline T in(unsigned port) {
		T val;
		asm volatile ("in %w1, %0" : "=a" (val) : "Nd" (port));
		return val;
	}

    template<typename T>
	static inline void out(unsigned port,T val) {
		asm volatile ("out %0, %w1" : : "a" (val), "Nd" (port));
	}

    static void request(unsigned start,unsigned count) {
    	UtcbFrame uf;
    }

private:
    Ports();
    ~Ports();
    Ports(const Ports&);
    Ports& operator=(const Ports&);
};

}
