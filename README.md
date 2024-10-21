# About:
With the `docker` and `lxc` subcommand of rurima, you can search & get & unpack images from dockerhub or LXC mirror easily.      
rurima is designed to be the ruri container manager, but since the dockerhub and lxc support is done, I made them to be subcommands of rurima, so you can use these feature directly.      
# Download:
You can get rurima binary (staticly linked) for arm64, armv7, armhf, riscv64, i386, loong64, s390x, ppc64le and x86_64 from the release page.     
https://github.com/Moe-hacker/rurima/releases/tag/null          
# Dependent:   
rurima needs tar, xz, gzip, file and curl (with libssl), you can find these static binary for aarch64, armv7, x86_64, i386 or riscv64 in：      
[tar-static](https://github.com/Moe-sushi/tar-static)      
[xz-static](https://github.com/Moe-sushi/xz-static)     
[gzip-static](https://github.com/Moe-sushi/gzip-static)     
[file-static](https://github.com/Moe-sushi/file-static)      
[curl-static](https://github.com/Moe-sushi/curl-static)      
rurima also need proot for rootless support, without proot, it might cause bugs unpacking rootfs, sadly, I failed to build proot-static, please install it manually.      
# TODO:
Manage ruri containers and configs.   
Config support, a bit like Dockerfile.   
....
# Usage:
```
Usage: rurima [subcommand] [options]
Subcommands:
  docker: DockerHub support.
  lxc: LXC mirror support.
  unpack: Unpack rootfs.
  help: Show help message.
Options:
  -h, --help: Show help message.
```