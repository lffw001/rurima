[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.14099730.svg)](https://doi.org/10.5281/zenodo.14099730)
# About:
With the `docker` and `lxc` subcommand of rurima, you can search & get & unpack images from dockerhub or LXC mirror easily.      
rurima is designed to be the ruri container manager, but since the dockerhub and lxc support is done, I made them to be subcommands of rurima, so you can use these feature directly.      
# Example usage of docker subcommand:
Get whyour/qinglong docker image.      
```sh
rurima docker pull -i whyour/qinglong -s ./test
```
Get ubuntu docker image.      
```sh
rurima docker pull -i ubuntu -s ./test
```
Search for image `ubuntu`:      
```sh
rurima docker search -i ubuntu
```
Search tag for ubuntu:      
```sh
rurima docker tag -i ubuntu
```
Use docker mirror `dockerpull.org`
```sh
rurima docker pull -m dockerpull.org -i ubuntu -s ./test
```
Try every mirrorlist:      
```sh
rurima docker pull -T -i ubuntu -s ./test
```
Try failback mode:      
```sh
rurima docker pull -T -f -i ubuntu -s ./test
```
# Example usage of lxc subcommand:
Pull alpine edge.      
```sh
rurima lxc pull -o alpine -v edge -s ./test
```
Use mirrors.bfsu.edu.cn/lxc-images as mirror:   
```sh
rurima lxc pull -m mirrors.bfsu.edu.cn/lxc-images -o alpine -v edge -s ./test
```
# Powered by ruri
Rurima has a full integration of ruri now, you can just use it as a more powerful version of ruri.      
You can use ruri as a subcommand of rurima now, for more info about ruri, see [ruri](https://github.com/Moe-hacker/ruri)      
# Note & WIP:
This project does not follow OCI and can only be a `PARTIAL` replacement of docker, this project is still under development.           
# About suid or caps:
Rurima does not allow to set any suid/sgid (with root) or capability on it, it will check it in main() and error() if detected these unsafe settings.      
So, please always use sudo instead.     
# Reporting bugs:
Please use the debug version(rurima-dbg) in release to get debug logs, and please tell me the command you run to cause the unexpected behavior you think!                     
# Update for docker subcommand:
`docker` subcommand now supports auto convert docker config to ruri command, to make it more esay to start the container.        
But if you just want to pull the image but don't want to see the config, use `-q` arg to  disable showing that info.      
`docker` subcommand added `config` command to just get the config of a docker image, you can use `rurima docker config -q -i [image] -t [tag]` to get ruri command to start the container.      
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
---------

<p align="center">「僕らタイムフライヤー</p>
<p align="center">時を駆け上がるクライマー</p>
<p align="center">時のかくれんぼ</p>
<p align="center">はぐれっこはもういやなんだ」</p>