#pragma once

#include "sdk/vsm.hpp"

#include <z80emu.h>

using namespace llz80emu;

class DsimModel : public IDSIMMODEL
{
public:
	DsimModel();

	/* IDSIMMODEL methods */
	INT isdigital(CHAR* pinname);
	VOID setup(IINSTANCE* inst, IDSIMCKT* dsim);
	VOID runctrl(RUNMODES mode);
	VOID actuate(REALTIME time, ACTIVESTATE newstate);
	BOOL indicate(REALTIME time, ACTIVEDATA* data);
	VOID simulate(ABSTIME time, DSIMMODES mode);
	VOID callback(ABSTIME time, EVENTID eventid);
private:
	IINSTANCE* _inst;
	IDSIMCKT* _ckt;

	/* pins to be connected */
	struct {
		IDSIMPIN* addr[16]; // address pins
		IDSIMPIN* data[8]; // data pins
		IDSIMPIN* m1;
		IDSIMPIN* mreq;
		IDSIMPIN* iorq;
		IDSIMPIN* rd;
		IDSIMPIN* wr;
		IDSIMPIN* rfsh;
		IDSIMPIN* halt;
		IDSIMPIN* wait;
		IDSIMPIN* intr; // to avoid conflict with int
		IDSIMPIN* nmi;
		IDSIMPIN* reset;
		IDSIMPIN* busrq;
		IDSIMPIN* busack;
		IDSIMPIN* clk;
	} _pins;

#if defined(_DEBUG)
	IDEBUGPOPUP* _debug_popup; // for showing debug messages (hidden for release build)
#endif

	VOID update_pin(ABSTIME time, IDSIMPIN* pin, const z80_pins_t& pins, int bit);
	VOID update_pins(ABSTIME time, const z80_pins_t& pins);
	z80_pinbits_t get_pin_state();

	VOID clk_pin_handler(ABSTIME time, DSIMMODES mode);
	VOID nmi_pin_handler(ABSTIME time, DSIMMODES mode);

	z80emu _emu;
	bool _clk_set = false; // set when we've determined the initial clock state
};
