interface MemoryManagementUnit 
{ 
	async command mmuConf_t getConfigurationSegment(uint8_t segment);
	async command uint getConfigurationSegmentVal(uint8_t segment);
	async command uint8_t getPermissions(uint8_t segment);
	async command uint getOffset(uint8_t segment);
	
	async command void setConfiguration(uint8_t segment, mmuConf_t config);
	async command void setConfigurationSegmentVal(uint8_t segment, uint config);
	async command void setPermissions(uint8_t segment, uint permissions);
	async command void setOffset(uint8_t segment, uint offset);
	async command void setReadable(uint8_t segment);
	async command void setWriteable(uint8_t segment);
	async command void setExecutable(uint8_t segment);
	async command void clrReadable(uint8_t segment);
	async command void clrWriteable(uint8_t segment);
	async command void clrExecutable(uint8_t segment);

	async command bool isReadable(uint8_t segment);
	async command bool isWriteable(uint8_t segment);
	async command bool isExecutable(uint8_t segment);

	async command bool read(uint8_t segment, uint* data, uint len);
	async command bool readSegmentPart(uint8_t segment, uint* data, uint len, uint offset);
	async command bool write(uint8_t segment, uint* data, uint len);
	async command bool writeSegmentPart(uint8_t segment, uint* data, uint len, uint offset);
	async command volatile _IODEV int * const getExecutionAddress(uint8_t segment);
	async command volatile _IODEV int * const getExecutionAddressOffset(uint8_t segment, uint offset);
}