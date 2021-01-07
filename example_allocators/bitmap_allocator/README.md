Simple bitmap allocator implementation, to allocate
fixed size chunks of memory.
Supports per-pointer CHERI bounds protection in malloc with
cheri_bounds_set_exact() on all returned pointers.

Execute `make test` to (a) build a good client and a bad client for
this bump allocator, and (b) execute them on a running QEMU CHERI
instance.


