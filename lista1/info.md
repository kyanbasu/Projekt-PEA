# Running info

```
sudo cpupower -c 3 frequency-set -d 4200MHz -u 4200MHz
```

or

```
sudo cpupower -c 3 frequency-set -g performance
```

then run with

```
taskset -c 3 ./lista1.out
```

and check frequency with

```
watch -n 1 cpupower -c 3 frequency-info
```
