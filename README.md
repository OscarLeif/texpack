# texpack

texpack is a bundle of an editor and a packer applications that ease the work with spritesheets (specially designed for Cocos2D applications). It's capable of binpacking sprites and outputting a .plist indexing file.

The indexing file format complies with Texture Packer's .plist format, which is already implemented in Cocos2D, so everything is ready for usage.

The project is released under the MIT license - feel free to use it in any way you wish :)

# Using the GUI editor

texedit is a very simple tool for editing your spritesheets. A spritesheet is basically a folder with a JSON file with the same name inside it. You can create an empty spritesheet or automatically generate one from a folder of images (currently supporting .PNG and .JPEG). After loading/creating a spritesheet you can add or remove sprites to it. You can rename them and give them different offsets. Best keep all your spritesheets in one folder.

# Using the packing tool

Using texpack, the packaging software is as easy as pie. All you need to do is run the application with three command line arguments. The first one is the command "-ps", the second - a directory, containing all your spritesheets, the third one is the folder in which it should output all the compiled images and indexing files. That's it.

# Using inside Cocos2D-X

```c++
auto sprcache = SpriteFrameCache::getInstance();
sprcache->addSpriteFramesWithFile("terrain.plist");

auto spr = Sprite::createWithSpriteFrameName("grass");
```

Simple as that (:
