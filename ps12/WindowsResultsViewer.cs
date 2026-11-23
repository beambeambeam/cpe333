// Read-only viewer of captured stdout/stderr, not a terminal emulator.
using System;
using System.Drawing;
using System.IO;
using System.Web.Script.Serialization;
using System.Collections;
using System.Windows.Forms;

class WindowsResultsViewer {
    [STAThread]
    static void Main(string[] args) {
        Application.EnableVisualStyles();
        var json = new JavaScriptSerializer(); json.MaxJsonLength = 10000000;
        var manifest = json.DeserializeObject(File.ReadAllText(args[0])) as System.Collections.Generic.Dictionary<string,object>;
        var form = new Form { Text="PS12 Windows - Captured command results", Size=new Size(1500,1000), StartPosition=FormStartPosition.CenterScreen };
        var header = new Label { Dock=DockStyle.Top, Height=62, Font=new Font("Segoe UI",11), Padding=new Padding(15,8,0,0), Text="PS12 / Lab 1 / Windows  |  " + manifest["capturedAt"] + "\r\nRead-only view of captured cmd.exe output. Commands and exit codes come from results/05_windows_links.txt." };
        var tabs = new TabControl { Dock=DockStyle.Fill, Font=new Font("Segoe UI",10) };
        var titles = new [] {"00 Environment","01 Basic links","02a Delete source","02b Delete level 1","02c Hard links","03a Directories","03b Loop","04 Cross-drive"};
        int i=0;
        foreach (System.Collections.Generic.Dictionary<string,object> section in (object[])manifest["sections"]) {
            var tab = new TabPage(titles[i++]) { Padding=new Padding(10) };
            var heading = new Label { Text=(string)section["title"], Dock=DockStyle.Top, Height=35, Font=new Font("Segoe UI",13,FontStyle.Bold) };
            var table = new TableLayoutPanel { Dock=DockStyle.Fill, ColumnCount=2, RowCount=1 };
            table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,50)); table.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,50));
            var commands = (object[])section["commands"];
            int split=(commands.Length+1)/2;
            if (i==1) split=Math.Min(3,commands.Length);
            if (i==5) split=Math.Min(8,commands.Length);
            for (int col=0;col<2;col++) {
                int start=col==0?0:split, end=col==0?split:commands.Length;
                if (start>=end) continue;
                var first=(System.Collections.Generic.Dictionary<string,object>)commands[start];
                var text="Working directory: " + first["cwd"] + "\r\n\r\n";
                for(int j=start;j<end;j++) {
                    var c=(System.Collections.Generic.Dictionary<string,object>)commands[j];
                    text += "> " + c["command"] + "\r\n" + ((string)c["stdout"]+(string)c["stderr"]).TrimEnd() + "\r\n[exit status " + c["exitCode"] + "]\r\n\r\n";
                }
                var box=new RichTextBox { Dock=DockStyle.Fill, ReadOnly=true, WordWrap=true, BackColor=Color.FromArgb(20,25,34), ForeColor=Color.FromArgb(237,241,247), Font=new Font("Consolas",10), BorderStyle=BorderStyle.None, Margin=new Padding(0,0,10,0), Text=text };
                table.Controls.Add(box,col,0);
            }
            tab.Controls.Add(table); tab.Controls.Add(heading); tabs.TabPages.Add(tab);
        }
        form.Controls.Add(tabs); form.Controls.Add(header); Application.Run(form);
    }
}
