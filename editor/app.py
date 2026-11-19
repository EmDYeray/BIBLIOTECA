"""Linux scene editor for Starry Library.

The plan view edits the exact world-space scene consumed by the native renderer.
The Preview button starts the native OpenGL application with that scene.
"""
import copy
import json
from pathlib import Path
import shutil
import subprocess
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog, ttk
import urllib.request
from urllib.parse import quote

from .pack import export, scene_problems, validate

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT.parent
DATA = ROOT / "content.json"
COVERS = ROOT / "covers"
SCENE = PROJECT / "assets/content/scene.json"
KINDS = ("shelf", "fire", "lantern", "table", "chair", "bench", "book", "box", "spawn", "portal")
KIND_COLORS = {"shelf": "#806143", "fire": "#ef8b3d", "lantern": "#f2ce76", "table": "#92745f",
               "chair": "#b49770", "bench": "#7d8478", "book": "#5e9db6", "box": "#8b7059",
               "spawn": "#4dba78", "portal": "#936ad2"}


def nasa_models(query):
    url = "https://api.github.com/repos/nasa/NASA-3D-Resources/git/trees/master?recursive=1"
    request = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json",
                                                   "User-Agent": "Starry-Library-Editor"})
    with urllib.request.urlopen(request, timeout=20) as response:
        entries = json.load(response)["tree"]
    return [(item["path"], "https://github.com/nasa/NASA-3D-Resources/blob/master/" + quote(item["path"]))
            for item in entries if item["path"].lower().startswith("3d models/")
            and item["path"].lower().endswith((".glb", ".gltf", ".obj"))
            and query.lower() in item["path"].lower()][:60]


