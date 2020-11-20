module PatmosCpuInfoP {
  provides interface CpuInfo;
}

implementation {
  	async command uint CpuInfo.getCoreID()
  	{
  		return CPU_INFO_CORE_ID;
  	}
	async command uint CpuInfo.getClkFreq()
  	{
  		return CPU_INFO_FREQ;
  	}
	async command uint CpuInfo.getCoreCount()
	{
  		return CPU_INFO_NUM_CORES;
  	}
	async command uint CpuInfo.getFeatures()
	{
  		return CPU_INFO_FEATURES;
  	}
	async command uint CpuInfo.getExtMemSize()
	{
  		return CPU_INFO_EXTMEM_SIZE;
  	}
	async command extMemConf_t CpuInfo.getExtMemConf()
	{
  		uint temp = CPU_INFO_EXTMEM_CONF;
		extMemConf_t conf;

		conf.burstLength = (temp & EXTMEM_BURST_MASK) >> EXTMEM_BURST_SHIFT;
		conf.combinedWritesToExtMem = temp & EXTMEM_COMBINEDWRITES_MASK;
  		return conf;
  	}
  	async command uint CpuInfo.getExtMemConfVal()
  	{
  		return CPU_INFO_EXTMEM_CONF;
  	}
	async command uint CpuInfo.getICacheSize()
	{
  		return CPU_INFO_ICACHE_SIZE;
  	}
	async command chacheConf_t CpuInfo.getICacheConf()
	{
		uint temp = CPU_INFO_ICACHE_CONF;
		chacheConf_t conf;

		conf.cacheType = (temp & CACHE_TYPE_MASK) >> CACHE_TYPE_SHIFT;
		conf.replacementPolicy = (temp & CACHE_POLICY_MASK) >> CACHE_POLICY_SHIFT;
		conf.associativity = temp & CACHE_ASSOCIATIVITY_MASK;
  		return conf;
  	}
  	async command uint CpuInfo.getICacheConfVal()
	{
		return CPU_INFO_ICACHE_CONF;
	}
	async command uint CpuInfo.getDCacheSize()
	{
  		return CPU_INFO_DCACHE_SIZE;
  	}
	async command chacheConf_t CpuInfo.getDCacheConf()
	{
  		uint temp = CPU_INFO_DCACHE_CONF;
		chacheConf_t conf;

		conf.cacheType = (temp & CACHE_TYPE_MASK) >> CACHE_TYPE_SHIFT;
		conf.replacementPolicy = (temp & CACHE_POLICY_MASK) >> CACHE_POLICY_SHIFT;
		conf.associativity = temp & CACHE_ASSOCIATIVITY_MASK;
  		return conf;
  	}
	async command uint CpuInfo.getDCacheConfVal()
	{
		return CPU_INFO_DCACHE_CONF;
	}
	async command uint CpuInfo.getSCacheSize()
	{
  		return CPU_INFO_SCACHE_SIZE;
  	}
	async command uint CpuInfo.getSCacheConf()
	{
  		return CPU_INFO_SCACHE_CONF;
  	}
	async command uint CpuInfo.getISPMSize()
	{
  		return CPU_INFO_ISPM_SIZE;
  	}
	async command uint CpuInfo.getDSPMSize()
	{
  		return CPU_INFO_DSPM_SIZE;
  	}

  	async command uint CpuInfo.getBootSrcStart()
  	{
  		return CPU_INFO_BOOT_SRC_START;
  	}
	async command uint CpuInfo.getBootSrcSize()
	{
		return CPU_INFO_BOOT_SRC_SIZE;
	}
	async command uint CpuInfo.getBootDstStart()
	{
		return CPU_INFO_BOOT_DST_START;
	}
	async command uint CpuInfo.getBootDstSize()
	{
		return CPU_INFO_BOOT_DST_SIZE;
	}
}
