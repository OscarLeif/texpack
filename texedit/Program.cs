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

using System;
using System.Drawing;
using System.Windows.Forms;

namespace texedit
{
    static class Program
    {
        static void JsonExportPoint(Point pt, LitJson.JsonWriter writer)
        {
            writer.WriteObjectStart();
            writer.WritePropertyName("X");
            writer.Write(pt.X);
            writer.WritePropertyName("Y");
            writer.Write(pt.Y);
            writer.WriteObjectEnd();
        }

        static void JsonExportPointF(PointF pt, LitJson.JsonWriter writer)
        {
            writer.WriteObjectStart();
            writer.WritePropertyName("X");
            writer.Write(pt.X);
            writer.WritePropertyName("Y");
            writer.Write(pt.Y);
            writer.WriteObjectEnd();
        }

        static float JsonImportFloat(double data)
        {
            return (float)data;
        }

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            LitJson.JsonMapper.RegisterExporter<Point>(JsonExportPoint);
            LitJson.JsonMapper.RegisterExporter<PointF>(JsonExportPointF);
            LitJson.JsonMapper.RegisterImporter<double, float>(JsonImportFloat);

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
