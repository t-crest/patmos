/* Default NoC configuration for all-to-all communication in a 2x2 bitorus */

const int NOC_CORES = 4;
const int NOC_TABLES = 2;
const int NOC_TIMESLOTS = 5;
const int NOC_DMAS = 4;

const int noc_init_array[NOC_CORES][NOC_TABLES][NOC_TIMESLOTS] = {
	{
		{28,0,20,24,0},
		{0,13,8,52}
	},

	{
		{24,4,16,28,4},
		{13,0,52,8}
	},

	{
		{20,8,28,16,8},
		{8,52,0,13}
	},

	{
		{16,12,24,20,12},
		{52,8,13,0}
	},
};
