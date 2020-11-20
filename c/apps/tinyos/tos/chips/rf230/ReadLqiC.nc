configuration ReadLqiC {
	provides interface ReadLqi;
} implementation {
	components RF230RadioC;
	ReadLqi = RF230RadioC;
}
