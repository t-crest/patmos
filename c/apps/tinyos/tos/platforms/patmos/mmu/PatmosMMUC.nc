configuration PatmosMMUC {
  provides interface MemoryManagementUnit;
}

implementation 
{
    components PatmosMMUP;

    MemoryManagementUnit = PatmosMMUP;
}
