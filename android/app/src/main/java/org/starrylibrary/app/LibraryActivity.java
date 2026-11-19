package org.starrylibrary.app;

import android.content.res.AssetManager;
import android.os.Bundle;
import org.libsdl.app.SDLActivity;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.file.Files;
import java.security.MessageDigest;
import java.util.Arrays;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import org.json.JSONObject;

public final class LibraryActivity extends SDLActivity {
    @Override protected void onCreate(Bundle savedInstanceState) {
        try {
            File dir = new File(getFilesDir(), "assets");
            File marker = new File(dir, ".assets-1");
            if (!marker.exists()) {
                copyAssets(getAssets(), "", dir);
                if (!marker.createNewFile()) throw new IOException("Cannot mark installed assets");
            }
        } catch (IOException error) {
            throw new IllegalStateException("Cannot prepare library assets", error);
        }
        super.onCreate(savedInstanceState);
        // The current session retains its loaded textures; refreshed content appears on next launch.
        new Thread(() -> {
            try { refreshContent(new File(getFilesDir(), "assets")); }
            catch (Exception error) { android.util.Log.w("StarryLibrary", "Content sync skipped", error); }
        }, "library-content-sync").start();
    }

    private static void copyAssets(AssetManager assets, String source, File destination) throws IOException {
        String[] children = assets.list(source);
        if (children != null && children.length != 0) {
            if (!destination.isDirectory() && !destination.mkdirs()) throw new IOException(destination.toString());
            for (String child : children) {
                String path = source.isEmpty() ? child : source + "/" + child;
                copyAssets(assets, path, new File(destination, child));
            }
        } else {
            try (InputStream input = assets.open(source); FileOutputStream output = new FileOutputStream(destination)) {
                byte[] buffer = new byte[32768];
                int count;
                while ((count = input.read(buffer)) != -1) output.write(buffer, 0, count);
            }
        }
    }
    private static final String FEED = "https://raw.githubusercontent.com/EmDYeray/Library-of-Stars/library-content/";
    private static byte[] get(String path, int limit) throws IOException {
        HttpURLConnection connection = (HttpURLConnection) new URL(FEED + path).openConnection();
        connection.setConnectTimeout(6000);
        connection.setReadTimeout(15000);
        try (InputStream input = connection.getInputStream(); java.io.ByteArrayOutputStream output = new java.io.ByteArrayOutputStream()) {
            byte[] buffer = new byte[16384];
            int length;
            while ((length = input.read(buffer)) != -1) {
                if (output.size() + length > limit) throw new IOException("Content size limit");
                output.write(buffer, 0, length);
            }
            return output.toByteArray();
        } finally { connection.disconnect(); }
    }
    private static void refreshContent(File assets) throws Exception {
        JSONObject manifest = new JSONObject(new String(get("manifest.json", 8192), java.nio.charset.StandardCharsets.UTF_8));
        if (manifest.getInt("schema") != 1) throw new IOException("Unknown content schema");
        String digest = manifest.getString("sha256");
        if (!digest.matches("[0-9a-f]{64}")) throw new IOException("Invalid content digest");
        File current = new File(assets, "content");
        File version = new File(current, "version.txt");
        if (version.isFile() && digest.equals(new String(Files.readAllBytes(version.toPath())).trim())) return;
        String file = manifest.getString("file");
        if (!file.equals("packs/" + digest + ".zip")) throw new IOException("Invalid pack filename");
        int size = manifest.getInt("bytes");
        if (size <= 0 || size > 80_000_000) throw new IOException("Invalid pack size");
        byte[] zip = get(file, size);
        if (zip.length != size) throw new IOException("Truncated content pack");
        byte[] hash = MessageDigest.getInstance("SHA-256").digest(zip);
        StringBuilder hex = new StringBuilder();
        for (byte b : hash) hex.append(String.format(java.util.Locale.ROOT, "%02x", b & 255));
        if (!digest.contentEquals(hex)) throw new IOException("Content digest mismatch");
        File staging = new File(assets, "content-next");
        remove(staging);
        if (!staging.mkdirs()) throw new IOException("Cannot create content staging");
        long extracted = 0;
        try (ZipInputStream stream = new ZipInputStream(new java.io.ByteArrayInputStream(zip))) {
            ZipEntry entry;
            while ((entry = stream.getNextEntry()) != null) {
                String name = entry.getName();
                if (name.startsWith("/") || name.contains("..") || !name.matches("[a-zA-Z0-9_./-]+")) throw new IOException("Invalid pack path");
                File output = new File(staging, name);
                if (!output.getCanonicalPath().startsWith(staging.getCanonicalPath() + File.separator)) throw new IOException("Pack traversal");
                if (entry.isDirectory()) { if (!output.isDirectory() && !output.mkdirs()) throw new IOException("Directory failure"); continue; }
                File parent = output.getParentFile();
                if (!parent.isDirectory() && !parent.mkdirs()) throw new IOException("Directory failure");
                try (FileOutputStream target = new FileOutputStream(output)) {
                    byte[] buffer = new byte[16384]; int count;
                    while ((count = stream.read(buffer)) != -1) {
                        extracted += count;
                        if (extracted > 150_000_000) throw new IOException("Expanded content exceeds limit");
                        target.write(buffer, 0, count);
                    }
                }
                stream.closeEntry();
            }
        }
        Files.write(new File(staging, "version.txt").toPath(), digest.getBytes(java.nio.charset.StandardCharsets.US_ASCII));
        // App starts with packaged data; only switch overlays after the complete pack is verified.
        File old = new File(assets, "content-old");
        remove(old);
        if (current.exists() && !current.renameTo(old)) throw new IOException("Cannot rotate content");
        if (!staging.renameTo(current)) {
            if (old.exists()) old.renameTo(current);
            throw new IOException("Cannot activate content");
        }
        remove(old);
    }
    private static void remove(File file) throws IOException {
        if (!file.exists()) return;
        if (file.isDirectory()) {
            File[] children = file.listFiles();
            if (children != null) for (File child : children) remove(child);
        }
        if (!file.delete()) throw new IOException("Cannot delete " + file);
    }

}
