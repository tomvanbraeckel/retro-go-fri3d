/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"
#include "snapshot.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

static void S9xResetCPU (void);
static void S9xSoftResetCPU (void);


static void S9xResetCPU (void)
{
	S9xSoftResetCPU();
	Registers.SL = 0xff;
	Registers.P.W = 0;
	Registers.A.W = 0;
	Registers.X.W = 0;
	Registers.Y.W = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);
}

static void S9xSoftResetCPU (void)
{
	CPU.Cycles = 182; // Or 188. This is the cycle count just after the jump to the Reset Vector.
	CPU.V_Counter = 0;
	CPU.Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
	CPU.PCBase = NULL;
	CPU.NMIPending = FALSE;
	CPU.IRQLine = FALSE;
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
	CPU.FastROMSpeed = SLOW_ONE_CYCLE;
	CPU.InDMA = FALSE;
	CPU.InHDMA = FALSE;
	CPU.InDMAorHDMA = FALSE;
	CPU.InWRAMDMAorHDMA = FALSE;
	CPU.HDMARanInDMA = 0;
	CPU.CurrentDMAorHDMAChannel = -1;
	CPU.WhichEvent = HC_RENDER_EVENT;
	CPU.NextEvent  = SNES_RENDER_START_HC;
	CPU.WaitingForInterrupt = FALSE;
	CPU.AutoSaveTimer = 0;
	CPU.SRAMModified = FALSE;

	Registers.PBPC = 0;
	Registers.PB = 0;
	Registers.PCw = S9xGetWord(0xfffc);
	OpenBus = Registers.PCh;
	Registers.D.W = 0;
	Registers.DB = 0;
	Registers.SH = 1;
	Registers.SL -= 3;
	Registers.XH = 0;
	Registers.YH = 0;

	ICPU.ShiftedPB = 0;
	ICPU.ShiftedDB = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);

	CPU.NMITriggerPos = 0xffff;
	CPU.NextIRQTimer = 0x0fffffff;
	CPU.IRQFlagChanging = IRQ_NONE;

	S9xSetPCBase(Registers.PBPC);

	ICPU.S9xOpcodes = S9xOpcodesE1;
	ICPU.S9xOpLengths = S9xOpLengthsM1X1;

	S9xUnpackStatus();
}

void S9xReset (void)
{
	memset(Memory.RAM, 0x55, 0x20000);
	memset(Memory.VRAM, 0x00, 0x10000);
	memset(Memory.FillRAM + 0x2000, 0, 0x2800);

	S9xResetCPU();
	S9xResetPPU();
	S9xResetDMA();
	S9xResetAPU();

	if (Settings.DSP)
		S9xResetDSP();
}

void S9xSoftReset (void)
{
	memset(Memory.FillRAM + 0x2000, 0, 0x2800);

	S9xSoftResetCPU();
	S9xSoftResetPPU();
	S9xResetDMA();
	S9xSoftResetAPU();

	if (Settings.DSP)
		S9xResetDSP();
}