#pragma once
#include <stdint.h>
#include <Windows.h>
#include <fstream>
#include <filesystem>

#define READ_BIT(d, n) (d & (1 << n) ? 1 : 0)
