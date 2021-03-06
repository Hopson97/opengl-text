# OpenGL Text Testing

Testing to see if I can get opengl text rendering using sfml as a font loader.

## Progress [(Album)](https://imgur.com/a/HulQLra)

Text loaded to a texture:

![Text loaded to a texture:](https://i.imgur.com/LdkzzWk.png)

[Commit](https://github.com/Hopson97/opengl-text/tree/1217595952a72f73bd78b04eda9d59735d56876c)

___

Text rendered as a single vertex array, one character per quad.

![Text rendered to a quad](https://i.imgur.com/zY42eof.png)

[Commit](https://github.com/Hopson97/opengl-text/tree/e1a90a02a6b6a542813535ff324d2886910b04af)

____

Text as row of text, except the image texture seems to be upsidedown flipped.

![Upside down text row](https://i.imgur.com/EHqVyYT.png)

[Commit](https://github.com/Hopson97/opengl-text/commit/5813c3f56e0b1a9246035c2a2f367a5d224d7969)


____

Finally worked out texture coords except a lot of characters were positioned incorrectly

![All Letters](https://i.imgur.com/hVicTD8g.png)

Punctuation especially:

![Punctuation](https://i.imgur.com/jndDELM.png)

So added a bunch of hacky edge cases :/

Kinda works but idk prob is a better way:

![Final](https://cdn.discordapp.com/attachments/113418926909644800/671844350854168596/unknown.png)



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
