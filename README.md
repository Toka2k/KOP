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

### Currently working on:
- get neighbours

### To do:
- ARP
- DB - DB download, updates
- Channel scanning

### Currently finished implementing:
- Routing
    - Sequence number
    - Routing table
    - Hash function
- Packet Processing
    - Packet buffering - received packets, sending packets from to_send queue 
    - Modify all the functions to enqueue packets instead of sending them directly
- Decentralized DHCP
    - Address assigning
- Transmit and Receive functions
    - Listen Before Talk

### Development Diagrams

Joining network:

<img width="1220" height="181" alt="Join-Network" src="https://github.com/user-attachments/assets/56cce0eb-5ced-483c-afef-985f808b5c1e" />

Joining Routing Network:

<img width="914" height="771" alt="Join-Routing-Network" src="https://github.com/user-attachments/assets/58c78975-371b-4d13-ae15-e06b2dea4283" />

Send Data:

<img width="340" height="483" alt="Send-Data" src="https://github.com/user-attachments/assets/ad36591d-eea3-4c7c-acf6-694b309f34f9" />

Choose Channel:

<img width="724" height="350" alt="Choose-Channel" src="https://github.com/user-attachments/assets/75598ba1-f70a-476d-b9e7-356656bc87ba" />

Browsing Channels:

<img width="681" height="411" alt="Browsing-Channels" src="https://github.com/user-attachments/assets/32c5aef9-95b0-44c2-94a1-505d76438cd7" />
