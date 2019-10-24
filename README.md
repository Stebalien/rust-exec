Executes and compiles a rust program on-demand as a script. Use with binfmt.

# Install

1. Compile and install the binary to /usr/local/lib/rust-exec
2. Install rust.binfmt to /etc/binfmt.d/rust.conf

# Usage

To use, just mark a rust source file as executable then run it. The kernel will
invoke the rust compiler then run the resulting program.

# Todo

Rewrite it in rust. I wrote this in C to have as little overhead as possible but
it should _probably_ be written in rust.
