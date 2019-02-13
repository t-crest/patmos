# Enabling the distributed shared memory
To enable the two-way distributed shared memory, in `hardware/config/altde2-115.xml`, make sure that Patmos is built with multiple cores (4,9,...), and that TwoWay is selected as a CMP device:
```xml
  <cores count="4"/>
  <CmpDevs>
  <CmpDev name="TwoWay"/>
  </CmpDevs>
```