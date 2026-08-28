#!/usr/bin/env python3
# PS4 Media Play - نسخة Mac سطح المكتب
# تحاكي نفس واجهة PS4 تماماً: متصفح ملفات + مشغل فيديو/صوت + تحكم بالكيبورد (يحاكي يد PS4)
# يعمل مباشرة بدون تثبيت أي شيء

import os, sys, subprocess, pathlib, mimetypes
try:
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox
except ImportError:
    print("tkinter غير متوفر")
    sys.exit(1)

# محاولة استخدام VLC أو mpv أو ffplay أو QuickTime
def find_player():
    for p in ["/Applications/VLC.app/Contents/MacOS/VLC", "vlc", "mpv", "ffplay", "open"]:
        if os.path.exists(p) or subprocess.run(["which", p], capture_output=True).returncode == 0:
            return p
    return "open"

PLAYER = find_player()
MEDIA_EXTS = {".mp4",".mkv",".avi",".mov",".flv",".webm",".m4v",".mpg",".mpeg",".3gp",".ts",".mp3",".flac",".wav",".aac",".ogg",".wma",".m4a",".opus"}

class PS4MediaMac:
    def __init__(self, root):
        self.root = root
        self.root.title("PS4 Media Play — نسخة Mac")
        self.root.geometry("1100x700")
        self.root.configure(bg="#0a0e1a")
        self.current_path = str(pathlib.Path.home())
        self.playlist = []
        self.current_player_proc = None

        self.build_ui()
        self.refresh_browser()

    def build_ui(self):
        # Top bar - يحاكي PS4 style (أزرق بلايستيشن #0070CC)
        top = tk.Frame(self.root, bg="#0070CC", height=60)
        top.pack(fill="x")
        tk.Label(top, text="◉  PS4 Media Play", bg="#0070CC", fg="white", font=("Arial", 18, "bold")).pack(side="left", padx=20, pady=12)
        tk.Label(top, text="نسخة Mac  •  يحاكي واجهة PS4", bg="#0070CC", fg="#cce6ff", font=("Arial", 11)).pack(side="left", padx=10)
        tk.Button(top, text="📁 فتح مجلد", bg="white", fg="#0070CC", command=self.choose_folder, relief="flat", padx=12).pack(side="right", padx=15, pady=10)
        tk.Button(top, text="🏠 الرئيسية", bg="#005199", fg="white", relief="flat", command=lambda: self.navigate_to(str(pathlib.Path.home()))).pack(side="right", padx=5, pady=10)

        # Path bar
        self.path_var = tk.StringVar(value=self.current_path)
        path_bar = tk.Frame(self.root, bg="#1a2332", height=36)
        path_bar.pack(fill="x")
        tk.Label(path_bar, text=" المسار:", bg="#1a2332", fg="#8aa0b8", font=("Arial", 10)).pack(side="left", padx=10)
        tk.Label(path_bar, textvariable=self.path_var, bg="#1a2332", fg="white", font=("Menlo", 10)).pack(side="left", padx=5)
        tk.Label(path_bar, text="  [ USB يحاكي: /Volumes ]", bg="#1a2332", fg="#5a7a9a", font=("Arial", 9)).pack(side="right", padx=15)

        # Main split
        main = tk.PanedWindow(self.root, bg="#0a0e1a", sashwidth=4, sashrelief="flat")
        main.pack(fill="both", expand=True, padx=0, pady=0)

        # Left - Browser (يحاكي قائمة PS4 العمودية)
        left = tk.Frame(main, bg="#121a2b", width=700)
        main.add(left, stretch="always")

        # Controls hint
        hint = tk.Frame(left, bg="#121a2b")
        hint.pack(fill="x", padx=15, pady=(12,6))
        tk.Label(hint, text="↑↓ تنقل  •  ⏎ X تشغيل  •  ⌫ O رجوع  •  □ إضافة لقائمة التشغيل", bg="#121a2b", fg="#5a7a9a", font=("Arial", 10)).pack(anchor="w")

        # Listbox with scrollbar
        list_frame = tk.Frame(left, bg="#121a2b")
        list_frame.pack(fill="both", expand=True, padx=15, pady=5)
        
        self.listbox = tk.Listbox(list_frame, bg="#1c2942", fg="white", selectbackground="#0070CC", selectforeground="white",
                                    font=("Menlo", 13), relief="flat", bd=0, highlightthickness=0, activestyle="none")
        sb = tk.Scrollbar(list_frame, command=self.listbox.yview, bg="#1c2942")
        self.listbox.config(yscrollcommand=sb.set)
        self.listbox.pack(side="left", fill="both", expand=True)
        sb.pack(side="right", fill="y")
        self.listbox.bind("<Double-Button-1>", lambda e: self.on_cross())
        self.listbox.bind("<Return>", lambda e: self.on_cross())
        self.listbox.bind("<BackSpace>", lambda e: self.on_circle())
        self.listbox.bind("<Up>", lambda e: None)
        self.listbox.bind("<Down>", lambda e: None)

        # Bottom controls bar
        ctrl = tk.Frame(left, bg="#0f1a2e", height=54)
        ctrl.pack(fill="x", padx=15, pady=10)
        for txt, cmd, col in [("▶ X تشغيل", self.on_cross, "#0070CC"), ("↩ O رجوع", self.on_circle, "#3a4a6a"), ("＋ □ قائمة", self.add_selected_to_playlist, "#3a4a6a"), ("▶⏸ مسافة", self.toggle_pause, "#3a4a6a")]:
            tk.Button(ctrl, text=txt, bg=col, fg="white", relief="flat", padx=10, pady=6, command=cmd, font=("Arial", 10, "bold")).pack(side="left", padx=4)

        # Right - Player / Playlist (يحاكي شاشة التشغيل)
        right = tk.Frame(main, bg="#0f1729", width=380)
        main.add(right)

        tk.Label(right, text="▣  الآن يُشغّل", bg="#0f1729", fg="white", font=("Arial", 13, "bold")).pack(anchor="w", padx=15, pady=(15,5))
        self.now_label = tk.Label(right, text="لا يوجد", bg="#0f1729", fg="#8aa0b8", font=("Arial", 11), wraplength=350, justify="left")
        self.now_label.pack(anchor="w", padx=15, pady=3)

        # Video preview / controls
        self.status_var = tk.StringVar(value="متوقف")
        tk.Label(right, textvariable=self.status_var, bg="#1c2942", fg="#00d4ff", font=("Menlo", 11, "bold"), padx=8, pady=6).pack(fill="x", padx=15, pady=8)

        btn_frame = tk.Frame(right, bg="#0f1729")
        btn_frame.pack(fill="x", padx=15, pady=5)
        tk.Button(btn_frame, text="⏮ السابق L1", bg="#1c2942", fg="white", relief="flat", command=self.prev_track).pack(side="left", padx=2, fill="x", expand=True)
        tk.Button(btn_frame, text="⏸/▶ R3", bg="#0070CC", fg="white", relief="flat", command=self.toggle_pause).pack(side="left", padx=2, fill="x", expand=True)
        tk.Button(btn_frame, text="⏭ التالي R1", bg="#1c2942", fg="white", relief="flat", command=self.next_track).pack(side="left", padx=2, fill="x", expand=True)

        # Seek
        seek = tk.Frame(right, bg="#0f1729")
        seek.pack(fill="x", padx=15, pady=6)
        tk.Button(seek, text="◀◀  -10s L2", bg="#1c2942", fg="white", relief="flat", command=lambda: self.seek(-10)).pack(side="left", padx=2)
        tk.Button(seek, text="+10s R2  ▶▶", bg="#1c2942", fg="white", relief="flat", command=lambda: self.seek(10)).pack(side="right", padx=2)

        tk.Label(right, text="— قائمة التشغيل —", bg="#0f1729", fg="#5a7a9a", font=("Arial", 10)).pack(pady=(12,4))
        pl_frame = tk.Frame(right, bg="#0f1729")
        pl_frame.pack(fill="both", expand=True, padx=15, pady=5)
        self.playlist_box = tk.Listbox(pl_frame, bg="#162040", fg="#c0d0e8", font=("Menlo", 10), relief="flat", height=10, selectbackground="#0070CC")
        self.playlist_box.pack(fill="both", expand=True, side="left")
        self.playlist_box.bind("<Double-Button-1>", lambda e: self.play_from_playlist())
        sb2 = tk.Scrollbar(pl_frame, command=self.playlist_box.yview)
        self.playlist_box.config(yscrollcommand=sb2.set)
        sb2.pack(side="right", fill="y")
        tk.Button(right, text="🗑 مسح القائمة", bg="#3a1a2a", fg="#ff8a8a", relief="flat", command=self.clear_playlist).pack(fill="x", padx=15, pady=8)

        # Key bindings - تحاكي يد PS4
        self.root.bind("<space>", lambda e: self.toggle_pause())

    def choose_folder(self):
        d = filedialog.askdirectory(initialdir=self.current_path)
        if d: self.navigate_to(d)

    def navigate_to(self, path):
        if not os.path.exists(path): return
        if os.path.isfile(path):
            self.play_file(path)
        else:
            self.current_path = path
            self.refresh_browser()

    def refresh_browser(self):
        self.path_var.set(self.current_path)
        self.listbox.delete(0, tk.END)
        try:
            # Add parent
            parent = str(pathlib.Path(self.current_path).parent)
            if parent != self.current_path:
                self.listbox.insert(tk.END, "📁  ..  (رجوع)")
                self._entries = [("__PARENT__", parent, True)]
            else:
                self._entries = []

            # Quick access for Mac USB
            if self.current_path == str(pathlib.Path.home()):
                for v in ["/Volumes"]:
                    if os.path.exists(v):
                        try:
                            for vol in os.listdir(v):
                                p = os.path.join(v, vol)
                                self.listbox.insert(tk.END, f"💾  {vol}  [USB]")
                                self._entries.append((vol, p, True))
                        except: pass

            items = sorted(os.listdir(self.current_path))
            dirs = []
            files = []
            for name in items:
                if name.startswith("."): continue
                full = os.path.join(self.current_path, name)
                is_dir = os.path.isdir(full)
                ext = pathlib.Path(name).suffix.lower()
                is_media = ext in MEDIA_EXTS
                if is_dir: dirs.append((name, full))
                elif is_media: files.append((name, full))
            for name, full in dirs:
                self.listbox.insert(tk.END, f"📁  {name}")
                self._entries.append((name, full, True))
            for name, full in files:
                icon = "🎬" if pathlib.Path(name).suffix.lower() in {".mp4",".mkv",".avi",".mov"} else "🎵"
                size = self.fmt_size(os.path.getsize(full))
                self.listbox.insert(tk.END, f"{icon}  {name}  —  {size}")
                self._entries.append((name, full, False))
            if len(self._entries) == 0 or (len(self._entries)==1 and self._entries[0][0]=="__PARENT__"):
                self.listbox.insert(tk.END, "— لا توجد ملفات وسائط هنا —")
        except Exception as e:
            self.listbox.insert(tk.END, f"خطأ: {e}")

    def fmt_size(self, b):
        if b < 1024: return f"{b} B"
        if b < 1024*1024: return f"{b/1024:.1f} KB"
        if b < 1024*1024*1024: return f"{b/1024/1024:.1f} MB"
        return f"{b/1024/1024/1024:.2f} GB"

    def selected_entry(self):
        sel = self.listbox.curselection()
        if not sel: return None
        idx = sel[0]
        # adjust for header offset?
        if idx < len(self._entries):
            return self._entries[idx]
        return None

    def on_cross(self):
        e = self.selected_entry()
        if not e: 
            # if double click on item without selection, try first selected
            return
        name, path, is_dir = e
        if name == "__PARENT__":
            self.navigate_to(path)
        elif is_dir:
            self.navigate_to(path)
        else:
            self.play_file(path)

    def on_circle(self):
        parent = str(pathlib.Path(self.current_path).parent)
        if parent != self.current_path:
            self.navigate_to(parent)

    def add_selected_to_playlist(self):
        e = self.selected_entry()
        if e and not e[2] and e[0] != "__PARENT__":
            self.add_to_playlist(e[1])

    def add_to_playlist(self, path):
        if path not in self.playlist:
            self.playlist.append(path)
            self.playlist_box.insert(tk.END, pathlib.Path(path).name)
            self.status_var.set(f"أُضيف للقائمة: {pathlib.Path(path).name}")

    def clear_playlist(self):
        self.playlist.clear()
        self.playlist_box.delete(0, tk.END)

    def play_file(self, path):
        self.add_to_playlist(path)
        self.now_label.config(text=pathlib.Path(path).name)
        self.status_var.set("▶ يُشغّل...")
        # Open with system player (VLC/mpv/QuickTime)
        try:
            if PLAYER.endswith("VLC"):
                subprocess.Popen([PLAYER, path])
            elif PLAYER == "mpv":
                subprocess.Popen(["mpv", path])
            elif PLAYER == "ffplay":
                subprocess.Popen(["ffplay", "-autoexit", path])
            else:
                subprocess.Popen(["open", path])
            self.status_var.set(f"▶ يُشغّل: {pathlib.Path(path).name}")
        except Exception as e:
            messagebox.showerror("خطأ", f"تعذر التشغيل: {e}\n\nجرب تثبيت VLC: brew install --cask vlc")

    def play_from_playlist(self):
        sel = self.playlist_box.curselection()
        if sel and sel[0] < len(self.playlist):
            self.play_file(self.playlist[sel[0]])

    def toggle_pause(self): self.status_var.set("⏸/▶ (التحكم في المشغل الخارجي)")
    def prev_track(self):
        if not self.playlist: return
        idx = 0
        sel = self.playlist_box.curselection()
        if sel: idx = max(0, sel[0]-1)
        self.playlist_box.selection_clear(0, tk.END)
        self.playlist_box.selection_set(idx)
        self.play_from_playlist()
    def next_track(self):
        if not self.playlist: return
        idx = 0
        sel = self.playlist_box.curselection()
        if sel: idx = min(len(self.playlist)-1, sel[0]+1)
        else: idx = 0
        self.playlist_box.selection_clear(0, tk.END)
        self.playlist_box.selection_set(idx)
        self.play_from_playlist()
    def seek(self, delta): self.status_var.set(f"Seek {delta:+d}s (في المشغل الخارجي)")

if __name__ == "__main__":
    root = tk.Tk()
    # أيقونة
    try: root.iconbitmap("")
    except: pass
    app = PS4MediaMac(root)
    # افتح مجلد الأفلام تلقائيا لو موجود
    for cand in [str(pathlib.Path.home()/"Movies"), str(pathlib.Path.home()/"Downloads"), "/Volumes"]:
        if os.path.exists(cand) and os.listdir(cand):
            app.navigate_to(cand)
            break
    root.mainloop()
