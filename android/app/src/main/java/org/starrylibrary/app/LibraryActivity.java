package org.starrylibrary.app;

import android.content.res.AssetManager;
import android.os.Bundle;
import org.libsdl.app.SDLActivity;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

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
}
