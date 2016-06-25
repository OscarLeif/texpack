/*

Copyright (c) 2016 Botyto

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

using System.Collections.Generic;
using System.Drawing;
using System.IO;

namespace texedit
{
    class Sprite
    {
        public string Path;
        public string Name;
        public Point Offset;
        public PointF Scale;
    }

    class Spritesheet
    {
        public List<Sprite> Sprites { get; private set; }
        public string BaseDir       { get; private set; }
        public string ProjectName   { get; private set; }

        public Spritesheet()
        {
            Sprites = new List<Sprite>();
            BaseDir = "";
        }

        public void MoveDir(string dir)
        {
            BaseDir = dir;
        }

        public void Rename(string name)
        {
            ProjectName = name;
        }

        public void Load(string folder)
        {
            ProjectName = Path.GetFileName(folder);
            var settings_file = Path.Combine(folder, ProjectName + ".json");
            if (!File.Exists(settings_file))
                return;

            BaseDir = folder;

            string json = "";
            using (StreamReader sr = new StreamReader(settings_file))
                json = sr.ReadToEnd();
            LitJson.JsonReader reader = new LitJson.JsonReader(json);
            Sprites = LitJson.JsonMapper.ToObject<List<Sprite>>(reader);
        }

        public void Save()
        {
            var settings_file = Path.Combine(BaseDir, ProjectName + ".json");
            using (StreamWriter writer = new StreamWriter(settings_file))
            {
                LitJson.JsonWriter jwriter = new LitJson.JsonWriter(writer);
                jwriter.PrettyPrint = true;
                jwriter.IndentValue = 4;
                LitJson.JsonMapper.ToJson(Sprites, jwriter);
            }
        }
    }
}
