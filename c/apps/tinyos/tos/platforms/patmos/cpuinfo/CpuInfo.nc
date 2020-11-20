interface CpuInfo 
{ 
	async command uint getCoreID();
	async command uint getClkFreq();
	async command uint getCoreCount();
	async command uint getFeatures();
	async command uint getExtMemSize();
	async command uint getExtMemConfVal();
	async command extMemConf_t getExtMemConf();
	async command uint getICacheSize();
	async command uint getICacheConfVal();
	async command chacheConf_t getICacheConf();
	async command uint getDCacheSize();
	async command uint getDCacheConfVal();
	async command chacheConf_t getDCacheConf();
	async command uint getSCacheSize();
	async command uint getSCacheConf();
	async command uint getISPMSize();
	async command uint getDSPMSize();

	async command uint getBootSrcStart();
	async command uint getBootSrcSize();
	async command uint getBootDstStart();
	async command uint getBootDstSize();
}