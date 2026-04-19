#include "CompileRegs.h"

namespace compile_regs {
static int g_next = 0;

void reset() { g_next = 0; }

int newReg() { return g_next++; }
}
