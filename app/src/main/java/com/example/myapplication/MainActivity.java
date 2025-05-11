package com.example.myapplication;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

import com.example.myapplication.databinding.ActivityMainBinding;

import android.graphics.Bitmap;
import android.content.res.AssetManager;
import android.graphics.BitmapFactory;
import android.widget.ImageView;

import android.net.Uri;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.database.Cursor;
import android.provider.OpenableColumns;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'myapplication' library on application startup.
    static {
        System.loadLibrary("myapplication");
    }
    private ActivityMainBinding binding;

    int increse = 1;
    int decrese = -1;

    IntegerHolder chapterNumber = new IntegerHolder(0);
    IntegerHolder currentPage = new IntegerHolder(0);
    DoubleHolder currentImage = new DoubleHolder(0);
    IntegerHolder previousChapterNumber = new IntegerHolder(-1);
    String epubPath;

    private void clearAppCache() {
        try {
            File cacheDir = getCacheDir();
            if (cacheDir != null && cacheDir.isDirectory()) {
                deleteDir(cacheDir);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private boolean deleteDir(File dir) {
        if (dir != null && dir.isDirectory()) {
            String[] children = dir.list();
            if (children != null) {
                for (String child : children) {
                    boolean success = deleteDir(new File(dir, child));
                    if (!success) {
                        return false;
                    }
                }
            }
        }
        return dir.delete();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        clearAppCache();

        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        openBookChooser();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        ImageView imageView = findViewById(R.id.imageView);
        TextView tv = findViewById(R.id.sampleText);

        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            float x = event.getX(); // координата X касания
            float screenWidth = getResources().getDisplayMetrics().widthPixels;

            if (x > screenWidth / 2) {
                // Правая половина экрана
                String str = changePage(getAssets(), epubPath , chapterNumber, currentPage, currentImage, previousChapterNumber, increse, getCacheDir().getAbsolutePath());
                if (str.contains("temp_image.png") || (str.contains(".png") && str.contains("cache"))) {
                    Bitmap bitmap = BitmapFactory.decodeFile(str);  // /data/user/0/com.example.myapplication/cache/temp_image.png
                    imageView.setImageBitmap(bitmap);
                    imageView.setVisibility(View.VISIBLE);          // /data/user/0/com.example.myapplication/cache/1.png
                    tv.setVisibility(View.GONE);
                } else {
                    tv.setText(str);
                    imageView.setVisibility(View.GONE);
                    tv.setVisibility(View.VISIBLE);
                }
            }
            if (x < screenWidth / 2) {
                String str = changePage(getAssets(), epubPath, chapterNumber, currentPage, currentImage, previousChapterNumber, decrese, getCacheDir().getAbsolutePath());
                if (str.contains("temp_image.png") || (str.contains(".png") && str.contains("cache"))) {
                    Bitmap bitmap = BitmapFactory.decodeFile(str);
                    imageView.setImageBitmap(bitmap);
                    imageView.setVisibility(View.VISIBLE);
                    tv.setVisibility(View.GONE);
                } else {
                    tv.setText(str);
                    imageView.setVisibility(View.GONE);
                    tv.setVisibility(View.VISIBLE);
                }
            }
        }
        return super.onTouchEvent(event);
    }

    private static final int PICK_BOOK_FILE = 1;

    public void openBookChooser() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(Intent.createChooser(intent, "Виберіть книгу"), PICK_BOOK_FILE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == PICK_BOOK_FILE && resultCode == RESULT_OK && data != null) {
            Uri uri = data.getData();

            String filename = getFileName(uri);
            epubPath = copyUriToInternalStorage(uri, filename);

            String lowerCaseName = filename.toLowerCase();
            boolean isEpub = lowerCaseName.endsWith(".epub");
            boolean isFb2 = lowerCaseName.endsWith(".fb2");

            if (isEpub || isFb2) {
                String str = openBook(epubPath, chapterNumber, currentPage, currentImage, previousChapterNumber, getCacheDir().getAbsolutePath());

                ImageView imageView = findViewById(R.id.imageView);
                TextView tv = findViewById(R.id.sampleText);

                if (str.contains("temp_image.png") || (str.contains(".png") && str.contains("cache"))) {
                    Bitmap bitmap = BitmapFactory.decodeFile(str);
                    imageView.setImageBitmap(bitmap);
                    imageView.setVisibility(View.VISIBLE);
                    tv.setVisibility(View.GONE);
                } else {
                    tv.setText(str);
                    imageView.setVisibility(View.GONE);
                    tv.setVisibility(View.VISIBLE);
                }
            }
        }
    }

    private String getFileName(Uri uri) {
        String result = null;
        if (uri.getScheme().equals("content")) {
            try (Cursor cursor = getContentResolver().query(uri, null, null, null, null)) {
                if (cursor != null && cursor.moveToFirst()) {
                    result = cursor.getString(cursor.getColumnIndexOrThrow(OpenableColumns.DISPLAY_NAME));
                }
            }
        }
        if (result == null) {
            result = uri.getPath();
            int cut = result.lastIndexOf('/');
            if (cut != -1) {
                result = result.substring(cut + 1);
            }
        }
        return result;
    }

    public String copyUriToInternalStorage(Uri uri, String fileName) {
        try {
            InputStream in = getContentResolver().openInputStream(uri);
            File outFile = new File(getFilesDir(), fileName);
            OutputStream out = new FileOutputStream(outFile);

            byte[] buffer = new byte[1024];
            int length;
            while ((length = in.read(buffer)) > 0) {
                out.write(buffer, 0, length);
            }

            in.close();
            out.close();

            return outFile.getAbsolutePath();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public native String openBook(String path, IntegerHolder chapterNumber,
        IntegerHolder currentPage, DoubleHolder currentImage, IntegerHolder previousChapterNumber, String cacheDir);
    public native String changePage(AssetManager javaAssetManager, String jCacheDir,
        IntegerHolder chapterNumber, IntegerHolder currentPage, DoubleHolder currentImage,
        IntegerHolder previousChapterNumber, Integer incdec, String cacheDir);
    class IntegerHolder {
        private int value;

        public IntegerHolder(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public void setValue(int value) {
            this.value = value;
        }
    }

    class DoubleHolder {
        private double value;

        public DoubleHolder(double value) {
            this.value = value;
        }

        public double getValue() {
            return value;
        }

        public void setValue(double value) {
            this.value = value;
        }
    }
}