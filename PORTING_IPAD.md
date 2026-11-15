# Compilar Biblioteca para iPadOS 27 desde Linux

GitHub Actions ejecuta Xcode en un Mac alojado por GitHub. Desde Linux solo
necesitas subir el proyecto y lanzar el workflow.

## 1. Crea el repositorio

Inicia sesión en GitHub y abre [Nuevo repositorio](https://github.com/new).
Ponle `biblioteca-3d` y elige la visibilidad que prefieras. Déjalo vacío al
crearlo, con README, licencia y plantilla de `.gitignore` desactivados.
Los archivos del proyecto ya vienen en el ZIP.

## 2. Conecta Git con tu cuenta

En Arch o CachyOS instala Git y GitHub CLI si todavía te faltan.

```bash
sudo pacman -S --needed git github-cli
gh auth login --hostname github.com --git-protocol https --web --scopes workflow
gh auth setup-git
```

Completa la autorización en el navegador. El permiso `workflow` permite subir
el archivo de GitHub Actions. Si ya habías iniciado sesión y el push se
rechaza por ese permiso, usa `gh auth refresh --hostname github.com --scopes workflow`
y vuelve a subir el commit.

## 3. Sube la carpeta que sacaste del ZIP

Abre una terminal en la carpeta extraída `Biblioteca-3D`. Debe contener
`CMakeLists.txt` y `.github/workflows/ipados.yml`. Sustituye `TU_USUARIO` por
tu nombre de usuario en GitHub.

```bash
git init -b main
git add .
git commit -m "Primera versión de Biblioteca 3D"
git remote add origin https://github.com/TU_USUARIO/biblioteca-3d.git
git push -u origin main
```

Si Git te pide una identidad antes del primer commit, configura los datos
y vuelve a ejecutar el comando `git commit`.

```bash
git config user.name "Tu nombre"
git config user.email "Tu correo de GitHub"
```

## 4. Ejecuta el workflow y descarga el IPA

1. Abre tu repositorio y entra en **Actions**.
2. Selecciona **Biblioteca para iPadOS 27** en el menú de la izquierda.
3. Pulsa **Run workflow**, elige **main** y pulsa **Run workflow** de nuevo.
4. Cuando aparezca la marca verde, abre la ejecución. Busca **Artifacts**
   y descarga **Biblioteca-iPadOS27-sin-firma**. El ZIP del artefacto contiene
   `Biblioteca-iPadOS27-sin-firma.ipa`.

Si falla, abre el paso en rojo para ver el mensaje de error. El workflow
selecciona la imagen arm64 `xcode-27` y comprueba el SDK del dispositivo.
El IPA se compila sin firma y requiere una firma válida para instalarlo.
La primera ejecución del workflow y la instalación en un iPad siguen
pendientes de validación.

El target reutiliza las salas, las historias y el lector de escritorio.
Incluye entrada táctil, orientación horizontal y los recursos dentro del
bundle. El backend para iPad adapta los shaders a OpenGL ES 3. En el iPad se
camina manteniendo las flechas inferiores, se mira arrastrando sobre la escena,
se cambian hojas con un deslizamiento horizontal y se hace zoom con los botones
`−` y `+` del lector. El catálogo se desplaza con el dedo.

- [GitHub CLI y autorización](https://cli.github.com/manual/gh_auth_login)
- [Ejecutar un workflow manualmente](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/manually-run-a-workflow)
- [Descargar artefactos](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/download-workflow-artifacts)
- [Imagen Xcode 27 de GitHub Actions](https://github.blog/changelog/2026-10-15-xcode-27-runner-image-generally-available/)
