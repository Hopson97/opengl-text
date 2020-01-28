# gltexttest

Testing to see if I can get opengl text rendering using sfml as a font loader.

## Progress

Text loaded to a texture:

![Text loaded to a texture:](https://i.imgur.com/LdkzzWk.png)

[Commit](https://github.com/Hopson97/opengl-text/tree/1217595952a72f73bd78b04eda9d59735d56876c)

___

## Building and Running

### Libraries

SFML and GLM are required.

These can be installed from your project manager. For example, on Debian/ Ubuntu:

```sh
sudo apt install libsfml-dev libglm-dev
```

If this is not possible (eg windows), you can install these manually from their respective websites:

https://www.sfml-dev.org/download.php

https://github.com/g-truc/glm/tags

### Linux

To build, at the root of the project:

```sh
sh scripts/build.sh
```

To run, at the root of the project:

```sh
sh scripts/run.sh
```

To build and run in release mode, simply add the `release` suffix:

```sh
sh scripts/build.sh release
sh scripts/run.sh release
```

You can also create a deployable build (that can be sent) by doing:

```sh
sh scripts/deploy.sh
```
