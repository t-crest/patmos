configuration PatmosCpuInfoC {
  provides interface CpuInfo;
}

implementation 
{
    components PatmosCpuInfoP;

    CpuInfo = PatmosCpuInfoP;
}
