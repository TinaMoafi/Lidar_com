import socket
import json

UDP_IP = "127.0.0.1"
UDP_PORT = 25000

GATEWAY_IP = "127.0.0.1"
GATEWAY_PORT = 30000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Adapter listening on {UDP_IP}:{UDP_PORT}")

while True:
    data, addr = sock.recvfrom(65535)

    message = data.decode("utf-8").strip()
    lines = message.split("\n")

    frame = []
    valid = True

    for line in lines:
        parts = line.split(";")

        if len(parts) != 6:
            valid = False
            print("Invalid line:", line)
            continue

        try:
            t = float(parts[0])
            ray_id = int(parts[1])
            x = float(parts[2])
            y = float(parts[3])
            z = float(parts[4])
            intensity = float(parts[5])
        except ValueError:
            valid = False
            print("Invalid numeric value:", line)
            continue

        frame.append({
            "timestamp": t,
            "ray_id": ray_id,
            "x": x,
            "y": y,
            "z": z,
            "intensity": intensity
        })

    if len(frame) == 0:
        valid = False
        print("Empty frame")

    else:
        timestamp = frame[0]["timestamp"]

        for ray in frame:
            if ray["timestamp"] != timestamp:
                valid = False
                print("Timestamp mismatch inside frame")
                break

        ray_ids = [ray["ray_id"] for ray in frame]

        if ray_ids != sorted(ray_ids):
            valid = False
            print("Ray IDs are not ordered")

    if valid:
        lidar_frame_message = {
            "service": "LiDARService",
            "event": "LiDARFrame",
            "timestamp": frame[0]["timestamp"],
            "number_of_rays": len(frame),
            "rays": frame
        }

        someip_message = {
            "someip_header": {
                "service_id": "0x1234",
                "method_id": "0x0001",
                "message_type": "event",
                "interface_version": 1,
                "protocol_version": 1
            },
            "payload": lidar_frame_message
        }

        someip_serialized = json.dumps(someip_message)

        sock.sendto(someip_serialized.encode("utf-8"), (GATEWAY_IP, GATEWAY_PORT))

        print("Forwarded to mock SOME/IP gateway")
        print(
            f"Valid LiDAR frame | "
            f"time={frame[0]['timestamp']} | "
            f"rays={len(frame)} | "
            f"someip_message_size={len(someip_serialized)} bytes"
        )

    else:
        print(f"Invalid LiDAR frame | parsed rays={len(frame)}")