class Editor:
    def __init__(self, root):
        self.root = root
        root.title("Starry Library — editor de escenas")
        root.geometry("1220x760")
        self.data = json.loads(DATA.read_text())
        self.room = 0
        self.selected = None
        self.dragging = False
        self.palette = tk.StringVar(value="shelf")
        self.snap = tk.BooleanVar(value=True)
        self.undo_stack = []
        self.redo_stack = []
        self.property_vars = {key: tk.StringVar() for key in
                              ("name", "x", "y", "z", "yaw", "sx", "sy", "sz", "color", "material")}
        self.solid = tk.BooleanVar(value=True)
        self.animated = tk.BooleanVar()
        self.interactive = tk.BooleanVar()

        toolbar = ttk.Frame(root)
        toolbar.pack(fill="x", padx=10, pady=8)
        for title, command in (("Guardar", self.save), ("Probar en la app", self.preview),
                               ("Exportar paquete", self.package), ("Publicar", self.publish),
                               ("Modelos NASA", self.search_nasa), ("Nueva sala", self.new_room)):
            ttk.Button(toolbar, text=title, command=command).pack(side="left", padx=2)
        ttk.Checkbutton(toolbar, text="Ajuste 0.25 m", variable=self.snap).pack(side="left", padx=10)
        self.room_choice = ttk.Combobox(toolbar, state="readonly", width=24)
        self.room_choice.pack(side="right")
        self.room_choice.bind("<<ComboboxSelected>>", self.change_room)

        body = ttk.PanedWindow(root, orient="horizontal")
        body.pack(fill="both", expand=True, padx=10, pady=4)
        catalog = ttk.Frame(body, width=170)
        view = ttk.Frame(body)
        properties = ttk.Frame(body, width=260)
        body.add(catalog, weight=1);body.add(view, weight=5);body.add(properties, weight=2)

        ttk.Label(catalog, text="Catálogo", font=("sans", 13, "bold")).pack(anchor="w", pady=(0, 8))
        for kind in KINDS:
            ttk.Radiobutton(catalog, text=kind, variable=self.palette, value=kind).pack(anchor="w")
        ttk.Separator(catalog).pack(fill="x", pady=10)
        ttk.Button(catalog, text="Duplicar", command=self.duplicate).pack(fill="x", pady=2)
        ttk.Button(catalog, text="Eliminar", command=self.delete).pack(fill="x", pady=2)
        ttk.Button(catalog, text="Deshacer  Ctrl+Z", command=self.undo).pack(fill="x", pady=2)
        ttk.Button(catalog, text="Rehacer  Ctrl+Y", command=self.redo).pack(fill="x", pady=2)

        self.canvas = tk.Canvas(view, background="#17242a", highlightthickness=0, width=700, height=620)
        self.canvas.pack(fill="both", expand=True)
        self.canvas.bind("<Button-1>", self.click)
        self.canvas.bind("<B1-Motion>", self.drag)
        self.canvas.bind("<ButtonRelease-1>", self.release_drag)
        self.canvas.bind("<Configure>", lambda _event: self.draw())
        root.bind("<Control-z>", lambda _event: self.undo())
        root.bind("<Control-y>", lambda _event: self.redo())
        root.bind("<Delete>", lambda _event: self.delete())

        ttk.Label(properties, text="Transformación", font=("sans", 13, "bold")).pack(anchor="w")
        for key, label in (("name", "Nombre"), ("x", "Posición X"), ("y", "Posición Y"),
                           ("z", "Posición Z"), ("yaw", "Rotación Y"), ("sx", "Escala X"),
                           ("sy", "Escala Y"), ("sz", "Escala Z"), ("color", "Color #RRGGBB"),
                           ("material", "Material")):
            ttk.Label(properties, text=label).pack(anchor="w", pady=(5, 0))
            ttk.Entry(properties, textvariable=self.property_vars[key]).pack(fill="x")
        ttk.Checkbutton(properties, text="Sólido", variable=self.solid).pack(anchor="w", pady=(8, 0))
        ttk.Checkbutton(properties, text="Animado", variable=self.animated).pack(anchor="w")
        ttk.Checkbutton(properties, text="Interactivo", variable=self.interactive).pack(anchor="w")
        ttk.Button(properties, text="Aplicar propiedades", command=self.apply_properties).pack(fill="x", pady=8)
        ttk.Separator(properties).pack(fill="x", pady=8)
        ttk.Label(properties, text="Libro (ID 0–58)").pack(anchor="w")
        self.book_id = tk.StringVar(value="0")
        ttk.Entry(properties, textvariable=self.book_id).pack(fill="x")
        ttk.Button(properties, text="Editar historia y portada", command=self.edit_book).pack(fill="x", pady=4)
        self.status = ttk.Label(root, text="Vista superior en metros. Usa Probar en la app para la vista OpenGL.")
        self.status.pack(fill="x", padx=10, pady=(0, 6))
        self.refresh_rooms()

    @property
    def current_room(self):
        return self.data["rooms"][self.room]

    def snapshot(self):
        self.undo_stack.append(copy.deepcopy(self.data))
        if len(self.undo_stack) > 80:
            del self.undo_stack[0]
        self.redo_stack.clear()

    def undo(self):
        if not self.undo_stack:
            return
        self.redo_stack.append(copy.deepcopy(self.data))
        self.data = self.undo_stack.pop()
        self.selected = None
        self.refresh_rooms()

    def redo(self):
        if not self.redo_stack:
            return
        self.undo_stack.append(copy.deepcopy(self.data))
        self.data = self.redo_stack.pop()
        self.selected = None
        self.refresh_rooms()

    def refresh_rooms(self):
        self.room = min(self.room, len(self.data["rooms"]) - 1)
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
        ident = simpledialog.askstring("Identificador", "Identificador único:", parent=self.root)
        if not ident:
            return
        width = simpledialog.askfloat("Ancho", "Ancho en metros:", initialvalue=12, minvalue=3, parent=self.root)
        depth = simpledialog.askfloat("Profundidad", "Profundidad en metros:", initialvalue=10, minvalue=3, parent=self.root)
        height = simpledialog.askfloat("Altura", "Altura en metros:", initialvalue=5, minvalue=2.4, parent=self.root)
        if None in (width, depth, height):
            return
        right = max(room["origin"][0] + room["size"][0] / 2 for room in self.data["rooms"])
        self.snapshot()
        self.data["rooms"].append({"id": ident, "name": name, "builtin": False,
                                   "origin": [right + 2 + width / 2, -2.26, 6.4],
                                   "size": [width, height, depth], "height": height,
                                   "floor": "wood", "walls": "plaster", "ceiling": "plaster"})
        try:
            validate(self.data)
        except ValueError as error:
            self.data = self.undo_stack.pop()
            messagebox.showerror("Sala no válida", str(error))
            return
        self.room = len(self.data["rooms"]) - 1
        self.refresh_rooms()

    def room_objects(self):
        room = self.current_room
        ox, _, oz = room["origin"];sx, _, sz = room["size"]
        return [(index, item) for index, item in enumerate(self.data["objects"])
                if ox - sx / 2 <= item["position"][0] <= ox + sx / 2
                and oz - sz / 2 <= item["position"][2] <= oz + sz / 2]

    def canvas_to_world(self, event):
        room = self.current_room;w = max(1, self.canvas.winfo_width() - 60);h = max(1, self.canvas.winfo_height() - 60)
        x = room["origin"][0] + ((event.x - 30) / w - .5) * room["size"][0]
        z = room["origin"][2] + ((event.y - 30) / h - .5) * room["size"][2]
        if self.snap.get():
            x = round(x * 4) / 4;z = round(z * 4) / 4
        return x, z

    def world_to_canvas(self, position):
        room = self.current_room;w = self.canvas.winfo_width() - 60;h = self.canvas.winfo_height() - 60
        x = 30 + ((position[0] - room["origin"][0]) / room["size"][0] + .5) * w
        y = 30 + ((position[2] - room["origin"][2]) / room["size"][2] + .5) * h
        return x, y

    def click(self, event):
        x, z = self.canvas_to_world(event)
        candidates = self.room_objects()
        self.selected = next((index for index, item in reversed(candidates)
                              if abs(item["position"][0] - x) < .65 and abs(item["position"][2] - z) < .65), None)
        self.dragging = self.selected is not None
        if self.selected is None:
            self.snapshot()
            kind = self.palette.get();ident = f"{kind}-{len(self.data['objects']) + 1}"
            item = {"id": ident, "name": ident, "kind": kind,
                    "position": [x, self.current_room["origin"][1], z], "rotation": [0, 0, 0],
                    "scale": [1, 1, 1], "color": KIND_COLORS[kind], "material": "wood",
                    "solid": kind not in ("book", "spawn"), "animated": kind in ("fire", "lantern"),
                    "interactive": kind in ("book", "portal")}
            if kind == "book":
                try:
                    item["book"] = int(self.book_id.get())
                except ValueError:
                    messagebox.showerror("Libro", "El ID debe estar entre 0 y 58")
                    self.undo_stack.pop();return
            self.data["objects"].append(item);self.selected = len(self.data["objects"]) - 1
        self.load_properties();self.draw()

    def drag(self, event):
        if not self.dragging or self.selected is None:
            return
        if not getattr(self, "drag_snapshot", False):
            self.snapshot();self.drag_snapshot = True
        x, z = self.canvas_to_world(event);item = self.data["objects"][self.selected]
        item["position"][0] = x;item["position"][2] = z
        self.load_properties();self.draw()

    def release_drag(self, _event):
        self.dragging = False
        self.drag_snapshot = False

    def duplicate(self):
        if self.selected is None:
            return
        self.snapshot();item = copy.deepcopy(self.data["objects"][self.selected])
        root = item["id"] + "-copy";used = {value["id"] for value in self.data["objects"]};item["id"] = root
        suffix = 2
        while item["id"] in used:item["id"] = f"{root}-{suffix}";suffix += 1
        item["name"] += " copia";item["position"][0] += .5;item["position"][2] += .5
        self.data["objects"].append(item);self.selected = len(self.data["objects"]) - 1;self.draw()

    def delete(self):
        if self.selected is None:
            return
        self.snapshot();del self.data["objects"][self.selected];self.selected = None;self.draw()

    def load_properties(self):
        if self.selected is None:
            return
        item = self.data["objects"][self.selected]
        values = {"name": item["name"], "x": item["position"][0], "y": item["position"][1],
                  "z": item["position"][2], "yaw": item["rotation"][1], "sx": item["scale"][0],
                  "sy": item["scale"][1], "sz": item["scale"][2], "color": item["color"],
                  "material": item["material"]}
        for key, value in values.items():self.property_vars[key].set(str(value))
        self.solid.set(item["solid"]);self.animated.set(item["animated"]);self.interactive.set(item["interactive"])

    def apply_properties(self):
        if self.selected is None:
            return
        try:
            self.snapshot();item = self.data["objects"][self.selected]
            item["name"] = self.property_vars["name"].get().strip() or item["id"]
            item["position"] = [float(self.property_vars[key].get()) for key in ("x", "y", "z")]
            item["rotation"][1] = float(self.property_vars["yaw"].get())
            item["scale"] = [float(self.property_vars[key].get()) for key in ("sx", "sy", "sz")]
            item["color"] = self.property_vars["color"].get();item["material"] = self.property_vars["material"].get()
            item["solid"] = self.solid.get();item["animated"] = self.animated.get();item["interactive"] = self.interactive.get()
            validate(self.data);self.draw()
        except ValueError as error:
            self.data = self.undo_stack.pop();messagebox.showerror("Propiedades", str(error))

    def draw(self):
        c = self.canvas;c.delete("all");w, h = c.winfo_width(), c.winfo_height()
        c.create_rectangle(30, 30, w - 30, h - 30, fill="#e9e2d6", outline="#a68b65", width=3)
        room = self.current_room
        step = max(1, round(max(room["size"][0], room["size"][2]) / 16))
        for metre in range(step, int(room["size"][0]), step):
            x = 30 + metre / room["size"][0] * (w - 60);c.create_line(x, 30, x, h - 30, fill="#c8bca9")
        for metre in range(step, int(room["size"][2]), step):
            y = 30 + metre / room["size"][2] * (h - 60);c.create_line(30, y, w - 30, y, fill="#c8bca9")
        failures = scene_problems(self.data)
        for index, item in self.room_objects():
            x, y = self.world_to_canvas(item["position"]);radius = 15 if item["kind"] != "book" else 9
            colliding = any(item["name"] in failure and "overlaps" in failure for failure in failures)
            c.create_oval(x-radius, y-radius, x+radius, y+radius, fill=KIND_COLORS[item["kind"]],
                          outline="#e34f4f" if colliding else "#20b9dd" if index == self.selected else "#25383d", width=3)
            c.create_text(x, y+radius+11, text=item["name"], fill="#ecf2ee")
        c.create_text(45, 47, anchor="w", text=f"{room['name']} — {room['size'][0]} × {room['size'][2]} m",
                      fill="#1f3439", font=("sans", 12, "bold"))
        self.status.configure(text=(failures[0] if failures else
                              "Escena válida. Usa Probar en la app para la vista OpenGL."))

    def scene_document(self):
        return {key: copy.deepcopy(self.data.get(key, [])) for key in ("schema", "rooms", "connections", "objects")}

    def save(self, notify=True):
        try:
            validate(self.data);DATA.write_text(json.dumps(self.data, ensure_ascii=False, indent=2) + "\n")
            SCENE.parent.mkdir(parents=True, exist_ok=True)
            SCENE.write_text(json.dumps(self.scene_document(), ensure_ascii=False, indent=2) + "\n")
            if notify:messagebox.showinfo("Guardado", f"Proyecto: {DATA}\nEscena de la app: {SCENE}")
            return True
        except (OSError, ValueError) as error:
            messagebox.showerror("No se pudo guardar", str(error));return False

    def preview(self):
        if not self.save(False):return
        binary = PROJECT / "biblioteca"
        if not binary.is_file():
            messagebox.showinfo("Compilar", "Primero compila la app Linux con: make -j2")
            return
        subprocess.Popen([str(binary)], cwd=PROJECT)

    def edit_book(self):
        try:
            ident = int(self.book_id.get())
            if not 0 <= ident < 59:raise ValueError()
        except ValueError:
            messagebox.showerror("Libro", "ID válido: 0–58");return
        book = next((book for book in self.data["books"] if book["id"] == ident), None)
        if book is None:
            book = {"id": ident, "es": {"title": "", "pages": []}, "en": {"title": "", "pages": []}}
            self.data["books"].append(book)
        dialog = tk.Toplevel(self.root);dialog.title(f"Libro {ident}");dialog.geometry("820x680");fields = {}
        for locale in ("es", "en"):
            ttk.Label(dialog, text=f"{locale.upper()} — título y páginas (separador ===)").pack(anchor="w")
            title = ttk.Entry(dialog);title.pack(fill="x");title.insert(0, book[locale].get("title", ""))
            pages = tk.Text(dialog, height=10, wrap="word");pages.pack(fill="both", expand=True)
            pages.insert("1.0", "\n===\n".join(book[locale].get("pages", [])));fields[locale] = (title, pages)
        def cover():
            path = filedialog.askopenfilename(parent=dialog, filetypes=[("PNG", "*.png")])
            if path:COVERS.mkdir(exist_ok=True);name=f"book-{ident}.png";shutil.copyfile(path,COVERS/name);book["cover"]=name
        def import_pdf():
            path = filedialog.askopenfilename(parent=dialog, filetypes=[("PDF", "*.pdf")])
            if not path:return
            try:
                result=subprocess.run(["pdftotext","-layout",path,"-"],capture_output=True,text=True,check=True,timeout=25)
                pages=[page.strip() for page in result.stdout.split("\f") if page.strip()][:16]
                fields["es"][1].delete("1.0","end");fields["es"][1].insert("1.0","\n===\n".join(pages))
            except (FileNotFoundError,subprocess.SubprocessError) as error:messagebox.showerror("PDF",str(error))
        def apply():
            self.snapshot()
            for locale,(title,pages) in fields.items():
                value=pages.get("1.0","end-1c");book[locale]={"title":title.get(),"pages":[p.strip() for p in value.split("\n===\n")] if value.strip() else []}
            try:validate(self.data);dialog.destroy();self.save()
            except ValueError as error:messagebox.showerror("Libro",str(error))
        ttk.Button(dialog,text="Importar portada PNG",command=cover).pack(side="left")
        ttk.Button(dialog,text="Importar PDF original",command=import_pdf).pack(side="left")
        ttk.Button(dialog,text="Aplicar",command=apply).pack(side="right")

    def package(self):
        if not self.save(False):return
        path=filedialog.asksaveasfilename(defaultextension=".zip",initialfile="starry-content.zip")
        if path:
            try:
                manifest=export(self.data,COVERS,path);Path(path).with_name("manifest.json").write_text(json.dumps(manifest,indent=2)+"\n")
                messagebox.showinfo("Paquete listo",f"{path}\n{manifest['bytes']} bytes")
            except (OSError,ValueError) as error:messagebox.showerror("Exportación",str(error))

    def publish(self):
        path=filedialog.askopenfilename(title="Paquete exportado",filetypes=[("ZIP","*.zip")])
        if not path:return
        try:
            result=subprocess.run(["python3",str(PROJECT/"tools/publish-content.py"),path],cwd=PROJECT,text=True,capture_output=True,timeout=120,check=True)
            messagebox.showinfo("Publicado",result.stdout.strip())
        except (subprocess.CalledProcessError,subprocess.TimeoutExpired) as error:messagebox.showerror("Publicación",str(error)+"\n"+getattr(error,"stderr",""))

    def search_nasa(self):
        query=simpledialog.askstring("Modelos NASA","Misión u objeto:",parent=self.root)
        if not query:return
        try:results=nasa_models(query)
        except Exception as error:messagebox.showerror("NASA",str(error));return
        window=tk.Toplevel(self.root);window.title("NASA 3D Resources");listing=tk.Listbox(window,width=95,height=24);listing.pack(fill="both",expand=True)
        for name,_url in results:listing.insert("end",name)
        def open_model(_event):
            import webbrowser
            if listing.curselection():webbrowser.open(results[listing.curselection()[0]][1])
        listing.bind("<Double-Button-1>",open_model)


if __name__ == "__main__":
    Editor(tk.Tk()).root.mainloop()
