package com.android.support;

import android.content.Context;
import android.util.Log;
import android.view.InputEvent;
import android.view.Surface;
import android.widget.TextView;

import java.lang.reflect.Method;

public class JNIBridge {
    public static native void onCreate(Context ctx);

    public static native void onPause();

    public static native void onResume();

    public static native void surfaceCreate(Surface surface, int width, int height);

    public static native void surfaceChanged(int width, int height);

    public static native void surfaceDestroyed();

    public static native boolean injectEvent(InputEvent event);
}
