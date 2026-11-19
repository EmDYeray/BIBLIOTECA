"""Linux desktop room and book editor (Python standard library / Tk)."""
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog, ttk
import urllib.request
from urllib.parse import quote

from .pack import export, validate

ROOT = Path(__file__).resolve().parent
DATA = ROOT / "content.json"
COVERS = ROOT / "covers"
KIND_COLORS = {"shelf": "#806143", "fire": "#ef8b3d", "lantern": "#f2ce76", "table": "#92745f",
               "chair": "#b49770", "bench": "#7d8478", "book": "#5e9db6"}


def nasa_models(query):
    # GitHub's public tree API indexes NASA's official 3D asset collection.
    url = "https://api.github.com/repos/nasa/NASA-3D-Resources/git/trees/master?recursive=1"
    request = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json", "User-Agent": "Starry-Library-Editor"})
    with urllib.request.urlopen(request, timeout=20) as response:
        entries = json.load(response)["tree"]
    return [(item["path"], "https://github.com/nasa/NASA-3D-Resources/blob/master/" + quote(item["path"]))
            for item in entries if item["path"].lower().startswith("3d models/")
            and item["path"].lower().endswith((".glb", ".obj")) and query.lower() in item["path"].lower()][:40]


class Editor:
    def __init__(self, root):
        self.root = root
        root.title("Starry Library — editor de contenido")
        root.geometry("1060x720")
        self.data = json.loads(DATA.read_text())
        self.room = 0
        self.selected = None
        self.dragging = False
        self.palette = tk.StringVar(value="shelf")
        top = ttk.Frame(root)
        top.pack(fill="x", padx=10, pady=8)
        ttk.Button(top, text="Guardar", command=self.save).pack(side="left")
        ttk.Button(top, text="Exportar paquete", command=self.package).pack(side="left")
        ttk.Button(top, text="Publicar en GitHub", command=self.publish).pack(side="left")
        ttk.Button(top, text="Buscar modelos NASA", command=self.search_nasa).pack(side="left")
        ttk.Button(top, text="Nueva sala", command=self.new_room).pack(side="left")
        self.room_choice = ttk.Combobox(top, state="readonly", width=23)
        self.room_choice.pack(side="right")
        self.room_choice.bind("<<ComboboxSelected>>", self.change_room)
        body = ttk.PanedWindow(root, orient="horizontal")
        body.pack(fill="both", expand=True, padx=10, pady=4)
        left = ttk.Frame(body)
        right = ttk.Frame(body)
        body.add(left, weight=3)
        body.add(right, weight=2)
        self.canvas = tk.Canvas(left, background="#e9e2d6", highlightthickness=0, width=600, height=510)
        self.canvas.pack(fill="both", expand=True)
        self.canvas.bind("<Button-1>", self.click)
        self.canvas.bind("<B1-Motion>", self.drag)
        self.canvas.bind("<ButtonRelease-1>", lambda e: setattr(self, "dragging", False))
        self.canvas.bind("<Configure>", lambda e: self.draw())
        ttk.Label(right, text="Coloca objetos o arrástralos sobre el plano").pack(anchor="w")
        for kind in KIND_COLORS:
            ttk.Radiobutton(right, text=kind, variable=self.palette, value=kind).pack(anchor="w")
        ttk.Button(right, text="Eliminar objeto seleccionado", command=self.delete).pack(fill="x", pady=8)
        ttk.Separator(right).pack(fill="x", pady=10)
        ttk.Label(right, text="Libro existente (ID de 0 a 58)").pack(anchor="w")
        self.book_id = tk.StringVar(value="0")
        ttk.Entry(right, textvariable=self.book_id).pack(fill="x")
        ttk.Button(right, text="Editar título, páginas y portada", command=self.edit_book).pack(fill="x", pady=5)
        ttk.Label(right, text="El modelo NASA se conserva como enlace hasta preparar\nsu conversión al formato del pop-up.").pack(anchor="w", pady=15)
        self.refresh_rooms()

    def refresh_rooms(self):
        self.room_choice["values"] = [room["name"] for room in self.data["rooms"]]
        self.room_choice.current(self.room)
        self.draw()

    def change_room(self, _event):
        self.room = self.room_choice.current()
        self.selected = None
        self.draw()

    def new_room(self):
        name = simpledialog.askstring("Nueva sala", "Nombre de la sala:", parent=self.root)
        if not name:
            return
        ident = simpledialog.askstring("Identificador", "Identificador único (letras minúsculas):", parent=self.root)
        if not ident:
            return
        self.data["rooms"].append({"id": ident, "name": name, "objects": []})
        try:
            validate(self.data)
        except ValueError as error:
            self.data["rooms"].pop()
            messagebox.showerror("Sala no válida", str(error))
            return
        self.room = len(self.data["rooms"]) - 1
        self.refresh_rooms()

    def point(self, event):
        return (round(max(0, min(100, (event.x - 25) * 100 / max(1, self.canvas.winfo_width() - 50))), 1),
                round(max(0, min(100, (event.y - 25) * 100 / max(1, self.canvas.winfo_height() - 50))), 1))

    def click(self, event):
        x, y = self.point(event)
        objects = self.data["rooms"][self.room]["objects"]
        self.selected = next((index for index in range(len(objects) - 1, -1, -1)
                              if abs(objects[index]["x"] - x) < 5 and abs(objects[index]["y"] - y) < 5), None)
        self.dragging = self.selected is not None
        if self.selected is None:
            item = {"kind": self.palette.get(), "x": x, "y": y}
            if item["kind"] == "book":
                try:
                    item["book_id"] = int(self.book_id.get())
                    if not 0 <= item["book_id"] < 59:
                        raise ValueError()
                except ValueError:
                    messagebox.showerror("Libro", "El ID debe estar entre 0 y 58")
                    return
            objects.append(item)
            self.selected = len(objects) - 1
        self.draw()

    def drag(self, event):
        if self.dragging and self.selected is not None:
            item = self.data["rooms"][self.room]["objects"][self.selected]
            item["x"], item["y"] = self.point(event)
            self.draw()

    def delete(self):
        if self.selected is not None:
            del self.data["rooms"][self.room]["objects"][self.selected]
            self.selected = None
            self.draw()

    def draw(self):
        c = self.canvas
        c.delete("all")
        w, h = c.winfo_width(), c.winfo_height()
        c.create_rectangle(25, 25, w - 25, h - 25, fill="#faf5e9", outline="#5f655c", width=3)
        for index, item in enumerate(self.data["rooms"][self.room]["objects"]):
            x = 25 + item["x"] * (w - 50) / 100
            y = 25 + item["y"] * (h - 50) / 100
            c.create_oval(x - 15, y - 15, x + 15, y + 15, fill=KIND_COLORS[item["kind"]],
                          outline="#102c33" if index == self.selected else "", width=3)
            c.create_text(x, y + 27, text=item["kind"] + (f' {item["book_id"]}' if item["kind"] == "book" else ""), fill="#17333c")

    def save(self):
        try:
            validate(self.data)
            DATA.write_text(json.dumps(self.data, ensure_ascii=False, indent=2) + "\n")
            messagebox.showinfo("Guardado", str(DATA))
        except (OSError, ValueError) as error:
            messagebox.showerror("No se pudo guardar", str(error))

    def edit_book(self):
        try:
            ident = int(self.book_id.get())
            if not 0 <= ident < 59:
                raise ValueError()
        except ValueError:
            messagebox.showerror("Libro", "ID válido: 0–58")
            return
        book = next((b for b in self.data["books"] if b["id"] == ident), None)
        if book is None:
            book = {"id": ident, "es": {"pages": []}, "en": {"pages": []}}
            self.data["books"].append(book)
        dialog = tk.Toplevel(self.root)
        dialog.title(f"Libro {ident}")
        dialog.geometry("800x650")
        fields = {}
        for locale in ("es", "en"):
            ttk.Label(dialog, text=f"{locale.upper()} — título y páginas (separa páginas con una línea ===)").pack(anchor="w")
            title = ttk.Entry(dialog)
            title.pack(fill="x")
            title.insert(0, book.get(locale, {}).get("title", ""))
            pages = tk.Text(dialog, height=10, wrap="word")
            pages.pack(fill="both", expand=True)
            pages.insert("1.0", "\n===\n".join(book.get(locale, {}).get("pages", [])))
            fields[locale] = (title, pages)
        def cover():
            path = filedialog.askopenfilename(filetypes=[("PNG", "*.png")])
            if path:
                COVERS.mkdir(exist_ok=True)
                name = f"book-{ident}.png"
                shutil.copyfile(path, COVERS / name)
                book["cover"] = name
        ttk.Button(dialog, text="Importar portada PNG", command=cover).pack()
        def apply():
            for locale, (title, pages) in fields.items():
                value = pages.get("1.0", "end-1c")
                book[locale] = {"title": title.get(), "pages": [p.strip() for p in value.split("\n===\n")] if value.strip() else []}
            try:
                validate(self.data)
                dialog.destroy()
                self.save()
            except ValueError as error:
                messagebox.showerror("Libro", str(error))
        ttk.Button(dialog, text="Aplicar", command=apply).pack()

    def package(self):
        self.save()
        path = filedialog.asksaveasfilename(defaultextension=".zip", initialfile="starry-content.zip")
        if path:
            try:
                manifest = export(self.data, COVERS, path)
                Path(path).with_name("manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
                messagebox.showinfo("Paquete listo", f"{path}\n{manifest['bytes']} bytes")
            except (OSError, ValueError) as error:
                messagebox.showerror("Exportación", str(error))

    def publish(self):
        path = filedialog.askopenfilename(title="Paquete exportado", filetypes=[("Paquete ZIP", "*.zip")])
        if not path:
            return
        try:
            result = subprocess.run(["python3", str(ROOT.parent / "tools/publish-content.py"), path],
                                    cwd=ROOT.parent, text=True, capture_output=True, timeout=120, check=True)
            messagebox.showinfo("Publicado", result.stdout.strip())
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as error:
            messagebox.showerror("Publicación", str(error) + "\n" + getattr(error, "stderr", ""))

    def search_nasa(self):
        query = simpledialog.askstring("Modelos NASA", "Busca una misión u objeto:", parent=self.root)
        if not query:
            return
        try:
            results = nasa_models(query)
        except Exception as error:
            messagebox.showerror("NASA", str(error))
            return
        window = tk.Toplevel(self.root)
        window.title("Modelos de NASA 3D Resources")
        listing = tk.Listbox(window, width=90, height=22)
        listing.pack(fill="both", expand=True)
        for name, _ in results:
            listing.insert("end", name)
        def open_model(_event):
            import webbrowser
            if listing.curselection():
                webbrowser.open(results[listing.curselection()[0]][1])
        listing.bind("<Double-Button-1>", open_model)


if __name__ == "__main__":
    Editor(tk.Tk()).root.mainloop()
