/* vm.c - the arithmetic stack of a virtual machine.
 *
 * Module begun 2008-02-22 by Rainer Gerhards
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

#include "config.h"
#include <stdlib.h>
#include <assert.h>

#include "rsyslog.h"
#include "obj.h"
#include "vm.h"

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(vmstk)


/* ------------------------------ instruction set implementation ------------------------------ *
 * The following functions implement the VM's instruction set.
 */
#define BEGINop(instruction) \
	static rsRetVal op##instruction(vm_t *pThis, vmop_t *pOp) \
	{ \
		DEFiRet;

#define CODESTARTop(instruction) \
		ISOBJ_TYPE_assert(pThis, vm); \
		ISOBJ_TYPE_assert(pOp, vmop);

#define ENDop(instruction) \
		RETiRet; \
	}

BEGINop(OR) /* remember to set the instruction also in the ENDop macro! */
	// var_t *pVar[2];
	var_t *operand1;
	var_t *operand2;
CODESTARTop(OR)
	// Pop(2, &pVar); --> should also do the conversion outlined below
	vmstk.Pop(pThis->pStk, &operand1);
	vmstk.Pop(pThis->pStk, &operand2);
	// TODO: implement
	/* We must check data types and find out on which data type the
	 * operation needs to be performed. This may result in some data types
	 * being converted.
	 *
	 * Current school of thought:
	 * op1		op2	operation data type
	 * string	string	string
	 * string	number	number if op1 can be converted to number, string else
	 * date		string  date if op1 can be converted to date, string else
	 * number	number  number
	 * date		number	string (maybe we can do better?)
	 * date		date	date
	 *
	 * If a boolean value is required, we need to have a number inside the
	 * operand. If it is not, conversion rules to number apply. Once we
	 * have a number, things get easy: 0 is false, anything else is true.
	 * Please note that due to this conversion rules, "0" becomes false
	 * while "-4712" becomes true. Using a date as boolen is not a good
	 * idea. Depending on the ultimate conversion rules, it may always
	 * become true or false. As such, using dates as booleans is
	 * prohibited and the result defined to be undefined.
	 */
	vmstk.Push(pThis->pStk, operand1); /* result */
ENDop(OR)

BEGINop(AND) /* remember to set the instruction also in the ENDop macro! */
	var_t *operand1;
	var_t *operand2;
CODESTARTop(AND)
	vmstk.Pop(pThis->pStk, &operand1);
	vmstk.Pop(pThis->pStk, &operand2);
	// TODO: implement
	vmstk.Push(pThis->pStk, operand1); /* result */
ENDop(AND)

BEGINop(POP) /* remember to set the instruction also in the ENDop macro! */
CODESTARTop(POP)
/* TODO: implement */
ENDop(POP)

BEGINop(PUSHCONSTANT) /* remember to set the instruction also in the ENDop macro! */
CODESTARTop(PUSHCONSTANT)
	vmstk.Push(pThis->pStk, pOp->operand.pVar);
ENDop(PUSHCONSTANT)


/* ------------------------------ end instruction set implementation ------------------------------ */


/* Standard-Constructor
 */
BEGINobjConstruct(vm) /* be sure to specify the object type also in END macro! */
ENDobjConstruct(vm)


/* ConstructionFinalizer
 * rgerhards, 2008-01-09
 */
static rsRetVal
vmConstructFinalize(vm_t __attribute__((unused)) *pThis)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, vm);
	RETiRet;
}


/* destructor for the vm object */
BEGINobjDestruct(vm) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(vm)
ENDobjDestruct(vm)


/* debugprint for the vm object */
BEGINobjDebugPrint(vm) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDebugPrint(vm)
	dbgoprint((obj_t*) pThis, "rsyslog virtual machine, currently no state info available\n");
ENDobjDebugPrint(vm)


/* execute a program
 */
static rsRetVal
execProg(vm_t *pThis, vmprg_t *pProg)
{
	DEFiRet;
	vmop_t *pCurrOp; /* virtual instruction pointer */

	ISOBJ_TYPE_assert(pThis, vm);
	ISOBJ_TYPE_assert(pProg, vmprg);

#define doOP(OP) case opcode_##OP: CHKiRet(op##OP(pThis, pCurrOp)); break
	pCurrOp = pProg->vmopRoot; /* TODO: do this via a method! */
	while(pCurrOp != NULL && pCurrOp->opcode != opcode_END_PROG) {
		switch(pCurrOp->opcode) {
			doOP(OR);
			doOP(AND);
			doOP(POP);
			doOP(PUSHCONSTANT);
			default:
				ABORT_FINALIZE(RS_RET_INVALID_VMOP);
				dbgoprint((obj_t*) pThis, "invalid instruction %d in vmprg\n", pCurrOp->opcode);
				break;
		}
		/* so far, we have plain sequential execution, so on to next... */
		pCurrOp = pCurrOp->pNext;
	}
#undef doOP

	/* if we reach this point, our program has intintionally terminated
	 * (no error state).
	 */

finalize_it:
	RETiRet;
}



/* queryInterface function
 * rgerhards, 2008-02-21
 */
BEGINobjQueryInterface(vm)
CODESTARTobjQueryInterface(vm)
	if(pIf->ifVersion != vmCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->oID = OBJvm;

	pIf->Construct = vmConstruct;
	pIf->ConstructFinalize = vmConstructFinalize;
	pIf->Destruct = vmDestruct;
	pIf->DebugPrint = vmDebugPrint;
	pIf->ExecProg = execProg;
finalize_it:
ENDobjQueryInterface(vm)


/* Initialize the vm class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINObjClassInit(vm, 1) /* class, version */
	/* request objects we use */
	CHKiRet(objUse(vmstk));

	/* set our own handlers */
	OBJSetMethodHandler(objMethod_DEBUGPRINT, vmDebugPrint);
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, vmConstructFinalize);
ENDObjClassInit(vm)

/* vi:set ai:
 */
