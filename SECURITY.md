# Security Policy

## Supported Versions

Only the latest tagged release receives security fixes.

## Reporting a Vulnerability

Please **do not open a public GitHub issue** for security vulnerabilities.

Report privately to **niwciu@gmail.com** with the subject line `[SECURE_BOOTLOADER] Security Report`.
Include a description of the issue, steps to reproduce, and potential impact.
You can expect an acknowledgement within 72 hours and a fix or mitigation plan within 14 days.

---

## Security Model and Known Constraints

### Flash Read-Protection (RDP) — Production Requirement

The AES-128 key is compiled into the bootloader binary and stored in flash.
Without flash read-protection enabled, the key can be extracted by reading device flash over a debug probe.

**Before shipping any device**, enable the appropriate read-protection level for your MCU:

| Target | Mechanism | Setting |
|---|---|---|
| STM32Gx | RDP Level 1 (minimum) / Level 2 (permanent) | STM32CubeProgrammer → Option Bytes → RDP |
| ATmega328P | Lock Bits | `avrdude -U lock:w:0x0F:m` (disable SPM/LPM from application) |

Failure to set read-protection exposes the AES key to physical attack.

### Downgrade Protection

The firmware header carries two version fields:

| Field | Meaning |
|---|---|
| `app_version` | Version of the image being flashed |
| `prev_app_version` | Minimum version this image allows upgrading from |

The bootloader enforces `app_version >= prev_app_version` as a header self-consistency check.
This catches malformed or corrupted headers but **does not prevent flashing an older legitimately-signed image** (a classic downgrade attack), because the bootloader has no persistent record of the currently installed version.

Full rollback prevention requires one of:
- A monotonic counter stored in a dedicated flash metadata page, incremented on every successful flash.
- Reading the installed app's version from a known fixed offset in application flash, agreed upon between the application and bootloader.

Neither mechanism is implemented in this release. If your threat model includes rollback attacks, this must be addressed before deployment.

### AES Key Provisioning

The key and device ID are supplied at CMake configure time via `-DENCR_KEY` and `-DDEVICE_ID`.
Do not commit production keys to source control.
Use a secrets manager or CI secret injection to supply keys during build.

### Integrity Verification Scope

CRC-32/IEEE 802.3 is computed over the full decrypted plaintext image.
The CRC is carried in the encrypted header and verified after decryption.
This provides integrity against accidental corruption and replay with a different image.

Note: CRC-32 is not a cryptographic hash. An adversary who can write to the UART stream and knows the key could forge a valid image. The AES-128-CBC encryption layer (with per-image IV) is the primary authenticity barrier.
