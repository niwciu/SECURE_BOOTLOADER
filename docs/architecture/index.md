# Architecture

This section describes how the secure bootloader is structured, how it communicates, and how it protects firmware integrity.

| Page | Contents |
|------|----------|
| [System Overview](overview.md) | Block diagram, module responsibilities, boot flow |
| [Wire Protocol](protocol.md) | Command format, packet structure, protocol state machine |
| [Security Model](security.md) | AES-CBC encryption, CRC-32 integrity, atomic flash write |
