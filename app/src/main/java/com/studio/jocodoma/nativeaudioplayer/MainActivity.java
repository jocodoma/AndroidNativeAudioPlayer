package com.studio.jocodoma.nativeaudioplayer;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity:";
    // private String audioFilePath = "/storage/emulated/0/Download/Baby_Love_before.wav";
    private int nativeSampleRate = 0;
    private int nativeSampleBufSize = 0;
    private Button btnPlay;
    private boolean isPlayButtonPressed = false;

    // Requesting permission to READ_EXTERNAL_STORAGE (app-defined int constant)
    private final int PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE = 200;


    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted, yay! Do the
                    // contacts-related task you need to do.
                    Toast.makeText(this, "Permissions Granted to access external storage", Toast.LENGTH_LONG).show();
                    btnPlay.setEnabled(true);
                } else {
                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                    Toast.makeText(this, "Permissions Denied to access external storage. App won't work.", Toast.LENGTH_LONG).show();
                }
                return;
            }

            // other 'case' lines to check for other
            // permissions this app might request.
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        queryNativeAudioParameters();

        btnPlay = findViewById(R.id.button_play);
        btnPlay.setEnabled(false);
        btnPlay.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                Toast.makeText(MainActivity.this, "Button clicked", Toast.LENGTH_LONG).show();
                if(isPlayButtonPressed) {
                    isPlayButtonPressed = false;
                    btnPlay.setText("Play");
                    stopAudio();
                } else {
                    isPlayButtonPressed = true;
                    btnPlay.setText("Stop");
                    playAudio();
                }
            }
        });

        // Check permissions
        // Reference: https://developer.android.com/training/permissions/requesting#java
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            // Permission is not granted
            // Should we show an explanation?
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.READ_CONTACTS)) {
                // Show an explanation to the user *asynchronously* -- don't block
                // this thread waiting for the user's response! After the user
                // sees the explanation, try again to request the permission.
            } else {
                // No explanation needed; request the permission
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
                        PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE);

                // PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE is an
                // app-defined int constant. The callback method gets the
                // result of the request.
            }
        } else {
            // Permission has already been granted
            btnPlay.setEnabled(true);
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onRestart() {
        super.onRestart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void queryNativeAudioParameters() {
        AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        if(myAudioMgr == null) {
            Log.e(TAG, "Fail to get AudioManager");
            return;
        }
        nativeSampleRate  =  Integer.parseInt(myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE));
        nativeSampleBufSize = Integer.parseInt(myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER));
    }

    private void playAudio() {
        // Get audio file path
        String audioFilePath = Environment.getExternalStorageDirectory().getAbsolutePath();
        audioFilePath += "/Download/Baby_Love_before.wav";
        new NativeAudioPlayer().execute(audioFilePath);
    }

    private void stopAudio() {
        stopAudioPlayer();
    }

    private class NativeAudioPlayer extends AsyncTask<String, Void, Void> {
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            // UI (main) thread
        }

        @Override
        protected Void doInBackground(String... strings) {
            // Worker thread
            // ToDo: Need to check sampling rate of input audio file
            playAudioPlayer(strings[0], nativeSampleRate, nativeSampleBufSize);
            return null;
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
            // UI (main) thread
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            // UI (main) thread
            stopAudio();
            btnPlay.setText("Play");
        }

        @Override
        protected void onCancelled(Void aVoid) {
            super.onCancelled(aVoid);
            // UI (main) thread
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void playAudioPlayer(String filePath, int sampleRate, int bufferSize);
    public native void stopAudioPlayer();

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }
}
