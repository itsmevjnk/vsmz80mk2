#include "DsimModel.h"
#include <stdio.h>

#define DC(s) (CHAR*)(s) // convert const char* (e.g. string literal) to CHAR* (VSM SDK string) - NOTE: ugly code!

/* debug printing */
#if defined(DEBUG)
#define DEBUG_LOG(...) _debug_popup->print(__VA_ARGS__); _debug_popup->print((CHAR*)"\n")
#else
#define DEBUG_LOG(...)
#endif

DsimModel::DsimModel() : _emu(false) {

}

INT DsimModel::isdigital(CHAR* pinname) {
	return TRUE;											// Indicates all the pins are digital
}

VOID DsimModel::setup(IINSTANCE* instance, IDSIMCKT* dsimckt) {
	//int n;
	//char s[8];

	_inst = instance;
	_ckt = dsimckt;

#if defined(DEBUG) // create debug popup window
	CREATEPOPUPSTRUCT* cps = new CREATEPOPUPSTRUCT;
	cps->caption = DC("Z80 Simulator Debugger Log");			// WIN Header
	cps->flags = PWF_VISIBLE | PWF_SIZEABLE;				// Show + Size
	cps->type = PWT_DEBUG;									// WIN DEBUG
	cps->height = 500;
	cps->width = 400;
	cps->id = 123;
	_debug_popup = (IDEBUGPOPUP*)instance->createpopup(cps);
#endif

	/* connect pins */
	DEBUG_LOG(DC("Connecting pins..."));
	char temp_buf[4]; // temporary buffer for address and data pin names
	for(int i = 0; i < 16; i++) {
		sprintf_s(temp_buf, "A%d", i); // NOTE: we can probably get away with not using snprintf
		_pins.addr[i] = _inst->getdsimpin(temp_buf, true);
	}
	for(int i = 0; i < 8; i++) {
		sprintf_s(temp_buf, "D%d", i);
		_pins.data[i] = _inst->getdsimpin(temp_buf, true);
	}
	_pins.m1 = _inst->getdsimpin(DC("$M1$"), true);
	_pins.mreq = _inst->getdsimpin(DC("$MREQ$"), true);
	_pins.iorq = _inst->getdsimpin(DC("$IORQ$"), true);
	_pins.rd = _inst->getdsimpin(DC("$RD$"), true);
	_pins.wr = _inst->getdsimpin(DC("$WR$"), true);
	_pins.rfsh = _inst->getdsimpin(DC("$RFSH$"), true);
	_pins.halt = _inst->getdsimpin(DC("$HALT$"), true);
	_pins.wait = _inst->getdsimpin(DC("$WAIT$"), true);
	_pins.intr = _inst->getdsimpin(DC("$INT$"), true);
	_pins.nmi = _inst->getdsimpin(DC("$NMI$"), true);
	_pins.reset = _inst->getdsimpin(DC("$RESET$"), true);
	_pins.busrq = _inst->getdsimpin(DC("$BUSRQ$"), true);
	_pins.busack = _inst->getdsimpin(DC("$BUSAK$"), true);
	_pins.clk = _inst->getdsimpin(DC("CLK"), true);

	update_pins(0, _emu.get_pins()); // initialise pins

	/* attach pin handlers for special pins */
	_pins.clk->sethandler(this, (PINHANDLERFN)&DsimModel::clk_pin_handler);
	_pins.nmi->sethandler(this, (PINHANDLERFN)&DsimModel::nmi_pin_handler);
}

VOID DsimModel::runctrl(RUNMODES mode) {

}

VOID DsimModel::actuate(REALTIME time, ACTIVESTATE newstate) {

}

BOOL DsimModel::indicate(REALTIME time, ACTIVEDATA* data) {
	return FALSE;
}

VOID DsimModel::simulate(ABSTIME time, DSIMMODES mode) {

}

VOID DsimModel::callback(ABSTIME time, EVENTID eventid) {

}

VOID DsimModel::update_pin(ABSTIME time, IDSIMPIN* pin, const z80_pins_t& pins, int bit) {
	if (pins.dir & (1ULL << bit)) pin->setstate(time, 1, (pins.state & (1ULL << bit)) ? SHI : SLO);
	else pin->setstate(time, 1, FLT);
}

