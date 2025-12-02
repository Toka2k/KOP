# Lora WWAN
I’m working on creating a LoRa-based WWAN (Wireless Wide Area Network).

### Hardware in use
- esp32
- EBYTE e220-400M33S Lora Module
- 3dbi antena

### Currently working on:
- Working on E220-400M33S driver
    - Allow custom settings
    - Error checking
    - Transmit(), Receive IRQ callback, Channel scanning.

### To do:
- Modify functions to wait for getting packet back.
- DB 
    - DB download
    - updates
- Channel scanning
- Add function for faulty transmission handling

### Currently finished implementing:
- Routing
    - Sequence number
    - Routing table
    - Hash function
    - Get neighbours
- Packet Processing
    - Packet buffering - received packets, sending packets from to_send queue 
    - Modify all the functions to enqueue packets instead of sending them directly
- Decentralized DHCP
    - Address assigning
- Transmit and Receive functions
    - Listen Before Talk
- ARP
- Main Loop
    - Setup - ISR -> Receive, initialize tables, etc.
    - Transmit and process task
- E220-400M33S Driver
    - IRQ Table
    - Basic functions