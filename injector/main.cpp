#include "injection.h"

int main() {
	int ret = 0;
	while (ret != 1) {
		ret = injection::manual_map::inject(injection::manual_map::GetProcId(L"cs2.exe"), L"example.dll");
	}
	return 0;
}