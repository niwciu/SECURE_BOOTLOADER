# Security Model

The bootloader provides three independent layers of protection that together ensure only authorised, intact firmware can be installed.

---

## Layer 1 — Device Authentication

The firmware header carries a 64-bit **device ID** that must match the value burned into the bootloader at compile time via `-DDEVICE_ID=`.

```c
uint64_t product_id = ((uint64_t)header.product_ID_MSB << 32)
                    | (uint64_t)header.product_ID_LSB;

if (product_id != DEVICE_ID) {
    send_byte(CMD_ERR | CMD_START);
    return;
}
```

A firmware image compiled for one device is **rejected by all others**, even if the image is otherwise valid. This prevents cross-device flashing attacks.

---

## Layer 2 — Encrypted Transport (AES-128-CBC)

All firmware page data is encrypted with **AES-128-CBC** before transmission. The bootloader decrypts each page before writing it to flash.

### Key management

The 16-byte AES key is injected at **compile time**:

```bash
cmake -S . -B Release -G Ninja \
    -DENCR_KEY="{0x21,0xBB,0xA1,0x8D,0xF4,0x9B,0x1E,0x77,0x26,0x6F,0x80,0x92,0x4C,0x35,0xE6,0xB8}"
```

The key is stored in the constant array `KEY[]` marked `static const`, which the linker places in flash. It never appears in a header, source file, or build artifact that leaves the build system.

### Initialisation vector (IV)

Each firmware image uses a unique 16-byte IV carried in the header:

```c
typedef struct {
    ...
    uint8_t iv[IV_BLOCK_SIZE];   // 16 bytes
    ...
} header_t;
```

`AES_CBC_init(KEY, header.iv)` is called in `do_start()` before any page data is received. Using a per-image IV prevents IV reuse across different firmware versions.

### Decryption

Each page is decrypted in-place inside the 4-byte-aligned `page_buf` before CRC accumulation or flash write:

```c
AES_CBC_decrypt_buffer((uint8_t *)page_buf, sizeof(page_buf));
crc = CRC_add_byte_tab(crc, (uint8_t *)page_buf, sizeof(page_buf));
```

!!! important "Decrypt before CRC"
    The CRC is computed over the **plaintext** image. This means the host must CRC the plaintext and encrypt it separately. An attacker who can intercept the UART stream sees only ciphertext; they cannot compute a valid CRC without the key.

---

## Layer 3 — Integrity Verification (CRC-32/IEEE 802.3)

After decrypting each page, the running CRC accumulates bytes across the entire image. When the last page is received, the final CRC is compared against `header.crc`:

```c
crc = CRC_result(crc);
if (crc != header.crc) {
    ok = false;   // → CMD_ERR | CMD_NEXT_PAGE
}
```

**CRC algorithm:** CRC-32/IEEE 802.3

| Parameter | Value |
|-----------|-------|
| Polynomial | `0x04C11DB7` |
| Initial value | `0xFFFFFFFF` |
| Input reflection (RefIn) | true |
| Output reflection (RefOut) | true |
| Final XOR (XorOut) | `0xFFFFFFFF` |

This is the same algorithm used by Ethernet, ZIP, and PNG. Off-the-shelf CRC utilities (Python `binascii.crc32`, `crc32` CLI, etc.) produce compatible results.

### Software vs. hardware CRC

| `CRC_MODULE` | Implementation | File |
|---|---|---|
| `SW` (default) | Bit-by-bit software | `src/CRC/crc.c` |
| `HW` | STM32 CRC peripheral | `hw/<TARGET>/Core/Bl_drv_interface/bl_crc_drv.c` |

Both implementations expose the same `crc_api.h` interface and produce identical results. The hardware path uses the `REV_IN` and `REV_OUT` bits of the STM32 CRC peripheral to handle the reflected polynomial.

---

## Atomic Flash Write — Preventing Partial Boot

The most critical security property is the **deferred first-word write**.

The application's reset vector is located at `APP_START` (the first 8 bytes: initial SP and reset handler address). These bytes are what `CORE_is_valid_app()` checks to decide whether a bootable image exists.

**The strategy:**

1. On the first `CMD_NEXT_PAGE` call, the first 8 bytes of the decrypted page are buffered into `app_begin_word[]` and **not** written to flash. The flash write starts at `APP_START + 8`.
2. All subsequent pages are written normally.
3. Only after the last page's CRC matches does the bootloader write `app_begin_word[]` to `APP_START`.

```c
if (page_pos == APP_START) {
    app_begin_word[0] = *data++;   // buffer first 8 bytes
    app_begin_word[1] = *data++;
    write_pos += 2U * sizeof(uint32_t);
    --write_count;
}
// ... write the rest of the page ...

if (--page_rem == 0) {
    crc = CRC_result(crc);
    if (crc == header.crc)
        FLASH_write(APP_START, &app_begin_word[0], 1);  // atomic commit
}
```

**Why this matters:**

| Scenario | Outcome |
|----------|---------|
| Transfer interrupted mid-way | `APP_START` still contains old (or erased) data → `CORE_is_valid_app()` returns `false` → bootloader resets, never boots partial image |
| CRC mismatch on last page | First 8 bytes never written → same protection |
| Power loss during the final write | Worst case: first 8 bytes partially written → `CORE_is_valid_app()` will likely fail (invalid SP or reset vector) → bootloader resets |

This provides **fail-safe rollback**: a failed or interrupted update always leaves the device in the bootloader, ready to retry, rather than booting corrupt firmware.

---

## Application Validity Check

`CORE_is_valid_app()` in `core_drv.c` performs four checks before jumping to the application:

```c
bool CORE_is_valid_app(void)
{
    const uint32_t stack = *(const uint32_t *)APP_START;
    const uint32_t reset = *(const uint32_t *)(APP_START + 4U);

    if (stack < SRAM_START || stack > SRAM_END) return false; // SP not in SRAM
    if (stack & 0x3U)                           return false; // SP not 4-byte aligned
    if ((reset & 1U) == 0U)                     return false; // not Thumb code
    if (reset < APP_START || reset >= APP_END)  return false; // reset vector not in app flash

    return true;
}
```

These checks catch:

- Blank / erased flash (all `0xFF` → SP = `0xFFFFFFFF`, outside SRAM)
- Reset vector pointing outside the application region
- Reset vector with Thumb bit clear (invalid on all Cortex-M targets)

---

## Threat Model Summary

| Threat | Mitigation |
|--------|-----------|
| Unauthenticated firmware | Device ID check in header rejects unknown images |
| Eavesdropping on UART | AES-128-CBC encryption; attacker sees only ciphertext |
| Firmware tampering in transit | CRC-32 over plaintext; any bit flip detected |
| Replay of old firmware | Per-image IV prevents identical ciphertext even for same plaintext |
| Partial / aborted transfer | Deferred first-word write; partial image never passes validity check |
| Flashing wrong device | 64-bit device ID bound at compile time |
| Power loss during flash | Validity check catches incomplete writes; bootloader retries |

!!! warning "Known limitations"
    - The AES key is stored in flash; physical extraction with a debugger or glitching may expose it. Enable read-out protection (RDP) on STM32 targets for production deployments.
    - CRC-32 is not a cryptographic MAC; it detects accidental corruption but not a determined adversary who has the key. For higher assurance, replace with HMAC-SHA256.
    - The protocol has no sequence numbers; replay attacks are possible if the IV is reused.
