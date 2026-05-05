# Security Policy

This project is a prototype and does not yet provide a stable ABI/API.

Please report security issues privately to the repository maintainer instead of
opening a public issue. Include a minimal reproducer, affected version or commit,
and whether the issue requires a malicious local user, a crafted PipeWire graph,
or untrusted input from another process.

The most security-sensitive area is native PipeWire stream handling. Bugs in
buffer ownership, thread marshalling, or object lifetime can crash embedding
applications.

