# i3c-tools - Set of tools to interact with i3c devices from user space

* Version: 0.1
* Author: Vitor Soares <vitor.soares@synopsys.com>

This package has the i3ctransfer tool used to send private SDR transfer
over I3C bus to a particular device expose via i3cdev module to /dev folder.
The i3ctransfer tool allow to construct and concat multiple I3C private
transfer into a single transfers. It aim to help developer during the I3C HC
and I3C device drivers development.

At this time due the lack of HDR transfer support on I3C subsystem isn't
published the i3chdrtransfer tool that allow to send commands in HDR-DDR
or HDR-TS mode.

Please refer to:
	https://patchwork.kernel.org/cover/11282837/
for i3cdev module support.

# Build

```
$ gcc -Wall -I include/i3c i3ctransfer.c -o i3ctransfer
```

# Usage

Write one data byte in single private transfer
```
$ i3ctransfer -d /dev/<i3c dev> -w 0xde
```

Write multiple data bytes in single private transfer
```
$ i3ctransfer -d /dev/<i3c dev> -w "0xde,0xad,0xbe,0xef"
```

Write multiple data bytes in multiple private transfers
```
$ i3ctransfer -d /dev/<i3c dev> -w "0xde,0xad" -w "0xbe,0xef"
```

Read multiple data bytes in single private transfer
```
$ i3ctransfer -d /dev/<i3c dev> -r <data length>
```

Read multiple data bytes in multiple private transfer
```
$ i3ctransfer -d /dev/<i3c dev> -r <data length> -r <data length>
```

Read and write multiple data bytes in multiple private transfer
```
$ i3ctransfer -d /dev/<i3c dev> -w "0xde,0xad,0xbe,0xef" -r <data length>
```

Parameters:
* i3c dev: I3C device exposed by i3cdev under /dev folder
* data length: Data lenth to read on this message

