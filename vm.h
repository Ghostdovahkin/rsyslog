/* The vm object.
 *
 * This implements the rsyslog virtual machine. The initial implementation is
 * done to support complex user-defined expressions, but it may evolve into a
 * much more useful thing over time.
 *
 * The virtual machine uses rsyslog variables as its memory storage system.
 * All computation is done on a stack (vmstk). The vm supports a given
 * instruction set and executes programs of type vmprg, which consist of
 * single operations defined in vmop (which hold the instruction and the
 * data).
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#ifndef INCLUDED_VM_H
#define INCLUDED_VM_H

#include "vmstk.h"
#include "vmprg.h"

/* the vm object */
typedef struct vm_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	vmstk_t *pStk;		/* The stack */
} vm_t;


/* interfaces */
BEGINinterface(vm) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(vm);
	rsRetVal (*Construct)(vm_t **ppThis);
	rsRetVal (*ConstructFinalize)(vm_t __attribute__((unused)) *pThis);
	rsRetVal (*Destruct)(vm_t **ppThis);
	rsRetVal (*ExecProg)(vm_t *pThis, vmprg_t *pProg);
ENDinterface(vm)
#define vmCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */


/* prototypes */
PROTOTYPEObj(vm);

#endif /* #ifndef INCLUDED_VM_H */
