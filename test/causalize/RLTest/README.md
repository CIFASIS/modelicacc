# RLTest benchmarks

Scripts that benchmark the causalization of the `TestRL1.mo`, `TestRL2.mo` and
`TestRL3.mo` models for increasing sizes `N`, using either ModelicaCC + QSS
Solver or OpenModelica (OMC).

| Script                  | Toolchain                                   | Run from         | Values of `N`                     |
|-------------------------|---------------------------------------------|------------------|-----------------------------------|
| `modelicacc_script.py`  | ModelicaCC `causalize` + QSS Solver (`mmoc`) | `RLTest/`        | 100, 1000, 10000, 100000, 1000000 |
| `omc_script.py`         | OpenModelica `omc` (new backend)            | `RLTest/omc/`    | 100, 1000                         |

For every model and every `N`, each script writes a temporary copy of the
model with `N` (and every `x[N]` index access, in the ModelicaCC script)
replaced, compiles it `runs` times and prints the average time of every stage.

## Dependencies

Common:

- Python 3 (standard library only for `omc_script.py`).
- `make` and a C compiler (`gcc`), used to build the generated C models.

### ModelicaCC

Build ModelicaCC from the repository root. The script calls
`../../../bin/causalize`, so the binaries must be in the source tree `bin/`
folder (the default output location of the CMake build):

```bash
cd <modelicacc>
autoconf
./configure
make
```

ModelicaCC build dependencies:
```bash
* autoconf 2.69 (avoid 2.71)
* boost1.81
* cmake
* doxygen
* g++
* libginac-dev 
* make   
```

### QSS Solver

`modelicacc_script.py` compiles the causalized models with the QSS Solver
`mmoc` compiler through its Python API (`qss_solver` module).

1. Get and build QSS Solver following its own README:

   ```bash
   git clone https://github.com/CIFASIS/qss-solver.git ~/work/qss-solver
   ```
   or download and install version 6.2.2 `deb` package from:
   ```bash
   wget https://github.com/CIFASIS/qss-solver/releases/download/v6.2.2/qss-solver-6.2.2.deb
   ```

2. Make the paths in `modelicacc_script.py` match your installation if yopu compile the code, otherwise the default QSS Solver path installation will work:

   - The `qss_solver` Python module location, added to `sys.path` at the top
     of the script:

     ```python
     sys.path.insert(0, '/opt/CIFASIS-CONICET/qss-solver/src/python')
     ```

   - The QSS Solver base folder in `setup_environment()`, used to define the
     environment variables required by `mmoc`:

     ```python
     qss_base = Path.home() / "qss-solver"
     ```

     | Variable      | Value                 |
     |---------------|-----------------------|
     | `MMOC_BIN`    | `<qss_base>/bin`      |
     | `MMOC_MODELS` | `<qss_base>/models`   |
     | `MMOC_OUTPUT` | `<qss_base>/output`   |
     | `MMOC_BUILD`  | `<qss_base>/build`    |

     `MMOC_OUTPUT` and `MMOC_BUILD` are created if they don't exist.

   You can check that the module is found with:

   ```bash
   PYTHONPATH=<qss-solver>/src/python python3 -c "import qss_solver"
   ```

> **Note:** `setup_environment()` edits `$MMOC_BUILD/mmoc.sh` in place,
> removing its `# Validate input parameters` block so that `mmoc` can be
> called with a single argument.

### OpenModelica

`omc_script.py` calls `omc` from the `PATH` with the new backend and backend
clock dumps:

```bash
omc -s --newBackend -d=dumpBackendClocks <model>.mo
```

Install an OpenModelica release that supports `--newBackend`, version `1.27.1` or later (see
<https://openmodelica.org/download/>) and check it with:

```bash
omc --version
```

## Running

Both scripts take a single argument: the number of runs per model and per `N`.

### ModelicaCC + QSS Solver

```bash
cd test/causalize/RLTest
python3 modelicacc_script.py 10
```

For each run the script:

1. Runs `../../../bin/causalize --tearing temp_benchmark_<model>.mo` and
   collects the time of each causalization stage.
2. Moves `<model>_causalized.mo` to `causalized/<model>.mo`.
3. Compiles it with `mmoc` (`ModelicaCC compilation`).
4. Builds the generated C model with `make -f <model>.makefile`
   (`C compilation`) and records the size of `<model>.c` (`C File Size`).

At the end it deletes the `*_sbg_input*` and `*_causalized*` files of the
folder.

### OpenModelica

```bash
cd test/causalize/RLTest/omc
python3 omc_script.py 10
```

For each run the script:

1. Runs `omc` on `temp_benchmark_<model>.mo` (generated from `../<model>.mo`)
   and collects the `Causalize`, `Tearing` and `Solve` backend clocks, plus
   the wall time of the call (`Total compile time`).
2. Adds up the size of all the generated `.c` files whose name contains the
   model name (`C File Size`).
3. Builds the generated C model with `make -f <model>.makefile`
   (`Build C model`).

## Output

Both scripts log to the console and to `simulation.log` in the folder they are
run from, ending with a table per model:

```
============================== TIME MEASURES: TestRL1.mo ==============================
N          | Stage Name                          | Average Time (ms)
-------------------------------------------------------------------------
100        | Horizontal sorting SBG builder      | ...
```

The column header says `ms`, but the units depend on the stage:

| Stage                                                          | Unit                    |
|----------------------------------------------------------------|-------------------------|
| ModelicaCC causalization stages                                | ms                      |
| `ModelicaCC compilation`, `C compilation`                      | s                       |
| `Total compile time`, `Build C model`                          | s                       |
| OMC backend clocks (`Causalize`, `Tearing`, `Solve`)           | as reported by `omc`    |
| `C File Size`                                                  | KB                      |

## Warnings

- Both scripts delete generated files from the folder they run in, before
  starting and before each compilation:
  - `modelicacc_script.py`: `*.c`, `*.h`, `*.makefile`, `*.sh`, `*.ini`.
  - `omc_script.py`: `*.c`, `*.h`, `*.makefile`, `*.sh`, `*.json`, `*.xml`,
    `*.bin`.

  Don't keep other files with these extensions in `RLTest/` or `RLTest/omc/`.
- `simulation.log` is overwritten on every execution.
- Large values of `N` (100000 and 1000000) can take a long time and a lot of
  memory, especially for the C compilation step. Edit `lista_n` in the
  `__main__` block of each script to change the evaluated sizes.
