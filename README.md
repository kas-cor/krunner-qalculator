## ABOUT

qalculatorrunner adds support for doing mathematical computations
via qalculate in krunner. The functionality is very similar to
the default calculator runner.

Qalculate offers arbitrary precision calculations and will solve
certain algebraic equations. It also offers symbolic computation
for exact results--a large improvement over the default calculator
runner.

## INSTALL

Before you build qalculatorrunner, you must have the qalc program from
libqalculate in $PATH.

To build the qalculate runner, go to the root folder and run

```bash
./install.sh
```

## UNINSTALL

```bash
./uninstall.sh
```

## USE

qalculatorrunner invokes the qalc command from libqalculate for all expressions
involving an '=' character.
