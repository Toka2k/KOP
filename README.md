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

### Currently working on:
Designing protocols:
- Devices Joining the network
- Devices Deciding on common Channel to use
- Data Packets

### Currently finished implementing:
- Hash function
- Routing table

### Development Diagrams
Joining network:
<img width="1220" height="181" alt="Join-Network" src="https://github.com/user-attachments/assets/56cce0eb-5ced-483c-afef-985f808b5c1e" />
Joining Routing Network:
<img width="914" height="771" alt="Join-Routing-Network" src="https://github.com/user-attachments/assets/58c78975-371b-4d13-ae15-e06b2dea4283" />

Send Data:

<img width="340" height="483" alt="Send-Data" src="https://github.com/user-attachments/assets/ad36591d-eea3-4c7c-acf6-694b309f34f9" />