VOID DsimModel::update_pins(ABSTIME time, const z80_pins_t& pins) {
	for (int i = 0; i < 16; i++) update_pin(time, _pins.addr[i], pins, Z80_PIN_A_BASE + i);
	for (int i = 0; i < 8; i++) update_pin(time, _pins.data[i], pins, Z80_PIN_D_BASE + i);
	update_pin(time, _pins.m1, pins, Z80_PIN_M1);
	update_pin(time, _pins.mreq, pins, Z80_PIN_MREQ);
	update_pin(time, _pins.iorq, pins, Z80_PIN_IORQ);
	update_pin(time, _pins.rd, pins, Z80_PIN_RD);
	update_pin(time, _pins.wr, pins, Z80_PIN_WR);
	update_pin(time, _pins.rfsh, pins, Z80_PIN_RFSH);
	update_pin(time, _pins.halt, pins, Z80_PIN_HALT);
	update_pin(time, _pins.wait, pins, Z80_PIN_WAIT);
	update_pin(time, _pins.intr, pins, Z80_PIN_INT);
	// update_pin(time, _pins.nmi, pins, Z80_PIN_NMI);
	update_pin(time, _pins.reset, pins, Z80_PIN_RESET);
	update_pin(time, _pins.busrq, pins, Z80_PIN_BUSREQ);
	update_pin(time, _pins.busack, pins, Z80_PIN_BUSACK);
}

z80_pinbits_t DsimModel::get_pin_state() {
	z80_pinbits_t state = 0; // we only need to care about input pins
	z80_pins_t pins = _emu.get_pins();
	if (!(pins.dir & Z80_D0)) {
		/* data bus is input/floating */
		for(int i = 0; i < 8; i++) state |= (z80_pinbits_t)ishigh(_pins.data[i]->istate()) << (Z80_PIN_D_BASE + i);
	}
	state |= (z80_pinbits_t)ishigh(_pins.busrq->istate()) << Z80_PIN_BUSREQ;
	state |= (z80_pinbits_t)ishigh(_pins.intr->istate()) << Z80_PIN_INT;
	// state |= (z80_pinbits_t)ishigh(_pins.nmi->istate()) << Z80_PIN_NMI;
	state |= (z80_pinbits_t)ishigh(_pins.reset->istate()) << Z80_PIN_RESET;
	state |= (z80_pinbits_t)ishigh(_pins.wait->istate()) << Z80_PIN_WAIT;
	return state;
}

VOID DsimModel::clk_pin_handler(ABSTIME time, DSIMMODES mode) {
	bool pos = _pins.clk->isposedge(), neg = _pins.clk->isnegedge();
	if (pos || neg) {
		if (!_clk_set) {
			_emu.set_clkpin(!pos); // we'll go high/low now
			_clk_set = true;
		}
		update_pins(time, _emu.clock(get_pin_state())); // get input pin state, clock CPU, then update our pins
	}

#if defined(DEBUG)
	if (pos) {
		z80_registers_t regs = _emu.get_regs();
		DEBUG_LOG(DC("AF=%04X BC=%04X DE=%04X HL=%04X"), regs.REG_AF, regs.REG_BC, regs.REG_DE, regs.REG_HL);
		DEBUG_LOG(DC("AF'=%04X BC'=%04X DE'=%04X HL'=%04X"), regs.REG_AF_S, regs.REG_BC_S, regs.REG_DE_S, regs.REG_HL_S);
		DEBUG_LOG(DC("PC=%04X SP=%04X IX=%04X IY=%04X I=%02X R=%02X"), regs.REG_PC, regs.REG_SP, regs.REG_IX, regs.REG_IY, regs.REG_I, regs.REG_R);
		DEBUG_LOG(DC("Instr: 0x%02X"), regs.instr);
	}
#endif
}

VOID DsimModel::nmi_pin_handler(ABSTIME time, DSIMMODES mode) {
	if (_pins.nmi->isnegedge()) {
		DEBUG_LOG(DC("NMI triggered"));
		_emu.trigger_nmi();
	}
}
