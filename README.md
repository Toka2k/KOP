# Lora WWAN
I’m working on creating a LoRa-based WWAN (Wireless Wide Area Network).

My idea is to build the network using LoRa devices and have it assign addresses from any routing device, kind of like a decentralized DHCP system. For routing, I plan to use something similar to RIP, which means that every router would need to keep a full map of the entire network in memory. This creates several technical challenges that I’ll need to figure out.

Once I get the routing part working the way I want, my next step is to power the routing nodes with a battery and solar panel. That way, they could be deployed anywhere and keep running indefinitely without needing maintenance.

### Hardware in use
- esp32

### Main limitations:
- Half duplex
- Memory size
- Bandwitdth
- EU Regulations

### Currently finished implementing:
- Hash function
- Routing table
