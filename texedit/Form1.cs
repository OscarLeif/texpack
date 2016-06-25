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
using System.IO;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms;
using System.Collections.Generic;

namespace texedit
{
    public partial class Form1 : Form
    {
        #region Fields

        Bitmap emptyBitmap;
        Spritesheet sheet;
        Point lastMousePos;
        bool dragging;
        bool modified;

        #endregion

        #region General

        public Form1()
        {
            InitializeComponent();

            emptyBitmap = new Bitmap("empty.png");
            pic_preview.InitialImage = emptyBitmap;
            pic_preview.ErrorImage = emptyBitmap;

            sheet = new Spritesheet();
            Text = "TexEdit - ...";
            updatePreview();
            modified = false;
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            AskSave();
        }

        void AskSave()
        {
            if (modified)
            {
                var result = MessageBox.Show("Save changes?", "Save changes", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (result == DialogResult.Yes)
                {
                    sheet.Save();
                    modified = false;
                }
            }
        }

        int SelectedIndex { get { return list_sprites.SelectedIndex; } }

        List<int> SelectedIndices
        {
            get
            {
                List<int> result = new List<int>();
                foreach (var idx in list_sprites.SelectedIndices)
                    result.Add((int)idx);
                return result;
            }
        }

        #endregion

        #region Properties

        private void txt_name_TextChanged(object sender, System.EventArgs e)
        {
            
        }

        private void btn_path_Click(object sender, System.EventArgs e)
        {
            OpenFileDialog d = new OpenFileDialog();
            d.Filter = "PNG|*.png";
            d.CheckFileExists = true;
            d.CheckPathExists = true;
            d.ShowDialog();
            var newfile = d.FileName;

            if (string.IsNullOrEmpty(newfile)) return;
            if (SelectedIndices.Count != 1) return;

            sheet.Sprites[SelectedIndex].Path = newfile;
            modified = true;

            updateProperties();
            updatePreview();
        }

        private void txt_name_Leave(object sender, System.EventArgs e)
        {
            //No selection
            if (SelectedIndices.Count == 0)
            {
                updateProperties();
                return;
            }

            //Empty field
            if (string.IsNullOrEmpty(txt_name.Text))
            {
                updateProperties();
                return;
            }
            else if (SelectedIndices.Count == 1)
            {
                foreach (var spr in sheet.Sprites)
                    if (spr.Name == txt_name.Text && spr != sheet.Sprites[SelectedIndex])
                    {
                        MessageBox.Show("Sprites have duplicate names!", "Duplicate names", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        return;
                    }

                modified = true;
                sheet.Sprites[SelectedIndex].Name = txt_name.Text;
                list_sprites.Items[SelectedIndex] = txt_name.Text;
            }

            updateProperties();
        }

        private void txt_offx_TextChanged(object sender, System.EventArgs e)
        {
            modified = true;
            foreach (var idx in SelectedIndices)
                sheet.Sprites[idx].Offset.X = getNumericInput(txt_offx, sheet.Sprites[idx].Offset.X);
            
            updatePreview();
        }

        private void txt_offy_TextChanged(object sender, System.EventArgs e)
        {
            modified = true;
            foreach (var idx in SelectedIndices)
                sheet.Sprites[idx].Offset.Y = getNumericInput(txt_offy, sheet.Sprites[idx].Offset.Y);

            updatePreview();
        }

        private void txt_sclx_TextChanged(object sender, System.EventArgs e)
        {
            modified = true;
            foreach (var idx in SelectedIndices)
                sheet.Sprites[idx].Scale.X = getNumericInput(txt_sclx, sheet.Sprites[idx].Scale.X);

            updatePreview();
        }

        private void txt_scly_TextChanged(object sender, System.EventArgs e)
        {
            modified = true;
            foreach (var idx in SelectedIndices)
                sheet.Sprites[idx].Scale.Y = getNumericInput(txt_scly, sheet.Sprites[idx].Scale.Y);

            updatePreview();
        }

        T getNumericInput<T>(TextBox textbox, T def)
        {
            T newval = def;
            MethodInfo nfo = typeof(T).GetMethod("Parse", new[] { typeof(string) });
            if (nfo == null) return newval;
            try { newval = (T)nfo.Invoke(null, new[] { textbox.Text }); }
            catch (Exception) { }
            return newval;
        }

        void filterTextbox<T>(ref TextBox textbox) where T : struct
        {
            string newval = textbox.Text;
            MethodInfo nfo = typeof(T).GetMethod("Parse", new [] { typeof(string) });
            if (nfo == null) return;
            T result = default(T);
            try { result = (T)nfo.Invoke(null, new[] { newval }); }
            catch (Exception) { }
            textbox.Text = result.ToString();
        }

        void updateProperties()
        {
            if (SelectedIndices.Count == 0)
            {
                txt_name.SetText("");
                txt_path.SetText("");
                txt_offx.SetText("");
                txt_offy.SetText("");
                txt_sclx.SetText("");
                txt_scly.SetText("");
            }
            else
            {
                txt_name.SetText(sheet.Sprites[SelectedIndex].Name);
                foreach (var idx in SelectedIndices)
                    if (txt_name.Text != sheet.Sprites[idx].Name)
                    { txt_name.SetText(""); break; }

                txt_path.SetText(sheet.Sprites[SelectedIndex].Path);
                foreach (var idx in SelectedIndices)
                    if (txt_path.Text != sheet.Sprites[idx].Path)
                    { txt_path.SetText(""); break; }

                txt_offx.SetText(sheet.Sprites[SelectedIndex].Offset.X.ToString());
                foreach (var idx in SelectedIndices)
                    if (txt_offx.Text != sheet.Sprites[idx].Offset.X.ToString())
                    { txt_offx.SetText(""); break; }

                txt_offy.SetText(sheet.Sprites[SelectedIndex].Offset.Y.ToString());
                foreach (var idx in SelectedIndices)
                    if (txt_offy.Text != sheet.Sprites[idx].Offset.Y.ToString())
                    { txt_offy.SetText(""); break; }

                txt_sclx.SetText(sheet.Sprites[SelectedIndex].Scale.X.ToString());
                foreach (var idx in SelectedIndices)
                    if (txt_sclx.Text != sheet.Sprites[idx].Scale.X.ToString())
                    { txt_sclx.SetText(""); break; }

                txt_scly.SetText(sheet.Sprites[SelectedIndex].Scale.Y.ToString());
                foreach (var idx in SelectedIndices)
                    if (txt_scly.Text != sheet.Sprites[idx].Scale.Y.ToString())
                    { txt_scly.SetText(""); break; }
            }
        }

        #endregion

        #region Menu

        private void exitToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            Close();
        }

        private void openSpritesheetToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            AskSave();

            FolderBrowserDialog browser = new FolderBrowserDialog();
            browser.Description = "Navigate to spritesheet folder";
            browser.SelectedPath = Directory.GetCurrentDirectory();
            var result = browser.ShowDialog();
            if (result != DialogResult.OK) return;
            string dir = browser.SelectedPath;

            modified = false;
            sheet = new Spritesheet();
            sheet.Load(dir);
            Text = "TexEdit - " + sheet.ProjectName;

            updateList();
            updateProperties();
            updatePreview();
        }

