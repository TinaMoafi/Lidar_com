
## Current pipeline

CSV LiDAR data
→ Python LiDAR emulator
→ UDP packet on port 25000
→ C++ adapter
→ vSomeIP SOME/IP event
→ Eclipse S-CORE SOME/IP gateway

## Adapter SOME/IP identifiers

- Service ID: 0x1234
- Instance ID: 0x0001
- Event ID: 0x8001
- Event Group ID: 0x0001
- Application name: lidar_adapter

## Adapter role

The adapter is the external bridge. It receives LiDAR frames through UDP, parses them, serializes them as binary payloads, and publishes them as SOME/IP events.

## S-CORE gateway role

The S-CORE SOME/IP gateway should receive the LiDAR SOME/IP event through its someipd/gatewayd architecture and later forward it internally to the visualizer microservice.
