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

### Downgrade / Rollback

Intentional firmware downgrades are supported. The bootloader accepts any valid, correctly signed image regardless of its `app_version` value, allowing a device to be recovered by reflashing an older working release.

If your threat model requires preventing rollback attacks, this must be enforced on the host side (e.g. the update tool refuses to package older versions) or by adding a monotonic counter in a dedicated flash metadata page — neither mechanism is implemented in this bootloader.

### AES Key Provisioning

The key and device ID are supplied at CMake configure time via `-DENCR_KEY` and `-DDEVICE_ID`.
Do not commit production keys to source control.
Use a secrets manager or CI secret injection to supply keys during build.

### Integrity Verification Scope

CRC-32/IEEE 802.3 is computed over the full decrypted plaintext image.
The CRC is carried in the encrypted header and verified after decryption.
This provides integrity against accidental corruption and replay with a different image.

Note: CRC-32 is not a cryptographic hash. An adversary who can write to the UART stream and knows the key could forge a valid image. The AES-128-CBC encryption layer (with per-image IV) is the primary authenticity barrier.
