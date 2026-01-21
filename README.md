# Lora LPWAN
I’m working on creating a LoRa-based LPWAN (Low Power Wide Area Network).

### Hardware in use
- esp32
- EBYTE e220-400M33S Lora Module
- 3dbi antena

### Currently working on:
- DB 
    - modify functions to load bytes manually into payload to prevent messing up endianness

### To do:
- Add flags to signal when the protocol finishes
    - for example dhcp assings me address, the device releases its semaphore to unblock task waiting for that address
- Potentialy unsafe packet_buffering, may need to rewrite it to task queues.

- DB 
    - updates
- Create a branch
    - Edit function Transmit to "send" packets to received buffer so that way we can self check

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
- DB 
    - DB download
    - DB end
    - DB part download
- Main Loop
    - Setup - ISR -> Receive, initialize tables, etc.
    - Transmit and process task
- E220-400M33S Driver
    - IRQ Table
    - Basic functions
    - Allow custom settings
    - Transmit(), Receive IRQ callback, Channel scanning.
    - Error checking
    - Channel scanning
    - Event based handling irqs