module PatmosMMUP {
  provides interface MemoryManagementUnit;
}

implementation 
{
	void writeToMMU(volatile _IODEV int* const mmu_ptr, uint* data, uint len)
	{
		uint i;
		for(i = 0; i < len; i++)
			mmu_ptr[i] = data[i];
	}
	void readFromMMU(uint* data, volatile _IODEV int* const mmu_ptr, uint len)
	{
		uint i;
		for(i = 0; i < len; i++)
			data[i] = mmu_ptr[i];
	}

	async command mmuConf_t MemoryManagementUnit.getConfigurationSegment(uint8_t segment)
	{
		uint temp;
		mmuConf_t conf;
		temp = IO_MMU_SEGMENT_CONF(segment);

		conf.readable = temp & MMU_READ_MASK;
		conf.writeable = temp & MMU_WRITE_MASK;
		conf.executable = temp & MMU_EXEC_MASK;
		conf.offset = temp & MMU_OFFSET_MASK;
  		return conf;
	}
	async command uint MemoryManagementUnit.getConfigurationSegmentVal(uint8_t segment)
	{
		return IO_MMU_SEGMENT_CONF(segment);
	}
	async command uint8_t MemoryManagementUnit.getPermissions(uint8_t segment)
	{
		return IO_MMU_SEGMENT_CONF(segment) & (MMU_READ_MASK | MMU_WRITE_MASK | MMU_EXEC_MASK);
	}
	async command uint MemoryManagementUnit.getOffset(uint8_t segment)
	{
		return  IO_MMU_SEGMENT_CONF(segment) & MMU_OFFSET_MASK;
	}

	async command void MemoryManagementUnit.setConfiguration(uint8_t segment, mmuConf_t config)
	{
		IO_MMU_SEGMENT_CONF(segment) = ((config.readable << MMU_READ_BIT) | (config.writeable << MMU_WRITE_BIT) | (config.executable << MMU_EXEC_BIT) | (config.offset & MMU_OFFSET_MASK)); 
	}
	async command void MemoryManagementUnit.setConfigurationSegmentVal(uint8_t segment, uint config)
	{
		IO_MMU_SEGMENT_CONF(segment) = config;
	}
	async command void MemoryManagementUnit.setPermissions(uint8_t segment, uint permissions)
	{
		if(permissions & MMU_READ_MASK)
			IO_MMU_SEGMENT_CONF(segment) |= MMU_READ_MASK;
		else
			IO_MMU_SEGMENT_CONF(segment) &= ~MMU_READ_MASK;

		if(permissions & MMU_WRITE_MASK)
			IO_MMU_SEGMENT_CONF(segment) |= MMU_WRITE_MASK;
		else
			IO_MMU_SEGMENT_CONF(segment) &= ~MMU_WRITE_MASK;

		if(permissions & MMU_EXEC_MASK)
			IO_MMU_SEGMENT_CONF(segment) |= MMU_EXEC_MASK;
		else
			IO_MMU_SEGMENT_CONF(segment) &= ~MMU_EXEC_MASK;
	}
	async command void MemoryManagementUnit.setOffset(uint8_t segment, uint offset)
	{
		IO_MMU_SEGMENT_CONF(segment) = (IO_MMU_SEGMENT_CONF(segment) & (MMU_READ_MASK | MMU_WRITE_MASK | MMU_EXEC_MASK)) | (offset & MMU_OFFSET_MASK);  
	}
	async command void MemoryManagementUnit.setReadable(uint8_t segment)
	{
		IO_MMU_SEGMENT_CONF(segment) |= (1 << MMU_READ_BIT);
	}
	async command void MemoryManagementUnit.setWriteable(uint8_t segment)
	{
		IO_MMU_SEGMENT_CONF(segment) |= (1 << MMU_WRITE_BIT);
	}
	async command void MemoryManagementUnit.setExecutable(uint8_t segment)
	{
		IO_MMU_SEGMENT_CONF(segment) |= (1 << MMU_EXEC_BIT);
	}
	async command void MemoryManagementUnit.clrReadable(uint8_t segment)
	{
		IO_MMU_SEGMENT_CONF(segment) &= ~(1 << MMU_READ_BIT);
	}
	async command void MemoryManagementUnit.clrWriteable(uint8_t segment)
	{
		IO_MMU_SEGMENT_CONF(segment) &= ~(1 << MMU_WRITE_BIT);
	}
	async command void MemoryManagementUnit.clrExecutable(uint8_t segment)
	{
		IO_MMU_SEGMENT_CONF(segment) &= ~(1 << MMU_EXEC_BIT);
	}

	async command bool MemoryManagementUnit.isReadable(uint8_t segment)
	{
		return IO_MMU_SEGMENT_CONF(segment) & MMU_READ_MASK;
	}
	async command bool MemoryManagementUnit.isWriteable(uint8_t segment)
	{
		return IO_MMU_SEGMENT_CONF(segment) & MMU_WRITE_MASK;
	}
	async command bool MemoryManagementUnit.isExecutable(uint8_t segment)
	{
		return IO_MMU_SEGMENT_CONF(segment) & MMU_EXEC_MASK;
	}

	async command bool MemoryManagementUnit.read(uint8_t segment, uint* data, uint len)
	{
		if(!call MemoryManagementUnit.isReadable(segment) || len > ((IO_MMU_SEGMENT_CONF(segment) & MMU_OFFSET_MASK)+1) || sizeof(data) < len)
			return FALSE;

		readFromMMU(data, IO_MMU_SEGMENT_BASE(segment), len);
		return TRUE;
	}
	async command bool MemoryManagementUnit.readSegmentPart(uint8_t segment, uint* data, uint len, uint offset)
	{
		if(!call MemoryManagementUnit.isReadable(segment) || len > ((IO_MMU_SEGMENT_CONF(segment) & MMU_OFFSET_MASK)+1) || sizeof(data) < len)
			return FALSE;

		readFromMMU(data, &(IO_MMU_SEGMENT_BASE(segment)[offset]), len);
		return TRUE;
	}
	async command bool MemoryManagementUnit.write(uint8_t segment, uint* data, uint len)
	{
		if(!call MemoryManagementUnit.isWriteable(segment) || len > ((IO_MMU_SEGMENT_CONF(segment) & MMU_OFFSET_MASK)+1) || sizeof(data) < len)
			return FALSE;

		writeToMMU(IO_MMU_SEGMENT_BASE(segment), data, len);
		return TRUE;
	}
	async command bool MemoryManagementUnit.writeSegmentPart(uint8_t segment, uint* data, uint len, uint offset)
	{
		if(!call MemoryManagementUnit.isWriteable(segment) || len > ((IO_MMU_SEGMENT_CONF(segment) & MMU_OFFSET_MASK)+1) || sizeof(data) < len)
			return FALSE;

		writeToMMU(&(IO_MMU_SEGMENT_BASE(segment)[offset]), data, len);
		return TRUE;
	}
	async command volatile _IODEV int* const MemoryManagementUnit.getExecutionAddress(uint8_t segment)
	{
		return (volatile _IODEV int * const) IO_MMU_SEGMENT_BASE(segment);
	}
	async command volatile _IODEV int * const MemoryManagementUnit.getExecutionAddressOffset(uint8_t segment, uint offset)
	{
		return (volatile _IODEV int * const) &(IO_MMU_SEGMENT_BASE(segment)[offset]);
	}
}