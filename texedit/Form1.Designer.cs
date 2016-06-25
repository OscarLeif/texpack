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

namespace texedit
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.list_sprites = new System.Windows.Forms.ListBox();
            this.txt_offx = new texedit.NumericTextBox();
            this.lbl_offx = new System.Windows.Forms.Label();
            this.lbl_offy = new System.Windows.Forms.Label();
            this.txt_offy = new texedit.NumericTextBox();
            this.lbl_sclx = new System.Windows.Forms.Label();
            this.txt_sclx = new texedit.NumericTextBox();
            this.lbl_scly = new System.Windows.Forms.Label();
            this.txt_scly = new texedit.NumericTextBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newSpritesheetToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.emptySpritesheetToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.automatedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openSpritesheetToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveSpritesheetToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pic_preview = new System.Windows.Forms.PictureBox();
            this.btn_add = new System.Windows.Forms.Button();
            this.btn_del = new System.Windows.Forms.Button();
            this.txt_name = new System.Windows.Forms.TextBox();
            this.lbl_name = new System.Windows.Forms.Label();
            this.txt_path = new System.Windows.Forms.TextBox();
            this.btn_path = new System.Windows.Forms.Button();
            this.lbl_path = new System.Windows.Forms.Label();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pic_preview)).BeginInit();
            this.SuspendLayout();
            // 
            // list_sprites
            // 
            this.list_sprites.FormattingEnabled = true;
            this.list_sprites.Location = new System.Drawing.Point(12, 26);
            this.list_sprites.Name = "list_sprites";
            this.list_sprites.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.list_sprites.Size = new System.Drawing.Size(120, 394);
            this.list_sprites.TabIndex = 0;
            this.list_sprites.SelectedIndexChanged += new System.EventHandler(this.list_sprites_SelectedIndexChanged);
            // 
            // txt_offx
            // 
            this.txt_offx.AllowSpace = false;
            this.txt_offx.Location = new System.Drawing.Point(672, 142);
            this.txt_offx.Name = "txt_offx";
            this.txt_offx.Size = new System.Drawing.Size(100, 20);
            this.txt_offx.TabIndex = 1;
            this.txt_offx.TextChanged += new System.EventHandler(this.txt_offx_TextChanged);
            // 
            // lbl_offx
            // 
            this.lbl_offx.AutoSize = true;
            this.lbl_offx.Location = new System.Drawing.Point(622, 145);
            this.lbl_offx.Name = "lbl_offx";
            this.lbl_offx.Size = new System.Drawing.Size(48, 13);
            this.lbl_offx.TabIndex = 2;
            this.lbl_offx.Text = "Offset X:";
            this.lbl_offx.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lbl_offy
            // 
            this.lbl_offy.AutoSize = true;
            this.lbl_offy.Location = new System.Drawing.Point(622, 171);
            this.lbl_offy.Name = "lbl_offy";
            this.lbl_offy.Size = new System.Drawing.Size(48, 13);
            this.lbl_offy.TabIndex = 4;
            this.lbl_offy.Text = "Offset Y:";
            this.lbl_offy.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // txt_offy
            // 
            this.txt_offy.AllowSpace = false;
            this.txt_offy.Location = new System.Drawing.Point(672, 168);
            this.txt_offy.Name = "txt_offy";
            this.txt_offy.Size = new System.Drawing.Size(100, 20);
            this.txt_offy.TabIndex = 3;
            this.txt_offy.TextChanged += new System.EventHandler(this.txt_offy_TextChanged);
            // 
            // lbl_sclx
            // 
            this.lbl_sclx.AutoSize = true;
            this.lbl_sclx.Location = new System.Drawing.Point(622, 221);
            this.lbl_sclx.Name = "lbl_sclx";
            this.lbl_sclx.Size = new System.Drawing.Size(47, 13);
            this.lbl_sclx.TabIndex = 6;
            this.lbl_sclx.Text = "Scale X:";
            this.lbl_sclx.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // txt_sclx
            // 
            this.txt_sclx.AllowSpace = false;
            this.txt_sclx.Enabled = false;
            this.txt_sclx.Location = new System.Drawing.Point(672, 218);
            this.txt_sclx.Name = "txt_sclx";
            this.txt_sclx.Size = new System.Drawing.Size(100, 20);
            this.txt_sclx.TabIndex = 5;
            this.txt_sclx.TextChanged += new System.EventHandler(this.txt_sclx_TextChanged);
            // 
            // lbl_scly
            // 
            this.lbl_scly.AutoSize = true;
            this.lbl_scly.Location = new System.Drawing.Point(622, 247);
            this.lbl_scly.Name = "lbl_scly";
            this.lbl_scly.Size = new System.Drawing.Size(47, 13);
            this.lbl_scly.TabIndex = 8;
            this.lbl_scly.Text = "Scale Y:";
            this.lbl_scly.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // txt_scly
            // 
            this.txt_scly.AllowSpace = false;
            this.txt_scly.Enabled = false;
            this.txt_scly.Location = new System.Drawing.Point(672, 244);
            this.txt_scly.Name = "txt_scly";
            this.txt_scly.Size = new System.Drawing.Size(100, 20);
            this.txt_scly.TabIndex = 7;
            this.txt_scly.TextChanged += new System.EventHandler(this.txt_scly_TextChanged);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(784, 24);
            this.menuStrip1.TabIndex = 9;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newSpritesheetToolStripMenuItem,
            this.openSpritesheetToolStripMenuItem,
            this.saveSpritesheetToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // newSpritesheetToolStripMenuItem
            // 
            this.newSpritesheetToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.emptySpritesheetToolStripMenuItem,
            this.automatedToolStripMenuItem});
            this.newSpritesheetToolStripMenuItem.Name = "newSpritesheetToolStripMenuItem";
            this.newSpritesheetToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
            this.newSpritesheetToolStripMenuItem.Text = "New spritesheet";
            // 
            // emptySpritesheetToolStripMenuItem
            // 
            this.emptySpritesheetToolStripMenuItem.Name = "emptySpritesheetToolStripMenuItem";
            this.emptySpritesheetToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
            this.emptySpritesheetToolStripMenuItem.Text = "Empty spritesheet";
            this.emptySpritesheetToolStripMenuItem.Click += new System.EventHandler(this.emptySpritesheetToolStripMenuItem_Click);
            // 
            // automatedToolStripMenuItem
            // 
            this.automatedToolStripMenuItem.Name = "automatedToolStripMenuItem";
            this.automatedToolStripMenuItem.Size = new System.Drawing.Size(168, 22);
            this.automatedToolStripMenuItem.Text = "Automated";
            this.automatedToolStripMenuItem.Click += new System.EventHandler(this.automatedToolStripMenuItem_Click);
            // 
            // openSpritesheetToolStripMenuItem
            // 
            this.openSpritesheetToolStripMenuItem.Name = "openSpritesheetToolStripMenuItem";
            this.openSpritesheetToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
            this.openSpritesheetToolStripMenuItem.Text = "Open spritesheet";
            this.openSpritesheetToolStripMenuItem.Click += new System.EventHandler(this.openSpritesheetToolStripMenuItem_Click);
            // 
            // saveSpritesheetToolStripMenuItem
            // 
            this.saveSpritesheetToolStripMenuItem.Name = "saveSpritesheetToolStripMenuItem";
            this.saveSpritesheetToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
            this.saveSpritesheetToolStripMenuItem.Text = "Save spritesheet";
            this.saveSpritesheetToolStripMenuItem.Click += new System.EventHandler(this.saveSpritesheetToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
            this.aboutToolStripMenuItem.Text = "About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // pic_preview
            // 
            this.pic_preview.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.pic_preview.Location = new System.Drawing.Point(138, 26);
            this.pic_preview.Name = "pic_preview";
            this.pic_preview.Size = new System.Drawing.Size(478, 423);
            this.pic_preview.TabIndex = 10;
            this.pic_preview.TabStop = false;
            this.pic_preview.Click += new System.EventHandler(this.pic_preview_Click);
            this.pic_preview.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pic_preview_MouseDown);
            this.pic_preview.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pic_preview_MouseMove);
            this.pic_preview.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pic_preview_MouseUp);
            // 
            // btn_add
            // 
            this.btn_add.Location = new System.Drawing.Point(12, 426);
            this.btn_add.Name = "btn_add";
            this.btn_add.Size = new System.Drawing.Size(91, 23);
            this.btn_add.TabIndex = 11;
            this.btn_add.Text = "Add sprite";
            this.btn_add.UseVisualStyleBackColor = true;
            this.btn_add.Click += new System.EventHandler(this.btn_add_Click);
            // 
            // btn_del
            // 
            this.btn_del.Location = new System.Drawing.Point(109, 426);
            this.btn_del.Name = "btn_del";
            this.btn_del.Size = new System.Drawing.Size(22, 23);
            this.btn_del.TabIndex = 12;
            this.btn_del.Text = "X";
            this.btn_del.UseVisualStyleBackColor = true;
            this.btn_del.Click += new System.EventHandler(this.btn_del_Click);
            // 
            // txt_name
            // 
            this.txt_name.Location = new System.Drawing.Point(626, 42);
            this.txt_name.Name = "txt_name";
            this.txt_name.Size = new System.Drawing.Size(147, 20);
            this.txt_name.TabIndex = 13;
            this.txt_name.TextChanged += new System.EventHandler(this.txt_name_TextChanged);
            this.txt_name.Leave += new System.EventHandler(this.txt_name_Leave);
            // 
            // lbl_name
            // 
            this.lbl_name.AutoSize = true;
            this.lbl_name.Location = new System.Drawing.Point(622, 26);
            this.lbl_name.Name = "lbl_name";
            this.lbl_name.Size = new System.Drawing.Size(68, 13);
            this.lbl_name.TabIndex = 14;
            this.lbl_name.Text = "Frame name:";
            // 
            // txt_path
            // 
            this.txt_path.Location = new System.Drawing.Point(626, 83);
            this.txt_path.Name = "txt_path";
            this.txt_path.ReadOnly = true;
            this.txt_path.Size = new System.Drawing.Size(115, 20);
            this.txt_path.TabIndex = 15;
            // 
            // btn_path
            // 
            this.btn_path.Location = new System.Drawing.Point(747, 81);
            this.btn_path.Name = "btn_path";
            this.btn_path.Size = new System.Drawing.Size(25, 23);
            this.btn_path.TabIndex = 16;
            this.btn_path.Text = "...";
            this.btn_path.UseVisualStyleBackColor = true;
            this.btn_path.Click += new System.EventHandler(this.btn_path_Click);
            // 
            // lbl_path
            // 
            this.lbl_path.AutoSize = true;
            this.lbl_path.Location = new System.Drawing.Point(625, 67);
            this.lbl_path.Name = "lbl_path";
            this.lbl_path.Size = new System.Drawing.Size(26, 13);
            this.lbl_path.TabIndex = 17;
            this.lbl_path.Text = "File:";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 461);
            this.Controls.Add(this.lbl_path);
            this.Controls.Add(this.btn_path);
            this.Controls.Add(this.txt_path);
            this.Controls.Add(this.lbl_name);
            this.Controls.Add(this.txt_name);
            this.Controls.Add(this.btn_del);
            this.Controls.Add(this.btn_add);
            this.Controls.Add(this.pic_preview);
            this.Controls.Add(this.lbl_scly);
            this.Controls.Add(this.txt_scly);
            this.Controls.Add(this.lbl_sclx);
            this.Controls.Add(this.txt_sclx);
            this.Controls.Add(this.lbl_offy);
            this.Controls.Add(this.txt_offy);
            this.Controls.Add(this.lbl_offx);
            this.Controls.Add(this.txt_offx);
            this.Controls.Add(this.list_sprites);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.Text = "TexEdit";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pic_preview)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox list_sprites;
        private System.Windows.Forms.Label lbl_offx;
        private System.Windows.Forms.Label lbl_offy;
        private System.Windows.Forms.Label lbl_sclx;
        private System.Windows.Forms.Label lbl_scly;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newSpritesheetToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openSpritesheetToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem emptySpritesheetToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem automatedToolStripMenuItem;
        private System.Windows.Forms.PictureBox pic_preview;
        private System.Windows.Forms.Button btn_add;
        private System.Windows.Forms.Button btn_del;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveSpritesheetToolStripMenuItem;
        private System.Windows.Forms.TextBox txt_name;
        private System.Windows.Forms.Label lbl_name;
        private System.Windows.Forms.TextBox txt_path;
        private System.Windows.Forms.Button btn_path;
        private System.Windows.Forms.Label lbl_path;
        private NumericTextBox txt_offx;
        private NumericTextBox txt_offy;
        private NumericTextBox txt_sclx;
        private NumericTextBox txt_scly;
    }
}

