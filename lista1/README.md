# Running info

set frequencies with

```
sudo cpupower -c 3 frequency-set -d 4200MHz -u 4200MHz
```

or by setting power profile

```
sudo cpupower -c 3 frequency-set -g performance
```

compile

```
make clean & make
```

then run with

```
taskset -c 3 ./lista1.out
```

and check frequency with

```
watch -n 1 cpupower -c 3 frequency-info
```
