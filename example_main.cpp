#include "ui.hpp"

//This is the actual struct name :) lol
SomeOutput main_out(std::vector<long double> in) {
	return {0, std::accumulate(in.begin(), in.end(), 0.0f) / in.size(), o_PURE };
}

long double dti_1_linker(std::vector<long double> a) {
	//a will just have 1 open price
	return a[0];
}
long double dti_2_linker(std::vector<long double> a) {
	//a will just have 1 close price
	return a[0];
}

int main(int, char**)
{
	DT_Input open = { OPEN, 1, 0 };
	DT_Input close = { CLOSE, 1, 0 };

	DataType* dt_open = new DataType(dti_1_linker, { open });
	DataType* dt_close = new DataType(dti_2_linker, { close });

	Test* nt = new Test(main_out, {dt_open, dt_close});

	init(nt, { {100, 101, 102}, {101, 102, 103} }, {}, { {{0,105,o_PURE}, {0, 106, o_PURE}} });
}
