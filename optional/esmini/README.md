# ESMini Simulator Binding Plugin

To prepare running the smoketests, first export required packages like so:

```
make export-vendor export \
  && make -C ../osi export-vendor export \
  && make -C ../../ export-all
```

Then, build the packages and run the smoketests:

```
make smoketest-deps smoketest
```
