# rpi_ecat_slave — EtherCAT test slave on Raspberry Pi 5

EtherCAT partner of the gateway for configurations 3 and 4 (gateway = master).
Raw Ethernet socket + software ESC (`src/abstract_layer/modules/esc/soft_esc.c`)
+ SOES v3.1.0 (`third_party/SOES`). The SII image is the one of the SOES
`linux_lan9252demo` (vendor 0x1337, product 1234, name `evb9252_dig`); the
object dictionary (`od/`) carries 8 bytes per direction, matching the 8-byte
CANopen PDOs of test set S1:

| Direction | Object | Size | Test behaviour |
|---|---|---|---|
| outputs (RxPDO 0x1600, master → slave) | 0x7000:01 UNSIGNED64 | 8 B | data from the CAN RPDO 0x210 |
| inputs (TxPDO 0x1A00, slave → master) | 0x6000:01 UNSIGNED64 | 8 B | echo: inputs = outputs |

The slave updates its inputs after a frame has passed, so the inputs read in
cycle n echo the outputs of cycle n-1.

No Distributed Clocks. The slave is the last device on the line: frames are
processed and sent back on the same port.

## 1. Raspberry Pi setup (once)

1. Raspberry Pi Imager → **Raspberry Pi OS Lite (64-bit)**. In the advanced
   options set hostname, user, **enable SSH** and **configure Wi-Fi**.
   SSH goes over Wi-Fi; `eth0` is reserved for EtherCAT.
2. Packages:
   ```sh
   sudo apt update && sudo apt install -y build-essential cmake git ethtool
   ```
3. Take `eth0` away from NetworkManager and keep it silent (no IP traffic):
   ```sh
   printf '[keyfile]\nunmanaged-devices=interface-name:eth0\n' | \
       sudo tee /etc/NetworkManager/conf.d/99-ethercat.conf
   echo 'net.ipv6.conf.eth0.disable_ipv6 = 1' | sudo tee /etc/sysctl.d/99-ethercat.conf
   sudo systemctl restart NetworkManager && sudo sysctl --system
   sudo ip link set eth0 up
   sudo ethtool --set-eee eth0 eee off   # energy-efficient Ethernet off (may be unsupported)
   ```
   `ip addr show eth0` must show `UP` and no `inet` address.

## 2. Copy the code

From `gateway_abstract_layer/` on the Mac (`<user>@<rpi>` = SSH login):

```sh
rsync -avR third_party/SOES third_party/SOEM src/abstract_layer/modules/esc \
      src/software_core/ethercat tools/rpi_ecat_slave tests/soft_esc_loop \
      <user>@<rpi>:gw/
```

## 3. Build

```sh
cd ~/gw
sh tests/soft_esc_loop/build.sh          # host co-simulation, must end with RESULT: PASS
cmake -S tools/rpi_ecat_slave -B build-slave -DCMAKE_BUILD_TYPE=Release
cmake --build build-slave
```

After an update that changes `CMakeLists.txt`, configure from scratch
(`rm -rf build-slave` and both commands again): `rsync -a` keeps the file
times of the Mac, so CMake may not notice the change and keep old include
paths.

## 4. Run

Cable: gateway Ethernet (RJ-45) ↔ RPi `eth0`, directly.

```sh
sudo ./build-slave/rpi_ecat_slave eth0
```

Status line once per second:

```
AL=0x08 frames=500 dg=... denied=0 sii=18 outUpd=... out=8877665544332211 in=8877665544332211 proc_avg=..us proc_max=..us txerr=0
```

- `AL` — AL status: 0x01 INIT, 0x02 PRE-OP, 0x04 SAFE-OP, 0x08 OP, +0x10 error
- `frames` — EtherCAT frames processed and sent back in the last second
- `denied` — SyncManager accesses refused by the soft-ESC (should stay 0)
- `out`/`in` — last outputs from the gateway / inputs sent back (hex, little endian)
- `proc_avg`/`proc_max` — receive → send time per frame in the last second

The program uses `SCHED_FIFO` priority 80 and locked memory (hence `sudo`).
Latency spikes on the standard kernel are expected; PREEMPT_RT comes in
stage 5c together with GPIO markers t0/t5.
