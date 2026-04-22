#!/usr/bin/env python3
"""Check if outbound UDP/123 (NTP) works from this network.

This sends a minimal NTP client request to one or more public NTP servers
and waits for a valid response.

Notes:
- This is a practical connectivity test, not a guarantee. Some networks allow
  UDP/123 only to specific destinations.
- If DNS is blocked/misconfigured, name resolution can fail even if UDP/123 is
  allowed. You can pass IP addresses to avoid DNS.

Usage:
  python3 utils/check_ntp_udp123.py
  python3 utils/check_ntp_udp123.py --server time.google.com --timeout 2.5
"""

from __future__ import annotations

import argparse
import socket
import struct
import sys
import time
from dataclasses import dataclass


NTP_PORT = 123
NTP_PACKET_LEN = 48
NTP_UNIX_EPOCH_DELTA = 2208988800  # seconds between 1900-01-01 and 1970-01-01


@dataclass(frozen=True)
class ProbeResult:
    server: str
    ok: bool
    message: str
    rtt_ms: float | None = None
    unix_time: int | None = None


def _build_ntp_request() -> bytes:
    # LI=0, VN=3, Mode=3 (client) => 0x1B
    return b"\x1b" + b"\x00" * (NTP_PACKET_LEN - 1)


def _parse_ntp_response(packet: bytes) -> int:
    if len(packet) < NTP_PACKET_LEN:
        raise ValueError(f"short packet: {len(packet)} bytes")

    # Transmit Timestamp (seconds) is at bytes 40..43 (big-endian)
    transmit_seconds = struct.unpack("!I", packet[40:44])[0]
    if transmit_seconds == 0:
        raise ValueError("zero transmit timestamp")

    unix_seconds = int(transmit_seconds - NTP_UNIX_EPOCH_DELTA)
    if unix_seconds < 0:
        raise ValueError("parsed time before Unix epoch")

    return unix_seconds


def probe_ntp(server: str, timeout_s: float) -> ProbeResult:
    request = _build_ntp_request()

    try:
        addr_info = socket.getaddrinfo(server, NTP_PORT, type=socket.SOCK_DGRAM)
    except socket.gaierror as e:
        return ProbeResult(server=server, ok=False, message=f"DNS/resolve failed: {e}")

    last_err: str | None = None

    for family, socktype, proto, _canonname, sockaddr in addr_info:
        if socktype != socket.SOCK_DGRAM:
            continue

        sock = socket.socket(family, socket.SOCK_DGRAM, proto)
        try:
            sock.settimeout(timeout_s)

            start = time.monotonic()
            sock.sendto(request, sockaddr)
            data, _ = sock.recvfrom(512)
            rtt_ms = (time.monotonic() - start) * 1000.0

            try:
                unix_seconds = _parse_ntp_response(data)
            except Exception as e:  # noqa: BLE001 - want to report parse errors
                return ProbeResult(
                    server=server,
                    ok=False,
                    message=f"got UDP reply but it didn't look like NTP: {e}",
                    rtt_ms=rtt_ms,
                )

            return ProbeResult(
                server=server,
                ok=True,
                message="received valid NTP response",
                rtt_ms=rtt_ms,
                unix_time=unix_seconds,
            )
        except socket.timeout:
            last_err = f"timeout after {timeout_s:.2f}s"
        except OSError as e:
            last_err = f"socket error: {e}"
        finally:
            try:
                sock.close()
            except Exception:
                pass

    return ProbeResult(server=server, ok=False, message=last_err or "no usable address")


def _fmt_time(unix_seconds: int) -> str:
    # Format in local time and UTC to make it easy to sanity-check.
    local = time.strftime("%Y-%m-%d %H:%M:%S %Z", time.localtime(unix_seconds))
    utc = time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime(unix_seconds))
    return f"{local} ({utc})"


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Check outbound UDP/123 (NTP) connectivity")
    parser.add_argument(
        "--server",
        action="append",
        default=[],
        help="NTP server hostname or IP (repeatable). Default: a few public servers.",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=2.0,
        help="Timeout per server in seconds (default: 2.0)",
    )

    args = parser.parse_args(argv)

    servers = args.server or [
        "time.cloudflare.com",
        "time.google.com",
        "pool.ntp.org",
        "time.windows.com",
    ]

    results: list[ProbeResult] = []
    for s in servers:
        results.append(probe_ntp(s, timeout_s=args.timeout))

    any_ok = any(r.ok for r in results)

    for r in results:
        if r.ok:
            when = _fmt_time(r.unix_time or 0)
            print(f"OK   {r.server:24} {r.message}; rtt={r.rtt_ms:.1f}ms; time={when}")
        else:
            extra = f"; rtt={r.rtt_ms:.1f}ms" if r.rtt_ms is not None else ""
            print(f"FAIL {r.server:24} {r.message}{extra}")

    if any_ok:
        print("\nResult: UDP/123 appears reachable (at least to one server).")
        return 0

    print("\nResult: No valid NTP responses received. UDP/123 may be blocked,")
    print("        or DNS is failing, or those specific servers are unreachable.")
    return 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