        private void automatedToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            AskSave();

            FolderBrowserDialog browser = new FolderBrowserDialog();
            browser.Description = "Navigate to spritesheet folder";
            browser.SelectedPath = Directory.GetCurrentDirectory();
            var result = browser.ShowDialog();
            if (result != DialogResult.OK) return;
            string dir = browser.SelectedPath;
            
            modified = false;
            sheet = new Spritesheet();
            sheet.MoveDir(dir);
            sheet.Rename(Path.GetFileName(dir));
            Text = "TexEdit - " + sheet.ProjectName;

            var files_png = Directory.GetFiles(dir, "*.png", SearchOption.TopDirectoryOnly);
            foreach (var file in files_png)
                sheet.Sprites.Add(new Sprite { Name = Path.GetFileName(file), Path = file, Offset = new Point(0, 0), Scale = new PointF(1.0f, 1.0f) });

            var files_jpeg = Directory.GetFiles(dir, "*.jpeg", SearchOption.TopDirectoryOnly);
            foreach (var file in files_jpeg)
                sheet.Sprites.Add(new Sprite { Name = Path.GetFileName(file), Path = file, Offset = new Point(0, 0), Scale = new PointF(1.0f, 1.0f) });

            updateList();
            updateProperties();
            updatePreview();
        }

        private void emptySpritesheetToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            AskSave();

            FolderBrowserDialog browser = new FolderBrowserDialog();
            browser.Description = "Navigate to spritesheet folder";
            browser.SelectedPath = Directory.GetCurrentDirectory();
            var result = browser.ShowDialog();
            if (result != DialogResult.OK) return;
            string dir = browser.SelectedPath;

            modified = false;
            sheet = new Spritesheet();
            sheet.MoveDir(dir);
            sheet.Rename(Path.GetFileName(dir));
            Text = "TexEdit - " + sheet.ProjectName;

