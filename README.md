[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.14099730.svg)](https://doi.org/10.5281/zenodo.14099730)
# About:
With the `docker` and `lxc` subcommand of rurima, you can search & get & unpack images from dockerhub or LXC mirror easily.      
rurima is designed to be the ruri container manager, but since the dockerhub and lxc support is done, I made them to be subcommands of rurima, so you can use these feature directly.      
# Update for docker subcommand:
`docker` subcommand now supports auto convert docker config to ruri command, to make it more esay to start the container.        
But if you just want to pull the image but don't want to see the config, use `-q` arg to  disable showing that info.      

`docker` subcommand added `config` command to just get the config of a docker image, you can use `rurima docker config -i [image] -t [tag]|grep "ruri \-w"|cat` to get ruri command to start the container.      
# Using 
# WARNING:      
```
* Your warranty is void.
* I am not responsible for anything that may happen to your device by using this program.
* You do it at your own risk and take the responsibility upon yourself.
* Docker is a registered trademark of Docker, Inc. This program has no relationship with it.
* This program has no Super Cow Powers.
```
# NOTICE:
This program is not official tool of docker or dockerhub, you can report bugs here, but this program has no relation with docker.      
Docker is a registered trademark of Docker, Inc.      
# Download:
You can get rurima binary (staticly linked) for arm64, armv7, armhf, riscv64, i386, loong64, s390x, ppc64le and x86_64 from the release page.     
https://github.com/Moe-hacker/rurima/releases/tag/null          
# Dependent:   
rurima needs tar, xz, gzip, file, you can find these static binary for aarch64, armv7, x86_64, i386 or riscv64 in：      
[tar-static](https://github.com/Moe-sushi/tar-static)      
[xz-static](https://github.com/Moe-sushi/xz-static)     
[gzip-static](https://github.com/Moe-sushi/gzip-static)     
[file-static](https://github.com/Moe-sushi/file-static)      
rurima also need `curl`, but th static build have bugs, and I found that Android 15 has a built-in curl: `/bin/curl`.          
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
