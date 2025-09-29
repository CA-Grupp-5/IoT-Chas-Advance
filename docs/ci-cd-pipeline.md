# Docker and CI/CD workflow (temporary title)

This guide explains how to manually build, run, and interact with the Docker container for development, testing, and experimentation in the IoT PlatformIO project. It also covers some Docker concepts, commands, and recommended practices (see end of documentation).

## Setup

**Preqrequisites:**

- Docker CLI
- (optional) Docker Desktop

### 1. Build the Docker image

From the root of the project (where Dockerfile is located), build the Docker image:

```
docker build -t iot-chas-advance:<tag-name> .
```

- ```docker build``` build the image specified in the Dockerfile.
- ```-t <image-name>:<tag-name``` assign a name to the image and a tag to the image
	- without a tag, Docker will default to ```latest```
- ```.``` the current directory
- recommended tags to use:
	- ```:dev``` local development
	- ```:test``` testing
	- ```v1.0``` versioned releases

### 2. Run the container

Start the container interactively with a mounted volume so changes in container and on the local files carry over:
```
docker run --rm -it -v $(pwd):/workspace iot-chas-advance:<tag-name> sh
```

Or if you don't want to automatically remove the container after exiting, just remove ```--rm```

- ```docker run``` start and run the container
- ```--rm``` removes container when you exit.
- ```--it``` start an interactive shell program to interact with the container
- ```-v $(pwd):/workspace``` mounts the local project folder to ```workspace``` inside the container. Changes made to the local folder carry over, and vice versa.
- ```iot-chas-advance:<tag-name>``` use the whole name and the tag you gave the image
- ```sh``` opens a bash shell inside the container and let you interact with the container

### 3. Interact with the project folder

You should now be located inside the container. Verify the mounted/copied container folder by running:
```ls /workspace```

You should see something like:
```
Arduino     S3      docs    <other files at root>
```

#### Some basic shell commands to run inside container

```
pwd                 print working director
ls -la              list files and directories, visible and hidden
cd <dir>	        change directory
cat <file>          display file content.
(optional)
nano <file>
vim <file>	    edit file
```

#### Basic PlatformIO commands

**Help**

```
pio --help
```

**Build projects**

```
# Build Arduino
cd /workspace/Arduino
pio run -e uno_r4_wifi

# Build ESP32
cd /workspace/Arduino
pio run -e esp32_broker
```

**Static Code Analysis**

```
pio check -e uno_r4_wifi --fail-on-defect=medium --skip-packages
```

**Upload firmware if board is connected**

```
pio run -e uno_r4_wifi -t upload
```

**List boards and targets**

```
pio boards
pio run --list-targets
```

#### Manually run pre-commit hooks (or tests)

If ```pre-commit``` is installed, run from ```/workspace```
```
pre-commit run --all-files
```

**(optional)Installing tools**
```
pip install <package-name>

apt-get update && apt-get install -y <tool>
```

### 4. Exit the container
The exit and stop the container
```
exit
<CTRL+D>			exit container shell
```

Container will stop and be removed
As long as the local project folder is mounted as a volume, any changes in ```/workspace``` carry over.

---
## Docker guide
Outside the container, you can run some Docker commands:

List containers, even stopped containers
```
docker ps -a
```

List images
```
docker images
```
Clean up/remove stopped containers, unused networks, unused build cache, and dangling images(image that isn't taggedor referenced by any container). Good way to keep the Docker space clean, just be mindful that **it will remove a lot of stuff**.

```
docker system prune
```

Inspect the contents of a container, running or stopped
```
docker inspect <name or id of container>
```

Restart a stopped container
```
docker start <name or id of container>
```

Access and interact with a stopped container
```
docker exec -it <name or id of container> sh
```

Remove a container

```
docker rm <name or id of container>
```

Remove image

```
docker rmi <name or id of image>
```

### Docker images and Container basics

- **Docker image**: Self-contained snapshot of the project, including all dependencies and tools needed to run the project. You package everything your local project would need to run and execute just like it does in your local environment. This will make the containerized project run in a consistent way no matter what system it is running on: your computer, your teammate's computer, a remote server, och in the Cloud.
	- In our case: Python, pip, PlatformIO dependencies, and Arduino Uno R4 and ESP32 S3 Zero specific libraries

- **Docker container**: The running instance of the image. Interact with it, run commands, test, and experiment inside.
	- In our case: From inside the container, we can interact just like in our local environment (running python, pip, platformio, bash, shell, git etc). Meaning we can build, run static checks, or tests.

- **COPY**: In the Dockerfile, "COPY" copies the local project folder and its contents into the image at build time. Changes to local files after building will not carry over to the content inside the container. Think of it as a "snapshot" of the project at build time.
> For CI/CD and product, just use "COPY"

- **Volume mounting**: Volume mounting: At run time, ovverrides the folder in the container with the local project folder. Changes inside the container carry over to the local host files, and vice versa.
> Helpful for development and testing, since you don't need to rebuild the image every time any changes are made to the code

### Image tag/versioning

At build time, it's recommended to give the image a tag, in order to differentiate the image(if you are building and running different versions, for example v.1.0, v.2.0, test, production etc.)
