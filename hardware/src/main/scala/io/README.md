# I/O devices for Patmos must follow some conventions:

* I/O devices are implemented as classes that extend a
  Device. Currently, only CoreDevice (i.e., a device with an OcpCore
  interface) is supported as base class.

* There must be a proxy object that handles parameter parsing and
  instantiation of the actual I/O device class. The proxy object
  should extend DeviceObject. It must contain a method "create(params:
  Map[String, String])" that returns an instance of the respective I/O
  device. Furthermore, it must contain a trait "Pins" that describes
  the I/O device's external ports. The description must be provided in
  a field <dev>Pins, where <dev> is the name of the proxy object with
  the first character converted to lower case.

Example:
```scala
object FooBar extends DeviceObject {
  def create(params: Map[String, String]) : FooBar = {
    val foo = ...
    val bar = ...
    Module(new FooBar(foo, bar))
  }
}

class FooBar(foo : ..., bar: ...) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    override pins: Bundle { val abs: ??; val xyz: ?? } = new Bundle() {
       val abc = ...
       val xyz = ...
    }
  }
  ...
}
```
