# CI/CD Workflow Documentation

This document outlines the CI/CD pipelines for the IoT-Chas-Advance repository, as well as the required local setup for developers.

## Table of Content:

-   [1. Local development workflow](#1-local-development-workflow)
    -   [Mandatory Local Setup](#mandatory-local-setup-secretsconfig-variables)
    -   [Pre-commit Git Hooks](#pre-commit-git-hooks)
    -   [Pre-commit Setup](#pre-commit-setup)
    -   [(Optional) Run hooks manually](#optional-run-hooks-manually)
    -   [Local workflow with pre-commit](#local-workflow-with-pre-commit)
-   [2. Continuous Integration (CI) Workflow](#2-continuous-integration-ci-workflow)
    -   [CI Jobs](#ci-jobs)
-   [3. Continuous Deployment (CD) Workflow](#3-continuous-deployment-cd-workflow)
    -   [CD Jobs](#cd-jobs)
-   [4. Manual Docker Environment](#4-manual-docker-environment)
    -   [Docker Setup](#docker-setup)
    -   [Build the Docker Image](#build-the-docker-image)
    -   [Run the container (for development)](#run-the-container-for-development)
    -   [Interact inside the container](#interact-inside-the-container)
    -   [Some basic shell commands](#some-basic-shell-commands-to-run-inside-container)
    -   [Basic PlatformIO commands](#basic-platformio-commands)
    -   [Exit the container](#exit-the-container)
    -   [Docker tooling guide (outside container)](#docker-tooling-guide-outside-container)
    -   [Docker concept summary](#docker-concept-summary)

## 1. Local development workflow

### Mandatory Local Setup (Secrets/config variables)

> NOTE: This is WIP until we have implemented a way to configure during the runtime

Before building the firmware locally, you must provide your network settings.

1. Copy the Secrets template: Make a copy of the committed `<board>/include/secrets.example.h` and rename it to`<board>/include/secrets.h`

2. Edit `secrets.h`: Update this file with your own, real local Wi-Fi credentials (`SECRET_SSID`, `SECRET_PASSWORD`), and your local server IP address (`SERVER_IP` octets).

    > **NOTE:** This file is used only for local development and is excluded by `.gitignore`. The CI pipeline uses generic placeholder values for releases.

3. (Optional) Setup environment variables: Copy the committed `.env.example` file and rename it to `.env`. This file is used for local `act` simulation (to simulate some Github Actions locally). For further reading about [act](https://nektosact.com/).

### Pre-commit Git hooks

We use a pre-commit hook (`.pre-commit-config.yaml`) in the local development before pushing changes to the remote repo. The pre-commit hook is also used in the Github Actions CI pipeline (see below). The purpose is to enforce consistent code standard, formatting, and linting.

**What pre-commit hook does:**

-   Formatting: Runs `clang-format` on C/C++ files to enforce code standard specified in `.clang-format`.
-   Cleanup: Checks for valid YAML/JSON and trailing whitespace

The pre-commit hook will run automatically when you commit, and can also be run manually(see step 3 below).

#### Pre-commit Setup

**Prerequisites:**

-   Python
-   pip

1. Install pre-commit to system

If you haven't installed yet;

```
pip install pre-commit
```

2. Install pre-commit to local repository

After cloning the repo, run;

```
pre-commit install
```

The hooks will be "set up" in the local .git/hooks directory. This will enable the hooks to run automatically on each commit/ or enable them to run manually.

#### (Optional) Run hooks manually

Run this command from project root to check all files

```
pre-commit run --all-files
```

#### Local workflow with pre-commit

1. Stage changes

```
git add <files>
```

2. Commit

```
git commit -m "message"
```

-   If files are not correctly formatted, the clang format will fail the first time pre-commit hook runs. But it will automatically format and lint the code.
-   After it has failed and auto format/auto lint, stage the files again and then run commit again. The pre-commit hooks will now pass.

3. Push

```
git push origin <branch-name>
```

## 2. Continuous Integration (CI) Workflow

**File:** `.github/workflows/ci.yaml`

The CI pipeline runs for every push to a feature/fix branch and every Pull Request(PR). The purpose of the CI pipeline is to validate the code is clean and follows the code standard, pass static checks, and can be successfully compiled before it is released.

### CI Jobs

1. pre-commit:
    - same pre-commit hook used in local development.
    - enforces code style standard and formatting before static analysis and compilation check
    - only auto-commit formatting fixes on develop/main branches
    - triggers on push and pull requests.
2. check-build-firmware:
    - runs only after `pre-commit` passes
    - injects/generates and loads secrets
    - runs static code analysis and then compiles the code for both of the target boards.
    - uploads the compiled firmware binaries as artifacts
3. docker-build:
    - runs only after `check-build-firmware` passes
    - builds the Docker image
    - uploads image archive `image.tar.gz` as an artifact

## 3. Continuous Deployment (CD) Workflow

**File:** `.github/workflows/cd.yaml`

The CD pipeline runs only after a successful merge or push to `main` branch (or when manually triggered with the `workflow_dispatch` on Github). The purpose of the CD pipeline is to deploy the validated artifacts (the firmware binaries and the compress Docker image).

### CD Jobs

1. release-firmware:
    - creates a Github release and attaches the firmware binaries as assetts that can be downloaded and flashed to hardware
    - downloads all the artifacts from the CI workflow (if it was successful)
    - generate a version tag `vYYYYMMDD.RUN_NUMBER`
    - a new, tagged release entry is listed on the repository's Release page with the binary files attached as assets
2. deploy-service:
    - deploys the Docker image as to Container Hub
    - downloads the compressed `iot-docker-image`artifact
    - authenticates to Docker Hub
    - tags the image with the commit SHA
    - pushes the image to Docker Hub
    - the image is available as an image that can be pulled and run as a container

## 4. Manual Docker Environment

This section explains how to manually build, run, and interact with the Docker container for development, testing, and experimentation outside the CI/CD pipeline.

#### Docker Setup

**Prerequisites:**

-   Docker CLI
-   (Optional) Docker Desktop
    > Note: If you use Docker Desktop on Windows, you'll need WSL2 or Hyper-V. For further information and installation guides, see [Docker Docs - Install Docker Desktop on Windows](https://docs.docker.com/desktop/setup/install/windows-install/)

#### Build the Docker Image

From the root of the project, build the Docker image by running:

```
docker build -t iot-chas-advance:<tag-name> .
```

-   `docker build`: builds the image from the `Dockerfile`
-   `-t <image-name>:<tag-name>`: assigns a name and tag to the image. Without a tag, Docker defaults to `latest`
-   `.`: the current directory
-   Recommended tags: `:dev`(local development), `:test` (testing), `v1.0`(versioned releases)

#### Run the container (for development)

Start the container interactively with shell and mounts a volume to the current directory. Changes to local files carry over to the container, and vice versa.

```
docker run --rm -it -v $(pwd):/workspace iot-chas-advance:<tag-name> sh
```

-   `--rm`: removes the container when you exit the container. Remove this flag if you want to keep the container for later.
-   `-it`: starts an interactive terminal(`-i`) with a pseudo-TTY(`-t`)
-   `-v $(pwd):/workspace`: volume mounting. Mounts the local project folder to `/workspace` inside the container.
-   `sh`: opens a shell program inside the container.

Inside the container, verify the mounted folder by running `ls /workspace`.

#### Interact inside the container

Once inside the container shell `/workspace`, you can run PlatformIO, Python, pre-commit, git, and curl commands.

##### Some basic shell commands to run inside container

```
pwd                 print working director
ls -la              list files and directories, visible and hidden
cd <dir>	        change directory
cat <file>          display file content.
(optional)
nano <file>
vim <file>	    edit file
```

##### Basic PlatformIO commands

**Help**

```
pio --help
```

**Build projects**

```
# Build Arduino
cd /workspace/Arduino
pio run -e uno_r4_wifi

```

**Static Code Analysis**

```
pio check -e uno_r4_wifi --fail-on-defect=medium --skip-packages
```

**List boards and targets**

```
pio boards
pio run --list-targets
```

**(optional)Installing tools**

```
pip install <package-name>

apt-get update && apt-get install -y <tool>
```

#### Exit the container

To exit and stop the container
Alt. 1:

```
exit
```

Alt. 2:

```
<CTRL+D>
```

Container will stop and be removed (if `--rm` flag is used)

#### Docker tooling guide (outside container)

These commands are useful for managing the Docker environment:

**List containers, even stopped containers**

```
docker ps -a
```

**List images**

```
docker images
```

**Clean up stopped containers, unused networks, and dangling images**

> Note: Use with caution.

```
docker system prune
```

**Inspect the contents of a container, running or stopped**

```
docker inspect <name or id of container>
```

**Restart a stopped container**

```
docker start <name or id of container>
```

**Access and interact with a stopped container**

```
docker exec -it <name or id of container> sh
```

**Remove a container**

```
docker rm <name or id of container>
```

**Remove image**

```
docker rmi <name or id of image>
```

#### Docker concept summary:

-   Docker image: Self-contained, read-only snapshot of the project, including all code, dependencies, and tools. Ensures consistency across all environments (local, CI runner, production).
-   Docker container: Running instance of an image. This is the isolated enironment where you interact with the project.
-   `COPY`(Dockerfile): Copies local files into the image at build time. Changesto local files after build will not carry over to the content inside the container. Best for CI/CD environments.
-   Volume mounting(`-v`): Overrides a container directory with a local host directory at run time. Changes inside the container carry over to the local host files, and vice versa. Best for active development.
