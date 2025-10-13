# Workflow guide (temporary title)

## Pre-commit setup (local development)

This project uses pre-commit hooks in the local development environment to enforce code formatting, compilation checks, and (some) config file validation(yaml, json).
The pre-commit hook will run automatically when you try to commit, and can also be run manually(see step 3 below).

### 1. Install pre-commit

If you haven't installed yet;

```
pip install pre-commit
```

> Note: You have to have python and pip installed on the system

### 2. Install hooks

After cloning the repo, run;

```
pre-commit install
```

The hooks will be "set up" in the local .git/hooks directory. This will enable the hooks to run automatically on each commit/ or enable them to run manually.

### 3. (Optional) run hooks manually

Run this command from project root to check all files

```
pre-commit run --all-files
```

### Included hooks

-   clang-format: uses the rules from .clang-format to check and format all C/C++ files
-   Platformio checks: builds/compiles and runs static code analysis on both Arduino and ESP32 S3
-   YAML/JSON checks: checks and validates config files(`.yaml` and `.json`)
    > `--skip-packages` skips library files in order to just check the validity of our own code.
    > YAML/JSON checks skips **except .vscode/extensions.json**, since this generated file includes comments which most JSON parses won't allow

### Workflow

#### 1. Stage changes

```
git add <files>
```

#### 2. Commit

```
git commit -m "message"
```

-   If files are not formatted based on the code rules, the clang format will fail but also automatically format the files.
    -   run the pre-commit again to make the clang format pass
-   if the compile and static analysis checks fail, fix the issues, and try again before pushing

#### 3. Push

```
git push origin <branch-name>
```
