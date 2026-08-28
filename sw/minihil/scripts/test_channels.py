import os
import subprocess
import socket
import json
import time
import sys

def send_rpc(sock, method, params=None):
    req = {
        "jsonrpc": "2.0",
        "method": method,
        "id": 1
    }
    if params is not None:
        req["params"] = params
    payload = json.dumps(req) + "\n"
    sock.sendall(payload.encode('utf-8'))
    
    # Read response until newline
    resp_bytes = b''
    while b'\n' not in resp_bytes:
        chunk = sock.recv(1)
        if not chunk:
            break
        resp_bytes += chunk
    
    resp_str = resp_bytes.decode('utf-8').strip()
    if not resp_str:
        return None
    return json.loads(resp_str)

def main():
    script_dir = os.path.dirname(os.path.realpath(__file__))
    daemon_path = os.path.abspath(os.path.join(script_dir, "..", "build", "minihild"))
    
    print(f"Spawning daemon from: {daemon_path}")
    proc = subprocess.Popen([daemon_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    # Wait for startup
    time.sleep(1.0)
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect(("localhost", 9000))
        print("Connected to minihild.")
        
        for relay_id in range(1, 9):
            print(f"\n--- Testing Relay Channel {relay_id} ---")
            
            # Test 1: Toggle Mode (set_relay)
            print(f"[{relay_id}] Testing set_relay (Toggle mode)...")
            res = send_rpc(sock, "set_relay", {"relay_id": relay_id, "state": True})
            assert res["result"]["success"] is True
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is True
            
            res = send_rpc(sock, "get_relay_status", {"relay_id": relay_id})
            assert f"Channel {relay_id}: ON (Toggle)" in res["result"]["status"]
            
            res = send_rpc(sock, "set_relay", {"relay_id": relay_id, "state": False})
            assert res["result"]["success"] is True
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is False
            
            # Test 2: Timer Mode (start_timer)
            print(f"[{relay_id}] Testing start_timer...")
            res = send_rpc(sock, "start_timer", {"relay_id": relay_id, "seconds": 2})
            assert res["result"]["success"] is True
            
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is True
            
            res = send_rpc(sock, "get_relay_status", {"relay_id": relay_id})
            assert f"Channel {relay_id}: ON (Timer, rem:" in res["result"]["status"]
            
            print(f"[{relay_id}] Waiting 2.5 seconds for timer to expire...")
            time.sleep(2.5)
            
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is False
            
            res = send_rpc(sock, "get_relay_status", {"relay_id": relay_id})
            assert f"Channel {relay_id}: OFF (Toggle)" in res["result"]["status"]
            
            # Test 3: Pulse Mode (start_pulse)
            print(f"[{relay_id}] Testing start_pulse...")
            res = send_rpc(sock, "start_pulse", {"relay_id": relay_id, "duration_ms": 500})
            assert res["result"]["success"] is True
            
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is True
            
            res = send_rpc(sock, "get_relay_status", {"relay_id": relay_id})
            assert f"Channel {relay_id}: ON (Pulse, rem:" in res["result"]["status"]
            
            print(f"[{relay_id}] Waiting 0.7 seconds for pulse to expire...")
            time.sleep(0.7)
            
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is False
            
            # Test 4: Blink Mode (start_blink)
            print(f"[{relay_id}] Testing start_blink...")
            res = send_rpc(sock, "start_blink", {"relay_id": relay_id, "on_ms": 150, "off_ms": 150, "count": 2})
            assert res["result"]["success"] is True
            
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is True
            
            res = send_rpc(sock, "get_relay_status", {"relay_id": relay_id})
            assert f"Channel {relay_id}: ON (Blink, count: 2, phase: ON)" in res["result"]["status"]
            
            print(f"[{relay_id}] Waiting 0.2 seconds (should transition to blink OFF phase)...")
            time.sleep(0.2)
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is False
            res = send_rpc(sock, "get_relay_status", {"relay_id": relay_id})
            assert f"Channel {relay_id}: OFF (Blink, count: 2, phase: OFF)" in res["result"]["status"]
            
            print(f"[{relay_id}] Waiting 0.2 seconds (should transition to blink ON phase)...")
            time.sleep(0.2)
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is True
            res = send_rpc(sock, "get_relay_status", {"relay_id": relay_id})
            assert f"Channel {relay_id}: ON (Blink, count: 1, phase: ON)" in res["result"]["status"]
            
            print(f"[{relay_id}] Waiting 0.4 seconds (should finish blink)...")
            time.sleep(0.4)
            res = send_rpc(sock, "get_relays")
            assert res["result"][str(relay_id)] is False
            res = send_rpc(sock, "get_relay_status", {"relay_id": relay_id})
            assert f"Channel {relay_id}: OFF (Toggle)" in res["result"]["status"]
            
        print("\nAll tests passed successfully for all 8 channels!")
        
    except Exception as e:
        print(f"Test failed: {e}")
        # Print stdout/stderr of daemon to debug
        proc.terminate()
        stdout, stderr = proc.communicate()
        print(f"Daemon stdout:\n{stdout}")
        print(f"Daemon stderr:\n{stderr}")
        sys.exit(1)
    finally:
        sock.close()
        proc.terminate()
        proc.wait()

if __name__ == '__main__':
    main()
