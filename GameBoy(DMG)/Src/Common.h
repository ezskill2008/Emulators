#pragma once
#include <stdint.h>
#include <Windows.h>
#include <fstream>
#include <filesystem>

#define BIT_READ(a, n) ((a & (1 << n)) ? 1 : 0)
#define BIT_SET(a, n, on) { if (on) a |= (1 << n); else a &= ~(1 << n);}