            updateList();
            updateProperties();
            updatePreview();
        }

        private void aboutToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            MessageBox.Show("TexEdit v0.1", "About TexEdit", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void saveSpritesheetToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            if (sheet == null)
                return;

            sheet.Save();
            modified = false;
        }

        #endregion

        #region Sprites list

        private void list_sprites_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            updateProperties();
            updatePreview();
        }

        private void btn_add_Click(object sender, System.EventArgs e)
        {
            OpenFileDialog d = new OpenFileDialog();
            d.Filter = "PNG|*.png";
            d.CheckFileExists = true;
            d.CheckPathExists = true;
            d.Multiselect = true;
            d.ShowDialog();
            foreach (var file in d.FileNames)
            {
                Sprite spr = new Sprite();
                spr.Path = file;
                spr.Name = Path.GetFileName(file);
                spr.Offset = new Point(0, 0);
                spr.Scale = new PointF(1.0f, 1.0f);
                sheet.Sprites.Add(spr);
                modified = true;
            }

            updateList();
        }

        private void btn_del_Click(object sender, System.EventArgs e)
        {
            if (SelectedIndices.Count == 0)
                return;

            modified = true;
            foreach (var idx in SelectedIndices)
                sheet.Sprites.RemoveAt(idx);

            updateList();
        }

        void updateList()
        {
            var oldsel = list_sprites.SelectedItem;
            list_sprites.Items.Clear();
            foreach (var spr in sheet.Sprites)
                list_sprites.Items.Add(spr.Name);
            list_sprites.SelectedItem = oldsel;

            updatePreview();
        }

        #endregion

        #region Preview

        private void pic_preview_Click(object sender, System.EventArgs e)
        {
            updatePreview();
        }

        private void pic_preview_MouseDown(object sender, MouseEventArgs e)
        {
            if (dragging) return;
            if (e.Button != MouseButtons.Left) return;
            if (SelectedIndices.Count != 1) return;

            lastMousePos = e.Location;
            dragging = true;

            updatePreview();
            updateProperties();
        }

        private void pic_preview_MouseUp(object sender, MouseEventArgs e)
        {
            if (!dragging) return;
            if (e.Button != MouseButtons.Left) return;
            if (SelectedIndices.Count != 1) return;

            /*Size delta = new Size(e.Location) - new Size(lastMousePos);
            sheet.Sprites[SelectedIndex].Offset.X -= delta.Width;
            sheet.Sprites[SelectedIndex].Offset.Y += delta.Height;*/
            dragging = false;

            updateProperties();
            updatePreview();
        }

        private void pic_preview_MouseMove(object sender, MouseEventArgs e)
        {
            if (!dragging) return;
            if (e.Button != MouseButtons.Left) return;
            if (SelectedIndices.Count != 1) return;

            modified = true;
            Size delta = new Size(e.Location) - new Size(lastMousePos);
            sheet.Sprites[SelectedIndex].Offset.X -= delta.Width;
            sheet.Sprites[SelectedIndex].Offset.Y += delta.Height;

            lastMousePos = e.Location;
            updateProperties();
            updatePreview();
        }

        void updatePreview()
        {
            if (list_sprites.SelectedItem == null)
            {
                pic_preview.Image = emptyBitmap;
            }
            else
            {
                Image src = Image.FromFile(Path.Combine(sheet.BaseDir, sheet.Sprites[SelectedIndex].Path));
                Image dst = new Bitmap(pic_preview.Width, pic_preview.Height);
                using (Graphics gr = Graphics.FromImage(dst))
                {
                    //Calculations :#
                    PointF scale = sheet.Sprites[SelectedIndex].Scale;
                    Point origin = sheet.Sprites[SelectedIndex].Offset;
                    origin.X *= -1;
                    origin.X += pic_preview.Size.Width / 2;
                    origin.Y += pic_preview.Size.Height / 2;

                    //Image
                    gr.DrawImage(src,
                        new RectangleF(origin.X, origin.Y - src.Height*scale.Y, src.Width*scale.X, src.Height*scale.Y),
                        new RectangleF(0, 0, src.Width, src.Height),
                        GraphicsUnit.Pixel);

                    //Guides
                    gr.DrawLine(Pens.Black, new Point(0, pic_preview.Height / 2), new Point(pic_preview.Width, pic_preview.Height / 2));
                    gr.DrawLine(Pens.Black, new Point(pic_preview.Width / 2, 0), new Point(pic_preview.Width / 2, pic_preview.Height));
                }

                pic_preview.Image = dst;
            }
        }

        #endregion
    }
}
